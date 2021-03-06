/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : check_mode.c
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
#include "rs485_mode.h" 
/*----------------------- Constant / Macro Definitions -----------------------*/
#include "debug.h"
/*------------------------ Variable Define/Declarations ----------------------*/
static uint8_t check_door_open_out_flag = 0;
static door_exp_e door_exp;

/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 函 数 名  : evt_frame_to_queue
 功能描述  : 事件加入待发送队列
 输入参数  : evt_type type   
             door_exp_e exp  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void evt_frame_to_queue(evt_type type, door_exp_e exp)
{

    rs485_frame_msg *p_msg;
    reader_evt_frame_t *frame_data;
 
    /* 分配一个链表节点 */
    p_msg = append_frame_queue(9); 

    if(p_msg == NULL) return;

    frame_data = (reader_evt_frame_t *)set_frame_head(p_msg->pbuf, EVT_UPDATA_CMD, 8, 0);

    /* 获取数据总长度 */
    p_msg->len = *(p_msg->pbuf+FRAME_OFFSET_LEN)+1;
      
    frame_data->evt_type = type;
    frame_data->expand = exp;
    frame_data->sum = get_check_sum(p_msg->pbuf,p_msg->len);   

}
/*****************************************************************************
 函 数 名  : check_door_evt
 功能描述  : 检测门开关事件
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void check_door_evt(void)
{
    evt_type evt;
    door_exp_e exp_evt;
    
    if(get_io_evt(DOOR_STATUS_IO))  //门开关事件检测
    {
       
        if(get_io_level(DOOR_STATUS_IO)) //门开事件
        {     
             evt = DOOR_OPEN_EVT;  
             /* 获取开门原因 */
             exp_evt = set_open_door_evt();
        }
        else
        {
             evt = DOOR_CLOSE_EVT;
             exp_evt = (door_exp_e)0;
             check_door_open_out_flag = 1;
        }
        evt_frame_to_queue(evt,exp_evt);
    }
}

/*****************************************************************************
 函 数 名  : check_door_open_out
 功能描述  : 检测门开超时
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void check_door_open_out(void)
{
    static uint16_t time_1s = 0;
    static uint16_t open_time = 0; 
    
    if(get_io_level(DOOR_STATUS_IO))                      //门开
    {
                 
        if(!IS_TIMER_EVT(TIME_EVT_8))   return;          //门开超时检测
        if(time_1s++ < 125) return;
        time_1s = 0;
        
        if(open_time++ < get_sys_param(OPEN_TIMEOUT_PARAM)) return;      
        open_time = 0; 
        
        if(check_door_open_out_flag == 0) return;         //一次门开关正常后再启动

        check_door_open_out_flag = 0;
        beep_set_mode(BEEP_OPEN_OUT_MODE);
        led_set_mode(LED_OPEN_OUT_MODE);
        evt_frame_to_queue(OPEN_TIMEOUT_EVT,(door_exp_e)0);         
        
    }
    else
    {
        open_time = 0;
        time_1s = 0;
        /* 清除门开超时声光指示 */
        led_clear_mode(LED_OPEN_OUT_MODE);
        beep_clear_mode(BEEP_OPEN_OUT_MODE);
    }
}

