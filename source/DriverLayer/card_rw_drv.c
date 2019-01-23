/********************************************************************************
**
** 文件名:     	 card_rw_drv.c 
** 版权所有:   (c) 	 2016 厦门立林科技有限公司
** 文件描述:   
**
*********************************************************************************
**             修改历史记录
**===============================================================================
**| 日期        | 作者     |  修改记录
**===============================================================================
**| 2016/04/21  | yeqizhi  |  创建该文件
**
*********************************************************************************/
 
/*-------------------------------- Includes ----------------------------------*/
#include "card_rw_drv.h"
#include "card_encrypt.h"
#include "lib_iso14443Apcd.h"
#include "mfos.h"
#include "string.h"
#include "rs485_drv.h"
#include "debug.h"
/*----------------------- Constant / Macro Definitions -----------------------*/
#define STEP_MAX 6
#define DATE_BUFFER_NUM    2
#define block_judge(block_cn)     ((((block_cn+1)&0x03)==0)||(block_cn==0))?DATA_KEY:DATA_USER 

/*------------------------ Variable Declarations -----------------------------*/
//立林科技默认的cpu卡参数
unsigned char const  ADF_LL[2]={0x10,0x05};
unsigned char const  EF_LL[2]={0x00,0x01};
unsigned char const  KEY_CN_IN_LL=0x05;
unsigned char const  KEY_LEN_LL=0x10;
unsigned char const  KEY_CN_READ_LL=0x01;
unsigned char const  KEY_CN_WRITE_LL=0x02;
unsigned char const  EF_TYPE[2]={0x2A,0x20};
unsigned char const  DATE_LEN=200;
unsigned char const  EF_TYPE_RECORD=0x2A;

static CARD_PARA_STR  s_card_para;

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/

/*******************************************************************
** 函数名:       card_ic_select 
** 函数描述:     判断所读到的卡是否为期望选择的卡
** 参数:       	 输入:期望选择的卡的uid
** 返回:       	 0->成功
** 注意: 	 
** 记录: 	pass
********************************************************************/
static uint8_t card_ic_select(uint8_t *card_uid)
{
		uint8_t status;	
		uint8_t s_card_uid[4];//用来保存pcd扫描得到的卡号

        status = scan_ISO14443A_card(s_card_uid);
        if(status != ISO14443A_SUCCESSCODE) status = scan_ISO14443A_card(s_card_uid);
        if(status != ISO14443A_SUCCESSCODE)return CARD_NONE_ERR;  
        //将pcd扫描得到的卡号与期望选择的卡号对比
		if(memcmp(s_card_uid,card_uid,4))return CARD_ERR  ;
		return ISO14443A_SUCCESSCODE;
}
/*****************************************************************************
 函 数 名  : card_ic_write
 功能描述  : IC卡的写操作
 输入参数  : IC卡写块通信数据指针  
 输出参数  : 无
 返 回 值  : 数据长度为0，代表错误，有
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月22日
    作    者   : yeqizhi
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t card_ic_write(IC_BLOCK_RW_STR *date_buff,CARD_DATA_TYPE date_type)
{    
    uint8_t result,step,result_expand;
    uint8_t SecNR;          //扇区号
    IC_BLOCK_RW_STR *s_date_buff;
    IC_RESULT_STR  *frame_result;
    rs485_frame_msg *p_msg;
//    uint8_t len = 0;
    
    s_date_buff = (IC_BLOCK_RW_STR *)date_buff;
    ///
//    len = sizeof(IC_BLOCK_RW_STR);
//    debug("card_ic_write cmd (len = %d):\r\n",len);
//    debug_print((uint8_t*)date_buff,len);
    ///
    if(date_type != block_judge(s_date_buff->block_cn))return 0;
    result = SUCCESS;
    for(step=1;step<STEP_MAX;step++)
    {
        switch ( step )
        {
            case 1 :/* 选择IC卡  */              
                if(card_ic_select(s_date_buff->uid))
                {
                    step = STEP_MAX;
                    result = FAIL;
                    result_expand = CARD_ERR;
                }

                break;
            case 2 :/* 获取密钥 */
                SecNR = s_date_buff->block_cn>>2;
            	if((s_date_buff->key_cn!=MC_AUTH_A)&&(s_date_buff->key_cn!=MC_AUTH_B))    //无密钥号，使用立林默认密钥
            	{
            	    s_date_buff->key_cn = 0x01;  //写操作，立林默认为KEYB认证
            	    gen_key_ic(s_date_buff->key_cn, s_date_buff->uid, SecNR, s_card_para.project_num, s_date_buff->key);   //计算卡的扇区密钥
            	    s_date_buff->key_cn = MC_AUTH_B;
                }
                break;
            case 3 :/* 密钥认证  */
                if(mf_enhanced_auth(s_date_buff->block_cn,s_date_buff->uid,s_date_buff->key,s_date_buff->key_cn))
                {
                    step = STEP_MAX;
                    result = FAIL;
                    result_expand = IC_AUTH_ERR;
                }
                break;
            case 4 : /* 写块步骤  */
                if(PCDNFCT2_Write(s_date_buff->block_cn,s_date_buff->block_buffer))
                {
                    step = STEP_MAX;
                    result = FAIL;
                    result_expand = BLOCK_ERR;
                }
                break;
            case 5 : /* halt卡  */
