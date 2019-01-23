/********************************************************************************
**
** �ļ���:     	 card_cpu_exe.c 
** ��Ȩ����:   (c) 	 2016 �������ֿƼ����޹�˾
** �ļ�����:   
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**| ����        | ����     |  �޸ļ�¼
**===============================================================================
**| 2016/04/21  | yeqizhi  |  �������ļ�
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
#define CPU_IN_AUTH_RETRY_TIMES 5 //�ڲ���֤���Դ���

/*----------------------- Constant / Macro Definitions -----------------------*/



typedef struct REV_DATE_BUFF{
    uint8_t date_sign;            //�Ƿ������ݵı�־
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
 �� �� ��  : card_cpu_init
 ��������  : ������ʼ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��22��
    ��    ��   : yeqizhi
    �޸�����   : �����ɺ���

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
 �� �� ��  : card_operate_buffer
 ��������  : ������ͨ�����ݻ�ȡ
 �������  : uint8_t len         
             uint8_t *date_buff  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��22��
    ��    ��   : yeqizhi
    �޸�����   : �����ɺ���

*****************************************************************************/

void card_rw_buffer(uint8_t len,uint8_t *date_buff)
{
    uint8_t i;
    if(len>DATE_BUFF_MAX)return;
    
    //��ӡ���ݳ���
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
 �� �� ��  : card_cpu_real_time
 ��������  : CPU���ӿڹ��ܵ�ִ��
 �������  : void  
 �������  : ��
 �� �� ֵ  : 
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2016��4��22��
    ��    ��   : yeqizhi
    �޸�����   : �����ɺ���

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
					
					{//����ϱ�
						rs485_frame_msg *p_msg;
	                    CPU_IN_AUTH_RESULT_STR *frame_data;
						CPU_IN_AUTH_T* in_auth;  
						
						in_auth = ( CPU_IN_AUTH_T* ) &frame_head->payload;
						p_msg = append_frame_queue(40);     //��ȡ���е�ַ
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
            //�ϱ����
//                p_msg = append_frame_queue(len+10);//len+7
//                if(p_msg == NULL) return;	
//                ic_rw_result = (IC_RESULT_STR *)&frame_head->payload;
//                if(ic_rw_result->result == SUCCESS)
            }
            
            date_buffer[i].date_sign  = FALSE;  //������ݻ�������־λ
            i = DATE_BUFFER_NUM;   //һ��ֻ����һ�����������ݣ�����ѭ����

            interval_timer_sfz = SFZ_INTERVAL_TIME;
            interval_timer_ic = IC_INTERVAL_TIME;
        }
    }
}
/*---------------------------------------------------------------------------*/