/*****************************************************************************
 函 数 名  : check_reader_move
 功能描述  : 检测读头是否移离
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void check_reader_move(void)
{
    
    if(get_io_evt(MOVE_IO))  
    {
        if(get_io_level(MOVE_IO))                  //读头移离
        {
            beep_set_mode(BEEP_OPEN_FORCE_MODE);
            evt_frame_to_queue(MOVE_EVT,MOVE_EXP);         
        }
        else
        {
            beep_clear_mode(BEEP_OPEN_FORCE_MODE);
        }
    }
}

/*****************************************************************************
 函 数 名  : check_key_open
 功能描述  : 检测开门按钮是否被按下
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void check_key_open(void)
{
    
    if(get_io_evt(KEY_OPEN_IO))  //出门按钮
    {
        if(get_io_level(KEY_OPEN_IO) == 0)
        {              
            lock_set_mode(KEY_UNLOCK_MODE,get_sys_param(UNLOCK_TIME_PARAM));
//						beep_set_mode(BEEP_CARD_2_MODE);
        }
    }
}

/*****************************************************************************
 函 数 名  : check_fire
 功能描述  : 检测火警是否被触发
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void check_fire(void)
{
  static uint8_t report_flag=0;//事件上报标志
	
	//防止火警状态下，读头初始化未完成，就上报火警事件(源地址地址错误)�
	//等到初始化完成时，地址改变而无法识别主机的应答，造成一直上报火警事件
	if(get_reader_addr() == 0) return;
	
	if(get_io_level(FIRE_STATUS_IO) == 0)   //火警触发
	{
		lock_set_mode(FIRE_UNLOCK_MODE,0xff);               
		beep_set_mode(BEEP_OPEN_FORCE_MODE);
		led_set_mode(LED_OPEN_OUT_MODE);
		if(report_flag == 0)
		{
		    report_flag = 1;
    		evt_frame_to_queue(DOOR_OPEN_EVT,FIRE_EXP);
        }
	} 
	else
	{
		if(get_io_evt(FIRE_STATUS_IO)) 
		{
		    report_flag = 0;
			lock_set_mode(FIRE_UNLOCK_MODE,0xfe);
			beep_clear_mode(BEEP_OPEN_FORCE_MODE);
			led_clear_mode(LED_OPEN_OUT_MODE);
		}
	}
}

/*****************************************************************************
 函 数 名  : check_reader_addr
 功能描述  : 检测读头地址是否变化
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void check_reader_addr(void)
{
    static uint8_t old_addr = 0x17;
    uint8_t addr;

    if(get_reader_addr() == 0) return;    //等待读头地址检测完成
    
    addr = get_reader_addr();
    if(addr != old_addr)   //读头地址变化
    {    
        beep_set_mode(BEEP_ADDR_MODE);
        free_frame_queue();
        old_addr = addr;
    } 
}


/*****************************************************************************
 函 数 名  : set_open_door_evt
 功能描述  : 将锁状态转换为开门原因
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
door_exp_e set_open_door_evt(void)
{
    unlock_mode_p  mode ;
    mode = get_lock_mode();
    
    switch(mode)
    {
          case NORMAL_LOCK_MODE:
               door_exp = FORCE_OPEN_EXP;
          break;
          case CARD_UNLOCK_MODE:
               door_exp = CARD_OPEN_EXP;
          break;
          case FIRE_UNLOCK_MODE:
               door_exp = FIRE_EXP;
          break;
          case CMD_UNLOCK_MODE:
               door_exp = CMD_OPEN_EXP;
          break;
          case KEY_UNLOCK_MODE:
               door_exp = KEY_OPEN_EXP;
          break;
          default:
          break;
    }

    return door_exp;
}
/*****************************************************************************
 函 数 名  : check_evt_real_time
 功能描述  : 检测事件
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void check_evt_real_time(void)
{  
    check_reader_addr();
    check_fire();
    check_key_open();
    check_reader_move();
    check_door_open_out();
    check_door_evt();
}

/*****************************************************************************
 函 数 名  : get_door_lock_sta
 功能描述  : 获取门锁状态
 输入参数  : door_lock_sta_e type  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t  get_door_lock_sta(door_lock_sta_e type)
{
    uint8_t sta;
    
    if(DOOR_STA == type)
    {
        if(get_io_level(DOOR_STATUS_IO))
        {
            sta = OPEN;
        }
        else
        {
            sta = CLOSE;
        }
    }
    else
    {
        if(get_io_level(LOCK_IO) == 0)
        {
            sta = OPEN;
        }
        else
        {
            sta = CLOSE;
        }
    }
		
    return sta;
}
