/**
  ******************************************************************************
  * @file    lib_iso14443Apcd.c
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Manage the iso14443A communication
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
	******************************************************************************
  */ 
#include "lib_iso14443Apcd.h"
#include "lib_PCD.h"
#include "drv_interrupt.h"
#include <string.h>
#include "debug.h"
#include "lib_PCD.h"
#include "mfos.h"
#include "led_drv.h"
/* ISO14443A */
#define PCD_TYPEA_ARConfigA	0x01
#define PCD_TYPEA_ARConfigB	0xDF
#define PCD_TYPEA_TIMERW    0x5A

extern PCD_PROTOCOL TechnoSelected;
extern IC_VERSION IcVers;
extern uint8_t	*u95HFBuffer;// [RFTRANS_95HF_MAX_BUFFER_SIZE+3];
extern DeviceMode_t devicemode;
extern TagType_t nfc_tagtype;
/* --------------------------------------------------
 * code templates for ISO14443A protocol
 * command =  Command code | Length | data(Le)
 * -------------------------------------------------- */


uint16_t FSC = 32;
uint16_t FWI = 4;  /* Default value */
//uint8_t u95HFBufferAntiCol[RFTRANS_95HF_MAX_BUFFER_SIZE+3];
ISO14443A_CARD 	ISO14443A_Card;

/**
 * @brief  Reset the ISO14443A data structure
 * @param  void
 * @return void
 */
static void ISO14443A_InitStructure( void )
{
	/* Initializes the data structure used to store results */
	memset(ISO14443A_Card.ATQA, 0x00, ISO14443A_ATQA_SIZE);
	memset(ISO14443A_Card.UID , 0x00, ISO14443A_MAX_UID_SIZE);
	ISO14443A_Card.CascadeLevel 	= 0;
	ISO14443A_Card.UIDsize 			= 0;		
	ISO14443A_Card.ATSSupported 	= false;
	ISO14443A_Card.IsDetected   	= false;

}
/**根据ATQA判断UID的级别:1级->UID共4字节,2级->UID共7字节,3级->UID共10字节,
 * @brief  this function completes the ISO144443 type A structure according to the ATQA response
 */
static void ISO14443A_CompleteStructure(uint8_t *pATQA)
{
	/* according to FSP ISO 11443-3 the b7&b8 bits of ATQA tag answer is UID size bit frame */
	/* Recovering the UID size */
	switch ((ISO14443A_Card.ATQA[0] & ISO14443A_UID_MASK)>>6)
	{
			case ATQ_FLAG_UID_SINGLE_SIZE:
				ISO14443A_Card.UIDsize 			= ISO14443A_UID_SINGLE_SIZE;
				ISO14443A_Card.CascadeLevel 	= CASCADE_LVL_1;
			break;
			case ATQ_FLAG_UID_DOUBLE_SIZE:
				ISO14443A_Card.UIDsize 			= ISO14443A_UID_DOUBLE_SIZE;
				ISO14443A_Card.CascadeLevel 	= CASCADE_LVL_2;
			break;
			case ATQ_FLAG_UID_TRIPLE_SIZE:
				ISO14443A_Card.UIDsize 			= ISO14443A_UID_TRIPLE_SIZE;
				ISO14443A_Card.CascadeLevel 	= CASCADE_LVL_3;
			break;
	}
#if DEBUG_ISO14443A_CONFIG
	debug("[%d]ISO14443A_CARD UID Leavel Is %d\r\n",__LINE__,ISO14443A_Card.CascadeLevel);
#endif
}
/**
 * @brief  this functions convert FSCI to FSC
 * @param  FSCI	: FSCI value
 * @return FSC
 */
static uint16_t FSCIToFSC(uint8_t FSCI)
{
	if (FSCI == 0) return 16;
	else if (FSCI == 1) return 24;
	else if (FSCI == 2) return 32;
	else if (FSCI == 3) return 40;
	else if (FSCI == 4) return 48;
	else if (FSCI == 5) return 64;
	else if (FSCI == 6) return 96;
	else if (FSCI == 7) return 128;
	else return 256;
}


/**发送防冲突命令:uidLevel 20 08 接收:
 * @brief  This functions manage the anticollision
 * @param  *pDataRead	: Pointer to the PCD response
 * @param  CascadeLevel	: information on the current cascade level
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_AC( uint8_t *pDataRead, u8 CascadeLevel )
{
	u8 AnticolParameter [7] = {0x00, ISO14443A_NVM_20, 0x08,0x00,0x00,0x00,0x00};
	u8 NbResponseByte = 0;   //接收到的应答数据长度
	u8 NbResponseByteOrigin = 0; //应答数据起始字节
	u8 Collision = 0;            //是否冲突标志
	u8 ByteCollisionIndex = 0; //冲突的字节
	u8 BitCollisionIndex = 0;  //冲突的位

	u8 RemainingBit = 0;         //冲突后执行的位即冲突后,让冲突的位为0的卡继续执行
	u8 NewByteCollisionIndex = 0;//新的冲突的字节
	u8 NewBitCollisionIndex = 0; //新的冲突的位
	u8 UID[4] = {0x00,0x00,0x00,0x00};
	int8_t status;

	/* prepare command regarding cascade level on-going */
	AnticolParameter[0] = CascadeLevel;
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]send ISO14443A_AC cmd:\r\n",__LINE__);
    debug("%x %x %x\r\n",AnticolParameter[0],AnticolParameter[1],AnticolParameter[2]);
