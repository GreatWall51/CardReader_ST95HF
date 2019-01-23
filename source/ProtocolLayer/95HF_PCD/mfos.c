/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 mfos.c 
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
 
 
/*------------------------------- Includes ----------------------------------*/
#include "mfos.h"
#include "lib_PCD.h"
#include "stdlib.h"
#include "crypto1.h"
#include "debug.h"
#include "timer_drv.h"
#include "mirror-subr.h"
#include "led_drv.h"

//test
#include "rs485_drv.h"
#include "sim_uart_drv.h"
#include "drv_interrupt.h"
/*---------------------- Constant / Macro Definitions -----------------------*/

 
/*----------------------- Type Declarations ---------------------------------*/


/*------------------- Global Definitions and Declarations -------------------*/
extern struct Crypto1State s_pcs;
extern uint8_t	            *u95HFBuffer;// [RFTRANS_95HF_MAX_BUFFER_SIZE+3];
/*----------------------- Variable Declarations -----------------------------*/

 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/

uint64_t bytes_to_num(uint8_t *src, uint32_t len)
{
  uint64_t num = 0;
  if(src == NULL) return 0;
  while (len--) {
    num = (num << 8) | (*src);
    src++;
  }
  return num;
}

uint8_t oddparity(const uint8_t bt)
{
  // cf http://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
  return (0x9669 >> ((bt ^(bt >> 4)) & 0xF)) & 1;
}

