/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : flash_drv.c
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

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "timer_drv.h"
#include "define.h" 
#include <string.h>
#include <stdlib.h> 
#include "flash_drv.h"
/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

uint8_t  flash_write(uint32_t start_addr, uint8_t *paddr, uint32_t size)
{
   uint8_t data;
   uint16_t i;
   uint32_t end_addr;
   uint32_t value;
   uint32_t page_num;
   uint32_t address;
   uint32_t write_cnt;


   if(start_addr < FLASH_USER_START_ADDR || size < 1 || paddr == NULL) return 0;
   if(((start_addr - FLASH_USER_START_ADDR) % 1024) != 0) return 0;
   
   value = size % FLASH_PAGE_SIZE;
   page_num = size / FLASH_PAGE_SIZE;
   
   if(value != 0)
   {
       page_num += 1;
   }
   end_addr += (start_addr + (page_num * FLASH_PAGE_SIZE));
   
   FLASH_Unlock();    
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); 

  /* Erase the FLASH pages */
  for(i = 0; (i < page_num); i++)
  {
    if (FLASH_ErasePage(start_addr + (FLASH_PAGE_SIZE * i)) != FLASH_COMPLETE)
    {
     /* Error occurred while sector erase. 
         User can add here some code to deal with this error  */
      while (1)
      {
      }
    }
  }
  /* Program the user Flash area word by word
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  address = start_addr;
  write_cnt = 0;

  while (write_cnt < size)
  {
    if (FLASH_ProgramWord(address, *(paddr+write_cnt)) == FLASH_COMPLETE)
    {
       flash_read(address, &data, 1);
       if(data != *(paddr+write_cnt))
       {
            FLASH_Lock(); 
            return 0;
       }
       address = address + 4;
       write_cnt++;
    }
    else
    { 
      /* Error occurred while writing data in Flash memory. 
         User can add here some code to deal with this error */
      while (1)
      {
      }
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  FLASH_Lock(); 

  return 1;
}


void flash_read(uint32_t start_addr, uint8_t *paddr, uint32_t size)
{
  uint32_t read_cnt = 0;
  uint32_t value;
  uint32_t address;

  if(paddr == NULL || size < 1) return;
  address = start_addr;

  while (read_cnt < size)
  {
    value = *(__IO uint32_t *)address;
    *(paddr+read_cnt) = (uint8_t)value;
    address = address + 4;
    read_cnt++;
  }
}

