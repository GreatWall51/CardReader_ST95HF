/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : Card_read_app.c
  版 本 号   : 初稿
  作    者   : yeqizhi
  生成日期   : 2016年2月24日
  最近修改   :
  功能描述   : 实现读卡操作
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月24日
    作    者   : yeqizhi
    修改内容   : 创建文件

    
模块使用说明:

1、读卡操作先进行初始化寄存参数，默认支持IC卡读卡号，void card_read_init(void)

   如果要有防复制或者读取身份证功能需设置参数，void card_read_set_para(CARD_PARA_STR *card_para)。

2、在主函数里调用扫描函数，void card_read_real_time(void)

3、读到卡号后以事件标志进行输出，uint8_t card_get_event(card_info *card_data_str);

4、模块所需资源:

   (1)时间模块:Timer_drv.h
   
   (2)防复制需要密钥算法:card_encrypt.h
   
   (3)卡芯片基础驱动:Fm1722_drv.h

******************************************************************************/



/*-------------------------------- Includes ----------------------------------*/
 #include "timer_drv.h"
 #include <string.h>
 #include "card_read_drv.h"
 #include "define.h"
 #include "lib_iso14443Apcd.h"
 #include "lib_iso14443Bpcd.h"
 #include "lib_iso15693pcd.h"
 #include "memory_manage.h "
 #include "lib_ConfigManager.h"
 #include "param_mode.h"
 #include "debug.h"
/*----------------------- Constant / Macro Definitions -----------------------*/
#define CARD_UID_MAX_LEN 11

/*------------------------ Variable Define/Declarations ----------------------*/

CARD_INF0_T s_card_data_str = {0};
static CARD_PARA_STR  s_card_para;
static uint8_t  s_card_uid_new[CARD_UID_MAX_LEN];
static uint8_t  s_card_uid_new_last[CARD_UID_MAX_LEN];
static uint8_t  s_card_event;
static uint8_t work_mode = IC_MODE;
static uint8_t have_card_flag=0;//第0位为有无IC卡标志,第1位为有无身份证标志,第2位为有NFC卡标志,
/*----------------------------- External Variables ---------------------------*/

uint8_t interval_timer_sfz = 0;
uint8_t interval_timer_ic = 0;//识别IC卡的时间间隔,同一张IC卡快速刷的时间间隔    
uint8_t interval_timer_nfc = 0;
/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/




/*****************************************************************************
 函 数 名  : card_set_para
 功能描述  : 卡处理的参数设置
 输入参数  : CARD_PARA_STR *card_para,是否支持读身份证和IC卡防复制
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年6月27日
    作    者   : yeqizhi
    修改内容   : 新生成函数
    对于读卡模块,需要设置card_ab_type
*****************************************************************************/
void card_read_set_para(CARD_PARA_STR *card_para)
{
    memcpy(&s_card_para.card_ab_type,card_para,sizeof(CARD_PARA_STR));
}

/*******************************************************************
** 函数名:       card_init 
** 函数描述:    实现读卡芯片的初始化
** 参数:         mode_type :TYPEA_MODE/TYPEB_MODE
** 返回:       	 
** 注意: 	 
** 记录: 	 2016/02/24,  yeqizhi创建
********************************************************************/
 void card_read_init(void)
 {	 
    CARD_PARA_STR card_para;
    
    memset(&s_card_data_str,0,sizeof(CARD_INF0_T));
    memory_init();
    ConfigManager_HWInit();
    card_para.card_ab_type = 1;
    card_para.anti_copy = 0;//0x80-0X8F
    memcpy(card_para.project_num,get_anti_copy_param(),19);
    card_read_set_para(&card_para);
 }
