
#include "stm32f0xx_spi.h"
#include "stm32f0xx_exti.h"
#include "drv_spi.h"
#include "drv_95HF.h"
#include "stdbool.h"
#include "debug.h"


#ifndef MIN
#define MIN(x,y) 				((x < y)? x : y)
#endif
volatile bool uDataReady        = false;
volatile bool RF_DataExpected 		= false;
volatile bool RF_DataReady 			= false;


void IRQOut_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);					  											 
	GPIO_InitStructure.GPIO_Pin = EXTI_GPIO_PIN ;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(EXTI_GPIO_PORT, &GPIO_InitStructure);

	RFTRANS_95HF_IRQOUT_HIGH();
}

void RFTRANS_IRQ_check_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable GPIOB clock */
    RCC_AHBPeriphClockCmd(EXTI_PortSourceGPIOB, ENABLE);

    /* Enable GPIOC clock */
//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

   /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
   
    /* Configure PB1 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = CHECK_IRQ_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(CHECK_IRQ_GPIO_PORT, &GPIO_InitStructure);
      
    /* Connect EXTI1 Line to PB pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, CHECK_IRQ_PIN_SOUR);
    
    /* Configure EXTI1 line */
    EXTI_InitStructure.EXTI_Line = CHECK_IRQ_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);


    /* Enable and set EXTI1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = CHECK_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
void EXTI0_1_IRQHandler(void)
{	
    if(EXTI_GetITStatus(CHECK_IRQ_EXTI_LINE) != RESET)
    {
			if(RF_DataExpected)
			RF_DataReady = true;

			/* Answer to command ready*/
			uDataReady = true;	
			
//			debug("catch irq signal!\r\n");
			EXTI_ClearITPendingBit(CHECK_IRQ_EXTI_LINE);
    }
}



//检测中断使能
void drvInt_Enable_Reply_IRQ(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	
	RF_DataExpected = true;
	uDataReady = false;
	
	/* Configure RF transceiver IRQ EXTI line */
	EXTI_InitStructure.EXTI_Line 		= CHECK_IRQ_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Falling;    
	EXTI_InitStructure.EXTI_LineCmd 	= ENABLE;
	EXTI_Init(&EXTI_InitStructure); 
}
//关闭检测中断
void drvInt_Disable_95HF_IRQ(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	
	RF_DataExpected = false;
	uDataReady = false;
	
	/* Configure RF transceiver IRQ EXTI line */
	EXTI_InitStructure.EXTI_Line 		= CHECK_IRQ_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd 	= DISABLE;
	EXTI_Init(&EXTI_InitStructure); 

}
//输出中断使能
void drvInt_Enable_RFEvent_IRQ(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	
	RF_DataExpected = true;
	uDataReady = false;
	
	/* Configure RF transceiver IRQ EXTI line */
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger 	= EXTI_Trigger_Falling;    
	EXTI_InitStructure.EXTI_LineCmd 	= ENABLE;
	EXTI_Init(&EXTI_InitStructure); 
}


void spi_init(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* GPIOA Periph clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	
    /* Configure PA4 in output pushpull mode */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA,&GPIO_InitStructure);

    /* Enable SPI1 and GPIOA clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    /* Configure SPI1 pins: NSS, SCK, MISO and MOSI */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

      /* Connect PXx to SD_SPI_SCK */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_0);

  /* Connect PXx to SD_SPI_MISO */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_0); 

  /* Connect PXx to SD_SPI_MOSI */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_0);  
    
     SPI_I2S_DeInit(SPI1);
 //   SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);
    SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);
    /* SPI1 configuration */ 
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    /* Enable SPI1  */
    SPI_Cmd(SPI1, ENABLE); 
		
}
/**  
 *	@brief  Sends one byte over SPI and recovers a response
 *  @param  SPIx : where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  data : data to send
 *  @retval data response from SPIx 
 */
uint8_t SPI_SendReceiveByte(SPI_TypeDef* SPIx, uint8_t data) //here we use SPI1
{		
	while((SPIx->SR & SPI_I2S_FLAG_TXE) == 0);
		*(uint8_t*)&SPIx->DR = data;

	while((SPIx->SR & SPI_I2S_FLAG_RXNE) == 0);

	return SPIx->DR;
}


/**
 *	@brief  reveive a byte array over SPI
 *  @param  SPIx	 	: where x can be 1, 2 or 3 to select the SPI peripheral
 *  @param  pCommand  	: pointer on the buffer to send
 *  @param  length	 	: length of the buffer to send
 *  @param  pResponse 	: pointer on the buffer response
 *  @retval None 
 */
void SPI_SendReceiveBuffer(SPI_TypeDef* SPIx, uc8 *pCommand, uint16_t length, uint8_t *pResponse) 
{
	uint16_t i;
		if(length >= RFTRANS_95HF_MAX_BUFFER_SIZE)
	{
		for(i=0; i<length; i++)
		{
		SPI_SendReceiveByte(SPIx, pCommand[i]);
		}
		return ;
	}
	/* the buffer size is limited to SPI_RESPONSEBUFFER_SIZE */
	length = MIN (RFTRANS_95HF_MAX_BUFFER_SIZE,length);
	for(i=0; i<length; i++)
		pResponse[i] = SPI_SendReceiveByte(SPIx, pCommand[i]);
}

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