#endif
	/* sends the command to the PCD device*/
	errchk(PCD_SendRecv(0x03,AnticolParameter,pDataRead));
	
	NbResponseByte = pDataRead[1]; 
	NbResponseByteOrigin = NbResponseByte;
	Collision = (pDataRead[NbResponseByte-1] & 0x80);

	ByteCollisionIndex = pDataRead[NbResponseByte];
	BitCollisionIndex = pDataRead[NbResponseByte+1];
	
	/* case that should not happend, as occurs because we have miss another collision */
	if( BitCollisionIndex == 8)
	{
		return ISO14443A_ERRORCODE_DEFAULT;
	}
			
	/* check for collision (Tag of different UID length at the same time not managed so far) */
	while( Collision == 0x80)
	{		
		/* clear collision detection */
		Collision = 0x00;
		
		/* send the command to the PCD device*/
		AnticolParameter[1] = ISO14443A_NVM_20 + ((ByteCollisionIndex) <<4) + (BitCollisionIndex+1);
		if( ByteCollisionIndex == 0)
		{	
			AnticolParameter[2] = pDataRead[2] & ((u8)(~(0xFF<<(BitCollisionIndex+1)))); /* ISO said it's better to put collision bit to value 1 */
			AnticolParameter[3] = (BitCollisionIndex+1) | 0x40; /* add split frame bit */
			UID [0] = AnticolParameter[2];
		}
		else if( ByteCollisionIndex == 1)
		{
			AnticolParameter[2] = pDataRead[2];
			AnticolParameter[3] = pDataRead[3] & ((u8)(~(0xFF<<(BitCollisionIndex+1)))); /* ISO said it's better to put collision bit to value 1 */			
			AnticolParameter[4] = (BitCollisionIndex+1) | 0x40; /* add split frame bit */
			UID [0] = AnticolParameter[2];
			UID [1] = AnticolParameter[3];
		}
		else if( ByteCollisionIndex == 2)
		{
			AnticolParameter[2] = pDataRead[2];
			AnticolParameter[3] = pDataRead[3];
			AnticolParameter[4] = pDataRead[4] & ((u8)(~(0xFF<<(BitCollisionIndex+1)))); /* ISO said it's better to put collision bit to value 1 */			
			AnticolParameter[5] = (BitCollisionIndex+1) | 0x40; /* add split frame bit */;
			UID [0] = AnticolParameter[2];
			UID [1] = AnticolParameter[3];
			UID [2] = AnticolParameter[4];
		}
		else if( ByteCollisionIndex == 3)
		{
			AnticolParameter[2] = pDataRead[2];
			AnticolParameter[3] = pDataRead[3];
			AnticolParameter[4] = pDataRead[4];
			AnticolParameter[5] = pDataRead[5] & ((u8)(~(0xFF<<(BitCollisionIndex+1)))); /* ISO said it's better to put collision bit to value 1 */			
			AnticolParameter[6] = (BitCollisionIndex+1) | 0x40; /* add split frame bit */;
			UID [0] = AnticolParameter[2];
			UID [1] = AnticolParameter[3];
			UID [2] = AnticolParameter[4];
			UID [3] = AnticolParameter[5];
		}
		else
			return ISO14443A_ERRORCODE_DEFAULT;
				
		/* send part of the UID */
		PCD_SendRecv((0x03+ByteCollisionIndex+1),AnticolParameter,pDataRead);
		
		if(pDataRead[0] != 0x80)
				return ISO14443A_ERRORCODE_DEFAULT;
		
		/* check if there is another collision to take into account*/
		NbResponseByte = pDataRead[1]; 
		Collision = (pDataRead[NbResponseByte-1]) & 0x80;
		
		if ( Collision == 0x80)
		{
			NewByteCollisionIndex = pDataRead[NbResponseByte];
			NewBitCollisionIndex = pDataRead[NbResponseByte+1];
		}
					
		/* we can check that non-alignement is the one expected */
		RemainingBit = 8 - (0x0F & (pDataRead[2+(NbResponseByte-2)-1]));
		if( RemainingBit == BitCollisionIndex+1)
		{		
			/* recreate the good UID */
			if( ByteCollisionIndex == 0)
			{
				UID [0] = ((~(0xFF << (BitCollisionIndex+1))) & AnticolParameter[2]) | pDataRead[2] ;
				UID [1] = pDataRead[3];
				UID [2] = pDataRead[4];
				UID [3] = pDataRead[5];
			}
			else if( ByteCollisionIndex == 1)
			{
				UID [1] = ((~(0xFF << (BitCollisionIndex+1))) & AnticolParameter[3]) | pDataRead[2] ;
				UID [2] = pDataRead[3];
				UID [3] = pDataRead[4];
			}
			else if( ByteCollisionIndex == 2)
			{
				UID [2] = ((~(0xFF << (BitCollisionIndex+1))) & AnticolParameter[4]) | pDataRead[2] ;
				UID [3] = pDataRead[3];
			}
			else if( ByteCollisionIndex == 3)
			{
				UID [3] = ((~(0xFF << (BitCollisionIndex+1))) & AnticolParameter[5]) | pDataRead[2] ;
			}
			else
				return ISO14443A_ERRORCODE_DEFAULT;
		}				
		else
			return ISO14443A_ERRORCODE_DEFAULT;		
		
			/* prepare the buffer expected by the caller */
			pDataRead[0] = 0x80;
			pDataRead[1] = NbResponseByteOrigin;
			pDataRead[2] = UID [0];
			pDataRead[3] = UID [1];
			pDataRead[4] = UID [2];
			pDataRead[5] = UID [3];	
			pDataRead[6] = UID[0]^UID[1]^UID[2]^UID[3];	
		
		/* if collision was detected restart anticol */
		if ( Collision == 0x80)
		{
			if( ByteCollisionIndex != NewByteCollisionIndex )
			{
				ByteCollisionIndex += NewByteCollisionIndex;
				BitCollisionIndex = NewBitCollisionIndex;	
			}
			else
			{
				ByteCollisionIndex += NewByteCollisionIndex;
				BitCollisionIndex += (NewBitCollisionIndex+1);				
			}
		}
	}
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]recv ISO14443A_AC ask:!\r\n",__LINE__);
    debug("%x %x %x %x %x %x %x\r\n",pDataRead[0],pDataRead[1],pDataRead[2],pDataRead[3],pDataRead[4],pDataRead[5],pDataRead[6]);
    debug("[%d]ISO14443A_AC success!\r\n",__LINE__);
    
    debug("[%d]ISO14443A_CARD UID Is:\r\n",__LINE__);
    debug("%x %x %x %x:\r\n",pDataRead[2],pDataRead[3],pDataRead[4],pDataRead[5]);
#endif
	return ISO14443A_SUCCESSCODE;
Error:
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A_AC error!\r\n",__LINE__);
#endif
	return ISO14443A_ERRORCODE_DEFAULT; 
}

