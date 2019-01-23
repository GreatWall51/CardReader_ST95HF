/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : debug.h
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
#ifndef _DEBUG_H_
#define  _DEBUG_H_
/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"   
#include <stdio.h> 

/*----------------------- Constant / Macro Definitions -----------------------*/
#define DEBUG_COM_CONFIG     0 //debug串口为串口2，波特率115200，用于蓝牙的
#define CFG_ENABLE_PRINTF_DBG      1

extern int g_dbg_last_line;
extern char* g_dbg_last_file;

extern int g_dbg_pre_line;
extern char* g_dbg_pre_file;

extern uint32_t g_dbg_chk_timeout;
extern size_t g_dbg_chk_val;

#define sbi(ADDRESS,BIT) 	((ADDRESS->BSRR = BIT))	// Set bit
/*------------------------ Variable Define/Declarations ----------------------*/


/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
#if DEBUG_COM_CONFIG
#define debug(P...)  do{\
                    sbi(GPIOA,GPIO_Pin_1);\
                     printf(P);\
                     }while(0);//fnSendStr(P) 

#define debug_hex(data,len) do{ const int l=(len); int x;        \
                    for(x=0 ; x<l ; x++) debug("0x%02x ",*((data)+x));   \
                    debug("\r\n");}while(0)

#define debug_char(data,len) do{ const int l=(len); int x;     \
                    for(x=0 ; x<l ; x++) debug("%c",*((data)+x));      \
                    debug("\r\n");}while(0)

#define debug_dec(data,len) do{ const int l=(len); int x;     \
                     for(x=0 ; x<l ; x++) debug("%d ",*((data)+x));    \
                     debug("\r\n");}while(0)
#else
#define debug(P...) do{}while(0)//
#define debug_hex(data,len)         do{}while(0)
#define debug_char(data,len)        do{}while(0)
#define debug_dec(data,len)        do{}while(0)
#endif

#if CFG_ENABLE_PRINTF_DBG
#define debug_dead(P...)  do{\
                    sbi(GPIOA,GPIO_Pin_1);\
                     printf(P);\
                     }while(0);//fnSendStr(P) 
/* 定时监视变量值 */
#define DEBUG_CHK_LINE_VAL(VAL) do{g_dbg_chk_val = (size_t)(VAL);}while(0)

/* 在要监视处执行 */
#define DEBUG_CHK_LINE() do{\
    g_dbg_pre_file= g_dbg_last_file;\
    g_dbg_pre_line = g_dbg_last_line;\
    g_dbg_last_file = __FILE__;\
    g_dbg_last_line = __LINE__;\
    g_dbg_chk_timeout = 0;\
}while(0)

/* 在Timer中断内执行，TIME_OUT为超时打印计数*ms */
#define DEBUG_CHK_LINE_TIMER_INT(TIME_OUT) do{\
    if(g_dbg_chk_timeout > TIME_OUT)\
    {\
        debug_uart_disable_int();\
        debug_dead("![%x]! %s:%d -> %s:%d ->!\r\n", g_dbg_chk_val, g_dbg_pre_file, g_dbg_pre_line, g_dbg_last_file, g_dbg_last_line);\
        g_dbg_chk_timeout = 0;\
    }\
    g_dbg_chk_timeout++;\
}while(0)

#else
#define debug_dead(P...) do{}while(0)//
#define DEBUG_CHK_LINE() do{}while(0)
#define DEBUG_CHK_LINE_VAL(VAL) do{}while(0)
#define DEBUG_CHK_LINE_TIMER_INT(TIME_OUT) do{}while(0)
#endif


void debug_init(void);
void debug_callback(uint8_t *p_frame);
void debug_real_time(void);
void debug_print(uint8_t* buf,uint8_t len);

#endif

