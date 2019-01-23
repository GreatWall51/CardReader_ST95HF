/******************************************************************************
* Copyright 2018-2023 liuweiqiang@leelen.cn
* FileName: 	 des.h
* Desc:
*
*
* Author: 	 Liuwq
* Date: 	 2018/05/25
* Notes:
*
* -----------------------------------------------------------------
* Histroy: v1.0   2018/05/25, Liuwq create this file
*
******************************************************************************/
#ifndef _DES_H_
#define _DES_H_


/*------------------------------- Includes ----------------------------------*/


/*----------------------------- Global Defines ------------------------------*/
#define ENCRY	0x0
#define DECRY	0x1

/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/

unsigned char TripleMAC ( unsigned char* initdata,unsigned char* insdata,unsigned char datalen,unsigned char* key );
void TDES ( unsigned char MsgIn[],unsigned char Msgout[], unsigned char Key[], unsigned char Mode );
void DES ( unsigned char MsgIn[], unsigned char MsgOut[], unsigned char Key[], unsigned char Mode );


#endif //_DES_H_