/**
 * @brief  this function carries out the first level of the anticollision
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_ACLevel1( uint8_t *pDataRead )
{
#if DEBUG_ISO14443A_CONFIG	
    uint8_t test_index=0x88;
#endif
	uint8_t *pDataToSend = &(u95HFBuffer[PCD_DATA_OFFSET]),
					Length = 0,
					BccByte ;
	int8_t status;

	/* Perform anti-collision */
	errchk(ISO14443A_AC(pDataRead, SEL_CASCADE_LVL_1));
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A_Card UIDsize is %d\r\n",__LINE__,ISO14443A_Card.UIDsize);
#endif
	/* Saves the UID in the structure */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_SINGLE_SIZE)
	{
		memcpy( ISO14443A_Card.UID, &pDataRead[PCD_DATA_OFFSET], ISO14443A_UID_SINGLE_SIZE+1);
	}
	else
	{	
		memcpy(ISO14443A_Card.UID, &pDataRead[PCD_DATA_OFFSET+1], ISO14443A_UID_PART);
	}
	
	/* copies the BCC byte of the card response*/
	BccByte = pDataRead[PCD_DATA_OFFSET + ISO14443A_UID_SINGLE_SIZE ];
	
	/* Preparing the buffer who contains the SELECT command */
	pDataToSend[Length ++]	= SEL_CASCADE_LVL_1;
	pDataToSend[Length ++] = ISO14443A_NVM_70;
	
	/* Inserts the previous reply in the next command */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_SINGLE_SIZE)
	{
		memcpy(&pDataToSend[Length], ISO14443A_Card.UID, ISO14443A_UID_SINGLE_SIZE );
		Length += ISO14443A_UID_SINGLE_SIZE ;
	}
	else
	{
		pDataToSend[Length ++] = 0x88;
		memcpy(&pDataToSend[Length], ISO14443A_Card.UID, ISO14443A_UID_PART );
		Length += ISO14443A_UID_PART ;
	}	
	pDataToSend[Length ++] = BccByte;
	/* Add the control byte : Append the CRC + 8 bits in first byte (standard frame)*/
	pDataToSend[Length ++] = PCD_ISO14443A_APPENDCRC | PCD_ISO14443A_A8BITSINFIRSTBYTE;
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]send ISO14443A_ACLevel1  cmd:\r\n",__LINE__);
    for(test_index=0;test_index<Length;test_index++)
    {
        debug("%x ",pDataToSend[test_index]);
    }
    debug("\r\n");
#endif
	errchk(PCD_SendRecv(Length,pDataToSend,pDataRead));
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]recv ISO14443A_ACLevel1  sak:\r\n",__LINE__);
    for(test_index=0;test_index<pDataRead[1]+2;test_index++)
    {
        debug("%x ",pDataRead[test_index]);
    }
    debug("\r\n");
#endif
	/* Recovering SAK byte */
	ISO14443A_Card.SAK = pDataRead[PCD_DATA_OFFSET];
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A_AnticollisionLevel1 SAK Is:%x\r\n",__LINE__,ISO14443A_Card.SAK);
    debug("[%d]ISO14443A_AnticollisionLevel1 success!\r\n",__LINE__);  
#endif
	return ISO14443A_SUCCESSCODE;
Error:
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A_AnticollisionLevel1 error!\r\n",__LINE__);  
#endif
	return ISO14443A_ERRORCODE_DEFAULT; 
}

/**
 * @brief  this function carries out the second level of the anticollision
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_ACLevel2( uint8_t *pDataRead )
{
	uint8_t *pDataToSend = &(u95HFBuffer[PCD_DATA_OFFSET]),
					Length = 0,
					BccByte ;
	int8_t status;

	/* Perform anti-collision */
	errchk(ISO14443A_AC(pDataRead, SEL_CASCADE_LVL_2));
	//errchk(PCD_SendRecv(0x03,AnnticolParameter,pDataRead));
	
	/* Copies the UID into the data structure */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_DOUBLE_SIZE)
	{
		memcpy(&ISO14443A_Card.UID[ISO14443A_UID_PART], &pDataRead[PCD_DATA_OFFSET], ISO14443A_UID_SINGLE_SIZE);
	}
	else
	{
		memcpy(&ISO14443A_Card.UID[ISO14443A_UID_PART], &pDataRead[PCD_DATA_OFFSET+1], ISO14443A_UID_PART);
	}
	
	/* copies the BCC byte of the card response*/
	BccByte = pDataRead[PCD_DATA_OFFSET + ISO14443A_UID_SINGLE_SIZE ];
	
	/* Preparing the buffer who contains the SELECT command */
	pDataToSend[Length ++]	= SEL_CASCADE_LVL_2;
	pDataToSend[Length ++] = ISO14443A_NVM_70;
	
	/* Copies the UID into the data structure */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_DOUBLE_SIZE)
	{
		memcpy(&(pDataToSend[Length]),&(ISO14443A_Card.UID[ISO14443A_UID_PART]) , ISO14443A_UID_SINGLE_SIZE);
		Length += ISO14443A_UID_SINGLE_SIZE ;
	}
	else
	{
		pDataToSend[Length ++] = 0x88;
		memcpy(&(pDataToSend[Length]),&(ISO14443A_Card.UID[ISO14443A_UID_PART]) , ISO14443A_UID_PART);
		Length += ISO14443A_UID_PART ;
	}
	pDataToSend[Length ++] = BccByte;
	/* Add the control byte : Append the CRC + 8 bits in first byte (standard frame)*/
	pDataToSend[Length ++] = PCD_ISO14443A_APPENDCRC | PCD_ISO14443A_A8BITSINFIRSTBYTE;
	
	/* emit the select command */
	errchk(PCD_SendRecv(Length,pDataToSend,pDataRead));
	
	/* Recovering SAK byte */
	ISO14443A_Card.SAK = pDataRead[PCD_DATA_OFFSET];

	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}


