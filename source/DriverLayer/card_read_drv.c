/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : Card_read_app.c
  �� �� ��   : ����
  ��    ��   : yeqizhi
  ��������   : 2016��2��24��
  ����޸�   :
  ��������   : ʵ�ֶ�������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��2��24��
    ��    ��   : yeqizhi
    �޸�����   : �����ļ�

    
ģ��ʹ��˵��:

1�����������Ƚ��г�ʼ���Ĵ������Ĭ��֧��IC�������ţ�void card_read_init(void)

   ���Ҫ�з����ƻ��߶�ȡ���֤���������ò�����void card_read_set_para(CARD_PARA_STR *card_para)��

2���������������ɨ�躯����void card_read_real_time(void)

3���������ź����¼���־���������uint8_t card_get_event(card_info *card_data_str);

4��ģ��������Դ:

   (1)ʱ��ģ��:Timer_drv.h
   
   (2)��������Ҫ��Կ�㷨:card_encrypt.h
   
   (3)��оƬ��������:Fm1722_drv.h

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
static uint8_t have_card_flag=0;//��0λΪ����IC����־,��1λΪ�������֤��־,��2λΪ��NFC����־,
/*----------------------------- External Variables ---------------------------*/

uint8_t interval_timer_sfz = 0;
uint8_t interval_timer_ic = 0;//ʶ��IC����ʱ����,ͬһ��IC������ˢ��ʱ����    
uint8_t interval_timer_nfc = 0;
/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/




/*****************************************************************************
 �� �� ��  : card_set_para
 ��������  : ������Ĳ�������
 �������  : CARD_PARA_STR *card_para,�Ƿ�֧�ֶ����֤��IC��������
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��6��27��
    ��    ��   : yeqizhi
    �޸�����   : �����ɺ���
    ���ڶ���ģ��,��Ҫ����card_ab_type
*****************************************************************************/
void card_read_set_para(CARD_PARA_STR *card_para)
{
    memcpy(&s_card_para.card_ab_type,card_para,sizeof(CARD_PARA_STR));
}

/*******************************************************************
** ������:       card_init 
** ��������:    ʵ�ֶ���оƬ�ĳ�ʼ��
** ����:         mode_type :TYPEA_MODE/TYPEB_MODE
** ����:       	 
** ע��: 	 
** ��¼: 	 2016/02/24,  yeqizhi����
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
** ������:       card_get_event 
** ��������:     ʵ���ⲿ�������ö���״̬�����ݵ�����,��������һ����ѭ����,
                 ���Զ���ط����øú���,���ǳ���ȫ������һ���,�����¼�,��
                 ������ͬһ�ſ�,�򲻻��ٲ����¼�.
** ����:       	 ���뿨���ݵĽṹ��ָ��
** ����:       	 �Ƿ�ˢ���ɹ����������ɿ����ݽṹ��ָ����з���
** ע��: 	    �����ݵĽṹ���CARD_INF0_T
** ��¼: 	 2016/02/24,  yeqizhi����
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
    static uint8_t ic_read_times = 0;//ic��ɨ���������,��ɨ��ʧ��ic_read_times�β���Ϊʧ��
    
    //ͬһ�ſ����ɨ��ʱ,ֻ����һ���¼�
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
	static uint8_t sfz_read_times = 0;//���֤ɨ���������,��ɨ������ʧ��ic_read_times��,����Ϊɨ��ʧ��
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
	static uint8_t nfc_read_times = 0;//���֤ɨ���������,��ɨ��ic_read_times��,������һ�γɹ�����Ϊ�����ɹ�
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
	/* ֧�����֤ɨ���FM1722ģʽ ...added by yeqizhi, 2016-04-21 */
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