//                PcdMIF_Halt();
                
                break;
            default:
                break;
        }
    } 
    
    /* 上传读卡结果,用于17型*/
//    frame_result = (IC_RESULT_STR *)date_buff;
//    if(status == SUCCESS)
//    {
//        frame_result->result = SUCCESS;
//        frame_result->payload[0] = 0x00;
//    }
//    else
//    {
//        frame_result->result = FAIL;
//        frame_result->payload[0] = status;
//    }

    /* 上传写卡结果,用于蓝牙读头 */
    p_msg = append_frame_queue(20);     //获取队列地址
    if(p_msg == NULL) return 0;	
    p_msg->len = 15;
    frame_result = (IC_RESULT_STR *)set_frame_head(p_msg->pbuf, CARD_RESULT_UPDATA_CMD, p_msg->len-1, 0);    
    memcpy(frame_result, s_date_buff, 6);
    frame_result->cmd_aux = IC_BLOCK_WRITE;
    if(result == SUCCESS)
    {
        frame_result->result = SUCCESS;
        frame_result->payload[0] = 0x00;
//        debug("PCDNFCT2_Write success!\r\n");
    }
    else
    {
        frame_result->result = FAIL;
        frame_result->payload[0] = result_expand;
        debug("PCDNFCT2_Write error!\r\n");
    }
    p_msg->pbuf[p_msg->len-1] = get_check_sum(p_msg->pbuf,p_msg->len);

    return (sizeof(IC_RESULT_STR));
}

/*****************************************************************************
 函 数 名  : card_ic_read
 功能描述  : IC卡的写操作
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月22日
    作    者   : yeqizhi
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t card_ic_read(IC_BLOCK_RW_STR *date_buff)
{
    uint8_t result,step,result_expand;
    uint8_t SecNR;          //扇区号
    IC_BLOCK_RW_STR *s_date_buff;
    IC_RESULT_STR *frame_data;
    rs485_frame_msg *p_msg;
//    uint8_t len = 0;
   /* 块12 控制数据:0x73,0xB5,0xCC,0xCE,0x30,0xF3,0x78,0x77,0x88,0x69,0x2D,0xBB,0x8F,0xB6,0x8B,0x82  */

    s_date_buff = (IC_BLOCK_RW_STR *)date_buff;    
    //debug///////////////////
