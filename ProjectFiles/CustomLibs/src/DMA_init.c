#include "MDR32F9Qx_dma.h"
#include "MDR32F9Qx_rst_clk.h"
#include "defines.h"
#include "DMA_init.h"

// Внешние переменные
extern uint16_t DAC_table[];
extern uint16_t main_array_for_ADC[];
extern uint16_t alternate_array_for_ADC[];	

// Структуры для DMA
DMA_ChannelInitTypeDef ADC1_DMA_structure;
DMA_ChannelInitTypeDef TIM2_DMA_structure;
DMA_CtrlDataInitTypeDef ADC1_primary_DMA_structure;				// Основная структура канала для ADC1
DMA_CtrlDataInitTypeDef ADC1_alternate_DMA_structure;				// Альтернативная структура канала для ADC1
DMA_CtrlDataInitTypeDef TIM2_primary_DMA_structure;
DMA_CtrlDataInitTypeDef TIM2_alternate_DMA_structure;

void Setup_DMA() 
{
	// Разрешить тактирование DMA
	RST_CLK_PCLKcmd (RST_CLK_PCLK_DMA | RST_CLK_PCLK_SSP1 |
	RST_CLK_PCLK_SSP2, ENABLE);

	// Запретить все прерывания, в том числе от SSP1 и SSP2
	NVIC->ICPR[0] = WHOLE_WORD;
	NVIC->ICER[0] = WHOLE_WORD;

	// Сбросить все настройки DMA
	DMA_DeInit();
	DMA_StructInit (&ADC1_DMA_structure);		// Проинициализировать sDMA_ADC1 стандартными значениями
	
	// Заполняем структуру sDMA_PriCtrlData_ADC1 для АЦП 1
	ADC1_primary_DMA_structure.DMA_SourceBaseAddr =								// Адрес откуда будем брать измерения 
	(uint32_t)(&(MDR_ADC->ADC1_RESULT));									// Соответственно это регистр ADC1_RESULT
	ADC1_primary_DMA_structure.DMA_DestBaseAddr = (uint32_t)(main_array_for_ADC);		// Адрес куда будем писать наши измерения
	ADC1_primary_DMA_structure.DMA_CycleSize = NUM_OF_MES;						// Сколько измерений (DMA передач) содержит 1 DMA цикл
	ADC1_primary_DMA_structure.DMA_SourceIncSize = DMA_SourceIncNo;				// Адрес ADC1_RESULT не требует инкремента, он статичен
	ADC1_primary_DMA_structure.DMA_DestIncSize = DMA_DestIncHalfword;			// Адрес места, куда будем писать измерения будет инкрементироваться на 16 бит, т.к. АЦП 12 битный и в 8 бит он не поместится
	ADC1_primary_DMA_structure.DMA_MemoryDataSize =								// Скажем DMA, Что мы работаем с 16 битными данными
	DMA_MemoryDataSize_HalfWord;	
	ADC1_primary_DMA_structure.DMA_NumContinuous = DMA_Transfers_1024;			// Сколько передач может пройти между процедурой арбитража
	ADC1_primary_DMA_structure.DMA_SourceProtCtrl = DMA_SourcePrivileged;		// Память, откуда берем значения кэшируемая (не факт)
	ADC1_primary_DMA_structure.DMA_DestProtCtrl = DMA_DestCacheable;				// Память, куда пишем значения кэшируемая (не факт)
	ADC1_primary_DMA_structure.DMA_Mode = DMA_Mode_PingPong;						// Режим "Пинг-понг" ст. 385 спецификации к К1986ВЕ92QI

	// Заполним структуру sDMA_AltCtrlData_ADC1 для АЦП 1	
	ADC1_alternate_DMA_structure.DMA_SourceBaseAddr =								// Адрес откуда будем брать измерения 
	(uint32_t)(&(MDR_ADC->ADC1_RESULT));									// Соответственно это регистр ADC1_RESULT
	ADC1_alternate_DMA_structure.DMA_DestBaseAddr = (uint32_t) (alternate_array_for_ADC);		// Адрес куда будем писать наши измерения (+ размер массива / 2 * 2 байта)
	ADC1_alternate_DMA_structure.DMA_CycleSize = NUM_OF_MES;						// Сколько измерений (DMA передач) содержит 1 DMA цикл
	ADC1_alternate_DMA_structure.DMA_SourceIncSize = DMA_SourceIncNo;				// Адрес ADC1_RESULT не требует инкремента, он статичен
	ADC1_alternate_DMA_structure.DMA_DestIncSize = DMA_DestIncHalfword;			// Адрес места, куда будем писать измерения будет инкрементироваться на 16 бит
	ADC1_alternate_DMA_structure.DMA_MemoryDataSize =								// Скажем DMA, Что мы работаем с 16 битными данными
	DMA_MemoryDataSize_HalfWord;
	ADC1_alternate_DMA_structure.DMA_NumContinuous = DMA_Transfers_1024;			// Сколько передач может пройти между процедурой арбитража
	ADC1_alternate_DMA_structure.DMA_SourceProtCtrl = DMA_SourcePrivileged;		// Память, откуда берем значения кэшируемая (не факт)
	ADC1_alternate_DMA_structure.DMA_DestProtCtrl = DMA_DestCacheable;				// Память, куда пишем значения кэшируемая (не факт)
	ADC1_alternate_DMA_structure.DMA_Mode = DMA_Mode_PingPong;						// Режим "Пинг-понг" ст. 385 спецификации к К1986ВЕ92QI
	
	// Заполним структуру для 1ого канала 
	ADC1_DMA_structure.DMA_PriCtrlData = &ADC1_primary_DMA_structure;						// Укажем основную структуру
	ADC1_DMA_structure.DMA_AltCtrlData = &ADC1_alternate_DMA_structure;						// Укажем альтернативную структуру
	ADC1_DMA_structure.DMA_Priority = DMA_Priority_Default;							// Обычный уровень приоритетности (нужен для арбитража)
	ADC1_DMA_structure.DMA_UseBurst = DMA_BurstClear;
	ADC1_DMA_structure.DMA_SelectDataStructure =	DMA_CTRL_DATA_PRIMARY;				// в качестве базовой берем основную структуру
	
	// Проинициализируем первый канал
	DMA_Init(DMA_Channel_ADC1, &ADC1_DMA_structure);
	MDR_DMA->CHNL_REQ_MASK_CLR = 1 << DMA_Channel_ADC1;
	MDR_DMA->CHNL_USEBURST_CLR = 1 << DMA_Channel_ADC1;

	// Будем использовать TIM2 для генерации сигнала DAC
	DMA_StructInit (&TIM2_DMA_structure);		// Проинициализировать sDMA_ADC1 стандартными значениями
	// Заполняем структуру sDMA_PriCtrlData_ADC1 для Таймера 2
	TIM2_primary_DMA_structure.DMA_SourceBaseAddr =								// Адрес откуда будем брать измерения 
	(uint32_t)(DAC_table);													// Соответственно это таблица DAC_table
	TIM2_primary_DMA_structure.DMA_DestBaseAddr = 
	(uint32_t)(&(MDR_DAC->DAC2_DATA));										// Адрес куда будем писать наши измерения (в DAC 2)
	TIM2_primary_DMA_structure.DMA_CycleSize = (SIN_RES);							// Сколько измерений (DMA передач) содержит 1 DMA цикл
	TIM2_primary_DMA_structure.DMA_SourceIncSize = DMA_SourceIncHalfword;		// DAC_table - 16 битный массив => инкремент = полуслово
	TIM2_primary_DMA_structure.DMA_DestIncSize = DMA_DestIncNo;					// Адрес места, куда будем писать измерения не будет инкрементироваться
	TIM2_primary_DMA_structure.DMA_MemoryDataSize =								// Скажем DMA, Что мы работаем с 16 битными данными
	DMA_MemoryDataSize_HalfWord;	
	TIM2_primary_DMA_structure.DMA_NumContinuous = DMA_Transfers_16;				// Сколько передач может пройти между процедурой арбитража
	TIM2_primary_DMA_structure.DMA_SourceProtCtrl = DMA_SourcePrivileged;		// Память, откуда берем значения кэшируемая (не факт)
	TIM2_primary_DMA_structure.DMA_DestProtCtrl = DMA_DestCacheable;				// Память, куда пишем значения кэшируемая (не факт)
	TIM2_primary_DMA_structure.DMA_Mode = DMA_Mode_PingPong;						// Стандартный режим работы DMA 
	
	// Заполняем структуру sDMA_AltCtrlData_ADC1 для Таймера 2
	TIM2_alternate_DMA_structure.DMA_SourceBaseAddr =								// Адрес откуда будем брать измерения 
	(uint32_t)(DAC_table);													// Соответственно это таблица DAC_table
	TIM2_alternate_DMA_structure.DMA_DestBaseAddr = 
	(uint32_t)(&(MDR_DAC->DAC2_DATA));										// Адрес куда будем писать наши измерения (в DAC 2)
	TIM2_alternate_DMA_structure.DMA_CycleSize = (SIN_RES);							// Сколько измерений (DMA передач) содержит 1 DMA цикл
	TIM2_alternate_DMA_structure.DMA_SourceIncSize = DMA_SourceIncHalfword;		// DAC_table - 16 битный массив => инкремент = полуслово
	TIM2_alternate_DMA_structure.DMA_DestIncSize = DMA_DestIncNo;					// Адрес места, куда будем писать измерения не будет инкрементироваться
	TIM2_alternate_DMA_structure.DMA_MemoryDataSize =								// Скажем DMA, Что мы работаем с 16 битными данными
	DMA_MemoryDataSize_HalfWord;	
	TIM2_alternate_DMA_structure.DMA_NumContinuous = DMA_Transfers_16;				// Сколько передач может пройти между процедурой арбитража
	TIM2_alternate_DMA_structure.DMA_SourceProtCtrl = DMA_SourcePrivileged;		// Память, откуда берем значения кэшируемая (не факт)
	TIM2_alternate_DMA_structure.DMA_DestProtCtrl = DMA_DestCacheable;				// Память, куда пишем значения кэшируемая (не факт)
	TIM2_alternate_DMA_structure.DMA_Mode = DMA_Mode_PingPong;						// Стандартный режим работы DMA 
	
	// Заполним структуру для канала TIM2 
	TIM2_DMA_structure.DMA_PriCtrlData = &TIM2_primary_DMA_structure;						// Укажем основную структуру
	TIM2_DMA_structure.DMA_Priority = DMA_Priority_Default;							// Обычный уровень приоритетности (нужен для арбитража)
	TIM2_DMA_structure.DMA_UseBurst = DMA_BurstClear;
	TIM2_DMA_structure.DMA_SelectDataStructure =	DMA_CTRL_DATA_PRIMARY;				// в качестве базовой берем основную структуру
	
	// Проинициализируем первый канал
	DMA_Init(DMA_Channel_TIM2, &TIM2_DMA_structure);
	MDR_DMA->CHNL_REQ_MASK_CLR = 1 << DMA_Channel_TIM2;
	MDR_DMA->CHNL_USEBURST_CLR = 1 << DMA_Channel_TIM2;

	// Установим значение приоретета прерывания DMA
	NVIC_EnableIRQ(DMA_IRQn);
	NVIC_SetPriority (DMA_IRQn, 100);
}

void DMA_IRQHandler() {
	if(DMA_GetFlagStatus(DMA_Channel_TIM2, DMA_FLAG_CHNL_ALT) == RESET) {
		DMA_CtrlInit(DMA_Channel_TIM2, DMA_CTRL_DATA_ALTERNATE, &TIM2_alternate_DMA_structure);	// реинициализируем альтернативную структуру
	}
	else  {
		DMA_CtrlInit(DMA_Channel_TIM2, DMA_CTRL_DATA_PRIMARY, &TIM2_primary_DMA_structure);		// реинициализируем основную структуру
	}
}
