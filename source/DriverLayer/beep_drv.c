/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : beep_drv.c
  �� �� ��   : ����
  ��    ��   : wzh
  ��������   : 2016��2��23��
  ����޸�   :
  ��������   : beep
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��2��23��
    ��    ��   : wzh
    �޸�����   : �����ļ�

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "timer_drv.h"
#include "define.h" 
#include "beep_drv.h"
#include <string.h>
#include <stdlib.h>
/*----------------------- Constant / Macro Definitions -----------------------*/
#define  BUFF_NUM     5

/*------------------------ Variable Define/Declarations ----------------------*/


typedef enum 
{
    BEEP_CARD_READ = 0,  //ˢ���¼�����λ        
    BEEP_CARD_1,         
    BEEP_CARD_2,
    BEEP_CARD_4,
    BEEP_ADDR,
    BEEP_OPEN_OUT,
    BEEP_OPEN_FORCE,
    BEEP_MODE_NUM,

}BEEP_MODE_INFO_E;

typedef struct
{
    uint8_t   beep_sta_bit;
    BEEP_MODE_E beep_sta;
    uint16_t  beep_timer[BEEP_MODE_NUM];
    uint16_t  beep_show_time;
}beep_sta_info_t;

beep_sta_info_t beep_sta_info;

uint16_t TimerPeriod = 0;
uint16_t Channel1Pulse = 0, Channel2Pulse = 0, Channel3Pulse = 0;

/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

static void beep_on(void)
{
	 TIM_Cmd(TIM15, ENABLE);
}

static void beep_off(void)
{
	 TIM_Cmd(TIM15, DISABLE);
}


static void beep_info_init(void)
{
    memset(&beep_sta_info,0,sizeof(beep_sta_info));
    
    beep_sta_info.beep_show_time = 0;
    beep_sta_info.beep_sta = BEEP_NORMAL_MODE;
    beep_sta_info.beep_sta_bit = 0;
    
}

static void beep_io_init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* GPIOA, GPIOB and GPIOE Clocks enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  
  /* GPIOA Configuration: Channel 1, 2, 3, 4 and Channel 1N as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

 
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_1);

  
  /* TIM1 Configuration ---------------------------------------------------
   Generate 7 PWM signals with 4 different duty cycles:
   TIM1 input clock (TIM1CLK) is set to APB2 clock (PCLK2)    
    => TIM1CLK = PCLK2 = SystemCoreClock
   TIM1CLK = SystemCoreClock, Prescaler = 0, TIM1 counter clock = SystemCoreClock
   SystemCoreClock is set to 48 MHz for STM32F0xx devices
   
   The objective is to generate 7 PWM signal at 17.57 KHz:
     - TIM1_Period = (SystemCoreClock / 17570) - 1
   The channel 1 and channel 1N duty cycle is set to 50%
   The channel 2 and channel 2N duty cycle is set to 37.5%
   The channel 3 and channel 3N duty cycle is set to 25%
   The channel 4 duty cycle is set to 12.5%
   The Timer pulse is calculated as follows:
     - ChannelxPulse = DutyCycle * (TIM1_Period - 1) / 100
   
   Note: 
    SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f0xx.c file.
    Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
    function to update SystemCoreClock variable value. Otherwise, any configuration
    based on this variable will be incorrect. 
  ----------------------------------------------------------------------- */
  /* Compute the value to be set in ARR regiter to generate signal frequency at 17.57 Khz */
  TimerPeriod = (SystemCoreClock / 2700 ) - 1;
  /* Compute CCR1 value to generate a duty cycle at 50% for channel 1 and 1N */
  Channel1Pulse = (uint16_t) (((uint32_t) 5 * (TimerPeriod - 1)) / 10);
  

  /* TIM1 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15 , ENABLE);
  
  /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);

  /* Channel 1, 2,3 and 4 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  TIM_OC1Init(TIM15, &TIM_OCInitStructure);

  /* TIM1 counter enable */
  TIM_Cmd(TIM15, DISABLE);

  /* TIM1 Main Output Enable */
  TIM_CtrlPWMOutputs(TIM15, ENABLE);


}

