/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : com_func_config.c
  �� �� ��   : ����
  ��    ��   : wzh
  ��������   : 2016��6��17��
  ����޸�   :
  ��������   : 
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��6��17��
    ��    ��   : wzh
    �޸�����   : �����ļ�

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "com_func_config.h"
#include "com_hal_layer.h" 
#include "uart_drv.h" 
#include "stm32f0xx.h"
/*----------------------- Constant / Macro Definitions -----------------------*/
#include "debug.h"
#define sbi(ADDRESS,BIT) 	((ADDRESS->BSRR = BIT))	// Set bit
#define cbi(ADDRESS,BIT) 	((ADDRESS->BRR = BIT))	// Clear bit
/*------------------------ Variable Define/Declarations ----------------------*/
 typedef enum
 {
	 RX,
	 TX
 }PIN_DIR;
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

//����2
static void weak_up_bt_init(void)//���ͻ���BTģ��
{
   GPIO_InitTypeDef  GPIO_InitStructure;
	
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);					  											 
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;                 
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
}
void uart2_init(void)
{
		uart_drv_init(COM2);
		weak_up_bt_init();
		cbi(GPIOA,GPIO_Pin_8);
}
void uart2_start_send(void)
{
    uint8_t value;
    com_hal_port_t *hal_port;

    hal_port = com_hal_get_port_info(PORT_BT);
    value = hal_port->s_frame[0];
    hal_port->s_sent_point++;
    USART_SendData(USART2, value);  
}

void uart2_send(uint8_t value)
{
    USART_SendData(USART2, value);   
}

void uart2_stop_send(void)
{
//	sbi(GPIOA,GPIO_Pin_8);
	cbi(GPIOA,GPIO_Pin_8);
}



//RS485
static void rs485_id1_set_pin(PIN_DIR dir)
{	
	if(dir == RX)   
	{		
		cbi(GPIOA,GPIO_Pin_11); 
	}   
	else   
	{
		sbi(GPIOA,GPIO_Pin_11); 
	}			 
}
void rs485_id1_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA, ENABLE);					  											 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	uart_drv_init(COM1);
	rs485_id1_set_pin(RX);
	
}
void rs485_id1_start_send(void)
{
    uint8_t value;
    com_hal_port_t *hal_port;

    hal_port = com_hal_get_port_info(PORT_UART1);
	  rs485_id1_set_pin(TX);
    value = hal_port->s_frame[0];
    hal_port->s_sent_point++;
    USART_SendData(USART1, value);  
}



void rs485_id1_send(uint8_t value)
{
    USART_SendData(USART1, value);   
}

void rs485_id1_stop_send(void)
{
	rs485_id1_set_pin(RX);
}

