/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName: 	 hal_BT.c 
* Desc:
* 
* 
* Author: 	 liuwq
* Date: 	 2017/06/14
* Notes: 
* 
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/14, liuwq create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/
#include <string.h>
#include "hal_BT.h"
#include "com_hal_layer.h"
#include "define.h"
#include "debug.h"
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/


/*------------------- Global Definitions and Declarations -------------------*/
static BT_FRAM_T bt_fram;
BT_FRAM_T *p_hal_bt_fram = NULL;
REV_BT_DATE_BUFF hal_bt_date[BT_DATE_BUFFER_NUM];
/*----------------------- Variable Declarations -----------------------------*/

 
/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/
 
/******************************************************************************
* Name: 	 get_bluetooth_check_sum 
*
* Function Desc: 	 计算蓝牙数据帧校验和
* Input Param: 	     point:蓝牙数据帧,len有效数据长度(不包括帧头,长度,和检验和)
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/07/22, Create this function by liuwq
******************************************************************************/
uint8_t get_bluetooth_check_sum(uint8_t *point, uint8_t len)
{
    uint8_t *pbuf;
    uint8_t i;
    uint8_t sum = 0;

    if(point == NULL || len  < 1) return 0;

    pbuf = (uint8_t *)point;
    
    for(i=3; i<(3+len); i++)
    {
        sum += *(pbuf+i);
    }
    return ((~sum)&0xff);
}

/******************************************************************************
* Name: 	 check_bluetooth_sum 
*
* Function Desc: 	 判断蓝牙数据帧校验和是否正确
* Input Param: 	 len有效数据长度(不包括帧头,长度,和检验和)
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/07/22, Create this function by liuwq
******************************************************************************/
uint8_t check_bluetooth_sum(uint8_t *point, uint8_t len)
{
    uint8_t i;
    uint8_t *pbuf;
    uint8_t sum = 0;

    if(point == NULL || len  < 1) return 0;
    pbuf = point;
    for(i=3; i<(3+len); i++)
    {
        sum += *(pbuf+i);
    }
    if(((~sum)&0xff) == *(pbuf+3+len))
    {
        return 1;     
    }
    return 0;    
}
/******************************************************************************
* Name: 	 bt_rw_buffer 
*
* Function Desc: 	 
* Input Param: 	 len:蓝牙数据帧总长度(包括帧头,长度,和检验和)
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/07/22, Create this function by liuwq
******************************************************************************/
void bt_rw_buffer(uint8_t len,uint8_t *date_buff)
{
    uint8_t i;
    if(len>BT_RECV_MAX_LEN) return;

    for(i=0;i<BT_DATE_BUFFER_NUM;i++)
    {
        if(hal_bt_date[i].date_sign == FALSE)
        {
            hal_bt_date[i].len = len;
            memcpy(hal_bt_date[i].data_buff,date_buff,len);
            hal_bt_date[i].date_sign = TRUE;
            return;
        }
    }
}
#include "rs485_drv.h"
void recv_bt_fram_real_time(void)
{
    static uint8_t data_len=0;
    uint8_t in_data=0,interval=0;
    p_hal_bt_fram = &bt_fram;
	while(com_hal_have_new_data(PORT_BT))
	{
		in_data = com_hal_receive_data(PORT_BT, &interval);
		if(interval >= 5)//10ms内没收到数据就认为一帧接收完成
		{
			p_hal_bt_fram->index = 0;
		}
//		ENTER_CRITICAL() ; 
		switch(p_hal_bt_fram->index)
        {
            case 0: /* waite for header */
            {
                if(in_data == 0xaa)
                {
                    p_hal_bt_fram->r_frame[p_hal_bt_fram->index++] = in_data;
                }
                else
                {
                    p_hal_bt_fram->index = 0;
                }
    							
            }break;
            case 1: /* we get len1  */
            {
                if((in_data > (BT_RECV_MAX_LEN-4)) || (in_data < 1)) /* overflow*/
                {
                    p_hal_bt_fram->index = 0;
                }
                else
                {
                    p_hal_bt_fram->r_frame[p_hal_bt_fram->index++] = in_data;
                }
    //								debug("get len1 = %x!\r\n",in_data);
            }break;
    		case 2: /* we get len2  */
            {
    		      p_hal_bt_fram->r_frame[p_hal_bt_fram->index++] = in_data;
            }break;
            default: /* receive data untill the end  */
            {
    //								debug("get data = %x\r\n",in_data);
                p_hal_bt_fram->r_frame[p_hal_bt_fram->index++] = in_data;
                if(p_hal_bt_fram->index >= (p_hal_bt_fram->r_frame[1] + 4))
                {
                    data_len = p_hal_bt_fram->r_frame[1] + 4;
//                    rs485_send_frame(RS485_ID1,&data_len,1);
                    bt_rw_buffer(data_len,p_hal_bt_fram->r_frame);
                    p_hal_bt_fram->index = 0;
//					if(check_bluetooth_sum(p_hal_bt_fram->r_frame, p_hal_bt_fram->r_frame[1]))
//					{

//					   rs485_send_frame(RS485_ID1,p_hal_bt_fram->r_frame,p_hal_bt_fram->index);
                        
//						//数据接收完成且校验正确
//						bt_rw_buffer(p_hal_bt_fram->index,p_hal_bt_fram->r_frame);
                        
//                       p_hal_bt_fram->index = 0;
//						memset(&bt_fram,0,sizeof(BT_FRAM_T));
						
//					}
                }
            }break;
        }
//        EXIT_CRITICAL();
}
}
/******************************************************************************
* Name: 	 bt_get_fram 
*
* Function Desc: 	 
* Input Param: 	 
* Output Param: 	 
* Return Value: 	 
* Global: 	 
* Note: 	 每次处理一帧
* Author: 	 liuwq
* ----------------------------------------------------------------------------
* Log: 	 2017/07/22, Create this function by liuwq
******************************************************************************/
uint8_t bt_get_fram(uint8_t *buf,uint8_t *len)
{
    uint8_t i;
    if(len == NULL || buf == NULL) return 0;

    for(i=0;i<BT_DATE_BUFFER_NUM;i++)
    {
        if((hal_bt_date[i].date_sign == TRUE) && (com_hal_is_sending(RS485_ID1) == 0))
        {
//             rs485_send_frame(RS485_ID1,hal_bt_date[i].data_buff,hal_bt_date[i].len);
            memcpy(buf,hal_bt_date[i].data_buff,hal_bt_date[i].len);
            *len = hal_bt_date[i].len;
            hal_bt_date[i].date_sign = FALSE;//清除数据缓冲区标志位
            i = BT_DATE_BUFFER_NUM;            //一次只处理一个缓冲区数据，回主循环。
            return TRUE;
        }
    }
	return FALSE;
}



void bt_send_fram(uint8_t *buf,uint8_t len)//len包括帧头,长度和校验码
{
	cbi(GPIOA,GPIO_Pin_8);
//	if(len == (buf[1]+4))
	{
		com_hal_send(PORT_BT,buf,len);
	}
}
void hal_bt_init(void)
{
    uint16_t index=0;
    memset(&bt_fram,0,sizeof(BT_FRAM_T));
    for(index=0;index < BT_DATE_BUFFER_NUM;index++)
    {
        memset(&hal_bt_date[index],0,sizeof(REV_BT_DATE_BUFF));
    }
    
}
/*---------------------------------------------------------------------------*/

