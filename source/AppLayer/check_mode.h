/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : check_mode.h
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
#ifndef _CHECK_MODE_H_
#define _CHECK_MODE_H_
/*-------------------------------- Includes ----------------------------------*/
 
 
/*----------------------- Constant / Macro Definitions -----------------------*/

typedef enum
{
   DOOR_STA,
   LOCK_STA,       
}door_lock_sta_e;

typedef enum
{
   DOOR_CLOSE_EVT = 0x00,   //�Ź��¼�
   DOOR_OPEN_EVT = 0x01,    //�ſ��¼�
   OPEN_TIMEOUT_EVT = 0x02,   //�ſ���ʱ
   MOVE_EVT = 0x03,          //��ͷ����
        
}evt_type;

typedef enum
{
   FORCE_OPEN_EXP = 0x00,    
   CARD_OPEN_EXP = 0x03,  
   KEY_OPEN_EXP = 0x04,  
   CMD_OPEN_EXP = 0x06,  
   FIRE_EXP = 0x07,                

   MOVE_EXP = 0x01,

}door_exp_e;


/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void check_evt_real_time(void);
door_exp_e set_open_door_evt(void);
uint8_t  get_door_lock_sta(door_lock_sta_e type);
#endif

