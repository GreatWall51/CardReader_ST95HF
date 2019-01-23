/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 debug_uart.h 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/05/18
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/05/18, liuwq create this file
* 
******************************************************************************/
#ifndef _DEBUG_UART_H_     
#define _DEBUG_UART_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
 
 
/*----------------------------- Global Typedefs -----------------------------*/
 
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
void debug_pin_init(void);
void debug_uart_disable_int(void);
void debug_uart_init(void);


#endif //_DEBUG_UART_H_