/**
 * @brief  this function carries out the second level of the anticollision
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_ACLevel3 ( uint8_t *pDataRead )
{
	uint8_t *pDataToSend = &(u95HFBuffer[PCD_DATA_OFFSET]),
					Length = 0,
					BccByte ;
	int8_t status;

	/* Perform anti-collision */
	errchk(ISO14443A_AC(pDataRead, SEL_CASCADE_LVL_3));

	/* Copies the UID into the data structure */
	memcpy(&ISO14443A_Card.UID[ISO14443A_UID_PART], &pDataRead[PCD_DATA_OFFSET], ISO14443A_UID_SINGLE_SIZE);
	/* copies the BCC byte of the card response*/
	BccByte = pDataRead[PCD_DATA_OFFSET + ISO14443A_UID_SINGLE_SIZE ];
	
	/* Preparing the buffer who contains the SELECT command */
	pDataToSend[Length ++]	= SEL_CASCADE_LVL_3;
	pDataToSend[Length ++] = ISO14443A_NVM_70;
	
	/* Copies the UID into the data structure */
	if(ISO14443A_Card.UIDsize == ISO14443A_UID_TRIPLE_SIZE)
	{
		memcpy(&(pDataToSend[Length]),&(ISO14443A_Card.UID[ISO14443A_UID_PART]) , ISO14443A_UID_SINGLE_SIZE);
		Length += ISO14443A_UID_SINGLE_SIZE ;
	}

	pDataToSend[Length ++] = BccByte;
	/* Add the control byte : Append the CRC + 8 bits in first byte (standard frame)*/
	pDataToSend[Length ++] = PCD_ISO14443A_APPENDCRC | PCD_ISO14443A_A8BITSINFIRSTBYTE;
	
	/* emitthe select command */
	errchk(PCD_SendRecv(Length,pDataToSend,pDataRead));
	
	/* Recovering SAK byte */
	ISO14443A_Card.SAK = pDataRead[PCD_DATA_OFFSET];

	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}




/**
 * @brief  this functions emits the RATS command to PICC device
 * @param  *pDataRead	: Pointer to the response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_RATS( uint8_t *pDataRead )
{
	int8_t status;	
	uint8_t FSCI;
	uc8 	 pdata[]= { 0xE0, 0x80, 0x28};

	/* send the command to the PCD device*/
	errchk(PCD_SendRecv(0x03,pdata,pDataRead));
	/* check the status byte of the PCD device */
	errchk(PCD_IsReaderResultCodeOk (SEND_RECEIVE,pDataRead) );
	/* check the CRC */
	errchk(PCD_IsCRCOk (PCD_PROTOCOL_ISO14443A,pDataRead) );

	FSCI = pDataRead[3]&0x0F;
	FSC = FSCIToFSC(FSCI);
	
	/* Check if FWI is present */
	if( (pDataRead[3] & 0x20) == 0x20)
	{
		if( (pDataRead[3] & 0x10) == 0x10)
			FWI = (pDataRead[5]&0xF0)>>4;
		else
			FWI = (pDataRead[4]&0xF0)>>4;	
	}
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A RATS Supported!\r\n",__LINE__);
#endif
	return ISO14443A_SUCCESSCODE;
Error:
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A_RATS error!\r\n",__LINE__);
#endif
	return ISO14443A_ERRORCODE_DEFAULT; 
}


/**
  * @}
  */ 


/** @addtogroup lib_iso14443Apcd_Public_Functions
 *  @{
 */

int8_t ISO14443A_ConfigFDTforAnticollision( void)
{
	u8 ProtocolSelectParameters [6]  = {0x00, 0x00, 0x00, 0x00, 0x02, 0x02}; /* last 2 bytes since QJE version */
	u8 WriteRegisterParameters [2]  = {PCD_TYPEA_TIMERW, TIMER_WINDOW_UPDATE_CONFIRM_CMD};
  u8 DemoGainParameters [2]  = {PCD_TYPEA_ARConfigA, PCD_TYPEA_ARConfigB};
	u8 NbParam = 0;
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;	
	
	if( IcVers >= QJE)
		NbParam = 6;
	else
		NbParam = 4;
		
	//send:2 7 2 0 0 0 0 2 2 
    //recv:0 0
	errchk(PCD_ProtocolSelect((NbParam+1),PCD_PROTOCOL_ISO14443A,ProtocolSelectParameters,pDataRead));

	//send:9 4 3a 0 5a 4  
    //recv:0 0
	errchk(PCD_WriteRegister    (0x04,TIMER_WINDOW_REG_ADD,0x00,WriteRegisterParameters,pDataRead));
	
	/* in order to adjust the demoduation gain of the PCD which is reseted at each protocol select */
	//send:9 4 68 1 1 df 
    //recv:0 0
	errchk(PCD_WriteRegister (0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,DemoGainParameters,u95HFBuffer));
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}


int8_t ISO14443A_ConfigFDTforRATS( void)
{
	u8 ProtocolSelectParameters [6]  = {0x00, 0x00, 0x00, 0x00, 0x03, 0x03}; /* last 2 bytes since QJE version */
	u8 WriteRegisterParameters [2]  = {PCD_TYPEA_TIMERW, TIMER_WINDOW_UPDATE_CONFIRM_CMD};
  u8 DemoGainParameters [2]  = {PCD_TYPEA_ARConfigA, PCD_TYPEA_ARConfigB};
	u8 NbParam = 0;
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;	
	
	if( IcVers >= QJE)
		NbParam = 6;
	else
		NbParam = 4;
	
	/* Change the PP:MM parameter to respect RATS timing TS-DP-1.1 13.8.1.1*/
	/* min to respect */
	/* FDT PCD = FWTt4at,activation = 71680 (1/fc) */
	/* (2^PP)*(MM+1)*(DD+128)*32 = 71680 ==> PP = 4 MM=0 DD=12*/
	/* max to respect not mandatory and as Tag has a FWT activation of 5.2us  */
	/* adding 16.4ms does not make sense ... */
	/* FDT PCD = FWTt4at,activation + dela(t4at,poll) = 5286us + 16.4ms ~= 21.7ms */
	/* (2^PP)*(MM+1)*(DD+128)*32 = 21,7 ==> PP = 4 MM=0 DD=12*/
	ProtocolSelectParameters[1] = 4; //  PP
	ProtocolSelectParameters[2] = 0; //  MM
	ProtocolSelectParameters[3] = 12; // DD
	errchk(PCD_ProtocolSelect((NbParam+1),PCD_PROTOCOL_ISO14443A,ProtocolSelectParameters,pDataRead));
	errchk(PCD_WriteRegister    ( 0x04,TIMER_WINDOW_REG_ADD,0x00,WriteRegisterParameters,pDataRead));
	/* in order to adjust the demoduation gain of the PCD which is reseted at each protocol select */
	errchk(PCD_WriteRegister (0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,DemoGainParameters,u95HFBuffer));
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}