void beep_drv_init(void)
{
    beep_io_init();
    beep_info_init();
//    beep_set_mode(BEEP_CARD_1_MODE);
}
/*****************************************************************************
 �� �� ��  : led_show
 ��������  : ����LED��˸Ƶ��
 �������  : uint16_t high_time     - �ߵ�ƽ��ʱ��   
             uint16_t low_time      - �͵�ƽʱ��  
            
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2015��12��17��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
static uint8_t beep_show(uint16_t high_time, uint16_t low_time)
{
	static uint16_t  run_time;
	static uint16_t  old_states;
	static uint8_t  flag = 1;
    
	if(old_states != beep_sta_info.beep_sta)
	{
	    run_time = 0;
	    flag = 1;
	    old_states = beep_sta_info.beep_sta;
	}
	
    if(!IS_TIMER_EVT(TIME_EVT_8))    return 0;

    if(beep_sta_info.beep_show_time> 0)
    {
        if(flag)
        {
            beep_on();
            if(run_time++ > high_time)
            {               
                flag = 0;
                run_time = 0;
            }
        }
        else
        {
            beep_off();
            if(run_time++ > low_time)
            {               
                flag = 1;
                run_time = 0;
                beep_sta_info.beep_show_time-- ;           
            }
        }       
    } 
    else
    {
       // if(run_time++ > (interval_time - low_time))
        {
            run_time = 0;
            beep_sta_info.beep_show_time = 0;           
            return 1;
        }
    }
    return 0;
 
}

/*****************************************************************************
 �� �� ��  : bit_to_mode
 ��������  : ��״̬λת��Ϊ����ģʽ
 �������  : uint8_t mode_bit  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void bit_to_mode(uint8_t mode_bit)
{  
    switch(mode_bit)
    {

        case 0x00:
           beep_sta_info.beep_sta = BEEP_NORMAL_MODE;
           break;
           
        case 0x01:
           beep_sta_info.beep_sta = BEEP_OPEN_READ_MODE; 
           /*ֻ�и��¹�ģʽ�������¸���*/
           if(beep_sta_info.beep_timer[BEEP_CARD_READ] > 0)
           {
                beep_sta_info.beep_show_time = beep_sta_info.beep_timer[BEEP_CARD_READ];
                beep_sta_info.beep_timer[BEEP_CARD_READ]  = 0;
           }
           break;

        case 0x02:
           beep_sta_info.beep_sta = BEEP_CARD_1_MODE;
           if(beep_sta_info.beep_timer[BEEP_CARD_1] > 0)
           {
                beep_sta_info.beep_show_time = beep_sta_info.beep_timer[BEEP_CARD_1];
                beep_sta_info.beep_timer[BEEP_CARD_1]  = 0;
           }
           break;
           
        case 0x04:
           beep_sta_info.beep_sta = BEEP_CARD_2_MODE;
           if(beep_sta_info.beep_timer[BEEP_CARD_2] > 0)
           {
                beep_sta_info.beep_show_time = beep_sta_info.beep_timer[BEEP_CARD_2];
                beep_sta_info.beep_timer[BEEP_CARD_2]  = 0;
           }
           break;
           
        case 0x08:
           beep_sta_info.beep_sta = BEEP_CARD_4_MODE;
           if(beep_sta_info.beep_timer[BEEP_CARD_4] > 0)
           {
                beep_sta_info.beep_show_time = beep_sta_info.beep_timer[BEEP_CARD_4];
                beep_sta_info.beep_timer[BEEP_CARD_4]  = 0;
           }
           break;  
           
        case 0x10:
           beep_sta_info.beep_sta = BEEP_ADDR_MODE; 
           if(beep_sta_info.beep_timer[BEEP_ADDR] > 0)
           {
                beep_sta_info.beep_show_time = beep_sta_info.beep_timer[BEEP_ADDR];
                beep_sta_info.beep_timer[BEEP_ADDR]  = 0;
           }
           break; 
           
        case 0x20:
           /**/
           if(beep_sta_info.beep_sta != BEEP_OPEN_OUT_MODE)
           {
                /*���ʣ���������*/
                beep_sta_info.beep_show_time = beep_sta_info.beep_timer[BEEP_OPEN_OUT];
           }
           beep_sta_info.beep_sta = BEEP_OPEN_OUT_MODE;

           break;
           
        case 0x40:
           beep_sta_info.beep_sta = BEEP_OPEN_FORCE_MODE;                 
           break;

       default:
           break;
    }

}

