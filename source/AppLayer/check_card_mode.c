/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : check_card_mode.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月25日
  最近修改   :
  功能描述   : 刷卡模块
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月25日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "led_drv.h" 
#include "timer_drv.h"
#include "define.h"
#include "rs485_drv.h"
#include "param_mode.h"
#include "check_mode.h"
#include <string.h>
#include <stdlib.h>
#include "check_io_drv.h"
#include "beep_drv.h"
#include "lock_drv.h"
#include "check_card_mode.h" 
#include "card_read_drv.h"
/*----------------------- Constant / Macro Definitions -----------------------*/
#define DEBUG_RS485_LINK 0

CARD_INF0_T card_info_p;
uint8_t card_num = 0;
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

static void card_frame_to_queue(CARD_INF0_T *car_info_p)
{

    card_data_frame_t *frame_data;
    rs485_frame_msg *p_msg;
	
    p_msg = append_frame_queue(car_info_p->card_len + 9); 
    if(p_msg == NULL) return;


    frame_data = (card_data_frame_t *)set_frame_head(p_msg->pbuf, CAR_UPDATA_CMD, car_info_p->card_len + 8, 0);       
    p_msg->len = *(p_msg->pbuf+FRAME_OFFSET_LEN)+1;
      
    frame_data->card_type = car_info_p->card_type; 
    frame_data->card_len = car_info_p->card_len;
    memcpy(frame_data->card_uid, car_info_p->card_uid, car_info_p->card_len);
        
    p_msg->pbuf[p_msg->len-1] = get_check_sum(p_msg->pbuf,p_msg->len);
  
}



void check_card_real_time(void)
{	
        if(card_get_event(&card_info_p) == 1) //判断是否有刷卡
        { 
            led_set_mode(LED_CARD_READ_MODE);
            beep_set_mode(BEEP_OPEN_READ_MODE);
            card_frame_to_queue(&card_info_p);
        }
    
}