//    len = sizeof(IC_BLOCK_RW_STR);
//    debug("card_ic_read cmd %d:\r\n",len);
//    debug_print((uint8_t*)date_buff,len-16);
    //////////////////////////
    result = SUCCESS;
    for(step=1;step<STEP_MAX;step++)
    {
        switch (step)
        {
            case 1 :/* 选择IC卡  */              
                if(card_ic_select(s_date_buff->uid))
                {
                    step = STEP_MAX;
                    result = FAIL;
                    result_expand = CARD_ERR;
                }
                break;
            case 2 :/* 获取密钥 */
                SecNR = s_date_buff->block_cn/4;
            	if((s_date_buff->key_cn!=MC_AUTH_A)&&(s_date_buff->key_cn!=MC_AUTH_B))    //无密钥号，使用立林默认密钥
            	{
            	    s_date_buff->key_cn = 0x00;  //读操作，立林默认为KEYA认证
            	    gen_key_ic(s_date_buff->key_cn, s_date_buff->uid, SecNR, s_card_para.project_num, s_date_buff->key);   //计算卡的扇区密钥
            	    s_date_buff->key_cn = MC_AUTH_A;
                }
                break;
            case 3 :/* 密钥认证  */
                if(mf_enhanced_auth(s_date_buff->block_cn,s_date_buff->uid,s_date_buff->key,s_date_buff->key_cn))
                {
                    step = STEP_MAX;
                    result = FAIL;
                    result_expand = IC_AUTH_ERR;
                }
                break;
            case 4 : /* 读块步骤  */
                memset(s_date_buff->block_buffer,0,16);
                if(PCDNFCT2_Read(s_date_buff->block_cn,s_date_buff->block_buffer))
                {
                    step = STEP_MAX;
                    result = FAIL;
                    result_expand = BLOCK_ERR;
                }
                break;
            case 5 : /* halt卡  */
//                PcdMIF_Halt();
                break;
            default:
                break;
        }
    } 


    
    
    /* 上传读卡结果,用于17型*/
//    frame_data = (IC_RESULT_STR *)date_buff;
//    if(status == SUCCESS)
//    {                
//        frame_data->result = SUCCESS;
//        memcpy(&frame_data->payload,s_date_buff->block_buffer,16);
//        return (sizeof(IC_RESULT_STR));
//    }
//    else
//    {  
//        frame_data->result = FAIL;
//        frame_data->payload[0] = status;
//        return (sizeof(IC_RESULT_STR)-15);
//    } 	



     /* 上传读卡结果,用于蓝牙读头 */
