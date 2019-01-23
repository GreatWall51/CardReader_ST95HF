/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 mfos.h 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/05/22
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/05/22, liuwq create this file
* 
******************************************************************************/
#ifndef _MFOS_H_     
#define _MFOS_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "stdbool.h"
#include "crypto1.h"
/*----------------------------- Global Typedefs -----------------------------*/
 typedef enum {
  MC_AUTH_A = 0x60,
  MC_AUTH_B = 0x61,
  MC_READ = 0x30,
  MC_WRITE = 0xA0,
  MC_TRANSFER = 0xB0,
  MC_DECREMENT = 0xC0,
  MC_INCREMENT = 0xC1,
  MC_STORE = 0xC2
} mifare_cmd;
 

/*----------------------------- Global Defines ------------------------------*/
 #define MAX_FRAME_LEN 256
 #define odd_parity(i) (( (i) ^ (i)>>1 ^ (i)>>2 ^ (i)>>3 ^ (i)>>4 ^ (i)>>5 ^ (i)>>6 ^ (i)>>7 ^ 1) & 0x01)
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/
uint8_t mf_enhanced_auth(uint8_t e_block,uint8_t *uid,uint8_t *key,uint8_t key_type);
uint8_t PCDNFCT2_Read(uint8_t blocNbr, uint8_t *pBufferRead);
uint8_t PCDNFCT2_Write(uint8_t blocNbr, uint8_t *pBufferWrite);

#endif //_MFOS_H_
