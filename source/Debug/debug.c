/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : debug.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年7月15日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年7月15日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"  
#include "timer_drv.h"
#include "define.h"
#include "hal_timer.h" 
#include "debug.h"
#include "debug_uart.h"
#include "drv_interrupt.h"
#include "beep_drv.h"
#include "led_drv.h"
#include "debug.h"

/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Variable Define/Declarations ----------------------*/
uint8_t g_fm23_init_flg;

int g_dbg_last_line = 0;
int g_dbg_pre_line = 0;
char* g_dbg_last_file = NULL;
char* g_dbg_pre_file = NULL;
uint32_t g_dbg_chk_timeout = 0;
size_t g_dbg_chk_val = 0;

/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

void test_irq_output(void)
{
	static uint8_t flag = 0;

    
    if(IS_TIMER_EVT(TIME_EVT_512))
    {
        if(flag == 0)
        {
             cbi(GPIOB,GPIO_Pin_13); 
             flag = 1;
        }
        else
        {
            sbi(GPIOB,GPIO_Pin_13); 
            flag = 0;
        }
    }
}
 void test_led(void)
 {
    static uint8_t flag = 0;

    
    if(IS_TIMER_EVT(TIME_EVT_512))
    {
        if(flag == 0)
        {
             sbi(GPIOA,GPIO_Pin_0); 
             flag = 1;
        }
        else
        {
            cbi(GPIOA,GPIO_Pin_0); 
            flag = 0;
        }
    }
 }
#include "drv_interrupt.h"
void delay_test(void)
{
	sbi(GPIOA,GPIO_Pin_0); 
	delay_ms(500);
	cbi(GPIOA,GPIO_Pin_0); 
	delay_ms(500);
}
#include "card_rw_drv.h"
void card_pro_read_test(void)
{
	uint8_t uid[4] = {0x4e,0xd2,0x1d,0xa4};
	uint8_t keyA[6] = {0xA4,0xBD,0x0E,0x0F,0x5A,0xBF};
	IC_BLOCK_RW_STR card_rw_str;
    IC_RESULT_STR *card_rw_result;
	card_rw_str.cmd_aux = 0x11;
	memcpy(card_rw_str.uid,uid,4);
	card_rw_str.block_cn = 5;
	card_rw_str.key_cn = 0;//0x60;
	memcpy(card_rw_str.key,keyA,6);
	card_ic_read(&card_rw_str);
	card_rw_result = (IC_RESULT_STR *)&card_rw_str;
	debug_print(card_rw_result->payload,16);
}

void card_pro_write_test(void)
{
	uint8_t uid[4] = {0x4e,0xd2,0x1d,0xa4};
	uint8_t keyB[6] = {0x9E,0x13,0x66,0x95,0xf0,0x0A};
	uint8_t write_buf[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16};
	IC_BLOCK_RW_STR card_rw_str;
    IC_RESULT_STR *card_rw_result;

    
	card_rw_str.cmd_aux = 0x12;
	memcpy(card_rw_str.uid,uid,4);
	card_rw_str.block_cn = 5;
	card_rw_str.key_cn = 0;//0x61;
	memcpy(card_rw_str.key,keyB,6);
	memcpy(card_rw_str.block_buffer,write_buf,16);
	card_ic_write(&card_rw_str,DATA_USER);
	card_rw_result = (IC_RESULT_STR *)&card_rw_str;
	if(card_rw_result->result== 1)
	{
	    debug("write card success!\r\n");
	}
	else
	{
	    debug("write card error!\r\n");
	}
}

#include "param_mode.h"
void test_param(void)
{
    
     debug("sys param is:\r\n");
     debug_print( get_anti_copy_param(),20);
}



#include "card_read_drv.h"
extern void card_write_test(void);
extern void card_read_test(void);
void test_all_card(void)
{
	static uint8_t init = 1;
	CARD_INF0_T card_info;
	CARD_PARA_STR cara_para;
	uint8_t prj_num[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
											 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

	if(init == 1)
	{
		init =0;
		cara_para.card_ab_type = 1;//1->支持身份证和IC卡,0->只支持IC卡
		cara_para.anti_copy = 0x00;//防复制参数0x80-0x8F
		memcpy(cara_para.project_num,prj_num,16);
		cara_para.adf[0]=0x10;
		cara_para.adf[0]=0x05;
		cara_para.key_inter = 0x05;
		card_read_set_para(&cara_para);
		card_rw_set_para(&cara_para);
	}
	
	if(card_get_event(&card_info))
	{
		led_set_mode(LED_CARD_READ_MODE);
		beep_set_mode(BEEP_OPEN_READ_MODE);
		
		
		debug("[%d] Type %x Card UID is :\r\n",__LINE__,card_info.card_type);
		debug_print(card_info.card_uid,4);
//		test_param();
		
//		card_read_test();
        //需要打开读写函数的结果上报
//      card_pro_read_test();
//		card_pro_write_test();
	}
}

#include "lib_nfctype5pcd.h"
void test_15693_read_write(void)
{
//    ISO15693_read_sms();
//    ISO15693_read_text();
//    ISO15693_read_geo();
//    ISO15693_read_URI();
//    ISO15693_write_sms();
//    ISO15693_write_text();
//    ISO15693_write_geo();
//    ISO15693_write_URI();
}



void debug_init()
{
    debug_pin_init();
    debug_uart_init();
    debug("test .............\r\n");
}
void debug_callback(uint8_t *p_frame)
{

}
void debug_real_time(void)
{  
	
	test_all_card();
//	test_15693_read_write();
}
void debug_print(uint8_t* buf,uint8_t len)
{
    uint8_t index;
    if(buf == NULL || len <= 0) return;
    for(index=0;index<len;index++)
    {
        debug("%x ",buf[index]);
    }
    debug("\r\n");

}

