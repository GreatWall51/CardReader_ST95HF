/**
  ******************************************************************************
  * @file    drv_interrupt.h 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file configured the interruption raised by 95HF device.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_INTERRUPT_H
#define __DRV_INTERRUPT_H

#include "stm32f0xx_gpio.h"

#define SPI_INTERRUPT_MODE_ACTIVATED

//¶¨Ê±Æ÷
#define TIMER_TIMEOUT TIM3

void delayHighPriority_ms(uint16_t delay);
void StartTimeOut(uint16_t delay);
void StopTimeOut(void);
void delay_ms(uint16_t delay);
void drvInt_TimeoutTimerConfig(void);
//void delay_us(uint16_t delay);

#endif /* __DRV_INTERRUPT_H */
