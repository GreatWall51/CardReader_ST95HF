/**
  ******************************************************************************
  * @file    lib_iso14443Apcd.h
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the iso14443A communication
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
#ifndef __ISO14443APCD_H
#define __ISO14443APCD_H

#include "lib_iso14443Apcd.h"
#include "lib_iso14443A.h"

/*  status and error code ---------------------------------------------------------------------- */
//#define ISO14443A_SUCCESSCODE									0x00
#define ISO14443A_ERRORCODE_DEFAULT			0x61
#define ISO14443A_ERRORCODE_CRC				0x62
#define ISO14443A_ERRORCOD_INIT             1
#define ISO14443A_ERRORCOD_NOTAG            2
#define ISO14443A_ERRORCOD_REQA             3
#define ISO14443A_ERRORCOD_ANTICOLL         4
#define ISO14443A_ERRORCOD_AUTHEN           5
#define ISO14443A_ERRORCOD_SELECTA          6
#define ISO14443A_ERRORCOD_HALTA            7

/* Anticollison levels (commands)  ------------------------------------------------------------- */
#define SEL_CASCADE_LVL_1											0x93
#define SEL_CASCADE_LVL_2											0x95
#define SEL_CASCADE_LVL_3											0x97
#define COMMAND_RATS													0xE0
#define COMMAND_PPS														0xD0

/* Iblock  ------------------------------------------------------------------------------------- */
#define COMMAND_IBLOCK02												0x02
#define COMMAND_IBLOCK03												0x03
#define COMMAND_SBLOCK													0xC2
#define COMMAND_NACKBLOCK												0xB2
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
#define ISO14443A_UID_PART										3   //获得部分卡号
#define ISO14443A_UID_SINGLE_SIZE								4   //获得4字节卡号
#define	ISO14443A_UID_DOUBLE_SIZE								7   //获得7字节卡号
#define ISO14443A_UID_TRIPLE_SIZE								10 //获得10字节卡号


/* Mask used for ATQA ------------------------------------------------------------------------ */
#define ISO14443A_UID_MASK										0xC0
#define ISO14443A_AC_BIT_FRAME_MASK								0x1F
#define ISO14443A_CID_MASK										0x0F
#define ISO14443A_FSDI_MASK										0xF0

/* Size for ISO14443A variables ------------------------------------------------------------- */
#define ISO14443A_MAX_NAME_SIZE								50
#define ISO14443A_MAX_UID_SIZE			 					10
#define ISO14443A_ATQA_SIZE									2

/* SAK FLAG --------------------------------------------------------------------------------- */
#define SAK_FLAG_ATS_SUPPORTED									0x20
#define SAK_FLAG_UID_NOT_COMPLETE								0x04

/* ATQ FLAG */
#define ATQ_FLAG_UID_SINGLE_SIZE		0
#define	ATQ_FLAG_UID_DOUBLE_SIZE		1
#define ATQ_FLAG_UID_TRIPLE_SIZE		2

/* MultiTag Huntig -------------------------------------------------------------------------- */
#define ISO14443A_NB_TAG_MAX			10

typedef enum {
	CMD_ISO14443A_Rats,
	CMD_ISO14443A_GetRandom,
	CMD_ISO14443A_SelectFile,
	CMD_ISO14443A_ExAuthenticate,
	CMD_ISO14443A_CreatADF,
	CMD_ISO14443A_DeleteDF,
	CMD_ISO14443A_CreatKeyFile,
	CMD_ISO14443A_AddKey,
	CMD_ISO14443A_CreatEFFile,
	CMD_ISO14443A_WriteEF,
	CMD_ISO14443A_ReadEF,
	CMD_ISO14443A_InAuthenticate,

	ISO14443A_CPU_CMD_MAX,
} ISO14443A_CPU_CMD_E;
typedef enum {
	ISO14443A_SUCCESS = 0,
	ISO14443A_DEFAULT_ERR,
	ISO14443A_PARAM_ERR,
	ISO14443A_SEND_CMD_ERROR,
	ISO14443A_REQUEST_ERR,
	ISO14443A_ANTICOL_ERR,
	ISO14443A_SELECT_ERR,
	ISO14443A_HALT_ERR,
	ISO14443A_AUTHENT_ERR,
	ISO14443A_READ_ERR,
	ISO14443A_WRITE_ERR,
	ISO14443A_RATS_ERR,
	ISO14443A_GetRandom_ERR,
	ISO14443A_SelectFile_ERR,
	ISO14443A_ExAuthenticate_ERR,
	ISO14443A_InAuthenticate_ERR,
	ISO14443A_DeleteDF_ERR,
	ISO14443A_CreatADF_ERR,
	ISO14443A_CreatKeyFile_ERR,
	ISO14443A_AddKey_ERR,
	ISO14443A_CreatEFFile_ERR,
	ISO14443A_WriteEF_ERR,
	ISO14443A_ReadEF_ERR,
	ISO14443A_FILE_ALREADY_EXIST,
	ISO14443A_RECEIVE_TIMEOUT,
} ISO14443A_ERR_CODE_E;
/* ---------------------------------------------------------------------------------
 * --- Local Functions  
 * --------------------------------------------------------------------------------- */
void ISO14443A_Reset		(void);
int8_t ISO14443A_Init 		(void);
int8_t ISO14443A_IsPresent		(void);
int8_t ISO14443A_Anticollision(void);
int8_t ISO14443A_SelectA(void);
int8_t ISO14443A_HLTA(void);
uint8_t scan_ISO14443A_card(uint8_t* uid);
void get_iso14443A_card_info(ISO14443A_CARD* card_info);
uint8_t is_support_rats(void);
void clear_card_info(void);

uint8_t ISO14443A_SelectFile ( uint8_t* file_id );
uint8_t ISO14443A_GetRandom ( uint8_t* rece_len, uint8_t* data );
uint8_t ISO14443A_InAuthenticate ( uint8_t* des_in, uint8_t key_cn, uint8_t* des_out );


#endif /* __ISO14443A_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
