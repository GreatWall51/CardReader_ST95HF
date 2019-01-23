/* Includes ------------------------------------------------------------------------------ */
#include "define.h"
#include "drv_95HF.h"
#include "define.h"
#include "drv_spi.h"
#include "stdbool.h"
#include "drv_interrupt.h"
#include "miscellaneous.h"
#include "timer_drv.h"
#include "debug.h"
#include "rs485_drv.h"
#include "lib_ConfigManager.h"
// #include "card_read_drv.h"

#define RFTRANS_95HF_NSS_HIGH()  do{sbi(GPIOA,GPIO_Pin_4);}while(0) 
#define RFTRANS_95HF_NSS_LOW() do{cbi(GPIOA,GPIO_Pin_4);}while(0)


extern uint8_t	*u95HFBuffer;//[RFTRANS_95HF_MAX_BUFFER_SIZE+3];
extern volatile bool							uDataReady; 
__IO uint8_t						uTimeOut;
uint8_t EnableTimeOut = TRUE;


/* ConfigStructure */ 										 
drv95HF_ConfigStruct			drv95HFConfig;

/* drv95HF_Private_Functions */
static int8_t drv95HF_SPIPollingCommand(void);


/**
 *	@brief  this functions initializes the SPI in order to communicate with the 95HF device
 *  @param  None
 *  @retval void 
 */
static void drv95HF_InitializeSPI ( void )
{
	RFTRANS_IRQ_check_io_init();
	IRQOut_Config();
	spi_init();
}

/**
 *	@brief  This function sends a reset command over SPI bus
 *  @param  none
 *  @retval None
 */
static void drv95HF_SendSPIResetByte(void)
{
	/* Send reset control byte */
	SPI_SendReceiveByte(SPI1, RFTRANS_95HF_COMMAND_RESET);
}


/**
* @brief  	Initilize the 95HF device config structure
* @param  	None
* @retval 	None
*/
void drv95HF_InitConfigStructure (void)
{
	drv95HFConfig.uInterface = RFTRANS_95HF_INTERFACE_SPI;
	drv95HFConfig.uSpiMode = RFTRANS_95HF_SPI_POLLING;
	drv95HFConfig.uState = RFTRANS_95HF_STATE_POWERUP;
	drv95HFConfig.uCurrentProtocol = RFTRANS_95HF_PROTOCOL_UNKNOWN;
	drv95HFConfig.uMode = RFTRANS_95HF_MODE_UNKNOWN;
}


/**
 *	@brief  Send a reset sequence over SPI bus (Reset command ,wait ,negative pulse on IRQin).
 *  @param  None
 *  @retval None
 */
void drv95HF_ResetSPI(void)
{	
	/* Deselect Rftransceiver over SPI */
	RFTRANS_95HF_NSS_HIGH();
	delayHighPriority_ms(1);
	/* Select 95HF device over SPI */
	RFTRANS_95HF_NSS_LOW();
	/* Send reset control byte	*/
	drv95HF_SendSPIResetByte();
	/* Deselect 95HF device over SPI */
	RFTRANS_95HF_NSS_HIGH();
	delayHighPriority_ms(3);

	/* send a pulse on IRQ_in to wake-up 95HF device */
	drv95HF_SendIRQINPulse();
	delayHighPriority_ms(10);  /* mandatory before issuing a new command */

	drv95HFConfig.uState = RFTRANS_95HF_STATE_READY;
}


/**
 *	@brief  returns the value of interface pin. 
 *				  Low level means UART bus was choose.
 *				  High level means SPI bus was choose.
 *  @param  None
 *  @retval Bit_RESET : 0
 *  @retval Bit_SET : 1
 */
int8_t drv95HF_GetInterfacePinState ( void )
{
	return RFTRANS_95HF_INTERFACE_SPI;
}

///**  
// *  @brief  This function returns the IRQout state
// *  @param  None
// *  @retval Pin set : 1
// *  @retval Pin reset : 0
// */
//int8_t drv95HF_GetIRQOutState ( void )
//{
//	if(GPIO_ReadInputDataBit(EXTI_GPIO_PORT,EXTI_GPIO_PIN) != 0x00)
//	{
//		return 0x01;
//	}
//	else 
//	{
//		return 0x00;
//	}
//}

