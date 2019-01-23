/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : led_drv.h
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月23日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月23日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#ifndef _LED_DRV_H
#define _LED_DRV_H
 
 
/*----------------------- Constant / Macro Definitions -----------------------*/
#define  LED_TIME_200M      25   //8ms * 25 = 200ms
#define  LED_TIME_500M      62   //8ms * 25 = 200ms
#define  LED_TIME_3S   375 //8ms * 125 = 1000ms

/*------------------------ Variable Define/Declarations ----------------------*/
typedef enum 
{
    LED_NORMAL_MODE = 0x0,    
    LED_OPEN_OUT_MODE = 0x02,
    LED_CARD_READ_MODE = 0x01,
    
}LED_MODE_E;
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void led_set_mode(LED_MODE_E mode);
void led_real_time(void);
void led_drv_init(void);
void led_clear_mode(LED_MODE_E mode);
void led_on(void);
void led_off(void);
#endif

