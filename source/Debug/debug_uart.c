/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : debug_uart.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年9月6日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年9月6日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"  
#include "timer_drv.h"
#include "define.h"
#include "hal_timer.h" 
#include "debug_uart.h"
#include <stdio.h>
#include <string.h> 
/*----------------------- Constant / Macro Definitions -----------------------*/
USART_TypeDef* DBG_USART    = USART2; 
GPIO_TypeDef*  DBG_TX_PORT = GPIOA;
GPIO_TypeDef*  DBG_RX_PORT = GPIOA;

const uint32_t DBG_USART_CLK =RCC_APB1Periph_USART2;

const uint32_t DBG_TX_PORT_CLK = RCC_AHBPeriph_GPIOA;
const uint32_t DBG_RX_PORT_CLK = RCC_AHBPeriph_GPIOA;

const uint16_t DBG_TX_PIN = GPIO_Pin_2;
const uint16_t DBG_RX_PIN = GPIO_Pin_3;

const uint8_t  DBG_TX_PIN_SOURCE = GPIO_PinSource2;
const uint8_t  DBG_RX_PIN_SOURCE = GPIO_PinSource3;

const uint8_t  DBG_TX_AF = GPIO_AF_1;
const uint8_t  DBG_RX_AF = GPIO_AF_1;

/* 波特率 */
const uint32_t DBG_BAUD = 115200;




/*------------------------ Variable Define/Declarations ----------------------*/

 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 void putCh(uint8_t ch)
{
    while(!((DBG_USART->ISR)&(1<<7)));
	DBG_USART->TDR=ch;	
}

//加入以下代码,支持printf函数,而不需要选择use MicroLIB    
#pragma import(__use_no_semihosting)              
//标准库需要的支持函数                  
struct __FILE  
{  
	int handle;  
	/* Whatever you require here. If the only file you are using is */  
	/* standard output using printf() for debugging, no file handling */  
	/* is required. */  
};  
/* FILE is typedef’ d in stdio.h. */  
FILE __stdout; 
int _sys_exit(int x)  
{  
	x = x;  
	return 0;
}  
_ttywrch(int ch)
{
ch = ch;
}

//重定义fputc函数  
int fputc(int ch, FILE *f) 
{
    putCh(ch);
    return ch; 
}
 
/*---------------------------------------------------------------------------*/
void debug_uart_disable_int(void)
{
	NVIC_InitTypeDef 	NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(DBG_USART,USART_IT_TC,DISABLE);		
	USART_ITConfig(DBG_USART,USART_IT_RXNE,DISABLE);
	sbi(GPIOA,GPIO_Pin_1); 
}
void debug_pin_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);					  											 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	sbi(GPIOA,GPIO_Pin_1); 	
}
void debug_uart_init(void)
{
	USART_InitTypeDef USART_InitStructure;		
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef 	NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(DBG_TX_PORT_CLK | DBG_RX_PORT_CLK, ENABLE);

	/* Enable USART clock */
	if(DBG_USART == USART1)
	{
		RCC_APB2PeriphClockCmd(DBG_USART_CLK , ENABLE); 
	}
	else if(DBG_USART == USART2)
	{
		RCC_APB1PeriphClockCmd(DBG_USART_CLK, ENABLE);
	}
	/* Connect PXx to USARTx_Tx */
	GPIO_PinAFConfig(DBG_TX_PORT, DBG_TX_PIN_SOURCE, DBG_TX_AF);

	/* Connect PXx to USARTx_Rx */
	GPIO_PinAFConfig(DBG_RX_PORT, DBG_RX_PIN_SOURCE, DBG_RX_AF);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = DBG_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(DBG_TX_PORT, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = DBG_RX_PIN;
	GPIO_Init(DBG_RX_PORT, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = DBG_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	/* USART configuration */
	USART_Init(DBG_USART, &USART_InitStructure);

	USART_ClearITPendingBit(DBG_USART, USART_IT_TC);
	USART_ITConfig(DBG_USART,USART_IT_TC,DISABLE);		
	USART_ITConfig(DBG_USART,USART_IT_RXNE,DISABLE);
	/* Enable USART */
	USART_Cmd(DBG_USART, ENABLE);


 
}

