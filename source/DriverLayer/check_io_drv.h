/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : check_io_drv.h
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月23日
  最近修改   :
  功能描述   : check_io
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月23日
    作    者   : wzh
    修改内容   : 创建文件

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

