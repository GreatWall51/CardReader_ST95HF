/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : flash_drv.h
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年4月22日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月22日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/
#ifndef _FLASH_DRV_H_
#define  _FLASH_DRV_H_
/*-------------------------------- Includes ----------------------------------*/
 
 
/*----------------------- Constant / Macro Definitions -----------------------*/
 #define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
 #define FLASH_USER_START_ADDR   ((uint32_t)0x08007C00)   /* Start @ of user Flash area */
 #define FLASH_USER_END_ADDR     ((uint32_t)0x08010000)   /* End @ of user Flash area */

/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
uint8_t  flash_write(uint32_t start_addr, uint8_t *paddr, uint32_t size);
void flash_read(uint32_t start_addr, uint8_t *paddr, uint32_t size);
#endif