uint8_t creat_send_data(uint8_t* data,uint8_t* parity,uint8_t parity_len,uint8_t *creat_data,uint8_t* out_len)
{
    uint8_t out_index,in_index;
    if(data == NULL || parity == NULL || creat_data == NULL || parity_len == 0) return 1;
    out_index = 0;
    in_index = 0;
    *out_len= (parity_len<<1);
    while(parity_len--)
    {
        creat_data[out_index++] = data[in_index];
        creat_data[out_index++] = parity[in_index];
        in_index++;
    }
    return 0;
}
uint8_t rcv_data_separate(uint8_t* recv_data,uint8_t* data,uint8_t* out_len)
{
    uint8_t out_index;
    if(recv_data == NULL || data == NULL || out_len==NULL) return 1;
    if(((recv_data[1]-3)&0x01) != 0) return 1;
    
    *out_len = (recv_data[1]-3)>>1;
    for(out_index=0;out_index<(recv_data[1]-3);out_index++)
    {
        if((out_index&0x01) == 0)
        {
            data[out_index>>1] = recv_data[2+out_index];
        }else
        {
            //data[out_index>>1] = recv_data[3+out_index<<1];
        }
    }  
    return 0;
}
/******************************************************************************
* Name: 	 send_cmd 
*
* Function Desc: 命令发送,发送时自动添加帧头及校验码,接收时自动剥离帧头及校验码
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/06/03, Create this function by liuwq
******************************************************************************/
uint8_t send_cmd(uint8_t* tdata,uint8_t tlen,uint8_t* rdata,uint8_t* rlen)
{
    uint8_t index;
    uint8_t result = 1;
    uint8_t send_data[25] = {0};
    uint8_t send_data_len = 0;
    uint8_t Tx[10] = {0},Tx_Par[10] = {0},Rx[10] = {0};
    if(tdata == NULL || rdata == NULL) return 1;
    memcpy(Tx,tdata,tlen);
    iso14443a_crc_append(Tx,tlen);
    for(index=0;index<(tlen+2);index++)
	{
	    Tx_Par[index] = iso14443a_Par(Tx[index]);
	}
	mirror_bytes(Tx_Par,tlen+2);
    result = creat_send_data(Tx,Tx_Par,tlen+2,send_data,&send_data_len);
    if(result != 0) return 1;
	send_data[(tlen+2)<<1] = 0x18;
    result = PCD_SendRecv(((tlen+2)<<1)+1,send_data,u95HFBuffer);
    if(result != 0) return 1;
    result = rcv_data_separate(u95HFBuffer,Rx,rlen);
    if(result != 0) return 1;
    memcpy(rdata,Rx,*rlen);
    return 0;
}
/******************************************************************************
* Name: 	 send_cmd_par 
*
* Function Desc: 	 发送命令及校验码
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/06/03, Create this function by liuwq
******************************************************************************/
uint8_t send_cmd_par(uint8_t* tdata,uint8_t tlen,uint8_t* tpar,uint8_t* rdata,uint8_t* rlen)
{
    uint8_t send_data[25] = {0};
    uint8_t send_data_len = 0;
    uint8_t result = 1;
    uint8_t Tx[10] = {0},Tx_Par[10] = {0},Rx[10] = {0};
    if(tdata == NULL || rdata == NULL || tpar == NULL || rlen == NULL) return 1;
    memcpy(Tx,tdata,tlen);
    memcpy(Tx_Par,tpar,tlen);
    mirror_bytes(Tx_Par,tlen);  
    result = creat_send_data(Tx,Tx_Par,tlen,send_data,&send_data_len);
    if(result != 0) return 1;
    send_data[send_data_len++] = 0x18;
    result = PCD_SendRecv(send_data_len,send_data,u95HFBuffer);
    if(result != 0) return 1;
    result = rcv_data_separate(u95HFBuffer,Rx,rlen);
    if(result != 0) return 1;
    memcpy(rdata,Rx,*rlen);
    return 0;
}
/******************************************************************************
* Name: 	 mf_enhanced_auth 
*
* Function Desc: 	 
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/06/03, Create this function by liuwq
******************************************************************************/
uint8_t mf_enhanced_auth(uint8_t e_block,uint8_t *uid,uint8_t *key,uint8_t key_type)
{
    uint8_t Nr[4] = {0x00,0x00,0x00,0x00}; // Reader nonce
    uint32_t Nt = 0;// Tag nonce
    uint8_t cmd[10] = {0};
    uint8_t cmd_par[10] = {0};
//    uint8_t Rx[MAX_FRAME_LEN]; // Tag response
    uint8_t *Rx = u95HFBuffer;
    uint8_t index;
    uint8_t result =1;
    struct Crypto1State *pcs;

	
  // Prepare AUTH command
    cmd[0] = key_type;//选择认证密钥类型
    cmd[1] = e_block ;//| 0x03;//认证的密钥区
    result = send_cmd(cmd,2,Rx,&index);
    if(result != 0) return 1;
  // Save the tag nonce (Nt)
    Nt = bytes_to_num(Rx,4);
  // Init the cipher with key {0..47} bits
     pcs = crypto1_create(bytes_to_num(key, 6));
     if(pcs == NULL) 
    {
//        debug("[%d]error: pcs = NULL",__LINE__);
        crypto1_destroy(pcs);
        return 1;
    }
  // Load (plain) uid^nt into the cipher {48..79} bits
  crypto1_word(pcs, bytes_to_num(Rx,4) ^ bytes_to_num(uid, 4),0);

  // Generate (encrypted) nr+parity by loading it into the cipher
  for (index = 0; index < 4; index++) 
  {
    // Load in, and encrypt the reader nonce (Nr)
    cmd[index] = crypto1_byte(pcs, Nr[index], 0) ^ Nr[index];
    cmd_par[index] = filter(pcs->odd) ^ oddparity(Nr[index]);
  }

  // Skip 32 bits in the pseudo random generator
  Nt = prng_successor(Nt, 32);
  // Generate reader-answer from tag-nonce
  for (index = 4; index < 8; index++) 
  {
    // Get the next random byte
    Nt = prng_successor(Nt, 8);
    // Encrypt the reader-answer (Nt' = suc2(Nt))
    cmd[index] = crypto1_byte(pcs, 0x00, 0) ^(Nt & 0xff);
    cmd_par[index] = filter(pcs->odd) ^ oddparity(Nt);
  }
  result = send_cmd_par(cmd,8,cmd_par,Rx,&index);
  if(result != 0) 
  {
    crypto1_destroy(pcs);
    return 1;
  }
  Nt = prng_successor(Nt, 32);
  if (!((crypto1_word(pcs, 0x00, 0) ^ bytes_to_num(Rx, 4)) == (Nt & 0xFFFFFFFF))) {
		crypto1_destroy(pcs);
		return 1;
  }
//  crypto1_destroy(pcs);//放在读写函数中释放
  return 0;
}

/******************************************************************************
* Name: 	 PCDNFCT2_Read 
*
* Function Desc: 	 
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/06/03, Create this function by liuwq
******************************************************************************/
uint8_t PCDNFCT2_Read(uint8_t blocNbr, uint8_t *pBufferRead)
{
	uint8_t i;
	uint8_t result = 1;
	struct Crypto1State *pcs = &s_pcs;
	uint8_t send_data[25] = {0};
	uint8_t send_data_len = 0;
	uint8_t cmd[20]={0}, ArEnc[20]={0}, ArEncPar[20]={0},Rx[20],RxPar[20]={0};

	if(pBufferRead == NULL || pcs == NULL) return 1;
	cmd[0] = MC_READ;
	cmd[1] = blocNbr;
	iso14443a_crc_append(cmd, 2);    
	for (i = 0; i < 4; i++) {
			ArEnc[i] = crypto1_byte(pcs, 0, 0) ^ cmd[i];
			ArEncPar[i] = filter(pcs->odd) ^ oddparity(cmd[i]);
	}
	mirror_bytes(ArEncPar,4);  
	result = creat_send_data(ArEnc,ArEncPar,4,send_data,&send_data_len);
	if(result != 0) 
	{
	    crypto1_destroy(pcs);
	    return 1;
	}
	send_data[8] = 0x18;
	result = PCD_SendRecv(9,send_data,u95HFBuffer);
	if(result != 0) 
	{
//	    debug("PCDNFCT2_Read error!\r\n");
	    crypto1_destroy(pcs);
	    return 1;
	}
	result = rcv_data_separate(u95HFBuffer,Rx,&i);
	if(result != 0) 
	{
	    crypto1_destroy(pcs);
	    return 1;
	}
	if(i!=18) 
	{
	    crypto1_destroy(pcs);
	    return 1;//接收字节错误
	}
	for(i = 0; i < 18; i++)
	{
			Rx[i] = crypto1_byte(pcs, 0, 0) ^ Rx[i];
			RxPar[i] = filter(pcs->odd) ^ RxPar[i];
	}
	memcpy(pBufferRead,Rx,16);
	crypto1_destroy(pcs);
	return 0; 
}


