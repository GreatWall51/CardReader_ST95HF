/**
  ******************************************************************************
  * @file    lib_NDEF_SMS.h
  * @author  MMY Application Team
  * @version V1.0.0
  * @date    20-November-2013
  * @brief   This file help to manage SMS NDEF file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIB_NDEF_SMS_H
#define __LIB_NDEF_SMS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lib_NDEF.h"
	 
	 
typedef struct 
{
	char PhoneNumber[16];
	char Message[50];
	char Information[50];
}sSMSInfo;

/* Struct defined in lib_NDEF.h */
typedef struct sRecordInfo sRecordInfo;
	 	 
uint16_t NDEF_ReadSMS(sRecordInfo *pRecordStruct, sSMSInfo *pSMSStruct);
uint16_t NDEF_WriteSMS( sSMSInfo *pSMSStruct );
void NDEF_PrepareSMSMessage ( sSMSInfo *pSMSStruct, uint8_t *pNDEFMessage, uint16_t *size );

#ifdef __cplusplus
}
#endif
	 
#endif /* __LIB_NDEF_SMS_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
