/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 memory_manage.c 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/05/17
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/05/17, liuwq create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/
#include <stdlib.h>
#include "stm32f0xx.h"
#include <string.h>
#include "drv_95HF.h"
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/
 
 
/*------------------- Global Definitions and Declarations -------------------*/
 uint8_t *pTT5Tag = NULL;
 uint8_t *u95HFBuffer =NULL;
/*----------------------- Variable Declarations -----------------------------*/
 
 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/
void memory_init(void)
{
    pTT5Tag = (uint8_t *)malloc(512);
	memset(pTT5Tag,0,512);
	u95HFBuffer = (uint8_t *)malloc(RFTRANS_95HF_MAX_BUFFER_SIZE);
	memset(u95HFBuffer,0,RFTRANS_95HF_MAX_BUFFER_SIZE);
}
 
/*---------------------------------------------------------------------------*/

