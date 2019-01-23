/**
  ******************************************************************************
  * @file    lib_iso14443A.h
  * @author  MMY Application Team
  * @version V1.4
  * @date    12/06/2012
  * @brief   ISO14443A common constants
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  */ 
/* Define to prevent recursive inclusion -------------------------------------------------------*/
#ifndef __ISO14443A_H
#define __ISO14443A_H

#include "stdint.h"
#include "stdbool.h"

/*  status and error code ---------------------------------------------------------------------- */
#define ISO14443A_SUCCESSCODE				0x00
#define ISO14443A_ERRORCODE_DEFAULT			0x61
#define ISO14443A_ERRORCODE_CRC				0x62


/* Anticollison levels (commands)  ------------------------------------------------------------- */
#define SEL_CASCADE_LVL_1											0x93
#define SEL_CASCADE_LVL_2											0x95
#define SEL_CASCADE_LVL_3											0x97
#define COMMAND_REQA													0x26
#define COMMAND_WUPA													0x52
#define COMMAND_SELECT_LV1										0x93
#define COMMAND_SELECT_LV2										0x95
#define COMMAND_SELECT_LV3										0x97
#define COMMAND_HLTA													0x50

#define COMMAND_RATS													0xE0
#define COMMAND_PPS														0xD0
#define COMMAND_DESELECT											0xC2
#define COMMAND_DESELECTCID										0xCA

/* Iblock  ------------------------------------------------------------------------------------- */
#define COMMAND_IBLOCK02												0x02
#define COMMAND_IBLOCK03												0x03
#define COMMAND_SBLOCK													0xC2
#define COMMAND_NACKBLOCK_B2										0xB2
#define COMMAND_NACKBLOCK_B3										0xB3
#define COMMAND_ACKBLOCK												0xA2


/* numbr of the cascade level  ----------------------------------------------------------------- */
#define CASCADE_LVL_1															1
#define CASCADE_LVL_2															2
#define CASCADE_LVL_3															3
		
#define ISO14443A_NVM_10													0x10
#define ISO14443A_NVM_20													0x20
#define ISO14443A_NVM_30													0x30
#define ISO14443A_NVM_40													0x40
#define ISO14443A_NVM_50													0x50
#define ISO14443A_NVM_60													0x60
#define ISO14443A_NVM_70													0x70

/* UID Sizes ---------------------------------------------------------------------------------- */
#define ISO14443A_UIDSIZE_UNDEFINED								-1
#define ISO14443A_UID_PART												3
#define ISO14443A_UID_SINGLE_SIZE									4
#define	ISO14443A_UID_DOUBLE_SIZE									7
#define ISO14443A_UID_TRIPLE_SIZE									10


/* Mask used for ATQA ------------------------------------------------------------------------ */
#define ISO14443A_UID_MASK												0xC0
#define ISO14443A_AC_BIT_FRAME_MASK								0x1F
#define ISO14443A_CID_MASK												0x0F
#define ISO14443A_FSDI_MASK												0xF0

/* Size for ISO14443A variables ------------------------------------------------------------- */
#define ISO14443A_MAX_NAME_SIZE										50
#define ISO14443A_MAX_UID_SIZE			 							10
#define ISO14443A_ATQA_SIZE												2

/* SAK FLAG --------------------------------------------------------------------------------- */
#define SAK_FLAG_ATS_SUPPORTED										0x20
#define SAK_FLAG_UID_NOT_COMPLETE									0x04


/* ATQ FLAG */
#define ATQ_FLAG_UID_SINGLE_SIZE		0
#define	ATQ_FLAG_UID_DOUBLE_SIZE		1
#define ATQ_FLAG_UID_TRIPLE_SIZE		2

typedef struct{
	/* ATQA answer to request of type A*/
	uint8_t 	ATQA[ISO14443A_ATQA_SIZE];   //对REQA（0x26）和WUPA（0x52）的响应，主要作用是判别UID有几级，ATQA共有16位，根据bit7和bit8来判断是1级还是2级还是3级UID；
	uint8_t 	CascadeLevel;                //标识卡号有几级,分1级,2级,3级,卡号长度对应4字节,7字节,10字节
	/* UID : unique Identification*/
	uint8_t 	UIDsize;                    //卡号长度,有4byte,7byte,10byte
	uint8_t 	UID[ISO14443A_MAX_UID_SIZE];//卡号
	/* SAK : Select acknowledge*/
	uint8_t 	SAK;                        //Select ask,选择应答CPU卡:0x28,IC卡:0x08
	bool 		ATSSupported;               //Answer To Select 是否通过
	bool 		IsDetected;                 //RF场是否检测到TYPE_A卡
	char		LogMsg[120];
	//RATS指令时使用,RATS指令格式:开始字节 + 参数(FSDI和CID,1字节) + CRCA16...add by lwq
	uint8_t 	CID,   //RATS指令的参数字段的低4位,它定义编址了的PICC的逻辑号在0到14范围内,值15为RFU.
	                   //CID由PCD规定，并且对同一时刻处在ACTIVE状态中的所有PICC，它应是唯一的。
                       //CID在PICC被激活期间是固定的，并且PICC应使用CID作为其逻辑标识符，它包含在接收到的第一个无差错的RATS。
                       //PCD设置CID为0表示每次仅支持对一张PICC进行定位。
                       //CID的值一般为1.
				FSDI,  //RATS指令的参数字段的低4位,它用于编码FSD。FSD定义了PCD能收到的帧的最大长度。 一般为5,表示PCD能收到的帧的最大长度为64字节.
				       //综上所述,TARS指令的参数字段为0x51,所以RATS指令为: 0xE0  0x51
	//PPS指令时使用,
				DRI,   //发送因子,PSS1的b4b3位
				DSI;   //接收因子,PSS1的b2b1位
}ISO14443A_CARD;

#endif /* __ISO14443A_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
