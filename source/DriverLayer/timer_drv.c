/******************************************************************************
* Copyright 2010-2014 Renyucjs@163.com
* FileName: 	 TimerRun.c 
* Desc:
* 
* 
* Author: 	 Kimson
* Date: 	 2014/03/06
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2014/03/06, Kimson create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/

#include "timer_drv.h"
#include "define.h"
#include "hal_timer.h"
#include "debug.h"
/*------------------- Global Definitions and Declarations -------------------*/

/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/
 
/*----------------------- Variable Declarations -----------------------------*/
static uint32_t s_timeIntCnt = 0;//基础分频计数
uint16_t  g_SysTimerEvt; 
static unsigned int  s_SysTimerEvt;
//static uint16_t delay_Nms = 0;

/** 填写程序需要的定时分频事件 **/
const static TIME_EVT_ENUM s_ReqEvt[] = 
{
	TIME_EVT_1,
	TIME_EVT_2,
	TIME_EVT_8,
	TIME_EVT_16,
	TIME_EVT_32,
	TIME_EVT_64,
	TIME_EVT_128,	
	TIME_EVT_256,
	TIME_EVT_512,
	TIME_EVT_1024
};
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/


/******************************************************************************
* Name: 	 init_timer 
*
* Desc: 	 
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/03/14, Create this function by Kimson
 ******************************************************************************/
void init_timer(void)
{
    g_SysTimerEvt = 0;
    s_SysTimerEvt = 0;
    timer_init();
}
/******************************************************************************
* Name: 	 timer_real_time 
*
* Desc: 	 
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/03/14, Create this function by Kimson
 ******************************************************************************/
void timer_real_time(void)
{
    // Clear event
    g_SysTimerEvt = 0;
    // nothing happend
    if(!s_SysTimerEvt)return;
    //Set Event
    g_SysTimerEvt = s_SysTimerEvt;
    s_SysTimerEvt = 0;///TODO
}

/******************************************************************************
* Name: 	 timer_irq 
*
* Desc: 	 This function must be run in TIME_BASE period IRQ Handler
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2014/03/14, Create this function by Kimson
 ******************************************************************************/
void timer_irq_callback(void)
{
    unsigned char i;

//    DEBUG_CHK_LINE_TIMER_INT(1000);
    s_timeIntCnt ++;
//	  if(delay_Nms>0) delay_Nms--;
    for(i = 0; i < sizeof(s_ReqEvt)/sizeof(s_ReqEvt[0]); i++)
    {
        if(!(s_timeIntCnt & GET_EVT_MASK(s_ReqEvt[i])))
        {
            s_SysTimerEvt |= s_ReqEvt[i];
        }
    }
}
/*---------------------------------------------------------------------------*/

uint32_t get_current_time(void)
{
    return s_timeIntCnt;
}
//void delay_ms(uint16_t delay_time_ms)
//{
//		delay_Nms = delay_time_ms;
//		while(delay_Nms);
//}