int8_t ISO14443A_ConfigFDT( uint8_t WTXM)
{
	u8 ProtocolSelectParameters [6]  = {0x00, 0x00, 0x00, 0x00, 0x03, 0x03}; /* last 2 bytes since QJE version */
	u8 WriteRegisterParameters [2]  = {PCD_TYPEA_TIMERW, TIMER_WINDOW_UPDATE_CONFIRM_CMD};
  u8 DemoGainParameters [2]  = {PCD_TYPEA_ARConfigA, PCD_TYPEA_ARConfigB};
	u8 NbParam = 0;
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;	
	
	if( IcVers >= QJE)
		NbParam = 6;
	else
		NbParam = 4;
	
	/* FWI was updated thanks to ATS, if not the case use default value FWI = 4 TS-DP-1.1 13.6.2.11 */
	/* FDT PCD = FWT PICC + deltaFWT(t4at) + "deltaT(t4at,poll)" TS-DP-1.1 13.8*/
	/* If we perform some identification:
			FDT = (2^PP)*(MM+1)*(DD+128)*32/13.56 
			FDT = (2^(PP))*(1)*(2*DD+256)*16/13.56 + (2^(PP))*(MM)*(2*DD+256)*16/13.56
			FDT = (256*16/fc)*2^FWI + ((2^FWI) *256*16*1/fc)*MM with PP=FWI and DD=0
			FDT = (2^FWI)*4096*1/fc + FWT*MM (EQUATION 1)

			I_ With the choice to NOT add deltaT(t4at,poll) = 16,4ms
			1)	In the standard case (No extension time cmd received) we want
				FDT = FWT + delta FWT(T4AT) 
				FDT = FWT + 49152 (1/fc)

				If we take the rules that we will never set FWI to a value less than 4.
				(EQUATION 1 comes)
				FDT = FWT*MM + 65536*1/fc => delta FWT(T4AT) is respected

				As a conclusion with
				PP=FWI (with FWI>=4)
				MM=1
				DD=0
			we are following the specification. 
			
			2) In the case of extension time request, M will take the WTXM value.			*/
				
	if(FWI < 4 )
		FWI = 4; 
			
	ProtocolSelectParameters[1] = FWI; // PP
	ProtocolSelectParameters[2] = WTXM; //  MM
	ProtocolSelectParameters[3] = 0; // DD
		
	errchk(PCD_ProtocolSelect((NbParam+1),PCD_PROTOCOL_ISO14443A,ProtocolSelectParameters,pDataRead));
	errchk(PCD_WriteRegister    ( 0x04,TIMER_WINDOW_REG_ADD,0x00,WriteRegisterParameters,pDataRead));
  /* in order to adjust the demoduation gain of the PCD which is reseted at each protocol select */
	errchk(PCD_WriteRegister (0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,DemoGainParameters,u95HFBuffer));
	
	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT;
}


/**
 * @brief  this command initializes the PCD device for the IS014443A protocol
 * @param  none
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
int8_t ISO14443A_Init (void)
{
	int8_t 	status;

	FWI = 0x04;
	ISO14443A_InitStructure( );

	/* sends a protocol Select command to the pcd to configure it */
	errchk(ISO14443A_ConfigFDTforAnticollision());
	
	TechnoSelected = PCDPROTOCOL_14443A;
#if DEBUG_ISO14443A_CONFIG
	debug("[%d]ISO14443A_Init success!\r\n",__LINE__);
#endif
    /* GT min time to respect before sending REQ_A */
	delay_ms(5);
 
 return ISO14443A_SUCCESSCODE;
Error:
#if DEBUG_ISO14443A_CONFIG
	debug("[%d]ISO14443A_Init error!\r\n",__LINE__);
#endif
 return ISO14443A_ERRORCODE_DEFAULT;
	
}


/**发送请求,获得ATQA,即获得UID的级别
 * @brief  this functions sends the REQA command to the PCD device
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
static int8_t ISO14443A_REQA( uint8_t *pDataRead )
{
#if DEBUG_ISO14443A_CONFIG
    uint8_t test_index=0x88;
#endif
//	uc8 ReqA[] = { 0x26, 0x07};
	uc8 ReqA[] = { 0x52, 0x07};
	int8_t	status;

	/* sends the command to the PCD device*/
#if DEBUG_ISO14443A_CONFIG
	debug("[%d]send ISO14443A_REQA cmd:\r\n",__LINE__);
	for(test_index=0;test_index<2;test_index++)
	{
	    debug("%x ",ReqA[test_index]);
	}
	debug("\r\n");
#endif
	errchk(PCD_SendRecv(0x02,ReqA,pDataRead));
#if DEBUG_ISO14443A_CONFIG
	debug("[%d]recv ISO14443A_REQA ask:\r\n",__LINE__);
	for(test_index=0;test_index<pDataRead[1]+2;test_index++)
	{
	    debug("%x ",pDataRead[test_index]);
	}
	debug("\r\n");
#endif
	/* retrieves the ATQA response */
	memcpy(ISO14443A_Card.ATQA, &pDataRead[PCD_DATA_OFFSET], ISO14443A_ATQA_SIZE);
	/* completes the strucure according to the ATQA response */
	ISO14443A_CompleteStructure (ISO14443A_Card.ATQA );
#if DEBUG_ISO14443A_CONFIG
	debug("[%d]ISO14443A_REQA success!\r\n\r\n",__LINE__);
#endif
	return ISO14443A_SUCCESSCODE;
Error:
#if DEBUG_ISO14443A_CONFIG
//	debug("[%d]ISO14443A_REQA error!\r\n",__LINE__);
#endif
	return ISO14443A_ERRORCODE_DEFAULT; 
}


/**
 * @brief  Checks if a card is in the field
 * @param  None
 * @return ISO14443A_SUCCESSCODE (Anticollision done) / ISO14443A_ERRORCODE_DEFAULT (Communication issue)
 */