/**
 *	@brief  This function initialize MCU serial interface peripheral (SPI or UART)
 *  @param  None
 *  @retval None
 */
void drv95HF_InitilizeSerialInterface ( void )
{
	/* -- Get interface pin state to select UART or SPI mode -- */
	drv95HFConfig.uInterface = RFTRANS_95HF_INTERFACE_SPI;
	/* -- Initialize SPI Interface -- */ 
	drv95HF_InitializeSPI();
	/* -- IRQout configuration PA2 to send pulse on USART_RX of 95HF device */
	
	
}

/**
 *	@brief  This function enable the interruption
 *  @param  None
 *  @retval None
 */
void drv95HF_EnableInterrupt(void)
{
	/* enable interruption */
	drvInt_Enable_Reply_IRQ();

	/* set back driver in polling mode */
	drv95HFConfig.uSpiMode = RFTRANS_95HF_SPI_INTERRUPT;	
}

/**
 *	@brief  This function disable the interruption
 *  @param  None
 *  @retval None
 */
void drv95HF_DisableInterrupt(void)
{
	/* disable interruption */
	drvInt_Disable_95HF_IRQ();
	
	/* set back driver in polling mode */
	drv95HFConfig.uSpiMode = RFTRANS_95HF_SPI_POLLING;	
}


/**
 *	@brief  This function returns the Interface selected(UART or SPI)
 *  @param  none
 *  @retval RFTRANS_INTERFACE_UART : the UART interface is selected
 *  @retval RFTRANS_INTERFACE_SPI : the SPI interface is selected
 */
uint8_t drv95HF_GetSerialInterface ( void )
{
	return drv95HFConfig.uInterface;
}
/**
 *	@brief  This function sends a command over SPI bus
 *  @param  *pData : pointer on data to send to the xx95HF
 *  @retval void
 */
void drv95HF_SendSPICommand(uc8 *pData)
{
    uint8_t DummyBuffer[MAX_BUFFER_SIZE];
    uint16_t bufferlength = 0;

	/*  Select xx95HF over SPI  */
	RFTRANS_95HF_NSS_LOW();
	/* Send a sending request to xx95HF  */
	SPI_SendReceiveByte(SPI1, RFTRANS_95HF_COMMAND_SEND);
	if(*pData == ECHO)
	{
		/* Send a sending request to xx95HF */ 
		SPI_SendReceiveByte(SPI1, ECHO);
//		debug("[%d] drv95HF_SendReceive  ECHO cmd sak is %x\r\n",__LINE__,ask);
	}
	else
	{
    if( pData[RFTRANS_95HF_COMMAND_OFFSET] == 0x24 )
    {
      bufferlength = pData[RFTRANS_95HF_LENGTH_OFFSET] + RFTRANS_95HF_DATA_OFFSET + 256;
    }
    else if (pData[RFTRANS_95HF_COMMAND_OFFSET] == 0x44)
    {
      bufferlength = pData[RFTRANS_95HF_LENGTH_OFFSET] + RFTRANS_95HF_DATA_OFFSET + 512;
    }
    else
    {
      bufferlength = pData[RFTRANS_95HF_LENGTH_OFFSET] + RFTRANS_95HF_DATA_OFFSET;
    }

		/* Transmit the buffer over SPI */
#ifdef USE_DMA	
		SPI_SendReceiveBufferDMA(RFTRANS_95HF_SPI, pData, bufferlength, DummyBuffer);
#else
		SPI_SendReceiveBuffer(RFTRANS_95HF_SPI, pData, bufferlength, DummyBuffer);
#endif
	}
	
	/* Deselect xx95HF over SPI  */
	RFTRANS_95HF_NSS_HIGH();
}
/**
 *	@brief  This function polls 95HF chip until a response is ready or
 *				  the counter of the timeout overflows
 *  @retval PCD_POLLING_TIMEOUT : The time out was reached 
 *  @retval PCD_SUCCESS_CODE : A response is available
 */
