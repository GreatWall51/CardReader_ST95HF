/**
  ******************************************************************************
  * @file    drv_spi.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file provides a set of firmware functions to manages SPI communications
  ******************************************************************************
  * @copyright
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
*/ 

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __SPI_H
#define __SPI_H

/* Includes ----------------------------------------------------------------- */

#include "define.h"

//ST95HF上报数据的中断通知线
#define CHECK_IRQ_GPIO_PORT     GPIOB
#define CHECK_IRQ_PIN           GPIO_Pin_1
#define CHECK_IRQ_PIN_SOUR      EXTI_PinSource1
#define CHECK_IRQ_EXTI_LINE     EXTI_Line1
#define CHECK_IRQChannel        EXTI0_1_IRQn

//STM32F0唤醒ST95HF通知线
#define EXTI_GPIO_PORT      GPIOB
#define EXTI_GPIO_PIN       GPIO_Pin_0

#define RFTRANS_95HF_IRQOUT_HIGH()  			GPIO_WriteBit(EXTI_GPIO_PORT,EXTI_GPIO_PIN,Bit_SET)
#define RFTRANS_95HF_IRQOUT_LOW() 				GPIO_WriteBit(EXTI_GPIO_PORT, EXTI_GPIO_PIN,Bit_RESET)


#define SPI_RESPONSEBUFFER_SIZE		(uint16_t)528

void IRQOut_Config(void);
void RFTRANS_IRQ_check_io_init(void);
void drvInt_Enable_Reply_IRQ(void);
void drvInt_Disable_95HF_IRQ(void);
void drvInt_Enable_RFEvent_IRQ(void);

void spi_init(void);
uint8_t SPI_SendReceiveByte		(SPI_TypeDef* SPIx, uint8_t data); 
void SPI_SendReceiveBuffer	(SPI_TypeDef* SPIx, uc8 *pCommand, uint16_t length, uint8_t *pResponse);


#endif /* __SPI_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
