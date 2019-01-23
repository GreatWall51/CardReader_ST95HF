/**
  ******************************************************************************
  * @file    lib_nfctype5pcd.c
  * @author  MMY Application Team
  * @version V4.0.0
  * @date    02/06/2014
  * @brief   Generates the NFC type2 commands
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
#include "lib_nfctype5pcd.h"
#include "drv_interrupt.h"
#include "debug.h"
#include "lib_iso15693pcd.h"
//uint8_t TT5Tag[512] = {0};
extern uint8_t *pTT5Tag;
extern uint8_t 	NDEF_Buffer[];
/** @addtogroup _95HF_Libraries
 * 	@{
 *	@brief  <b>This is the library used by the whole 95HF family (RX95HF, CR95HF, ST95HF) <br />
 *				  You will find ISO libraries ( 14443A, 14443B, 15693, ...) for PICC and PCD <br />
 *				  The libraries selected in the project will depend of the application targetted <br />
 *				  and the product chosen (RX95HF emulate PICC, CR95HF emulate PCD, ST95HF can do both)</b>
 */

/** @addtogroup PCD
 * 	@{
 *	@brief  This part of the library enables PCD capabilities of CR95HF & ST95HF.
 */


/** @addtogroup NFC_type5_pcd
 * 	@{
 *	@brief  This file is used to exchange with NFC FORUM Type5 Tag.
*/


/** @addtogroup lib_nfctype5pcd_Private_Functions
 *  @{
 */

/**
  * @}
  */

/** @addtogroup lib_nfctype5pcd_Public_Functions
 *  @{
 */

/**  
* @brief  	this function send an ExtendedReadSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param		UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param		BlockNumber	:  	index of block to read
* @param		pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
int8_t PCDNFCT5_ExtendedReadSingleBlock (uc8 Flags, uc8 *UIDin, uc16 BlockNumber,uint8_t *pResponse )
{
uint8_t DataToSend[ISO15693_MAXLENGTH_READSINGLEBLOCK],
		NthByte=0;

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = PCDNFCT5_CMDCODE_EXTREADSINGLEBLOCK;

	if (Flags & ISO15693_MASK_ADDRORNBSLOTSFLAG)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	DataToSend[NthByte++] = BlockNumber & 0x00FF;
	DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;	

	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != PCD_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
}


/**  
* @brief  this function send an Extended GetSystemInfo command and returns ISO15693_SUCCESSCODE if the command 
* @brief	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  Flags		:  	inventory flags
* @param	UID			:  	Tag UID
* @retval ISO15693_SUCCESSCODE : the function is successful
* @retval ISO15693_ERRORCODE_DEFAULT : an error occured
*/
int8_t PCDNFCT5_ExtendedGetSystemInfo ( uc8 Flags, uc8 ParamRequest, uc8 *UIDin, uint8_t *pResponse)
{
uint8_t DataToSend[ISO15693_MAXLENGTH_GETSYSTEMINFO],
		NthByte=0;
int8_t	status;
			
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = PCDNFCT5_CMDCODE_EXTENDEDGETSYSINFO;
	DataToSend[NthByte++] = ParamRequest;

	if (Flags & ISO15693_MASK_ADDRORNBSLOTSFLAG)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	errchk(PCD_SendRecv(NthByte,DataToSend,pResponse));
	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != PCD_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
Error:
	*pResponse = SENDRECV_ERRORCODE_SOFT;
	*(pResponse+1) = 0x00;
	return ISO15693_ERRORCODE_DEFAULT;
}


/**  
* @brief  this function send an ExtendedReadSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
int8_t PCDNFCT5_ExtendedReadMultipleBlock (uc8 Flags, uc8 *UIDin, uint16_t BlockNumber, uc8 NbBlock, uint8_t *pResponse )
{
uint8_t DataToSend[ISO15693_MAXLENGTH_READSINGLEBLOCK],
		NthByte=0;


	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = PCDNFCT5_CMDCODE_EXTREADMULBLOCKS;

	if (Flags & ISO15693_MASK_ADDRORNBSLOTSFLAG)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}
	BlockNumber=BlockNumber<<5; // *32

  
  DataToSend[NthByte++] = BlockNumber & 0x00FF;
  DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;
	
	DataToSend[NthByte++] = NbBlock;
	DataToSend[NthByte++] = 0;

	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != PCD_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;

  /* Check also Response flag here */
  if(pResponse[2] != 0)
    return ISO15693_ERRORCODE_DEFAULT;
  
	return ISO15693_SUCCESSCODE;


}

