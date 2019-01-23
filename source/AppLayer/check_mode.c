/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : check_mode.c
  �� �� ��   : ����
  ��    ��   : wzh
  ��������   : 2016��2��24��
  ����޸�   :
  ��������   : 
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��2��24��
    ��    ��   : wzh
    �޸�����   : �����ļ�

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
 �� �� ��  : evt_frame_to_queue
 ��������  : �¼���������Ͷ���
 �������  : evt_type type   
             door_exp_e exp  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void evt_frame_to_queue(evt_type type, door_exp_e exp)
{

    rs485_frame_msg *p_msg;
    reader_evt_frame_t *frame_data;
 
    /* ����һ������ڵ� */
    p_msg = append_frame_queue(9); 

    if(p_msg == NULL) return;

    frame_data = (reader_evt_frame_t *)set_frame_head(p_msg->pbuf, EVT_UPDATA_CMD, 8, 0);

    /* ��ȡ�����ܳ��� */
    p_msg->len = *(p_msg->pbuf+FRAME_OFFSET_LEN)+1;
      
    frame_data->evt_type = type;
    frame_data->expand = exp;
    frame_data->sum = get_check_sum(p_msg->pbuf,p_msg->len);   

}
/*****************************************************************************
 �� �� ��  : check_door_evt
 ��������  : ����ſ����¼�
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��2��24��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void check_door_evt(void)
{
    evt_type evt;
    door_exp_e exp_evt;
    
    if(get_io_evt(DOOR_STATUS_IO))  //�ſ����¼����
    {
       
        if(get_io_level(DOOR_STATUS_IO)) //�ſ��¼�
        {     
             evt = DOOR_OPEN_EVT;  
             /* ��ȡ����ԭ�� */
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
 �� �� ��  : check_door_open_out
 ��������  : ����ſ���ʱ
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��2��24��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void check_door_open_out(void)
{
    static uint16_t time_1s = 0;
    static uint16_t open_time = 0; 
    
    if(get_io_level(DOOR_STATUS_IO))                      //�ſ�
    {
                 
        if(!IS_TIMER_EVT(TIME_EVT_8))   return;          //�ſ���ʱ���
        if(time_1s++ < 125) return;
        time_1s = 0;
        
        if(open_time++ < get_sys_param(OPEN_TIMEOUT_PARAM)) return;      
        open_time = 0; 
        
        if(check_door_open_out_flag == 0) return;         //һ���ſ���������������

        check_door_open_out_flag = 0;
        beep_set_mode(BEEP_OPEN_OUT_MODE);
        led_set_mode(LED_OPEN_OUT_MODE);
        evt_frame_to_queue(OPEN_TIMEOUT_EVT,(door_exp_e)0);         
        
    }
    else
    {
        open_time = 0;
        time_1s = 0;
        /* ����ſ���ʱ����ָʾ */
        led_clear_mode(LED_OPEN_OUT_MODE);
        beep_clear_mode(BEEP_OPEN_OUT_MODE);
    }
}

/*****************************************************************************
 �� �� ��  : check_reader_move
 ��������  : ����ͷ�Ƿ�����
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void check_reader_move(void)
{
    
    if(get_io_evt(MOVE_IO))  
    {
        if(get_io_level(MOVE_IO))                  //��ͷ����
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
 �� �� ��  : check_key_open
 ��������  : ��⿪�Ű�ť�Ƿ񱻰���
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void check_key_open(void)
{
    
    if(get_io_evt(KEY_OPEN_IO))  //���Ű�ť
    {
        if(get_io_level(KEY_OPEN_IO) == 0)
        {              
            lock_set_mode(KEY_UNLOCK_MODE,get_sys_param(UNLOCK_TIME_PARAM));
//						beep_set_mode(BEEP_CARD_2_MODE);
        }
    }
}

/*****************************************************************************
 �� �� ��  : check_fire
 ��������  : �����Ƿ񱻴���
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void check_fire(void)
{
  static uint8_t report_flag=0;//�¼��ϱ���־
	
	//��ֹ��״̬�£���ͷ��ʼ��δ��ɣ����ϱ����¼�(Դ��ַ��ַ����)�
	//�ȵ���ʼ�����ʱ����ַ�ı���޷�ʶ��������Ӧ�����һֱ�ϱ����¼�
	if(get_reader_addr() == 0) return;
	
	if(get_io_level(FIRE_STATUS_IO) == 0)   //�𾯴���
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
 �� �� ��  : check_reader_addr
 ��������  : ����ͷ��ַ�Ƿ�仯
 �������  : void  
 �������  : ��
 �� �� ֵ  : static
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

*****************************************************************************/
static void check_reader_addr(void)
{
    static uint8_t old_addr = 0x17;
    uint8_t addr;

    if(get_reader_addr() == 0) return;    //�ȴ���ͷ��ַ������
    
    addr = get_reader_addr();
    if(addr != old_addr)   //��ͷ��ַ�仯
    {    
        beep_set_mode(BEEP_ADDR_MODE);
        free_frame_queue();
        old_addr = addr;
    } 
}


/*****************************************************************************
 �� �� ��  : set_open_door_evt
 ��������  : ����״̬ת��Ϊ����ԭ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

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
 �� �� ��  : check_evt_real_time
 ��������  : ����¼�
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��2��24��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

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
 �� �� ��  : get_door_lock_sta
 ��������  : ��ȡ����״̬
 �������  : door_lock_sta_e type  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��13��
    ��    ��   : wzh
    �޸�����   : �����ɺ���

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
