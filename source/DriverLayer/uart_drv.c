/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : uart_drv.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年6月15日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年6月15日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "timer_drv.h" 
#include "uart_drv.h"
#include "com_hal_layer.h"
/*----------------------- Constant / Macro Definitions -----------------------*/


USART_TypeDef* COM_USART[COMn] = {USART1,USART2}; 
GPIO_TypeDef*  COM_TX_PORT[COMn] = {GPIOA,GPIOA};
GPIO_TypeDef*  COM_RX_PORT[COMn] = {GPIOA,GPIOA};

const uint32_t COM_USART_CLK[COMn] = {RCC_APB2Periph_USART1,RCC_APB1Periph_USART2};

const uint32_t COM_TX_PORT_CLK[COMn] = {RCC_AHBPeriph_GPIOA, RCC_AHBPeriph_GPIOA};
const uint32_t COM_RX_PORT_CLK[COMn] = {RCC_AHBPeriph_GPIOA, RCC_AHBPeriph_GPIOA};

const uint16_t COM_TX_PIN[COMn] = {GPIO_Pin_9, GPIO_Pin_2};
const uint16_t COM_RX_PIN[COMn] = {GPIO_Pin_10, GPIO_Pin_3};

const uint8_t  COM_TX_PIN_SOURCE[COMn] = {GPIO_PinSource9, GPIO_PinSource2};
const uint8_t  COM_RX_PIN_SOURCE[COMn] = {GPIO_PinSource10, GPIO_PinSource3};

const uint8_t  COM_TX_AF[COMn] = {GPIO_AF_1,GPIO_AF_1};
const uint8_t  COM_RX_AF[COMn] = {GPIO_AF_1,GPIO_AF_1};

/* 波特率 */
const uint32_t COM_BAUD[COMn] = {19200,115200};
/* 使能发送中断 */
const FunctionalState  COM_S_IRQ_ENABLE[COMn] = {ENABLE, ENABLE};
/* 发送中断类型 */
const uint32_t COM_S_IRQ_TYPE[COMn] = {USART_IT_TC,USART_IT_TC};//USART_IT_TXE

/* 使能接收中断 */
const FunctionalState  COM_R_IRQ_ENABLE[COMn] = {ENABLE, ENABLE};
/* 接收中断类型 */
const uint32_t COM_R_IRQ_TYPE[COMn] = {USART_IT_RXNE,USART_IT_RXNE};
/* 中断通道 */
const uint8_t  COM_IRQ_Channel[COMn] = {USART1_IRQn,USART2_IRQn};
/* 中断优先级 */
const uint8_t  COM_IRQ_PRIO[COMn] = {1, 0};


/*------------------------ Variable Define/Declarations ----------------------*/

/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM], ENABLE);

  /* Enable USART clock */
  if(COM == COM1)
  {
    RCC_APB2PeriphClockCmd(COM_USART_CLK[COM1] , ENABLE); 
  }
  else if(COM == COM2)
  {
    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM2], ENABLE);
  }
  /* Connect PXx to USARTx_Tx */
  GPIO_PinAFConfig(COM_TX_PORT[COM], COM_TX_PIN_SOURCE[COM], COM_TX_AF[COM]);

  /* Connect PXx to USARTx_Rx */
  GPIO_PinAFConfig(COM_RX_PORT[COM], COM_RX_PIN_SOURCE[COM], COM_RX_AF[COM]);
  
  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);
    
  /* Configure USART Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(COM_USART[COM], USART_InitStruct);
    
  /* Enable USART */
  USART_Cmd(COM_USART[COM], ENABLE);
}


void uart_drv_init(COM_TypeDef com_num)
{
	USART_InitTypeDef USART_InitStructure;		
	NVIC_InitTypeDef 	NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = COM_IRQ_Channel[com_num];
	NVIC_InitStructure.NVIC_IRQChannelPriority = COM_IRQ_PRIO[com_num];
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_InitStructure.USART_BaudRate = COM_BAUD[com_num];
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	STM_EVAL_COMInit(com_num, &USART_InitStructure);

	USART_ITConfig(COM_USART[com_num],COM_R_IRQ_TYPE[com_num],COM_R_IRQ_ENABLE[com_num]);		
	USART_ClearITPendingBit(COM_USART[com_num], COM_S_IRQ_TYPE[com_num]);
	USART_ITConfig(COM_USART[com_num],COM_S_IRQ_TYPE[com_num],COM_S_IRQ_ENABLE[com_num]);

}

#if 0
void USART1_IRQHandler(void)
{
    //注意！不能使用if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)来判断
     if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)    
     {
          USART_ClearITPendingBit(USART1, USART_IT_ORE);               //清除中断标志  
     }   
     
     if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
     {
         USART_ClearITPendingBit(USART1, USART_IT_TC);               //清除中断标志  
         com_hal_send_irq_callback(PORT_UART1);
     }
     /* 接收中断 */
     if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
     {   
         com_hal_receive_irq_callback(PORT_UART1, USART_ReceiveData(USART1));
     }  
}
#endif
void USART2_IRQHandler(void)
{
     if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)    
     {
         USART_ClearITPendingBit(USART2, USART_IT_ORE);               //清除中断标志 
     }   
     if(USART_GetITStatus(USART2, USART_IT_TC) != RESET)
     {     
         USART_ClearITPendingBit(USART2, USART_IT_TC);              //清除中断标志 
         com_hal_send_irq_callback(PORT_BT);
     }
     if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
     {    
//				uint8_t data;
//				data = USART_ReceiveData(USART2);
//				USART_SendData(USART2, data);
        com_hal_receive_irq_callback(PORT_BT, USART_ReceiveData(USART2));       
     }
}