/**  
* @brief  	this function send an ExtendedWriteSingleblock command.
* @param  	Flags		:  	Request flags
* @param		UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param		BlockNumber	:  	index of block to write
* @param		BlockLength :	Nb of byte of block length
* @param		DataToWrite :	Data to write into contacless tag memory
* @param		pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
int8_t PCDNFCT5_ExtendedWriteSingleBlock(uc8 Flags, uc8 *UIDin, uc16 BlockNumber,uc8 *DataToWrite,uint8_t *pResponse )
{
uint8_t DataToSend[MAX_BUFFER_SIZE],
		NthByte=0,
		BlockLength = ISO15693_NBBYTE_BLOCKLENGTH;

	
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = PCDNFCT5_CMDCODE_EXTWRITESINGLEBLOCK;

	if (Flags & ISO15693_MASK_ADDRORNBSLOTSFLAG)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;

	
	memcpy(&(DataToSend[NthByte]),DataToWrite,BlockLength);
	NthByte +=BlockLength;

	if ((Flags & ISO15693_MASK_OPTIONFLAG) == 0)
		PCD_SendRecv(NthByte,DataToSend,pResponse);
	else 
	{	PCD_SendRecv(NthByte,DataToSend,pResponse);
	 	delay_ms(20);
    PCD_SendEOF(pResponse);

    if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != ISO15693_SUCCESSCODE)
      return ISO15693_ERRORCODE_DEFAULT;
	}	

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != PCD_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
	
}

/**  
* @brief  	this function send an ExtendedLockSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	pResponse	: 	pointer on PCD  response
* @retval 	RESULTOK	: 	PCD  returns a succesful code
* @retval 	ERRORCODE_GENERIC	: 	 PCD  returns an error code
*/
int8_t PCDNFCT5_ExtendedLockSingleBlock ( uc8 Flags, uc8 *UIDin, uc16 BlockNumber,uint8_t *pResponse)
{
	uint8_t DataToSend[ISO15693_MAXLENGTH_LOCKSINGLEBLOCK],
		NthByte=0;
	
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = PCDNFCT5_CMDCODE_EXTENDEDLOCKBLOCK;

	if (Flags & ISO15693_MASK_ADDRORNBSLOTSFLAG)
	{	memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	DataToSend[NthByte++] = BlockNumber & 0xFF;
	DataToSend[NthByte++] = BlockNumber >> 8;

	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != PCD_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;

}

/**
 * @brief  This function reads the NDEF message from a tag type V and store result in the pTT5Tag buffer
 * @retval PCDNFCT5_OK : Command success
 * @retval PCDNFCT5_ERROR : Transmission error
 * @retval PCDNFCT5_ERROR_LOCKED : The tag cannot be read (CC lock)
 */
uint8_t PCDNFCT5_ReadNDEF( void )
{
    uint16_t size;
    uint8_t tagDensity = ISO15693_HIGH_DENSITY;//卡容量类型:
    /* Could be 2 or 4 bytes (when NDEF size > 0xff) */
    uint8_t tlv_size = 2;
    /* Could be 4 or 8 (whem MLEN > 0xff) */
    uint8_t  ccfile_size = 4;
    /* NDEF eof, always 1 byte: 0xFE */
    const uint8_t  eof_size = 1;

    // Try to determine the density by reading the first sector (128 bytes)
    /* The density flag is used for the tags relying on ISO15693 extended protocol bit for higher addresses, such as M24LR64k (released before the NFC-forum Type Type5 specification) */
    /* While the tags like the ST25DV64k are not flagged HIGH_DENSITY as they rely on the extended commands from Type Tag standard */
	if (ISO15693_ReadBytesTagData(ISO15693_HIGH_DENSITY, ISO15693_LRiS64K, pTT5Tag, 127, 0) != ISO15693_SUCCESSCODE)
	{
		if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRiS64K, pTT5Tag, 127, 0) != ISO15693_SUCCESSCODE)
        {
            if(ISO15693_ReadBytesTagData(ISO15693_STLEGLR_HIGH_DENSITY, ISO15693_LRiS64K, pTT5Tag, 127, 0) != ISO15693_SUCCESSCODE)
            return PCDNFCT5_ERROR;
      
            tagDensity = ISO15693_STLEGLR_HIGH_DENSITY;
        }
        else
        {
            tagDensity = ISO15693_LOW_DENSITY;
        }
	}
	
	// NDEF capable ?//是否支持NDEF数据交换格式
	if ((pTT5Tag[0] != 0xE1) && (pTT5Tag[0] != 0xE2))
		return PCDNFCT5_ERROR_NOT_FORMATED;
	
	// Check read access//判断读权限
	if ((pTT5Tag[1]&0x0C) != 0)
		return PCDNFCT5_ERROR_LOCKED;
	
	// Get the size of the message
	if (pTT5Tag[2] == 0x00)
  {
    /* 8 byte CCfile. */
    ccfile_size = 8;
    if (pTT5Tag[9] == 0xFF)
    {
      tlv_size = 4;
      size = (pTT5Tag[10]<<8)|pTT5Tag[11];
    } else {
      size = 0x00FF&pTT5Tag[9];    
    }
  } else {
    /* 4 bytes CCfile. */
    if (pTT5Tag[5] == 0xFF)
    {
      tlv_size = 4;
      size = (pTT5Tag[6]<<8)|pTT5Tag[7];
    } else {
      size = 0x00FF&pTT5Tag[5];
    }
	}
  
	// Check if there is enough memory to read the tag
	// If CC3 bit3 = 1 the size is higher than 2KB but we don't know the size...
	if ((size+ccfile_size+eof_size+tlv_size) > NFCT5_MAX_TAGMEMORY)
		return PCDNFCT5_ERROR_MEMORY_INTERNAL;
	
	// Read the rest of the tag if needed
	if (size > (128 - (ccfile_size+eof_size+tlv_size)))
	{
    if( tagDensity == ISO15693_STLEGLR_HIGH_DENSITY )
    {
      if (ISO15693_ReadBytesTagData(tagDensity, ISO15693_LRiS64K, &pTT5Tag[128], size - 128 + ccfile_size+eof_size+tlv_size, 128) != ISO15693_SUCCESSCODE)
      {
        return PCDNFCT5_ERROR;
      }
    }
    else
    {
      if (ISO15693_ReadBytesTagData(tagDensity, ISO15693_LRiS64K, &pTT5Tag[128], size - 128 + ccfile_size+eof_size+tlv_size, 128) != ISO15693_SUCCESSCODE)
      {
        return PCDNFCT5_ERROR;
      }
    }
  }
	
	return PCDNFCT5_OK;	
}

