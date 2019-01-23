/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : led_drv.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月23日
  最近修改   :
  功能描述   : led
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月23日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "led_drv.h" 
#include "timer_drv.h"
#include "define.h"
#include <string.h>
#include <stdlib.h>
/*----------------------- Constant / Macro Definitions -----------------------*/



/*------------------------ Variable Define/Declarations ----------------------*/

typedef enum 
{
    LED_CARD_READ = 0,  
    LED_OPEN_OUT,
    LED_MODE_NUM,

}LED_MODE_INFO_E;

typedef struct
{
    uint8_t  led_sta_bit;
    LED_MODE_E led_sta;
    uint16_t led_timer[LED_MODE_NUM];
    uint16_t led_show_time;
}led_sta_info_t;

led_sta_info_t led_sta_info;
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/


void led_on(void)
{
	sbi(GPIOA,GPIO_Pin_0);
}

void led_off(void)
{	
	cbi(GPIOA,GPIO_Pin_0);
}

static void led_info_init(void)
{
    memset(&led_sta_info,0,sizeof(led_sta_info));
    
    led_sta_info.led_show_time = 0;
    led_sta_info.led_sta_bit = 0;
    led_sta_info.led_sta = LED_NORMAL_MODE;  
}
static void led_io_init(void)
{
   GPIO_InitTypeDef  GPIO_InitStructure;
	
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);					  											 
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;                 
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
    led_on();
}

void led_drv_init(void)
{
    led_io_init();
    led_info_init();
}
/*****************************************************************************
 函 数 名  : led_show
 功能描述  : 控制LED闪烁频率
 输入参数  : UINT8 cycle_time     - 亮灭的周期   
             UINT8 times          - 闪烁次数   
             UINT8 interval_time  - 闪烁间隔
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年12月17日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static uint8_t led_show(uint16_t high_time, uint16_t low_time)
{
	static uint16_t  run_time = 0;
	static uint16_t  old_states = LED_NORMAL_MODE;
	static uint8_t  flag = 1;
    
	if(old_states != led_sta_info.led_sta)
	{
	    run_time = 0;
	    flag = 1;
	    old_states = led_sta_info.led_sta;
	}
	
    if(!IS_TIMER_EVT(TIME_EVT_8))    return 0;

    if(led_sta_info.led_show_time > 0)
    {
        if(flag)
        {
            led_on();
            if(run_time++ > high_time)
            {               
                flag = 0;
                run_time = 0;
            }
        }
        else
        {
            led_off();
            if(run_time++ > low_time)
            {               
                flag = 1;
                led_sta_info.led_show_time--;
                run_time = 0;
            }
        } 
    } 
    else
    {
        //if(run_time++ > (interval_time - low_time))
        {
            led_sta_info.led_show_time = 0;
            run_time = 0;
            return 1;
        }
    }
    return 0;
 
}


static void bit_to_mode(uint8_t mode_bit)
{
  
    switch(mode_bit)
    {

        case 0x00:
           led_sta_info.led_sta = LED_NORMAL_MODE; 
           break;
           
        case 0x01:
           led_sta_info.led_sta= LED_CARD_READ_MODE; 
           if(led_sta_info.led_timer[LED_CARD_READ] > 0)
           {
                led_sta_info.led_show_time = led_sta_info.led_timer[LED_CARD_READ];
                led_sta_info.led_timer[LED_CARD_READ] = 0;
           }
           break;

        case 0x02:
           if(led_sta_info.led_sta != LED_OPEN_OUT_MODE)  
           {
               led_sta_info.led_show_time = led_sta_info.led_timer[LED_OPEN_OUT];
           }
           led_sta_info.led_sta = LED_OPEN_OUT_MODE;
           break;

        default:
           break;
    }
 
}

static void hal_led_run(void)
{
    uint8_t i;
    uint8_t mode_bit = 0;


    /*用于超时灯光报警被打断可以被接续*/
    if(led_sta_info.led_sta == LED_OPEN_OUT_MODE)
    {
        led_sta_info.led_timer[LED_OPEN_OUT] = led_sta_info.led_show_time;
    }

    for(i=0; i<8; i++)
    {
        mode_bit = (1 << i);
        if(led_sta_info.led_sta_bit & mode_bit)
        {
            bit_to_mode(mode_bit);
            return;
        }
    }
    mode_bit = 0;
    bit_to_mode(mode_bit);
    
}

void led_clear_mode(LED_MODE_E mode)
{
    ENTER_CRITICAL() ;
    switch(mode)
    {
        case LED_CARD_READ_MODE: 
           led_sta_info.led_sta_bit &= ~(1 << LED_CARD_READ);
           break;

        case LED_OPEN_OUT_MODE:
           led_sta_info.led_sta_bit &= ~(1 << LED_OPEN_OUT);
           break;
           
        default:
           break;
    }  
    EXIT_CRITICAL();
}

void led_set_mode(LED_MODE_E mode)
{  

    if(mode == led_sta_info.led_sta) return;

    ENTER_CRITICAL() ;
    switch(mode)
    {
        case LED_CARD_READ_MODE: 
           led_sta_info.led_sta_bit |= (1 << LED_CARD_READ);
           led_sta_info.led_timer[LED_CARD_READ] = 1;
           break;
           
        case LED_OPEN_OUT_MODE:
           led_sta_info.led_sta_bit |= (1 << LED_OPEN_OUT);
           led_sta_info.led_timer[LED_OPEN_OUT] = 20;
           break;

        default:
           break;
    }
    EXIT_CRITICAL();
}

void led_real_time(void)
{
    hal_led_run();

    switch(led_sta_info.led_sta)
    {
        case  LED_NORMAL_MODE:
           led_on();
           break;
           
        case LED_CARD_READ_MODE:
           if(led_show(0, LED_TIME_200M) == 1)
           {
               led_clear_mode(LED_CARD_READ_MODE);
           }
           break;
           
        case LED_OPEN_OUT_MODE:
           if(led_show(LED_TIME_500M, LED_TIME_3S) == 1)
           {
               led_clear_mode(LED_OPEN_OUT_MODE);
           }
           break;
           
        default:
           break;
    }
 
}


