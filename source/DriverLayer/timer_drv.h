/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : timer_drv.h
  �� �� ��   : ����
  ��    ��   : wzh
  ��������   : 2016��2��23��
  ����޸�   :
  ��������   : timerdrv
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��2��23��
    ��    ��   : wzh
    �޸�����   : �����ļ�

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#ifndef _TIMER_DRV_H
#define _TIMER_DRV_H
 
#include "define.h"
#include "stm32f0xx.h"
/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

/*----------------------------- Global Defines ------------------------------*/
#define SBIT(N) ((uint16_t)((uint16_t)(0x01)<<(N)))
#define GET_EVT_MASK(TIME_EVT) ((uint16_t)(TIME_EVT-1)) 
#define TIMER_RELOAD_NUM(mcu_clk_mhz, clk_div, time_us) (0x10000 - (unsigned long)(time_us * (mcu_clk_mhz / clk_div)))

typedef enum {
    TIME_EVT_1 = SBIT(0),
    TIME_EVT_2 = SBIT(1),
    TIME_EVT_4 = SBIT(2),
    TIME_EVT_8 = SBIT(3),   
    TIME_EVT_16 = SBIT(4),
    TIME_EVT_32 = SBIT(5),
    TIME_EVT_64 = SBIT(6),
    TIME_EVT_128 = SBIT(7),
    TIME_EVT_256 = SBIT(8),
    TIME_EVT_512 = SBIT(9),
    TIME_EVT_1024 = SBIT(10),
    TIME_EVT_2048 = SBIT(11),
    TIME_EVT_4096 = SBIT(12),
    TIME_EVT_8192 = SBIT(13),
}TIME_EVT_ENUM;
/*----------------------------- Global Typedefs -----------------------------*/

/*----------------------------- External Variables --------------------------*/
extern uint16_t g_SysTimerEvt; 
 
/*------------------------ Global Function Prototypes -----------------------*/

#define TIME_BASE 1//*ms, �����Ķ�ʱ��λ

/* API �����ж��Ƿ�ָ����ʱ���� ...added by Kimson, 2014-03-19 */
#define IS_TIMER_EVT(TIME_EVT) ((g_SysTimerEvt & TIME_EVT) > 0)

void timer_irq_callback(void);
void init_timer(void);
void timer_real_time(void);
uint32_t get_current_time(void);
//void delay_ms(uint16_t delay_time_ms);
#endif //_TIMERRUN_H_


