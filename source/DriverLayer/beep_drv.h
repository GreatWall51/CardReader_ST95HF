/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : beep_drv.h
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

#ifndef _BEEP_DRV_H_
#define _BEEP_DRV_H_
/*-------------------------------- Includes ----------------------------------*/
 
 
/*----------------------- Constant / Macro Definitions -----------------------*/
typedef enum
{
    BEEP_NORMAL_MODE   = 0x0,                 //����ģʽ
    BEEP_OPEN_OUT_MODE = 0x04,                //�ſ���ʱ����
    BEEP_OPEN_FORCE_MODE = 0x05,              //��ǿ������
    BEEP_OPEN_READ_MODE = 0x06,               //ˢ������
    BEEP_CARD_2_MODE =0x02,                   //��2��
    
    BEEP_CARD_1_MODE = 0x01,                  //��1��
    BEEP_CARD_4_MODE =0x03,                   //��4��
    BEEP_ADDR_MODE  = 0x10,                   //��ַ�仯����
}BEEP_MODE_E;


/*------------------------ Variable Define/Declarations ----------------------*/
#define  BEEP_TIME_100M      20   //8ms * 25 = 200ms
#define  BEEP_TIME_500M      62   //8ms * 25 = 200ms
#define  BEEP_TIME_1H        25   //8ms * 25 = 200ms
#define  BEEP_TIME_1L        10   //8ms * 25 = 200ms
#define  BEEP_TIME_3S        375 //8ms * 125 = 1000ms

#define  BEEP_TIME_OK        50   //8ms * 25 = 200ms
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void beep_set_mode(BEEP_MODE_E mode);
void beep_real_time(void);
void beep_drv_init(void);
void beep_clear_mode(BEEP_MODE_E mode);
#endif