int8_t ISO14443A_Anticollision(void)
{
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;	
	
	/* Checks if an error occured and execute the Anti-collision level 1*/
	errchk(ISO14443A_ACLevel1(pDataRead) );
	/* UID Complete ? */
	if(ISO14443A_Card.SAK & SAK_FLAG_UID_NOT_COMPLETE)
	{
		/* Checks if an error occured and execute the Anti-collision level 2*/
		errchk(ISO14443A_ACLevel2(pDataRead) );
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A_AnticollisionLevel2 success!\r\n",__LINE__);
#endif
	}
	/* UID Complete ? */
	if(ISO14443A_Card.SAK & SAK_FLAG_UID_NOT_COMPLETE)
	{
		/* Checks if an error occured and execute the Anti-collision level 3*/
		errchk(ISO14443A_ACLevel3(pDataRead) );
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A_AnticollisionLevel3 success!\r\n",__LINE__);
#endif
	}

	/* Checks if the RATS command is supported by the card */
	if(ISO14443A_Card.SAK & SAK_FLAG_ATS_SUPPORTED)
	{
		ISO14443A_ConfigFDTforRATS();
		
		ISO14443A_Card.ATSSupported = true;
		errchk(ISO14443A_RATS(pDataRead));
	}
	/* Change the FDT to accept APDU */	
	ISO14443A_ConfigFDT(1);
	
	devicemode = PCD;
	/* Check the Tag type found */
	if ((ISO14443A_Card.SAK&0x60) == 0x00) /* TT2 */
		nfc_tagtype = TT2;
	else if( (ISO14443A_Card.SAK & 0x20) == 0x20) 
		nfc_tagtype = TT4A;	
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A_Anticollision success!\r\n",__LINE__);
#endif
	return ISO14443A_SUCCESSCODE;
Error:
#if DEBUG_ISO14443A_CONFIG	
    debug("[%d]ISO14443A_Anticollision error!\r\n",__LINE__);
#endif
	return ISO14443A_ERRORCODE_DEFAULT; 

}


/******************************************************************************
* Name: 	 ISO14443A_SelectA 
*
* Function Desc: 	 选中卡
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/05/17, Create this function by liuwq
******************************************************************************/
//int8_t ISO14443A_SelectA(void)
//{
//    uint8_t index = 0;
//    uint8_t *pDataRead = u95HFBuffer;
//	uint8_t pdata[8]= {0x93,0x70,0,0,0,0,0,0x28};

//    for(index = 0;index <(ISO14443A_Card.UIDsize+1);index++)
//    {
//        pdata[index+2] = ISO14443A_Card.UID[index];
//    }
//#if DEBUG_ISO14443A_CONFIG
//	debug("[%d]send ISO14443A_SelectA cmd:\r\n",__LINE__);
//	for(index=0;index<8;index++)
//	{
//	    debug("%x ",pdata[index]);
//	}
//	debug("\r\n");
//#endif
//	/* send the command to the PCD device*/
//	PCD_SendRecv(0x08,pdata,pDataRead);
//#if DEBUG_ISO14443A_CONFIG
//	debug("[%d]recv ISO14443A_SelectA ask:\r\n",__LINE__);
//	for(index=0;index<pDataRead[1]+2;index++)
//	{
//	    debug("%x ",pDataRead[index]);
//	}
//	debug("\r\n");
//#endif
//	
//	return ISO14443A_SUCCESSCODE;
//}

/******************************************************************************
* Name: 	 ISO14443A_Authentication 
*
* Function Desc: 	 扇区认证
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/05/17, Create this function by liuwq
******************************************************************************/
int8_t ISO14443A_Authentication(void)
{
    uint8_t *pDataRead = u95HFBuffer;
	uint8_t pdata[3]= {0x60,0x18,0x28};//keyA,block8
    PCD_SendRecv(3,pdata,pDataRead);
	return 0;
}

/**
 * @brief  this functions sends the HLTA command to the PCD device
 * @param  *pDataRead	: Pointer to the PCD response
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
int8_t ISO14443A_HLTA(void)
{
#if 0
#if DEBUG_ISO14443A_CONFIG
    uint8_t index = 0;
#endif
    uint8_t *pDataRead = u95HFBuffer;
	uc8 pdata[]= {0x50,0x00,0x28};
#if DEBUG_ISO14443A_CONFIG
	debug("[%d]send ISO14443A_HLTA cmd:\r\n",__LINE__);
	for(index=0;index<3;index++)
	{
	    debug("%x ",pdata[index]);
	}
	debug("\r\n");
#endif
	/* send the command to the PCD device*/
	PCD_SendRecv(0x03,pdata,pDataRead);
#if DEBUG_ISO14443A_CONFIG
	debug("[%d]recv ISO14443A_HLTA ask:\r\n",__LINE__);
	for(index=0;index<pDataRead[1]+2;index++)
	{
	    debug("%x ",pDataRead[index]);
	}
	debug("\r\n");
#endif
#endif
	return ISO14443A_SUCCESSCODE;
}


/**
 * @brief  Checks if a ISO14443A card is in the field
 * @return ISO14443A_SUCCESSCODE the function is succesful
 * @return ISO14443A_ERRORCODE_DEFAULT : an error occured
 */
int8_t ISO14443A_IsPresent( void )
{
	uint8_t *pDataRead = u95HFBuffer;
	int8_t 	status;
	
	errchk(ISO14443A_REQA(pDataRead));
	
	/* checks the status byte of the PCD device */
	errchk(PCD_IsReaderResultCodeOk (SEND_RECEIVE,pDataRead) );

	ISO14443A_Card.IsDetected = true;

	return ISO14443A_SUCCESSCODE;
Error:
	return ISO14443A_ERRORCODE_DEFAULT; 
}
//static uint8_t state_test = 0;
/******************************************************************************
* Name: 	 scan_ISO14443A_card 
*
* Function Desc: 	 
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/05/17, Create this function by liuwq
******************************************************************************/
uint8_t scan_ISO14443A_card(uint8_t* uid)
{

    uint8_t result = ISO14443A_ERRORCOD_NOTAG;
    if(uid == NULL) return ISO14443A_ERRORCOD_NOTAG;
		memset(&ISO14443A_Card,0,sizeof(ISO14443A_CARD));
    PCD_FieldOff();
    result = ISO14443A_Init();
    if(result!=ISO14443A_SUCCESSCODE) return ISO14443A_ERRORCOD_INIT;
    
    result = ISO14443A_IsPresent();
    if(result!=ISO14443A_SUCCESSCODE) return ISO14443A_ERRORCOD_REQA;

    result = ISO14443A_Anticollision();
    if(result!=ISO14443A_SUCCESSCODE) return ISO14443A_ERRORCOD_ANTICOLL;
    memcpy(uid,ISO14443A_Card.UID,ISO14443A_Card.UIDsize);
    return ISO14443A_SUCCESSCODE;
}