static int8_t drv95HF_SPIPollingCommand( void )
{
	uint8_t Polling_Status = 0;

	if(EnableTimeOut)
		StartTimeOut(1000);		/* 3sec for LLCP can be improved to adjust it dynamically */

	if (drv95HFConfig.uSpiMode == RFTRANS_95HF_SPI_POLLING)
	{
		
		do{
			/* in case of an HID interuption during the process that can desactivate the timeout */
			/* Enable the Time out timer */
			TIM_Cmd(TIMER_TIMEOUT, ENABLE);
			
			RFTRANS_95HF_NSS_LOW();
			
			delay_ms(2);
					
			/*  poll the 95HF transceiver until he's ready ! */
			Polling_Status  = SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_POLLING);
			
			Polling_Status &= RFTRANS_95HF_FLAG_DATA_READY_MASK;
	
		}	while( Polling_Status 	!= RFTRANS_95HF_FLAG_DATA_READY && uTimeOut != true );
		
		RFTRANS_95HF_NSS_HIGH();
	}	
	else if (drv95HFConfig.uSpiMode == RFTRANS_95HF_SPI_INTERRUPT)
	{
		/* Wait a low level on the IRQ pin or the timeout  */
		while( (uDataReady == false) & (uTimeOut == false) )
		{}
	}

	if(EnableTimeOut)
		StopTimeOut( );

	if ( uTimeOut == true )
	{
//	    debug("st95hf respond timeout!\r\n");
//        PCD_PORsequence();
		clear_card_info();
        ConfigManager_HWInit();
		return RFTRANS_95HF_POLLING_TIMEOUT;
	}

	return RFTRANS_95HF_SUCCESS_CODE;	
}

/**
 *	@brief  This fucntion recovers a response from 95HF device
 *  @param  *pData : pointer on data received from 95HF device
 *  @retval None
 */
void drv95HF_ReceiveSPIResponse(uint8_t *pData)
{
	uint8_t DummyBuffer[MAX_BUFFER_SIZE];
  uint16_t lengthToRead = 0;

	/* Select 95HF transceiver over SPI */
	RFTRANS_95HF_NSS_LOW();

	/* Request a response from 95HF transceiver */
	SPI_SendReceiveByte(RFTRANS_95HF_SPI, RFTRANS_95HF_COMMAND_RECEIVE);

	/* Recover the "Command" byte */
	pData[RFTRANS_95HF_COMMAND_OFFSET] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);

	if(pData[RFTRANS_95HF_COMMAND_OFFSET] == ECHO)
	{
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = 0x00;
		/* In case we were in listen mode error code cancelled by user (0x85 0x00) must be retrieved */
		pData[RFTRANS_95HF_LENGTH_OFFSET+1] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
		pData[RFTRANS_95HF_LENGTH_OFFSET+2] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
	}
	else if(pData[RFTRANS_95HF_COMMAND_OFFSET] == 0xFF)
	{
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = 0x00;
		pData[RFTRANS_95HF_LENGTH_OFFSET+1] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
		pData[RFTRANS_95HF_LENGTH_OFFSET+2] = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
	}
	else
	{
		/* Recover the "Length" byte */
		pData[RFTRANS_95HF_LENGTH_OFFSET]  = SPI_SendReceiveByte(RFTRANS_95HF_SPI, DUMMY_BYTE);
		/* Checks the data length */
		if((pData[RFTRANS_95HF_COMMAND_OFFSET] == 0x87) && (pData[RFTRANS_95HF_LENGTH_OFFSET] == 0x01))
		{
		    lengthToRead = 5;
		    SPI_SendReceiveBuffer(RFTRANS_95HF_SPI, DummyBuffer, lengthToRead, &pData[RFTRANS_95HF_DATA_OFFSET]);
		}
		else if( !( ( ( pData[RFTRANS_95HF_COMMAND_OFFSET] & 0xE0) == 0x80 ) && (pData[RFTRANS_95HF_LENGTH_OFFSET] == 0x00) ) )
        {
            lengthToRead = (uint16_t)(pData[RFTRANS_95HF_COMMAND_OFFSET] & 0x60);
            lengthToRead = (lengthToRead << 3) + pData[RFTRANS_95HF_LENGTH_OFFSET];
            /* Recover data 	*/
            SPI_SendReceiveBuffer(RFTRANS_95HF_SPI, DummyBuffer, lengthToRead, &pData[RFTRANS_95HF_DATA_OFFSET]);
        }

	}

	/* Deselect xx95HF over SPI */
	RFTRANS_95HF_NSS_HIGH();
	
}