//    rs485_send_frame(RS485_ID1,s_date_buff->block_buffer,16);
    p_msg = append_frame_queue(40);     //获取队列地址
    if(p_msg == NULL) return 0;	    
    if(result == SUCCESS)
    {
        p_msg->len = 30;
        frame_data = (IC_RESULT_STR *)set_frame_head(p_msg->pbuf, CARD_RESULT_UPDATA_CMD, p_msg->len-1, 0);    
        memcpy(frame_data, s_date_buff, 6);
        frame_data->cmd_aux = IC_BLOCK_READ;
        frame_data->result = result;
        memcpy(frame_data->payload,s_date_buff->block_buffer,16);
//        debug("read data :");
//        debug_print(s_date_buff->block_buffer,16);
    }
    else
    {
        p_msg->len = 15;
        frame_data = (IC_RESULT_STR *)set_frame_head(p_msg->pbuf, CARD_RESULT_UPDATA_CMD, p_msg->len-1, 0);    
        memcpy(frame_data, s_date_buff, 6);
        frame_data->cmd_aux = IC_BLOCK_READ;
        frame_data->result = FAIL;
        frame_data->payload[0]= result_expand;
//        debug("card_ic_read error : stept %d\r\n",result_expand);
    }
    p_msg->pbuf[p_msg->len-1] = get_check_sum(p_msg->pbuf,p_msg->len);

    return (sizeof(IC_RESULT_STR));
}
/*****************************************************************************
 函 数 名  : card_set_para
 功能描述  : 卡处理的参数设置
 输入参数  : CARD_PARA_STR *card_para  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年6月27日
    作    者   : yeqizhi
    修改内容   : 新生成函数
    对于读写模块只要传入工程号即可
*****************************************************************************/
void card_rw_set_para(CARD_PARA_STR *card_para)
{
    memcpy(&s_card_para.card_ab_type,card_para,sizeof(CARD_PARA_STR));
}
/////////////////////////////////////////////////////////////////////////////
#include "card_encrypt.h"
#include "des.h"
#define EACH_ITEM_MAX_LEN 0x20
//立林科技默认的cpu卡参数/////////////////////////////////////////
uint8_t const  LEELEN_MF[2]            = {0x3F, 0x00}; //leelen主文件夹id
uint8_t const  LEELEN_ADF[2]           = {0x10, 0x05}; //leelen应用文件夹id
uint8_t const  LEELEN_ADF_SIZE[2]      = {0X04, 0x00}; //文件夹总大小1KB
uint8_t const  LEELEN_KEY_FILE[2]      = {0x00, 0x01}; //密钥文件id不能为0，不然选中的时候应答错误
uint8_t const  LEELEN_KEY_FILE_SIZE[2] = {0X00, 0xa0}; //密钥文件大小10个密钥，每个密钥长度为16
uint8_t const  LEELEN_EF[2]            = {0x00, 0x02}; //EF文件id
uint8_t const  LEELEN_EX_KEY_CN        = 0x03;         //外部密钥号
uint8_t const  LEELEN_IN_KEY_CN        = 0x05;         //内部密钥号
uint8_t const  LEELEN_EF_RKEY_CN       = 0x01;         //读EF密钥号
uint8_t const  LEELEN_EF_WEY_CN        = 0x02;         //写EF密钥号
uint8_t const  LEELEN_KEY_LEN          = 0x10;         //密钥长度
uint8_t const  LEELEN_READ_KEY_CN      = 0x01;
uint8_t const  LEELEN_WRITE_KEY_CN     = 0x02;
uint8_t const  LEELEN_EF_TYPE[2]       = {0x2A, EACH_ITEM_MAX_LEN}; //记录类型0x2A，长度EACH_ITEM_MAX_LEN即32字节
uint8_t const  LEELEN_EF_ITEM_CNT      = 0x0a;         //定长记录的总个数
uint8_t const  LEELEN_EF_RW_KEY        = 0x91;         //读写密钥号
uint8_t const  LEELEN_EF_DATA_LEN      = 250;          //可写的记录文件总大小

