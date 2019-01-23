/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : hal_timer.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年7月11日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年7月11日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "timer_drv.h"
#include "define.h" 
#include "hal_timer.h"
#include "drv_interrupt.h"
/*----------------------- Constant / Macro Definitions -----------------------*/
extern void rs485_send_delay(void);
extern void com_hal_timer_real_time(void);
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
//extern void com_hal_timer_real_time(void);

/******************************************************************************
* Name: 	 timer0_init 
*
* Desc: 	 T = 10ms
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/12/27, Create this function by Kimson
 ******************************************************************************/
void timer_init(void)
{
    /* 硬件初始化 */
    SysTick_Config(SystemCoreClock / (1000 / TIME_BASE));	//Set SysTick Timer for 1ms interrupts
    drvInt_TimeoutTimerConfig();
}

/*****************************************************************************
 函 数 名  : SysTick_Handler
 功能描述  : 定时器中断入口
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年7月11日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void SysTick_Handler(void)
{
    //static unsigned char s_timeIntCnt = 0;//基础分频计数
//   com_hal_timer_real_time();
   timer_irq_callback(); 
	com_hal_timer_real_time();
   rs485_send_delay();
} 