/**
 *	@brief  This function send a command to 95HF device over SPI or UART bus and receive its response
 *  @param  *pCommand  : pointer on the buffer to send to the 95HF device ( Command | Length | Data)
 *  @param  *pResponse : pointer on the 95HF device response ( Command | Length | Data)
 *  @retval RFTRANS_95HF_SUCCESS_CODE : the function is succesful
 */
int8_t  drv95HF_SendReceive(uc8 *pCommand, uint8_t *pResponse)
{		
	u8 command = *pCommand;
	/* if we want to send a command we are not expected a interrupt from RF event */
	if(drv95HFConfig.uSpiMode == RFTRANS_95HF_SPI_INTERRUPT)
	{	
//		debug("[%d]SPI mode is RFTRANS_95HF_SPI_INTERRUPT\r\n",__LINE__);
		drvInt_Enable_Reply_IRQ();
	}
	
	if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
	{
//		debug("[%d]interface is SPI\r\n",__LINE__);
		/* First step  - Sending command 	*/
		drv95HF_SendSPICommand(pCommand);
		/* Second step - Polling	*/
		if (drv95HF_SPIPollingCommand() != RFTRANS_95HF_SUCCESS_CODE)
		{	
//			debug("[%d]drv95HF_SPIPollingCommand error!\r\n",__LINE__);
			*pResponse =RFTRANS_95HF_ERRORCODE_TIMEOUT;
			return RFTRANS_95HF_POLLING_TIMEOUT;	
		}
		/* Third step  - Receiving bytes */
		drv95HF_ReceiveSPIResponse(pResponse);
	}
	
	/* After listen command is sent an interrupt will raise when data from RF will be received */
	if(command == LISTEN)
	{	
		if(drv95HFConfig.uSpiMode == RFTRANS_95HF_SPI_INTERRUPT)
		{		
			drvInt_Enable_RFEvent_IRQ( );
		}
	}

	return RFTRANS_95HF_SUCCESS_CODE; 
}

/**
 *	@brief  This function send a command to 95HF device over SPI or UART bus
 *  @param  *pCommand  : pointer on the buffer to send to the 95HF ( Command | Length | Data)
 *  @retval None
 */
void drv95HF_SendCmd(uc8 *pCommand)
{
	if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
		/* First step  - Sending command 	*/
		drv95HF_SendSPICommand(pCommand);
#ifdef CR95HF	
	else if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_UART)
		/* First step  - Sending command	*/
		drv95HF_SendUARTCommand(pCommand);
#endif /* CR95HF */
}

/**
 *	@brief  This function is a specific command. It's made polling and reading sequence. 
 *  @param  *pResponse : pointer on the 95HF device response ( Command | Length | Data)
 *  @retval RFTRANS_95HF_SUCCESS_CODE : the function is succesful
 *  @retval RFTRANS_95HF_POLLING_RFTRANS_95HF : the polling sequence returns an error
 */
int8_t  drv95HF_PoolingReading (uint8_t *pResponse)
{
	
	*pResponse =RFTRANS_95HF_ERRORCODE_DEFAULT;
	*(pResponse+1) = 0x00;

	if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
	{
		/* First step - Polling	*/
		if (drv95HF_SPIPollingCommand( ) != RFTRANS_95HF_SUCCESS_CODE)
		{	*pResponse = RFTRANS_95HF_ERRORCODE_TIMEOUT;
			return RFTRANS_95HF_ERRORCODE_TIMEOUT;	
		}
		
		/* Second step  - Receiving bytes 	*/
		drv95HF_ReceiveSPIResponse(pResponse);
	}
#ifdef CR95HF	
	else if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_UART)
	{
		/* Second step - Receiving bytes 	*/
		drv95HF_ReceiveUARTResponse(pResponse);
	}