static void hal_beep_run(void)
{
    uint8_t i;
    uint8_t mode_bit;
   
    if(IS_TIMER_EVT(TIME_EVT_8))
    {
         /* ���ڳ��챻����������Ϻ󣬳�����Ա����� */                  
         if(beep_sta_info.beep_timer[BEEP_OPEN_FORCE] > 0)
         {    
             ENTER_CRITICAL();
             beep_sta_info.beep_timer[BEEP_OPEN_FORCE]--; 
             EXIT_CRITICAL();
         }        
    }
    if(beep_sta_info.beep_sta == BEEP_OPEN_OUT_MODE)
    {
         beep_sta_info.beep_timer[BEEP_OPEN_OUT] = beep_sta_info.beep_show_time;
    }
     
    /* ��Ӧbitλת��Ϊ����ģʽ */
    for(i=0; i<8; i++)
    {
        mode_bit = (1 << i);
        if(beep_sta_info.beep_sta_bit & mode_bit)
        {
            bit_to_mode(mode_bit);
            return;
        }
    }
    /* û����Ӧbitλ��Ҫת������Ϊ����ģʽ */
    mode_bit = 0;
    bit_to_mode(mode_bit);
}


/*****************************************************************************
 �� �� ��  : beep_clear_mode
 ��������  : �������ģʽbitλ
 �������  : BEEP_MODE_E mode  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
void beep_clear_mode(BEEP_MODE_E mode)
{
    ENTER_CRITICAL() ;
    switch(mode)
    {
        case BEEP_OPEN_READ_MODE: 
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_READ);
           break;

        case BEEP_CARD_1_MODE:
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_1);
           break;
           
        case BEEP_CARD_2_MODE:
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_2);
           break;
           
        case BEEP_CARD_4_MODE:
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_4);
           break;   
           
         case BEEP_ADDR_MODE:
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_ADDR);  
           break; 
           
        case BEEP_OPEN_OUT_MODE:
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_OPEN_OUT);
           break;
           
        case BEEP_OPEN_FORCE_MODE:
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_OPEN_FORCE);  
           beep_sta_info.beep_timer[BEEP_OPEN_FORCE] = 0;
           break; 
           
        default:
           break;
    } 
    EXIT_CRITICAL();
}

/*****************************************************************************
 �� �� ��  : beep_set_mode
 ��������  : ������Ӧ����bitλ
 �������  : BEEP_MODE_E mode  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
void beep_set_mode(BEEP_MODE_E mode)
{
    ENTER_CRITICAL() ;
    switch(mode)
    {
        case BEEP_OPEN_READ_MODE: 
           beep_sta_info.beep_sta_bit |= (1 << BEEP_CARD_READ);
            /* ͬʱֻ����һ�ֶ������ */
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_1); 
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_2);
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_4);
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_ADDR);
           beep_sta_info.beep_timer[BEEP_CARD_READ] = 1;
           break;

        case BEEP_CARD_1_MODE:
           beep_sta_info.beep_sta_bit |= (1 << BEEP_CARD_1);

           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_2);
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_4);
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_ADDR);
           beep_sta_info.beep_timer[BEEP_CARD_1] = 1;
           break;
           
        case BEEP_CARD_2_MODE:
           beep_sta_info.beep_sta_bit |= (1 << BEEP_CARD_2);
           
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_1);
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_4);
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_ADDR);
           beep_sta_info.beep_timer[BEEP_CARD_2] = 2;
           break;
           
        case BEEP_CARD_4_MODE:
           beep_sta_info.beep_sta_bit |= (1 << BEEP_CARD_4);

           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_1);
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_2);
            beep_sta_info.beep_sta_bit &= ~(1 << BEEP_ADDR);
           beep_sta_info.beep_timer[BEEP_CARD_4] = 4;
           break;
           
        case BEEP_ADDR_MODE:
           beep_sta_info.beep_sta_bit |= (1 << BEEP_ADDR);  

           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_1);
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_4);
            beep_sta_info.beep_sta_bit &= ~(1 << BEEP_CARD_2);
           beep_sta_info.beep_timer[BEEP_ADDR] = 1;
           break;  
           
        case BEEP_OPEN_OUT_MODE:          
           beep_sta_info.beep_sta_bit |= (1 << BEEP_OPEN_OUT);        
           /* ͬʱֻ����һ�ֳ������ */   
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_OPEN_FORCE);  
           beep_sta_info.beep_timer[BEEP_OPEN_OUT] = 20;  
           break;
           
        case BEEP_OPEN_FORCE_MODE:
           beep_sta_info.beep_sta_bit &= ~(1 << BEEP_OPEN_OUT);
           beep_sta_info.beep_sta_bit |= (1 << BEEP_OPEN_FORCE);  
           beep_sta_info.beep_timer[BEEP_OPEN_FORCE] = 7500; // 8 * 7500ms = 60s����1����
           break;
                     
        default:
           break;
    }
    EXIT_CRITICAL();
}