void card_write_test(void)
{
		uint8_t result;
		uint8_t card_uid[10] = {0};
		uint8_t keyB[6]={0x9E,0x13,0x66,0x95,0xf0,0x0A};
		uint8_t write_buf[20]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16};
		
		scan_ISO14443A_card(card_uid);
    mf_enhanced_auth(5,ISO14443A_Card.UID,keyB,0x61);

    result = PCDNFCT2_Write(5,write_buf);
		if(result == 0)
		{
			debug("write block success!\r\n");
		}
		else
		{
			debug("write block error!\r\n");
		}
}
void card_read_test(void)
{
	uint8_t card_uid[10] = {0};
	uint8_t read_buf[20] = {0};
	uint8_t keyB[6]={0x9E,0x13,0x66,0x95,0xf0,0x0A};
	
	scan_ISO14443A_card(card_uid);
  mf_enhanced_auth(5,ISO14443A_Card.UID,keyB,0x61);
	PCDNFCT2_Read(5,read_buf);
	debug("read data:\r\n");
	debug_print(read_buf,16);
}
/******************************************************************************
* Name: 	 get_iso14443A_card_info 
*
* Function Desc: 	 
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/05/12, Create this function by liuwq
******************************************************************************/
void get_iso14443A_card_info(ISO14443A_CARD* card_info)
{
    memcpy(card_info,&ISO14443A_Card,sizeof(ISO14443A_CARD));
}
void clear_card_info(void)
{
		memset((uint8_t*)&ISO14443A_Card,0,sizeof(ISO14443A_CARD));
}
uint8_t is_support_rats(void)
{
    return ISO14443A_Card.ATSSupported;
}
/////////////////////////////////////////////////////////////////////////////////
/* CPU卡操作的指令结构*/
typedef struct   CPU_CMD_S {
    uint8_t CLA;
    uint8_t INS;
    uint8_t P1;
    uint8_t P2;
    uint8_t LC;
    uint8_t payload;
} CPU_CMD_T;

uint8_t CPU_PCB(void)
{
	static uint8_t pcb = 0;
	
	if(0x03 == pcb)
	{
		pcb = 0x02;
	}
	else if(0x02 == pcb)
	{
		pcb = 0x03;
	}
	else
	{
		pcb = 0x02;
	}
	return pcb;
}

static uint8_t iso14443A_cpu_ask_check ( uint8_t cmd, uint8_t* ask ,uint16_t ask_len)
{
    uint8_t result = ISO14443A_DEFAULT_ERR;
    if ( ask == NULL)
    {
        return ISO14443A_PARAM_ERR;
    }
    switch ( cmd )
    {
    case CMD_ISO14443A_Rats:
        break;

    case CMD_ISO14443A_SelectFile:
	case CMD_ISO14443A_GetRandom:
	case CMD_ISO14443A_InAuthenticate:
	case CMD_ISO14443A_ExAuthenticate:
	case CMD_ISO14443A_ReadEF:
	case CMD_ISO14443A_WriteEF:
	case CMD_ISO14443A_DeleteDF:
		if ( (ask[ask_len-7] == 0x90 )&&( ask[ask_len-6] == 0x00 ))
		{
			result = ISO14443A_SUCCESS;
		}
		
	break;
		
		
		
		case CMD_ISO14443A_CreatADF:
		case CMD_ISO14443A_CreatKeyFile:
		case CMD_ISO14443A_AddKey:
		case CMD_ISO14443A_CreatEFFile:
		
		
			
        break;


    default:
        result = ISO14443A_DEFAULT_ERR;
        break;
    }
    return result;
}

