/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : lock_drv.c
  �� �� ��   : ����
  ��    ��   : wzh
  ��������   : 2016��2��23��
  ����޸�   :
  ��������   : lock
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��2��23��
    ��    ��   : wzh
    �޸�����   : �����ļ�

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "led_drv.h" 
#include "timer_drv.h"
#include "define.h"
#include "lock_drv.h"  
/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Variable Define/Declarations ----------------------*/
uint16_t unlock_time = 0; 
unlock_mode_p now_unlock_mode = NORMAL_LOCK_MODE; 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

extern void uart_putc(unsigned char c);

static void lock_on(void)
{
	sbi(GPIOB,GPIO_Pin_2);
}

static void lock_off(void)
{
	cbi(GPIOB,GPIO_Pin_2);
}


void lock_drv_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOB, ENABLE);					  											 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	lock_off();
}

void lock_set_mode(unlock_mode_p unlock_mode, uint8_t time)
{   
      if(time > 0) 
      {                                             //��ť�����������໥���
          if(unlock_mode >= now_unlock_mode || (now_unlock_mode != CMD_UNLOCK_MODE && (unlock_mode > FIRE_UNLOCK_MODE)))
          {
               ENTER_CRITICAL() ;
               unlock_time = time; 
               now_unlock_mode = unlock_mode;
               EXIT_CRITICAL();
          }
      }
}

unlock_mode_p  get_lock_mode(void)
{
    return now_unlock_mode;
}

void lock_real_time(void)
{ 
    static uint8_t  time_1s = 0;
    static uint8_t  old_unlock_time = 0;
    
    if(unlock_time > 0)
    {
        time_1s = 0;
        old_unlock_time = unlock_time;
        ENTER_CRITICAL() ;
        unlock_time = 0;
        EXIT_CRITICAL();
    }

    /* ������ */
    if(old_unlock_time == 0xff)
    {
        time_1s = 0;
        lock_on(); 
    }
    /* �ر��� */
    else if(old_unlock_time == 0xfe)
    {
        time_1s = 0;
        lock_off();

        ENTER_CRITICAL() ;
        now_unlock_mode = NORMAL_LOCK_MODE;
        EXIT_CRITICAL();
   
    }
    else if(old_unlock_time < 0xfe && old_unlock_time > 0)
    {     		
         lock_on();
         if(!IS_TIMER_EVT(TIME_EVT_8)) return;
         if(time_1s++ < 125) return;
         time_1s = 0;
         
         old_unlock_time--;

         #if DEBUG_COM_LOCK
         uart_putc(old_unlock_time);
         #endif
         
         if(old_unlock_time == 0)
         {
            lock_off(); 
            
            ENTER_CRITICAL() ;
            now_unlock_mode = NORMAL_LOCK_MODE;
            EXIT_CRITICAL();
         }
         
    }
    
}