extern void uart_putc(unsigned char c);

void beep_real_time(void)
{

    static uint16_t time = 0, time1 = 0;
    static uint16_t read_time = 0;
    static BEEP_MODE_E old_sta = BEEP_NORMAL_MODE;


    hal_beep_run();

    if(old_sta != beep_sta_info.beep_sta && (old_sta != BEEP_NORMAL_MODE)) //ģʽ�л���ʱ
    {
        beep_off();
       if(!IS_TIMER_EVT(TIME_EVT_8)) return;    
       if(read_time++ < 12)  return;       
       read_time = 0;
    }
    old_sta = beep_sta_info.beep_sta;
   
    switch(beep_sta_info.beep_sta)
    {
        case  BEEP_NORMAL_MODE:
           beep_off();
           break;
           
        case BEEP_OPEN_READ_MODE:      
           
           if(beep_show(BEEP_TIME_100M, 0) == 1)
           {   
               /*ˢ�����ȴ�����Чȷ��*/
               if(!IS_TIMER_EVT(TIME_EVT_8)) return;  
               if(time1++ < 62) return;
               time1 = 0;
               beep_clear_mode(BEEP_OPEN_READ_MODE);
           }
           break;
           
        case BEEP_ADDR_MODE:
           if(beep_show(BEEP_TIME_1H, 0) == 1)
           {                  
               beep_clear_mode(BEEP_ADDR_MODE);
           }
           break;
           
        case BEEP_CARD_1_MODE:
           
           if(beep_show(BEEP_TIME_OK, 0) == 1)
           {  
               /* ���ڴ�ϳ���ʱ�м�� */
               if(!IS_TIMER_EVT(TIME_EVT_8)) return;
               if(time++ < 100) return;
               time = 0;    
               beep_clear_mode(BEEP_CARD_1_MODE);
           }
           break;
           
        case BEEP_CARD_2_MODE:
           if(beep_show(BEEP_TIME_1H, BEEP_TIME_1L) == 1)
           {
                /* ���ڴ�ϳ���ʱ�м�� */
               if(!IS_TIMER_EVT(TIME_EVT_8)) return;
               if(time++ < 100) return;
               time = 0;               
     
               beep_clear_mode(BEEP_CARD_2_MODE);
           }
           break;
           
         case BEEP_CARD_4_MODE:
           if(beep_show(BEEP_TIME_1H, BEEP_TIME_1L) == 1)
           {
               if(!IS_TIMER_EVT(TIME_EVT_8)) return;
               if(time++ < 100) return;
               time = 0;               
   
               beep_clear_mode(BEEP_CARD_4_MODE);
           }
           break; 
           
        case BEEP_OPEN_OUT_MODE:
        
           if(beep_show(BEEP_TIME_500M, BEEP_TIME_3S) == 1)
           {
               beep_clear_mode(BEEP_OPEN_OUT_MODE);
           }
           break;
           
        case BEEP_OPEN_FORCE_MODE:
        
           beep_on();                     
           if(beep_sta_info.beep_timer[BEEP_OPEN_FORCE] > 0)   return;                     
           beep_clear_mode(BEEP_OPEN_FORCE_MODE);
           break;
              
        default:
           break;
    }
 
}

