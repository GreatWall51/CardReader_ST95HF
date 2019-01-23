/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : check_mode.h
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月24日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/
#ifndef _CHECK_MODE_H_
#define _CHECK_MODE_H_
/*-------------------------------- Includes ----------------------------------*/
 
 
/*----------------------- Constant / Macro Definitions -----------------------*/

typedef enum
{
   DOOR_STA,
   LOCK_STA,       
}door_lock_sta_e;

typedef enum
{
   DOOR_CLOSE_EVT = 0x00,   //门关事件
   DOOR_OPEN_EVT = 0x01,    //门开事件
   OPEN_TIMEOUT_EVT = 0x02,   //门开超时
   MOVE_EVT = 0x03,          //读头移离
        
}evt_type;

typedef enum
{
   FORCE_OPEN_EXP = 0x00,    
   CARD_OPEN_EXP = 0x03,  
   KEY_OPEN_EXP = 0x04,  
   CMD_OPEN_EXP = 0x06,  
   FIRE_EXP = 0x07,                

   MOVE_EXP = 0x01,

}door_exp_e;


/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void check_evt_real_time(void);
door_exp_e set_open_door_evt(void);
uint8_t  get_door_lock_sta(door_lock_sta_e type);
#endif

