/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 BT_mode.h 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/06/23
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/23, liuwq create this file
* 
******************************************************************************/
#ifndef _BT_MODE_H_     
#define _BT_MODE_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
 
/*----------------------------- Global Typedefs -----------------------------*/
 typedef struct
 {
    uint8_t head;
    uint8_t len_low;
    uint8_t len_hihg;
    uint8_t cmd;
    uint8_t payload;
 }bt_fram_t;
 
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
 void bt_frame_transmit_buffer(uint8_t len,uint8_t *date_buff);
 void bt_mode_init(void);
 void bt_mode_real_time(void);
 

#endif //_BT_MODE_H_
