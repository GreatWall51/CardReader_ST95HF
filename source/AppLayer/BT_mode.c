/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 BT_mode.c 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/06/23
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/23, liuwq create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/
#include "BT_mode.h"
#include "hal_BT.h"
#include "rs485_drv.h"
#include "string.h"
#include "timer_drv.h"
/*---------------------- Constant / Macro Definitions -----------------------*/
#define    TRUE    1
#define    FALSE   0
#define BT_FRAME_BUFFER_NUM 2  //蓝牙数据缓冲区个数
#define CHECK_BT_VERSION  0x84
/*----------------------- Type Declarations ---------------------------------*/
typedef struct {
    uint8_t date_sign;            //是否有数据的标志
    uint8_t len;
    uint8_t data_buff[BT_RECV_MAX_LEN];
}BT_DATE_BUFF_T;

uint8_t bt_recv_len = 0;
uint8_t bt_recv_buf[BT_RECV_MAX_LEN] = {0};

/*------------------- Global Definitions and Declarations -------------------*/
 BT_DATE_BUFF_T bt_date_buffer[BT_FRAME_BUFFER_NUM];
static uint8_t bt_link_timer = 0;        //蓝牙连接计时
static uint8_t bt_link_timeout = 0;      //蓝牙连接超时(2s)计时器
static uint8_t bt_func_flag = 0;         //主机是否支持蓝牙读头
static uint8_t bt_init_success_flag = 0; //蓝牙初始化完成标志1->完成,0->未完成
/*----------------------- Variable Declarations -----------------------------*/
 
 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name: 	 bt_frame_transmit_buffer 
*
* Function Desc: 	 缓存蓝牙数据帧
* Input Param: 	     len:蓝牙数据帧的总长度
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/06/26, Create this function by liuwq
******************************************************************************/
void bt_frame_transmit_buffer(uint8_t len,uint8_t *date_buff)
{
    uint8_t i;
    if(len>BT_RECV_MAX_LEN) return;

    for(i=0;i<BT_FRAME_BUFFER_NUM;i++)
    {
        if(bt_date_buffer[i].date_sign == FALSE)
        {
						bt_date_buffer[i].date_sign = TRUE;
            bt_date_buffer[i].len = len;
            memcpy(bt_date_buffer[i].data_buff,date_buff,len);
            return;
        }
    }
}
/******************************************************************************
* Name: 	 bt_frame_deal_real_time 
*
* Function Desc: 	 转发数据到蓝牙模块
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/06/26, Create this function by liuwq
******************************************************************************/

void bt_frame_deal_real_time(void)
{
    uint8_t i;
    bt_fram_t *bt_fram;
	
	  bt_link_timer = 0;
    for(i=0;i<BT_FRAME_BUFFER_NUM;i++)
    {
        if(bt_date_buffer[i].date_sign == TRUE)
        { 
            bt_fram = (bt_fram_t *)(bt_date_buffer[i].data_buff);
//    		if(bt_fram->cmd == CHECK_BT_VERSION) 
    		{
    			bt_func_flag=1;
    		}
            bt_send_fram(&bt_fram->head,bt_fram->len_low+4);
            bt_date_buffer[i].date_sign  = FALSE;  //清除数据缓冲区标志位
            i = BT_FRAME_BUFFER_NUM;   //一次只处理一个缓冲区数据，回主循环。
//					bt_link_timer = 1;//蓝牙有数据上传,重新开始计时,无数据上传500ms就关闭蓝牙
        }
    }
}
void disconnet_bt(void)
{
	uint8_t disconnect[5]={0xAA,0x01,0x00,0x83,0x7C};//断开连接指令
	bt_send_fram(disconnect,5);
//	rs485_send_frame(RS485_SIM,disconnect,5);
}
void check_bt_link_real_time(void)
{
	if(bt_link_timer >= 8) //512ms内没有数据交互,断开连接
	{
		bt_link_timeout = 0;
		bt_link_timer = 0;
		//发送断开蓝牙连接指令
//		disconnet_bt();
	}
	if(bt_link_timeout >= 70)//蓝牙连接超过4.5秒,断开连接
	{
		bt_link_timeout = 0;
		bt_link_timer = 0;
		disconnet_bt();
	}
	if(!IS_TIMER_EVT(TIME_EVT_64)) return;
	if(bt_link_timer > 0) 
		bt_link_timer++;
	if(bt_link_timeout > 0) 
		bt_link_timeout++;
}
/******************************************************************************
* Name: 	 bt_get_fram_real_time 
*
* Function Desc: 	 转发蓝牙模块接收的数据到主机
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/06/26, Create this function by liuwq
******************************************************************************/
//#include "sim_uart_drv.h"
void bt_report_fram_real_time(void)
{
    rs485_frame_msg *p_msg;
    bt_fram_t *bt_fram;
    if(bt_get_fram(bt_recv_buf,&bt_recv_len) != 1) return;

//		rs485_send_frame(RS485_ID1,bt_recv_buf,bt_recv_len);//测试,转发蓝牙接收到的数据到串口1
        bt_fram = (bt_fram_t *)bt_recv_buf;
	 if((bt_fram->cmd == 0x51) && (bt_fram->payload == 0x00)) 
	 {
        bt_init_success_flag = 1;
	 }
	 if(bt_init_success_flag != 1) return;
	 if(bt_func_flag != 1) return;
	 bt_link_timer = 1;//蓝牙有数据上传,重新开始计时,无数据上传500ms就关闭蓝牙
		
	//有蓝牙连接上读头,启动计时
	 if((bt_fram->cmd == 0x42) && (bt_fram->payload == 0x01)) 
	 {
//		 rs485_send_frame(RS485_SIM,bt_recv_buf,bt_recv_len);
		 bt_link_timeout = 1;
	 }
	
	//蓝牙离线,停止计时
	 if((bt_fram->cmd == 0x42) && (bt_fram->payload == 0x00)) 
	 {
		 bt_link_timer = 0;
		 bt_link_timeout = 0;
//		 disconnet_bt();
	 }
		
    p_msg = append_frame_queue(80);     //获取队列地址
    if(p_msg == NULL) return;	    
    //上报数据帧
    p_msg->len = 7+bt_recv_len;
    bt_fram = (bt_fram_t *)set_frame_head(p_msg->pbuf, BT_REPORD_DATA_CMD, p_msg->len-1, 0);    
    memcpy(bt_fram,bt_recv_buf,bt_recv_len);
    p_msg->pbuf[p_msg->len-1] = get_check_sum(p_msg->pbuf,p_msg->len);
}
void bt_mode_init(void)
{
    hal_bt_init();
    memset(bt_date_buffer,0,sizeof(bt_date_buffer));
//    memset(bt_recv_buf,0,BT_RECV_MAX_LEN);
}
void bt_mode_real_time(void)
{
    bt_frame_deal_real_time();
    bt_report_fram_real_time();
    check_bt_link_real_time();
}
 
/*---------------------------------------------------------------------------*/

