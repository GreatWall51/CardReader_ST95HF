/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : check_io_drv.h
  �� �� ��   : ����
  ��    ��   : wzh
  ��������   : 2016��2��23��
  ����޸�   :
  ��������   : check_io
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��2��23��
    ��    ��   : wzh
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef _CHECK_IO_H_
#define _CHECK_IO_H_
/*-------------------------------- Includes ----------------------------------*/
 
 
/*----------------------- Constant / Macro Definitions -----------------------*/
#define  MOVE_IO           0
#define  CARD_SFZ          1
#define  LOCK_IO           2
#define  KEY_OPEN_IO       5
#define  DOOR_STATUS_IO    6
#define  FIRE_STATUS_IO    7
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void check_io_drv_init(void);
void check_io_real_time(void);
uint8_t get_io_evt( uint8_t no );
uint8_t get_io_level( uint8_t no );
uint8_t get_io_check_ok(void);

void check_addr_real_time(void);
uint8_t get_reader_addr(void);
#endif

