/******************************************************************************
* Copyright 2010-2011 Renyucjs@163.com
* FileName: 	 com_config.h 
* Desc:
* 
* 
* Author: 	 Kimson
* Date: 	 2015/03/02
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2015/03/02, Kimson create this file
* 
******************************************************************************/
#ifndef _COM_CONFIG_H_     
#define _COM_CONFIG_H_    
 

 
 
/*----------------------------- Global Defines ------------------------------*/

#define  ENTER_CRITICAL()     do{__disable_irq();}while(0)
#define  EXIT_CRITICAL()      do{__enable_irq();}while(0)

/**  HAL Layer config **/
#define CFG_COM_HAL_SEND_BUF_SIZE      250
#define CFG_COM_HAL_MAX_INTERVAL_CNT    250 /* 最大的字符间隔计数,不可超过255 */
#define CFG_COM_HAL_MAX_SEND_TIME (300/TIME_BASE) /* 300ms */
#define CFG_COM_HAL_RECEIVE_CYC_SIZE 128 	/*!< 2*N */
#define COM_HAL_CYC_MASK ((CFG_COM_HAL_RECEIVE_CYC_SIZE)-1)


/*----------------------------- Global Typedefs -----------------------------*/
typedef enum {
    PORT_BT = 0,
		PORT_UART1,
    CFG_COM_PORT_CNT,  
}ENUM_COM_PORT_T;
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 
 

#endif //_COM_CONFIG_H_
