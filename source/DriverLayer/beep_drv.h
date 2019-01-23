/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : beep_drv.h
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月23日
  最近修改   :
  功能描述   : beep
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月23日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

#ifndef _BEEP_DRV_H_
#define _BEEP_DRV_H_
/*-------------------------------- Includes ----------------------------------*/
 
 
/*----------------------- Constant / Macro Definitions -----------------------*/
typedef enum
{
    BEEP_NORMAL_MODE   = 0x0,                 //正常模式
    BEEP_OPEN_OUT_MODE = 0x04,                //门开超时声音
    BEEP_OPEN_FORCE_MODE = 0x05,              //门强开声音
    BEEP_OPEN_READ_MODE = 0x06,               //刷卡声音
    BEEP_CARD_2_MODE =0x02,                   //响2声
    
    BEEP_CARD_1_MODE = 0x01,                  //响1声
    BEEP_CARD_4_MODE =0x03,                   //响4声
    BEEP_ADDR_MODE  = 0x10,                   //地址变化声音
}BEEP_MODE_E;


/*------------------------ Variable Define/Declarations ----------------------*/
#define  BEEP_TIME_100M      20   //8ms * 25 = 200ms
#define  BEEP_TIME_500M      62   //8ms * 25 = 200ms
#define  BEEP_TIME_1H        25   //8ms * 25 = 200ms
#define  BEEP_TIME_1L        10   //8ms * 25 = 200ms
#define  BEEP_TIME_3S        375 //8ms * 125 = 1000ms

#define  BEEP_TIME_OK        50   //8ms * 25 = 200ms
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void beep_set_mode(BEEP_MODE_E mode);
void beep_real_time(void);
void beep_drv_init(void);
void beep_clear_mode(BEEP_MODE_E mode);
#endif

