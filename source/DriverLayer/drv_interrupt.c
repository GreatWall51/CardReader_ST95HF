/**
  ******************************************************************************
  * @file    drv_interrupt.c 
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   This file configured the interruption raised by 95HF device.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
	******************************************************************************
  */ 
#include "drv_interrupt.h"
#include "stm32f0xx_exti.h"
#include "stdbool.h"
#include "stm32f0xx.h"
#include "define.h"

//#define TIMER_US_DELAY												TIM16
//#define TIMER_US_DELAY_PERIOD									35
//#define TIMER_US_DELAY_PRESCALER							1
//#define TIMER_US_DELAY_CLOCK									RCC_APB2Periph_TIM16
//#define TIMER_US_DELAY_IRQ									  TIM16_IRQn

#define TIMER_MS_DELAY												TIM14
#define TIMER_MS_DELAY_PERIOD									35
#define TIMER_MS_DELAY_PRESCALER							1
#define TIMER_MS_DELAY_CLOCK									RCC_APB1Periph_TIM14
#define TIMER_MS_DELAY_IRQ									  TIM14_IRQn

#define TIMER_TIMEOUT												TIM3
#define TIMER_TIMEOUT_PERIOD									35
#define TIMER_TIMEOUT_PRESCALER							1
#define TIMER_TIMEOUT_CLOCK									RCC_APB1Periph_TIM3
#define TIMER_TIMEOUT_IRQ									  TIM3_IRQn

extern volatile bool uDataReady;
extern volatile bool RF_DataExpected;
extern __IO uint8_t	 uTimeOut;

static uint16_t	counter_delay_ms;
//static uint16_t	counter_delay_us;
uint16_t delay_timeout = 0;


void StartTimeOut(uint16_t delay)
{
	/* Set the TimeOut flag to false */
	uTimeOut 	 = false;
	delay_timeout = delay;
	/* Set the timer counter */
	TIM_SetCounter(TIMER_TIMEOUT, delay);
  /* Enable the Time out timer */
	TIM_Cmd(TIMER_TIMEOUT, ENABLE);
}

/**
 *	@brief  Stop the timer used for the time out
 *  @param  None
 *  @retval None
 */
void StopTimeOut(void)
{	
	/* Disable the Time out timer */
	TIM_Cmd(TIMER_TIMEOUT, DISABLE);	
}

static void TimerDelay_us_Config( void )
{
//  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

//  /* TIM1 clock enable */
//    /* TIM1 clock enable */
//  if(TIMER_MS_DELAY == TIM1 || TIMER_MS_DELAY == TIM15\
//		|| TIMER_MS_DELAY == TIM16 || TIMER_MS_DELAY == TIM17)
//  {
//     RCC_APB2PeriphClockCmd(TIMER_US_DELAY_CLOCK, ENABLE);
//  }
//  else
//  {
//     RCC_APB1PeriphClockCmd(TIMER_US_DELAY_CLOCK, ENABLE);
//  }

//  /* Time Base configuration */
//  TIM_TimeBaseStructure.TIM_Prescaler = 47;  // 1MHz
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//  TIM_TimeBaseStructure.TIM_Period = 1000-1; //1us
//  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

//  TIM_TimeBaseInit(TIMER_US_DELAY,&TIM_TimeBaseStructure);

//  TIM_ARRPreloadConfig(TIMER_US_DELAY, ENABLE);
//  TIM_ITConfig(TIMER_US_DELAY, TIM_IT_Update, ENABLE);
//  TIM_Cmd(TIMER_US_DELAY,DISABLE);

//  NVIC_EnableIRQ(TIMER_US_DELAY_IRQ);
}

//void TIM16_IRQHandler(void)
//{
//	if(counter_delay_us>0) counter_delay_us--;
//	TIMER_US_DELAY->SR=0x0000;
//}
void delay_us(uint16_t delay)
{
//    counter_delay_us = delay;

    TimerDelay_us_Config ();

//    TIM_SetCounter(TIMER_US_DELAY,0);
//    /* TIM2 enable counter */
//    TIM_Cmd(TIMER_US_DELAY, ENABLE);
//    /* Wait for 'delay' us */
//    while(counter_delay_us != 0);
//    /* TIM2 disable counter */
//    TIM_Cmd(TIMER_US_DELAY, DISABLE);
}

static void TimerDelay_ms_Config(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  /* TIM1 clock enable */
    /* TIM1 clock enable */
  if(TIMER_MS_DELAY == TIM1 || TIMER_MS_DELAY == TIM15\
		|| TIMER_MS_DELAY == TIM16 || TIMER_MS_DELAY == TIM17)
  {
     RCC_APB2PeriphClockCmd(TIMER_MS_DELAY_CLOCK, ENABLE);
  }
  else
  {
     RCC_APB1PeriphClockCmd(TIMER_MS_DELAY_CLOCK, ENABLE);
  }

  /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 47;  // 1MHz
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 1000 -1; //1000us
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIMER_MS_DELAY,&TIM_TimeBaseStructure);

  TIM_ARRPreloadConfig(TIMER_MS_DELAY, ENABLE);
  TIM_ITConfig(TIMER_MS_DELAY, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIMER_MS_DELAY,DISABLE);

  NVIC_EnableIRQ(TIMER_MS_DELAY_IRQ);
}
void TIM14_IRQHandler(void)
{
	if(counter_delay_ms>0) counter_delay_ms--;
	TIMER_MS_DELAY->SR=0x0000;
}
void delay_ms(uint16_t delay)
{
    counter_delay_ms = (delay+1);

    TimerDelay_ms_Config ();

    TIM_SetCounter(TIMER_MS_DELAY, 0);
    /* TIM2 enable counter */
    TIM_Cmd(TIMER_MS_DELAY, ENABLE);
    /* Wait for 'delay' milliseconds */
    while(counter_delay_ms != 0);
    /* TIM2 disable counter */
    TIM_Cmd(TIMER_MS_DELAY, DISABLE);
}

void delayHighPriority_ms(uint16_t delay)
{
	delay_ms(delay);
}

void drvInt_TimeoutTimerConfig(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  /* TIM1 clock enable */
    /* TIM1 clock enable */
  if(TIMER_MS_DELAY == TIM1 || TIMER_MS_DELAY == TIM15\
		|| TIMER_MS_DELAY == TIM16 || TIMER_MS_DELAY == TIM17)
  {
     RCC_APB2PeriphClockCmd(TIMER_TIMEOUT_CLOCK, ENABLE);
  }
  else
  {
     RCC_APB1PeriphClockCmd(TIMER_TIMEOUT_CLOCK, ENABLE);
  }

  /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 47;  // 1MHz
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 2; //1us
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIMER_TIMEOUT,&TIM_TimeBaseStructure);

  TIM_ARRPreloadConfig(TIMER_TIMEOUT, ENABLE);
  TIM_ITConfig(TIMER_TIMEOUT, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIMER_TIMEOUT,DISABLE);

  NVIC_EnableIRQ(TIMER_TIMEOUT_IRQ);
}
void TIM3_IRQHandler(void)
{
	if (delay_timeout > 0)
		delay_timeout--;
	else
	{
		uTimeOut = true;	
		/* Disable the Time out timer */
		TIM_Cmd(TIMER_TIMEOUT, DISABLE);
	}
	/* Clear TIMER update interrupt flag */
	TIM_ClearITPendingBit(TIMER_TIMEOUT, TIM_IT_Update);
}

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/