///////////////////////////////////////////////////////////////////
#define ExAuthenticate_KEY_TYPE 0x39 //外部认证密钥类型
#define	InAuthenticate_KEY_TYPE 0x30 //内部认证密钥类型
#define	ReadEF_KEY_TYPE         0x39 //读EF密钥类型
#define	WriteEF_KEY_TYPE        0x39 //写EF密钥类型
/*---------------------------------------------------------------------------*/
/******************************************************************************
* Name:      cpu_internal_auth
*
* Desc:      CPU卡内部认证
* Param(in):
* Param(out):
* Return:    IC_SUCCESS->成功
* Global:
* Note:      可使用默认参数进行验证，需要传入卡号，ADF置为0
* Author:    liuwq
*------------------------------------------------------------------------------
* Log:   2017/11/08, Create this function by liuwq
*******************************************************************************/
uint8_t cpu_internal_auth ( CPU_IN_AUTH_T* date_buff )
{
    CPU_IN_AUTH_T* in_auth;
    uint8_t mf[2] = {0x3f, 0x00};
    uint8_t step, rlen, des_in[8], des_out[8], random[8]; //因为使用des会改变des_in的值，所以定义临时数组random
//	rs485_frame_msg *p_msg;
//	CPU_IN_AUTH_RESULT_STR *frame_data;

    in_auth = ( CPU_IN_AUTH_T* ) date_buff;
    for ( step = 1; step <= ( STEP_MAX + 1 ); step++ )
    {
        in_auth->result = FAIL;
        switch ( step )
        {
        case 1://选中卡
            in_auth->result = card_ic_select ( in_auth->uid );
            if ( in_auth->result )
            {
                step = STEP_MAX;
                in_auth->result = FAIL;
                in_auth->result_expand = CARD_ERR;
            }
            break;

        case 2://进入MF文件夹
//            Rats();
            in_auth->result = ISO14443A_SelectFile ( mf );
            if ( in_auth->result )
            {
                step = STEP_MAX;
                in_auth->result = FAIL;
                in_auth->result_expand = MF_ERR;
            }
            break;

        case 3://进入ADF文件夹
            /* ADF==00 00 所有参数使用立林的参数*/
            if ( ( in_auth->adf[0] == 0 ) && ( in_auth->adf[1] == 0 ) )
            {
                in_auth->adf[0]  = LEELEN_ADF[0];
                in_auth->adf[1]  = LEELEN_ADF[1];
                in_auth->key_cn  = LEELEN_IN_KEY_CN;
                in_auth->key_len = LEELEN_KEY_LEN;
                gen_key_cpu ( in_auth->uid, in_auth->adf, in_auth->key_cn, s_card_para.project_num, in_auth->key );
            }

            in_auth->result = ISO14443A_SelectFile ( in_auth->adf );
            if ( in_auth->result )
            {
                step = STEP_MAX;
                in_auth->result = FAIL;
                in_auth->result_expand = ADF_ERR;
            }
            break;

        case 4://内部认证
            if ( in_auth->key_len == 0x10 )
            {
                in_auth->result = ISO14443A_GetRandom ( &rlen, random );
                if ( in_auth->result )
                {
                    step = STEP_MAX;
                    in_auth->result = FAIL;
                    in_auth->result_expand = CPU_AUTH_ERR;
                    break;
                }
                memcpy ( des_in, random, 8 );
                TDES ( random, des_out, in_auth->key, ENCRY ); //本地进行加密计算
                in_auth->result = ISO14443A_InAuthenticate ( des_in, in_auth->key_cn, des_out ); //内部认证
                if ( in_auth->result )
                {
                    step = STEP_MAX;
                    in_auth->result = FAIL;
                    in_auth->result_expand = CPU_AUTH_ERR;
                }
            }
            else if ( in_auth->key_len == 0x08 )
            {
                in_auth->result = ISO14443A_GetRandom ( &rlen, random );
                if ( in_auth->result )
                {
                    step = STEP_MAX;
                    in_auth->result = FAIL;
                    in_auth->result_expand = CPU_AUTH_ERR;
                    break;
                }
                memcpy ( des_in, random, 8 );
                DES ( random, des_out, in_auth->key, ENCRY );
                in_auth->result = ISO14443A_InAuthenticate ( des_in, in_auth->key_cn, des_out ); //内部认证
                if ( in_auth->result )
                {
                    step = STEP_MAX;
                    in_auth->result = FAIL;
                    in_auth->result_expand = CPU_AUTH_ERR;
                }
            }
            else {}
            break;

        case 5://停卡
                in_auth->result = ISO14443A_HLTA();
                if(in_auth->result)
                {
                    step = STEP_MAX;
                }
                else//到此，内部认证成功
				{
						step = STEP_MAX + 1;
						in_auth->result = SUCCESS;
						in_auth->result_expand = 0;
				}
        break;

        default:
            ISO14443A_HLTA();
            break;
        }
    }
	
	
	 /* 上传认证结果,用于蓝牙读头 */
//    p_msg = append_frame_queue(40);     //获取队列地址
//    if(p_msg == NULL) return 0;	    
//    p_msg->len = 16;
//    frame_data = (CPU_IN_AUTH_RESULT_STR *)set_frame_head(p_msg->pbuf, CARD_RESULT_UPDATA_CMD, p_msg->len-1, 0);    
//    memcpy(frame_data, date_buff, 7);
//    frame_data->cmd_aux = CPU_IN_AUTH;
//    frame_data->result = in_auth->result;
//	frame_data->result_expand = in_auth->result_expand;
//    p_msg->pbuf[p_msg->len-1] = get_check_sum(p_msg->pbuf,p_msg->len);

//	debug_hex(p_msg->pbuf,p_msg->len);

    return in_auth->result;
}