#endif /* CR95HF */
	return RFTRANS_95HF_SUCCESS_CODE; 
}

/**
 *	@brief  Send a negative pulse on IRQin pin
 *  @param  none
 *  @retval None
 */
void drv95HF_SendIRQINPulse(void)
{
	if (drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
	{
		/* Send a pulse on IRQ_IN */
		RFTRANS_95HF_IRQOUT_HIGH() ;
		delayHighPriority_ms(10);
		RFTRANS_95HF_IRQOUT_LOW() ;
		delayHighPriority_ms(2);
		RFTRANS_95HF_IRQOUT_HIGH() ;
	}
	/* Need to wait 10ms after the pulse before to send the first command */
	delayHighPriority_ms(10);

}


#ifdef CR95HF

/**
 *	@brief  this functions initializes UART in order to communicate with 95HF device
 *  @param  BaudRate : value of the Baudrate to use during UART communication
 *  @retval None 
 */
void drv95HF_InitializeUART (uc32 BaudRate)
{
	USART_DeInit(RFTRANS_95HF_UART);
	/* enables clock	*/
	drv95HF_RCCConfigUART( );
	/* configures RX and TX 	*/
	/* TX : alternate function push pull	*/
	/* RX : floating input  	*/
	drv95HF_GPIOConfigUART( );
	/* set protcol 	*/
	drv95HF_StructureConfigUART(BaudRate );
}
#endif /* CR95HF */

/**
 *	@brief  this functions put the ST95HF in sleep/hibernate mode
 *  @param  WU_source : Source selected to wake up the device (WU_TIMEOUT,WU_TAG,WU_FIELD,WU_IRQ,WU_SPI)
 *  @param  mode : Can be IDLE_SLEEP_MODE or IDLE_HIBERNATE_MODE
 *  @retval None 
 */
void drv95HF_Idle(uc8 WU_source, uc8 mode)
{
	uint8_t pCommand[] = {RFTRANS_95HF_COMMAND_IDLE, IDLE_CMD_LENTH, 0, 0, 0, 0, 0 ,0x18 ,0x00 ,0x00 ,0x60 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00};

	if (mode == IDLE_SLEEP_MODE) /* SLEEP */
	{
		/* Select the wake up source*/
		pCommand[2] = WU_source;
		/* Select SLEEP mode */
		if (WU_source == WU_FIELD)
		{
			pCommand[3] = GETMSB(SLEEP_FIELD_ENTER_CTRL);
			pCommand[4] = GETLSB(SLEEP_FIELD_ENTER_CTRL);
		}
		else
		{
			pCommand[3] = GETMSB(SLEEP_ENTER_CTRL);
			pCommand[4] = GETLSB(SLEEP_ENTER_CTRL);
		}
		pCommand[5] = GETMSB(SLEEP_WU_CTRL);
		pCommand[6] = GETLSB(SLEEP_WU_CTRL);
	}
	else /* HIBERNATE */
	{
		/* Select the wake up source, only IRQ is available for HIBERNATE mode*/
		pCommand[2] = WU_IRQ;
		/* Select HIBERNATE mode */
		pCommand[3] = GETMSB(HIBERNATE_ENTER_CTRL);
		pCommand[4] = GETLSB(HIBERNATE_ENTER_CTRL);
		pCommand[5] = GETMSB(HIBERNATE_WU_CTRL);
		pCommand[6] = GETLSB(HIBERNATE_WU_CTRL);
		pCommand[10] = 0x00;
	}
	
	/* Send the command */
	if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_SPI)
		/* First step  - Sending command 	*/
		drv95HF_SendSPICommand(pCommand);
#ifdef CR95HF	
	else if(drv95HFConfig.uInterface == RFTRANS_95HF_INTERFACE_UART)
		/* First step  - Sending command	*/
		drv95HF_SendUARTCommand(pCommand);
#endif /* CR95HF */
	
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