/******************************************************************************
* Name: 	 ISO14443A_SelectFile
*
* Desc: 	 选中文件
* Param(in):
* Param(out):
* Return:
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/08, Create this function by liuwq
*******************************************************************************/
uint8_t ISO14443A_SelectFile ( uint8_t* file_id )
{
    uint8_t	result,*file;
	uint8_t send_cnd[15];
	uint16_t recv_len=0;
    CPU_CMD_T *select_file;

    if ( file_id == NULL )
    {
        return ISO14443A_ERRORCODE_DEFAULT;
    }
	send_cnd[0]= CPU_PCB();
	select_file = (CPU_CMD_T *)&send_cnd[1];
    select_file->CLA = 0x00;
    select_file->INS = 0xa4;
    select_file->P1 = 0x00;
    select_file->P2 = 0x00;
    select_file->LC = 0x02;
	file = &select_file->payload;
    file[0] = *file_id;
    file[1] = * ( file_id + 1 );
	file[2] = 0x28;//st95hf特性，表示硬件CRC
	#if DEBUG_CPU
	debug ( "send select file cmd:\r\n" );
	debug_hex (send_cnd,9 );
	#endif
	result = PCD_SendRecv(9,send_cnd,u95HFBuffer);
	recv_len = u95HFBuffer[1]+2;
	if(result == PCD_SUCCESSCODE)
	{
		if ( iso14443A_cpu_ask_check(CMD_ISO14443A_SelectFile,u95HFBuffer,recv_len) == ISO14443A_SUCCESS )
        {
			#if DEBUG_CPU
			debug ( "get SelectFile(id:%x %x) ask (len = %d):\r\n", *file_id, * ( file_id + 1 ), recv_len-7 );
			debug_hex ( &u95HFBuffer[2], recv_len-7 );
			debug("SelectFile success!\r\n");
			#endif
			return ISO14443A_SUCCESS;
        }
		else
		{
			#if DEBUG_CPU
			debug ( "ISO14443A_SelectFile ask error,error code:!\r\n" );
			debug_hex(&u95HFBuffer[3],2);
		  #endif
			return ISO14443A_SelectFile_ERR;
		}
	}
	else
	{
		#if DEBUG_CPU
		debug ( "send SelectFile cmd error!\r\n" );
		#endif
	}
	return PCD_ERRORCODE_DEFAULT;
}
/******************************************************************************
* Name: 	 ISO14443A_GetRandom
*
* Desc: 	 获取随机数
* Param(in):
* Param(out):
* Return: 	 0->success
* Global:
* Note:
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/07, Create this function by liuwq
*******************************************************************************/
uint8_t ISO14443A_GetRandom ( uint8_t* rece_len, uint8_t* data )
{
    uint8_t	result,send_cnd[15];
    CPU_CMD_T* get_random;

    if ( rece_len == NULL || data == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
		
	send_cnd[0] = CPU_PCB();
	get_random = ( CPU_CMD_T* ) &send_cnd[1];
    get_random->CLA = 0x00;
    get_random->INS = 0x84;
    get_random->P1 = 0x00;
    get_random->P2 = 0x00;
    get_random->LC = 0x08;
	get_random->payload = 0X28;//st95hf特性，表示硬件CRC
#if DEBUG_CPU
		debug ( "send GetRandom cmd:\r\n" );
		debug_hex(send_cnd,7);
#endif
    result = PCD_SendRecv (7, send_cnd,u95HFBuffer);
	*rece_len = u95HFBuffer[1]+2;
	if(*rece_len < 10) return ISO14443A_GetRandom_ERR;
	if(result == ISO14443A_SUCCESS)
	{
		if ( iso14443A_cpu_ask_check(CMD_ISO14443A_GetRandom,u95HFBuffer,*rece_len) == ISO14443A_SUCCESS )
		{
			#if DEBUG_CPU
			debug ( "GetRandom ask (len = %d):\r\n", *rece_len );
			debug_hex ( u95HFBuffer, *rece_len );
			debug ( "GetRandom success!\r\n" );
			#endif
			memcpy ( data, &u95HFBuffer[3], *rece_len - 7 );
			*rece_len=8;
			return ISO14443A_SUCCESS;
		}
		else
		{
			#if DEBUG_CPU
			debug ( "ISO14443A_Rats ask error,error code:!\r\n" );
			debug_hex(u95HFBuffer,*rece_len);
			#endif
		}
	}
	else
	{
		#if DEBUG_CPU
		debug ( "send GetRandom cmd error!\r\n" );
		#endif
	}
	*rece_len=0;
	return ISO14443A_SEND_CMD_ERROR;
}

/******************************************************************************
* Name: 	 ISO14443A_InAuthenticate
*
* Desc: 	 CPU卡内部认证
* Param(in): 	 des_in:认证的数据 des_out:认证的结果
* Param(out):
* Return:
* Global:
* Note: 	内部认证是为了对卡的合法性进行判断(判断是否为我们发的卡)
			要进行内部认证，必须要保证CPU卡内部有密钥文件且有密钥
			CPU卡根据传入的数组des_in，采用事先创建的密钥进行加密运算，将
			运算后的结果与传入的des_out对比，若一致，则认证成功
* Author: 	 liuwq
*------------------------------------------------------------------------------
* Log: 	 2017/11/08, Create this function by liuwq
*******************************************************************************/
uint8_t ISO14443A_InAuthenticate ( uint8_t* des_in, uint8_t key_cn, uint8_t* des_out )
{
    uint8_t	result, r_len = 0,send_cmd[20];
    CPU_CMD_T* in_authen;

    if ( des_out == NULL || des_in == NULL )
    {
        return ISO14443A_PARAM_ERR;
    }
	send_cmd[0] = CPU_PCB();
    in_authen = ( CPU_CMD_T* ) &send_cmd[1];
    in_authen->CLA = 0x00;
    in_authen->INS = 0x88;
    in_authen->P1  = 0x00;
    in_authen->P2  = key_cn;
    in_authen->LC  = 0x08;
    memcpy ( &in_authen->payload, des_in, in_authen->LC );
	send_cmd[14]=0x28;
	#if DEBUG_CPU
    debug ( "des_in:" );
    debug_hex ( des_in, 8 );
    debug ( "hope result:" );
    debug_hex ( des_out, 8 );
	debug("send InAuthenticate cmd:\r\n");
	debug_hex(send_cmd,15);
	#endif
    result = PCD_SendRecv ( 15, send_cmd,u95HFBuffer );
	r_len = u95HFBuffer[1]+2;
	if(r_len < 10) return ISO14443A_InAuthenticate_ERR;
	if ( result == ISO14443A_SUCCESS )
    {			
        if ( iso14443A_cpu_ask_check ( CMD_ISO14443A_InAuthenticate, u95HFBuffer, r_len ) == ISO14443A_SUCCESS )
        {
			#if DEBUG_CPU
			debug ( "get InAuthenticate ask(len = %d):\r\n",r_len);
			debug_hex ( u95HFBuffer,r_len);
			#endif
			if ( memcmp ( des_out, &u95HFBuffer[3], r_len-10 ) == 0 ) //内部密钥正确
			{
				#if DEBUG_CPU
				debug ( "InAuthenticate success!\r\n" );
				#endif
				return ISO14443A_SUCCESS;
			}
			else
			{
				#if DEBUG_CPU
				debug ( "InAuthenticate error,key unsame!\r\n" );
				#endif
				return ISO14443A_InAuthenticate_ERR;
			}
					
        }
		else
		{
			#if DEBUG_CPU
			debug ( "ISO14443A_InAuthenticate ask error,error code:!\r\n" );
			debug_hex(&u95HFBuffer[3],2);
		    #endif
			return ISO14443A_InAuthenticate_ERR;
		}
    }
	else
	{
		#if DEBUG_CPU
		debug ( "send ISO14443A_InAuthenticate cmd error!\r\n" );
		#endif
	}
    return ISO14443A_SEND_CMD_ERROR;
}
/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/