/*******************************************************************
** 函数名:       card_get_event 
** 函数描述:     实现外部函数调用读卡状态及数据的作用,主函数的一次主循环中,
                 可以多个地方调用该函数,但是程序全部跑完一遍后,会清事件,如
                 果还是同一张卡,则不会再产生事件.
** 参数:       	 输入卡数据的结构体指针
** 返回:       	 是否刷卡成功，卡数据由卡数据结构体指针进行返回
** 注意: 	    卡数据的结构体见CARD_INF0_T
** 记录: 	 2016/02/24,  yeqizhi创建
********************************************************************/
 uint8_t card_get_event(CARD_INF0_T *card_data_str)
 {
    if(s_card_event)
    {
        memcpy(card_data_str, &s_card_data_str,sizeof(CARD_INF0_T));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
 } 

static void card_ic_exe(void)
{
    static uint8_t ic_read_times = 0;//ic卡扫描次数过滤,即扫描失败ic_read_times次才认为失败
    
    //同一张卡多次扫描时,只产生一次事件
    if(ISO14443A_SUCCESSCODE == scan_ISO14443A_card(s_card_uid_new))
    {
        ic_read_times = 0;//
        have_card_flag |=0x01;
        if(memcmp(s_card_uid_new,s_card_uid_new_last,CARD_IC_LEN) == 0)
        {
            if(interval_timer_ic == 0)
            {
                //here, we get a new card
                s_card_event = TRUE;  
				
            }
                
            interval_timer_ic = IC_INTERVAL_TIME;
        }
        else
        {                
            s_card_event = TRUE;
            interval_timer_ic = IC_INTERVAL_TIME;
            memcpy(s_card_uid_new_last,s_card_uid_new,CARD_IC_LEN);  
        }
        if(s_card_event == TRUE)
        {
            s_card_data_str.card_type = TYPE_IC;
//            if(is_support_rats() && (get_sys_param(ANTI_COPY_PARAM) == 0x01))
						if(is_support_rats())
            {
                s_card_data_str.card_type = TYPE_CPU;
            }
            s_card_data_str.card_len = CARD_IC_LEN;
            memcpy(s_card_data_str.card_uid, s_card_uid_new,CARD_IC_LEN);   						
        }			
    }
	else
	{
    	if(ic_read_times > 0) ic_read_times--;
    	if(ic_read_times == 0)
    	{
    		memset(&s_card_data_str,0,sizeof(CARD_INF0_T));
    		have_card_flag &=0xfe;
    	}
	} 
}
static void card_sfz_exe(void)
{
	static uint8_t sfz_read_times = 0;//身份证扫描次数过滤,即扫描连续失败ic_read_times次,才认为扫描失败
	if(ISO14443B_SUCCESSCODE == scan_ISO14443B_card(s_card_uid_new))
	{

		sfz_read_times = 5;
		have_card_flag |= 0x02;
		if(memcmp(s_card_uid_new,s_card_uid_new_last,CARD_SFZ_LEN) == 0)
		{
			if(interval_timer_sfz == 0)
    		{
    		    //here, we get a new card                
				s_card_event = TRUE;
    		}
			interval_timer_sfz = SFZ_INTERVAL_TIME;
		}
		else
		{
			s_card_event = TRUE;
			interval_timer_sfz = SFZ_INTERVAL_TIME;
			memcpy(s_card_uid_new_last,s_card_uid_new,CARD_SFZ_LEN);
		}
		if(s_card_event == TRUE)
		{
			s_card_data_str.card_type = TYPE_SFZ;
			s_card_data_str.card_len = CARD_SFZ_LEN;
			memcpy(s_card_data_str.card_uid, s_card_uid_new,CARD_SFZ_LEN);
			interval_timer_sfz = SFZ_INTERVAL_TIME;
		}
	}
	else
	{
		if(sfz_read_times > 0) sfz_read_times--;    
		if(sfz_read_times == 0)
		{
			memset(&s_card_data_str,0,sizeof(CARD_INF0_T));
			have_card_flag &= 0xfd;
		}
	}
}


static void card_nfc_exe(void)
{
	static uint8_t nfc_read_times = 0;//身份证扫描次数过滤,即扫描ic_read_times次,其中有一次成功就认为读卡成功
	if(ISO15693_SUCCESSCODE == scan_ISO15693_card(s_card_uid_new))
	{
		nfc_read_times = 0;
		if(memcmp(s_card_uid_new,s_card_uid_new_last,CARD_NFC_LEN) == 0)
		{
			if(interval_timer_nfc == 0)
    		{
    		    //here, we get a new card                
				s_card_event = TRUE;
				have_card_flag |= 0x04;
    		}
			interval_timer_nfc = NFC_INTERVAL_TIME;
		}
		else
		{
			s_card_event = TRUE;
			interval_timer_nfc = NFC_INTERVAL_TIME;
			memcpy(s_card_uid_new_last,s_card_uid_new,CARD_NFC_LEN);
		}
		if(s_card_event == TRUE)
		{
			have_card_flag |= 0x04;
			s_card_data_str.card_type = TYPE_NFC;
			s_card_data_str.card_len = CARD_NFC_LEN;
			memcpy(s_card_data_str.card_uid, s_card_uid_new,CARD_NFC_LEN);
			interval_timer_nfc = NFC_INTERVAL_TIME;
		}
	}
	else
	{
		if(nfc_read_times > 0) nfc_read_times--;    
		if(nfc_read_times == 0)
		{
			memset(&s_card_data_str,0,sizeof(CARD_INF0_T));
			have_card_flag &= 0xfb;
		}
	}
}

void card_read_real_time(void)
{
    static uint8_t times = 0;
    s_card_event = FALSE;
    
    if(!IS_TIMER_EVT(TIME_EVT_64)) return; // every 64 ms 
    
    if(interval_timer_ic) interval_timer_ic--;
    if(interval_timer_sfz) interval_timer_sfz--;
    if(interval_timer_nfc) interval_timer_nfc--;
    memset(s_card_uid_new,0,CARD_UID_MAX_LEN);
	/* 支持身份证扫描的FM1722模式 ...added by yeqizhi, 2016-04-21 */
    if(s_card_para.card_ab_type) 
    {
        times++;
        if(times == IC_SCAN_TIMES_START)
        {          
            work_mode = IC_MODE;
        }
        else if(times == (IC_SCAN_TIMES_START+IC_SCAN_TIMES))
        {      
            work_mode = SFZ_MODE;
        } 
        else if(times == (IC_SCAN_TIMES_START+IC_SCAN_TIMES+SFZ_SCAN_TIMES))//3
        {
            work_mode = NFC_MODE;
            times=0;
        }
		else if(times >= (IC_SCAN_TIMES_START+IC_SCAN_TIMES+SFZ_SCAN_TIMES+NFC_SCAN_TIMES))//4
		{
			times =0;
		}
    }
    else
    {
        work_mode = IC_MODE;
    }    
  
    if(IC_MODE == work_mode)
    {
        card_ic_exe();  
    }
    else if(SFZ_MODE == work_mode)
    {
		card_sfz_exe();
    }
	else if(NFC_MODE == work_mode)
	{
		card_nfc_exe();
	}
}

uint8_t cehck_have_card(void)
{
    return have_card_flag;
}
uint8_t get_reader_mode(void)
{
    return work_mode;
}
/*---------------------------------------------------------------------------*/

