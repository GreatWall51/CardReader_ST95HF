/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 sim_uart_drv.h 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/06/19
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/19, liuwq create this file
* 
******************************************************************************/
#ifndef _SIM_UART_DRV_H_     
#define _SIM_UART_DRV_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
 
/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
void sim_uart_init(void);
void SimUART_RxBytes(uint8_t *RecvData,uint8_t* len);
void SimUART_TxBytes(uint8_t *SendData,uint8_t len);
#endif //_SIM_UART_DRV_H_
