/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : lock_drv.h
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月23日
  最近修改   :
  功能描述   : lock
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月23日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/
#ifndef _LOCK_DRV_H_
#define _LOCK_DRV_H_
/*-------------------------------- Includes ----------------------------------*/
 
 
/*----------------------- Constant / Macro Definitions -----------------------*/
typedef enum
{
    NORMAL_LOCK_MODE,    
    FIRE_UNLOCK_MODE,
    KEY_UNLOCK_MODE,
    CARD_UNLOCK_MODE,
    CMD_UNLOCK_MODE,
}unlock_mode_p;

/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void lock_set_mode(unlock_mode_p unlock_mode, uint8_t time);
void lock_real_time(void);
void lock_drv_init(void);
unlock_mode_p  get_lock_mode(void);
#endif