uint8_t PCDNFCT2_Write(uint8_t blocNbr, uint8_t *pBufferWrite)
{
	uint8_t i;
	uint8_t result = 1;
	struct Crypto1State *pcs = &s_pcs;
	uint8_t send_data[80] = {0};
	uint8_t send_data_len = 0;
	uint8_t cmd[20], ArEnc[20], ArEncPar[20];

    if(pBufferWrite == NULL || pcs == NULL) return 1;
    //第一步
    cmd[0] = MC_WRITE;
    cmd[1] = blocNbr;
    iso14443a_crc_append(cmd, 2);    
    for (i = 0; i < 4; i++) {
        ArEnc[i] = crypto1_byte(pcs, 0, 0) ^ cmd[i];
        ArEncPar[i] = filter(pcs->odd) ^ oddparity(cmd[i]);
    }
    mirror_bytes(ArEncPar,4);  
    result = creat_send_data(ArEnc,ArEncPar,4,send_data,&send_data_len);
    if(result != 0) 
    {
	    crypto1_destroy(pcs);
	    return 1;
	}
    send_data[8] = 0x18;
    result = PCD_SendRecv(9,send_data,u95HFBuffer);
    if(u95HFBuffer[2] != 0x90)
    {
        crypto1_destroy(pcs);
//        debug("PCDNFCT2_Write stept 1 error!\r\n");
        return 1;
    }
    memset(u95HFBuffer,0,3);
//	if(result != 0) 
//	{
//	    crypto1_destroy(pcs);
//	    return 1;
//	}
    for (i = 0; i < 4; i++) {
        ArEnc[i] = crypto1_bit(pcs, 0, 0) ^ cmd[i];
    }
    //第二步
    memcpy(cmd,pBufferWrite,16);
//    rs485_send_frame(RS485_SIM,pBufferWrite,16);
    iso14443a_crc_append(cmd, 16);    
    for (i = 0; i < 18; i++) {
        ArEnc[i] = crypto1_byte(pcs, 0, 0) ^ cmd[i];
        ArEncPar[i] = filter(pcs->odd) ^ oddparity(cmd[i]);
    }
    mirror_bytes(ArEncPar,18);  
    result = creat_send_data(ArEnc,ArEncPar,18,send_data,&send_data_len);
    if(result != 0) 
	{
	    crypto1_destroy(pcs);
	    return 1;
	}
    if(result != 0) 
    {
	    crypto1_destroy(pcs);
	    return 1;
	}
    send_data[36] = 0x18;
	result = PCD_SendRecv(37,send_data,u95HFBuffer);
//	debug_print(u95HFBuffer,7);
//	debug("2 u95HFBuffer[2] = %x,result = %x\r\n",u95HFBuffer[2],result);
	//应答87 01 90或80... 或90...都认为正确应答
	if(u95HFBuffer[2] == 0x90 || u95HFBuffer[0] == 0x90 || u95HFBuffer[0] == 0x80)
	{
		for (i = 0; i < 4; i++) {
        ArEnc[i] = crypto1_bit(pcs, 0, 0) ^ cmd[i];
    }
		crypto1_destroy(pcs);
		return 0; 
	}
	//应答错误处理
	crypto1_destroy(pcs);
	return 1;
	
#if 0
	if(u95HFBuffer[2] != 0x90)
    {
//        debug("PCDNFCT2_Write stept 2 error!\r\n");
			crypto1_destroy(pcs);
			return 1;
			
    }
//	if(result != 0) 
//	{
//	    crypto1_destroy(pcs);
//	    return 1;
//	}
	for (i = 0; i < 4; i++) {
        ArEnc[i] = crypto1_bit(pcs, 0, 0) ^ cmd[i];
    }
    crypto1_destroy(pcs);
	return 0; 
#endif
}


/*---------------------------------------------------------------------------*/

