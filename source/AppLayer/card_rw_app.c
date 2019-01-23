/********************************************************************************
**
** 文件名:     	 card_cpu_exe.c 
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
#include "card_read_drv.h"
#include "stm32f0xx.h"
#include "rs485_drv.h"
#include "card_rw_app.h"
#include "param_mode.h"
#include "debug.h"


/*----------------------- Constant / Macro Definitions -----------------------*/
#define    TRUE    1
#define    FALSE   0

#define DATE_BUFFER_NUM    2
#define CPU_IN_AUTH_RETRY_TIMES 5 //内部认证尝试次数

/*----------------------- Constant / Macro Definitions -----------------------*/



typedef struct REV_DATE_BUFF{
    uint8_t date_sign;            //是否有数据的标志
    uint8_t len;
    uint8_t data_buff[DATE_BUFF_MAX];
}REV_DATE_BUFF_STR;
 
/*------------------------ Variable Declarations -----------------------------*/

REV_DATE_BUFF_STR date_buffer[DATE_BUFFER_NUM];

extern uint8_t interval_timer_sfz;
extern uint8_t interval_timer_ic; 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/


/*****************************************************************************
 函 数 名  : card_cpu_init
 功能描述  : 参数初始化
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
void card_rw_init(void)
{
    CARD_PARA_STR card_para;
    
    date_buffer[0].date_sign = FALSE;
    date_buffer[0].len = 0;  
    date_buffer[1].date_sign = FALSE;
    date_buffer[1].len = 0;  
    memcpy(card_para.project_num,get_anti_copy_param(),16);
    card_rw_set_para(&card_para);
}
/*****************************************************************************
 函 数 名  : card_operate_buffer
 功能描述  : 卡操作通信数据获取
 输入参数  : uint8_t len         
             uint8_t *date_buff  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月22日
    作    者   : yeqizhi
    修改内容   : 新生成函数

*****************************************************************************/

void card_rw_buffer(uint8_t len,uint8_t *date_buff)
{
    uint8_t i;
    if(len>DATE_BUFF_MAX)return;
    
    //打印数据长度
    for(i=0;i<DATE_BUFFER_NUM;i++)
    {
        if(date_buffer[i].date_sign == FALSE)
        {
            date_buffer[i].len = len;
            memcpy(date_buffer[i].data_buff,date_buff,len);
            date_buffer[i].date_sign = TRUE;
            return;
        }
    }
}

/*****************************************************************************
 函 数 名  : card_cpu_real_time
 功能描述  : CPU卡接口功能的执行
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
void card_rw_real_time(void)
{
    uint8_t  len,i;
    frame_head_t  *frame_head;
//    uint32_t read_start;
//    uint32_t read_end;
//    uint32_t write_start;
//    uint32_t write_end;

    for(i=0;i<DATE_BUFFER_NUM;i++)
    {      
        len = 0;
        if(date_buffer[i].date_sign == TRUE)
        {   
//            if(get_reader_mode() != IC_MODE)   return;

            frame_head = (frame_head_t *)(date_buffer[i].data_buff);
            switch(frame_head->payload)
            {
                case IC_BLOCK_WRITE:
//                read_start = get_current_time();
                    len = card_ic_write((IC_BLOCK_RW_STR *)&frame_head->payload,DATA_USER);
//                    read_end = get_current_time();
//                    debug("read total time : %d ms\r\n",read_end-read_start);
                    break;
                case IC_BLOCK_READ:
//                write_start = get_current_time();
                    len = card_ic_read((IC_BLOCK_RW_STR   *)&frame_head->payload);
//                    write_end = get_current_time();
//                    debug("write total time : %d ms\r\n",write_end-write_start);
                    break;
                case CPU_IN_AUTH :
				{
					uint8_t retry_time = 0;
					uint8_t result = FAIL;
					for(retry_time = 0;retry_time<CPU_IN_AUTH_RETRY_TIMES;retry_time++)
					{
						result = cpu_internal_auth((CPU_IN_AUTH_T *)&frame_head->payload);
						if(result == SUCCESS) break;
					}
					
					{//结果上报
						rs485_frame_msg *p_msg;
	                    CPU_IN_AUTH_RESULT_STR *frame_data;
						CPU_IN_AUTH_T* in_auth;  
						
						in_auth = ( CPU_IN_AUTH_T* ) &frame_head->payload;
						p_msg = append_frame_queue(40);     //获取队列地址
						if(p_msg == NULL) return;	    
						p_msg->len = 16;
						frame_data = (CPU_IN_AUTH_RESULT_STR *)set_frame_head(p_msg->pbuf, CARD_RESULT_UPDATA_CMD, p_msg->len-1, 0);    
						memcpy(frame_data, &frame_head->payload, 7);
						frame_data->cmd_aux = CPU_IN_AUTH;
						frame_data->result = in_auth->result;
						frame_data->result_expand = in_auth->result_expand;
						p_msg->pbuf[p_msg->len-1] = get_check_sum(p_msg->pbuf,p_msg->len);
					}
				}
				
                    break;
                case CPU_EF_WRITE :
//                    len = cpu_ef_write((CPU_WRITE_STR *)&frame_head->payload);
                    break;
                case CPU_EF_READ :
//                    len = cpu_ef_read((CPU_READ_STR*)&frame_head->payload);
                    break;
                default:
                    break;
            }
            
            if(len)
            {
            //上报结果
//                p_msg = append_frame_queue(len+10);//len+7
//                if(p_msg == NULL) return;	
//                ic_rw_result = (IC_RESULT_STR *)&frame_head->payload;
//                if(ic_rw_result->result == SUCCESS)
            }
            
            date_buffer[i].date_sign  = FALSE;  //清除数据缓冲区标志位
            i = DATE_BUFFER_NUM;   //一次只处理一个缓冲区数据，回主循环。

            interval_timer_sfz = SFZ_INTERVAL_TIME;
            interval_timer_ic = IC_INTERVAL_TIME;
        }
    }
}
/*---------------------------------------------------------------------------*/

