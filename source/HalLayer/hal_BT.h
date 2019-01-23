/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 hal_BT.h 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/06/14
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/14, liuwq create this file
* 
******************************************************************************/
#ifndef _HAL_BT_H_     
#define _HAL_BT_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdint.h>
 
/*----------------------------- Global Typedefs -----------------------------*/
#define BT_RECV_MAX_LEN 80
#define BT_DATE_BUFFER_NUM 2

typedef struct{
    uint8_t index;
    uint8_t r_frame[BT_RECV_MAX_LEN];
}BT_FRAM_T;

 typedef struct{
    uint8_t date_sign;            //是否有数据的标志
    uint8_t len;
    uint8_t data_buff[BT_RECV_MAX_LEN];
}REV_BT_DATE_BUFF;
/*----------------------------- Global Defines ------------------------------*/
 
 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
void hal_bt_init(void);
void recv_bt_fram_real_time(void);
uint8_t bt_get_fram(uint8_t *buf,uint8_t *len);
void bt_send_fram(uint8_t *buf,uint8_t len);

#endif //_HAL_BT_H_