/**
 * @brief  This function writes the NDEF message to a tag type V from the pTT5Tag buffer
 * @retval PCDNFCT5_OK : Command success
 * @retval PCDNFCT5_ERROR : Transmission error
 * @retval PCDNFCT5_ERROR_LOCKED : The tag cannot be write or read (CC lock)
 * @retval PCDNFCT5_ERROR_MEMORY : Not enough memory available on the tag
 */
uint8_t PCDNFCT5_WriteNDEF( void )
{
	uint8_t RepBuffer[30];
	uint8_t firstSector[140], status;
	uint16_t size, tagSize;
	uint8_t tagDensity = ISO15693_HIGH_DENSITY;
  /* Could be 0xE1 or 0xE2 (for tags supporting the extended commands) */
  uint8_t ndefCapable = 0xE1;
  /* Could be 2 or 4 bytes (when NDEF size > 0xff) */
  uint8_t tlv_size = 2;
  /* Could be 4 or 8 (whem MLEN > 0xff) */
  uint8_t  ccfile_size = 4;
  /* NDEF eof, always 1 byte: 0xFE */
  const uint8_t  eof_size = 1;
  
	// Try to determine the density by ready the first sector (128 bytes)
  /* The density flag is used for the tags relying on ISO15693 extended protocol bit for higher addresses, such as M24LR64k (released before the NFC-forum Type Type5 specification) */
  /* While the tags like the ST25DV64k are not flagged HIGH_DENSITY as they rely on the extended commands from Type Tag standard */
	if (ISO15693_ReadBytesTagData(ISO15693_HIGH_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE)
	{
    if (ISO15693_ReadBytesTagData(ISO15693_LOW_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE)
    {
      if(ISO15693_ReadBytesTagData(ISO15693_STLEGLR_HIGH_DENSITY, ISO15693_LRiS64K, firstSector, 127, 0) != ISO15693_SUCCESSCODE)
        return PCDNFCT5_ERROR;
      
      tagDensity = ISO15693_STLEGLR_HIGH_DENSITY;
    }
    else
    {
      tagDensity = ISO15693_LOW_DENSITY;
    }
	}
  
	// Get the size of the message to write
  /* At this point, NDEF message is formatted for a 4 bytes CCfile */
  if (pTT5Tag[5] == 0xFF)
  {
    size = (pTT5Tag[6]<<8)|pTT5Tag[7];
    tlv_size = 4;
  } else {
    size = 0x00FF&pTT5Tag[5];
  }
  
	// NDEF CCfile present?
	if ((firstSector[0] != 0xE1) && (firstSector[0] != 0xE2))
	{
		/* Create the CC file */
		// We need the size
		if (tagDensity == ISO15693_STLEGLR_HIGH_DENSITY)
		{
      ISO15693_GetSystemInfo (0x0A , 0x00, RepBuffer);
    }
		else
    {
			ISO15693_GetSystemInfo (0x02 , 0x00, RepBuffer);
      if((RepBuffer[3] & 0x4) == 0)
      {
        /* Memory size is not present in GetSystemInfo response (try the extended command) */
        PCDNFCT5_ExtendedGetSystemInfo(0x2,0x3F,0x00,RepBuffer);
        ndefCapable = 0xE2;
      }
		}
    
    /* Nblock & Block size are @byte 12/13/14 but first 2 bytes of the response for the reader status */
    if (RepBuffer[14] == 0xFF)
			tagSize = (((RepBuffer[15]<<8)|RepBuffer[14])+1)*(RepBuffer[16]+1);
		else
			tagSize = (RepBuffer[14]+1)*(RepBuffer[15]+1);
		// NDEF capable
		pTT5Tag[0] = ndefCapable;
		// Version + Read/Write allowed
		pTT5Tag[1] = 0x40;
		// Size
		if (tagSize > 2040) {
      /* This is the 8byte CCfile as defined by NFC-Forum Type 5 */
      memmove(&pTT5Tag[8],&pTT5Tag[4],size+tlv_size+eof_size);
      ccfile_size += 4;
			pTT5Tag[2] = 0x0;
      if (tagDensity == ISO15693_STLEGLR_HIGH_DENSITY)
      {
        pTT5Tag[3] = 0x00;
      }
      else
      {
        pTT5Tag[3] = 0x01;
      }
			pTT5Tag[4] = 0x0;
			pTT5Tag[5] = 0x0;
			pTT5Tag[6] = (tagSize/8) >> 8;
			pTT5Tag[7] = (tagSize/8) & 0xFF;
    }
		else
		{
      /* Low density regular 4bytes CCfile */
			pTT5Tag[2] = tagSize/8;
			pTT5Tag[3] = 0x01;
		}
	}
	else
	{
    /* Read & copy the CCfile */
    if(firstSector[2] == 0)
    {
      /* 8 bytes CCfile - original NDEF message start at address 4, need to move it */
      memmove(&pTT5Tag[8],&pTT5Tag[4],size+tlv_size+eof_size);
      ccfile_size += 4;
      memcpy(pTT5Tag,firstSector,8);
      tagSize = firstSector[6];
      tagSize = ((tagSize << 8) + firstSector[7])*8;
    } else {
      /* 4 bytes CCfile */
      memcpy(pTT5Tag,firstSector,4);
      tagSize = firstSector[2] * 8;
    }
    
		// Check read and write access
		if ((pTT5Tag[1]&0x0F) != 0)
			return PCDNFCT5_ERROR_LOCKED;
	}
		
	// Check if the memory available on the tag is enough
  if (tagSize < (size+ccfile_size+tlv_size+eof_size))
		return PCDNFCT5_ERROR_MEMORY_TAG;
	
	// Write the tag
	errchk(ISO15693_WriteBytes_TagData(tagDensity, pTT5Tag, (size+ccfile_size+tlv_size+eof_size), 0));
	
	return PCDNFCT5_OK;	
Error:
	return PCDNFCT5_ERROR;
}



#include "lib_NDEF_SMS.h"
void ISO15693_read_sms(void)
{

//	uint8_t UID[10],index=0;
//	sRecordInfo RecordStruct;
//	sSMSInfo red_sms;
//	
//	if(scan_ISO15693_card(UID) == ISO15693_SUCCESSCODE)
//	{
//	    static uint8_t i=1;
//	    uint16_t result;
//	    if(i==1)
//	    {
//	        i=0;

//		    result = PCDNFCT5_ReadNDEF();
//		    debug("[%d] PCDNFCT5_ReadNDEF result is %x\r\n",__LINE__,result);
//		    
//		    memset(NDEF_Buffer,'\0',50);		    
//	        result = NDEF_IdentifyNDEF(&RecordStruct,NDEF_Buffer);
//	        debug("[%d] NDEF_IdentifyNDEF result is %x\r\n",__LINE__,result);	 

//		    result = NDEF_ReadSMS(&RecordStruct, &red_sms);
//		    debug("[%d] NDEF_ReadSMS result is %x\r\n",__LINE__,result);
//		    for(index = 0;index<16;index++)
//		    {
//		        debug("%c ",red_sms.PhoneNumber[index]);
//		    }
//		    debug("\r\n");
//		    for(index = 0;index<20;index++)
//		    {
//		        debug("%c ",red_sms.Information[index]);
//		    }
//		    debug("\r\n");
//		    for(index = 0;index<20;index++)
//		    {
//		        debug("%c ",red_sms.Message[index]);
//		    } 
//		    debug("\r\n");
//		    
//		}
//	}
}
void ISO15693_write_sms(void)
{
//	uint8_t UID[10];
//	sSMSInfo write_sms;

//    strcpy(write_sms.PhoneNumber,"15659834336");
//	strcpy(write_sms.Message,"leelen test msg!");
//	strcpy(write_sms.Information,"leelen sms test!");
//	if(scan_ISO15693_card(UID) == ISO15693_SUCCESSCODE)
//	{
//	    static uint8_t i=1;
//	    uint16_t result;
//	    if(i==1)
//	    {
//	        i=0;
//		    result = NDEF_WriteSMS(&write_sms);
//		    debug("[%d] ISO15693_write_sms result is %x\r\n",__LINE__,result);
//		}
//	}
}


#include "lib_NDEF_Text.h"
void ISO15693_read_text(void)
{
//	uint8_t UID[10],index=0;
//	
//	sRecordInfo RecordStruct;

//	if(scan_ISO15693_card(UID) == ISO15693_SUCCESSCODE)
//	{
//	    static uint8_t i=1;
//	    uint16_t result;
//	    if(i==1)
//	    {
//	        i=0;

//		    result = PCDNFCT5_ReadNDEF();
//		    debug("[%d] PCDNFCT5_ReadNDEF result is %x\r\n",__LINE__,result);
//		    
//		    memset(NDEF_Buffer,'\0',50);		    
//	        result = NDEF_IdentifyNDEF(&RecordStruct,NDEF_Buffer);
//	        debug("[%d] NDEF_IdentifyNDEF result is %x\r\n",__LINE__,result);	 
//	        
//		    debug("[%d] NDEF_ReadText result is %x\r\n",__LINE__,result);
//		    for(index = 0;index<25;index++)
//		    {
//		        debug("%c ",NDEF_Buffer[index]);
//		    }
//		    debug("\r\n");		    
//		}
//	}
}

void ISO15693_write_text(void)
{
//	uint8_t UID[10];
//	static char text[]="leelen text!";

//	if(scan_ISO15693_card(UID) == ISO15693_SUCCESSCODE)
//	{
//	    static uint8_t i=1;
//	    uint16_t result;
//	    if(i==1)
//	    {
//	        i=0;
//		    result = NDEF_WriteText(text);
//		    debug("[%d] ISO15693_write_text result is %x\r\n",__LINE__,result);
//		}
//	}
}

#include "lib_NDEF_Geo.h"
void ISO15693_read_geo(void)
{
//	uint8_t UID[10],index=0;
//	
//	sRecordInfo RecordStruct;
//    sGeoInfo 		geo;
//	if(scan_ISO15693_card(UID) == ISO15693_SUCCESSCODE)
//	{
//	    static uint8_t i=1;
//	    uint16_t result;
//	    if(i==1)
//	    {
//	        i=0;

//		    result = PCDNFCT5_ReadNDEF();
//		    debug("[%d] PCDNFCT5_ReadNDEF result is %x\r\n",__LINE__,result);
//		    memset(NDEF_Buffer,'\0',50);		    
//	        result = NDEF_IdentifyNDEF(&RecordStruct,NDEF_Buffer);
//	        debug("[%d] NDEF_IdentifyNDEF result is %x\r\n",__LINE__,result);	
//	        
//		    result = NDEF_ReadGeo(&RecordStruct, &geo);
//		    debug("[%d] NDEF_ReadGeo result is %x\r\n",__LINE__,result);
//		    for(index = 0;index<20;index++)
//		    {
//		        debug("%c ",geo.Latitude[index]);
//		    }
//		    debug("\r\n");
//		    for(index = 0;index<20;index++)
//		    {
//		        debug("%c ",geo.Longitude[index]);
//		    }
//		    debug("\r\n");
//		    for(index = 0;index<20;index++)
//		    {
//		        debug("%c ",geo.Information[index]);
//		    } 
//		    debug("\r\n");	    
//		}
//	}
}

void ISO15693_write_geo(void)
{
//	uint8_t UID[10];
//    sGeoInfo 		geo;
//   	strcpy(geo.Latitude,"leelen latitude!");
//	strcpy(geo.Longitude,"leelen longitude!");
//	strcpy(geo.Information,"leelen geo test!");
//   
//	if(scan_ISO15693_card(UID) == ISO15693_SUCCESSCODE)
//	{
//	    static uint8_t i=1;
//	    uint16_t result;
//	    if(i==1)
//	    {
//	        i=0;
//		    result = NDEF_WriteGeo(&geo);
//		    debug("[%d] ISO15693_write_geo result is %x\r\n",__LINE__,result);
//		}
//	}
}


#include "lib_NDEF_URI.h"
void ISO15693_read_URI(void)
{
//	uint8_t UID[10],index=0;
//	
//	sRecordInfo RecordStruct;
//    sURI_Info 	url;
//	if(scan_ISO15693_card(UID) == ISO15693_SUCCESSCODE)
//	{
//	    static uint8_t i=1;
//	    uint16_t result;
//	    if(i==1)
//	    {
//	        i=0;

//		    result = PCDNFCT5_ReadNDEF();
//		    debug("[%d] PCDNFCT5_ReadNDEF result is %x\r\n",__LINE__,result);
//		    
//		    memset(NDEF_Buffer,'\0',50);		    
//	        result = NDEF_IdentifyNDEF(&RecordStruct,NDEF_Buffer);
//	        debug("[%d] NDEF_IdentifyNDEF result is %x\r\n",__LINE__,result);	 
//	         
//		    result = NDEF_ReadURI(&RecordStruct, &url);
//		    debug("[%d] NDEF_ReadURI result is %x\r\n",__LINE__,result);
//		    for(index = 0;index<20;index++)
//		    {
//		        debug("%c ",url.protocol[index]);
//		    }
//		    debug("\r\n");
//		    for(index = 0;index<20;index++)
//		    {
//		        debug("%c ",url.URI_Message[index]);
//		    }
//		    debug("\r\n");
//		    for(index = 0;index<20;index++)
//		    {
//		        debug("%c ",url.Information[index]);
//		    } 
//		    debug("\r\n");	    
//		}
//	}
}
void ISO15693_write_URI(void)
{
//	uint8_t UID[10];//,index=0;
//    sURI_Info 	url;
//   	strcpy(url.protocol,"http://");
//	strcpy(url.URI_Message,"www.leelen.com/");
//	strcpy(url.Information,"leelen main web!");
//   
//	if(scan_ISO15693_card(UID) == ISO15693_SUCCESSCODE)
//	{
//	    static uint8_t i=1;
//	    uint16_t result;
//	    if(i==1)
//	    {
//	        i=0;
//		    result = NDEF_WriteURI(&url);
//		    debug("[%d] ISO15693_write_URI result is %x\r\n",__LINE__,result);
//		    
//		}
//	}
}


/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
