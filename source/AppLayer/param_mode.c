/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : param_mode.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月25日
  最近修改   :
  功能描述   : 设置参数模块
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
#include "param_mode.h" 
#include "flash_drv.h"
/*----------------------- Constant / Macro Definitions -----------------------*/
#define PARAM_SIZE_MAX   20

typedef struct
{
    uint8_t unlock_time;
    uint8_t anti_copy;
    uint8_t open_timeout;
}sys_param_info;

sys_param_info  sys_param;

uint8_t anti_copy_info[PARAM_SIZE_MAX] = {0};
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void param_mode_init(void)
{
    uint8_t i;
    
    sys_param.unlock_time = 10;
    sys_param.anti_copy  = 0;
    sys_param.open_timeout = 120;
    flash_read(FLASH_USER_START_ADDR, anti_copy_info, PARAM_SIZE_MAX); 
  
    if(anti_copy_info[PARAM_SIZE_MAX-1] != 0XAA)
    {
      for(i=0; i<16; i++)
      {
          anti_copy_info[i] = 0xff;
      }
      anti_copy_info[16] = 0x10;
      anti_copy_info[17] = 0x05;
      anti_copy_info[18] = 0x05;
      anti_copy_info[PARAM_SIZE_MAX-1] = 0xaa;
    }
}

/*****************************************************************************
 函 数 名  : set_sys_param
 功能描述  : 更新系统参数
 输入参数  : sys_param_type type  
             uint16_t param       
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void set_sys_param(sys_param_type type, uint16_t param)
{
    switch(type)
    {
        case UNLOCK_TIME_PARAM:
            sys_param.unlock_time = param; 
        break;
        case ANTI_COPY_PARAM:
            sys_param.anti_copy = param;
        break;
        case OPEN_TIMEOUT_PARAM:
            sys_param.open_timeout = param;
        break;
        default:
        break;
    }
}

/*****************************************************************************
 函 数 名  : get_sys_param
 功能描述  : 获取系统参数
 输入参数  : sys_param_type type  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t get_sys_param(sys_param_type type)
{
    uint8_t param = 0xff;
    
    switch(type)
    {
        case UNLOCK_TIME_PARAM:
            param = sys_param.unlock_time; 
        break;
        case ANTI_COPY_PARAM:
            param = sys_param.anti_copy ;
        break;
        case OPEN_TIMEOUT_PARAM:
            param = sys_param.open_timeout;
        break;
        default:
        break;
    }
    return param;
}

uint8_t *get_anti_copy_param(void)
{
    return anti_copy_info;
}
