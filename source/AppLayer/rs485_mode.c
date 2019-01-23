/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : rs485_mode.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月25日
  最近修改   :
  功能描述   : rs485通信模块
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月25日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "led_drv.h" 
#include "timer_drv.h"
#include "define.h"
#include "rs485_drv.h"
#include "param_mode.h"
#include "check_mode.h"
#include <string.h>
#include <stdlib.h>
#include "check_io_drv.h"
#include "beep_drv.h"
#include "lock_drv.h"
#include "check_card_mode.h"  
#include "rs485_mode.h" 
#include "flash_drv.h"
#include "card_rw_drv.h"
#include "card_rw_app.h"
#include "BT_mode.h"
/*----------------------- Constant / Macro Definitions -----------------------*/
#include "debug.h"
/*------------------------ Variable Define/Declarations ----------------------*/

typedef struct
{
    uint8_t old_frame_id;
    uint8_t old_dec_addr;
    uint16_t timer;
}frame_info_t;

typedef struct
{
    uint16_t s_link_sta_timer;
    uint16_t link_sta_timer;
    uint8_t link_status_buff[8];
    
}link_info_t;


static com_mac_send_pool_item_t  s_mac_send_pool[CFG_COM_MAC_POOL_SIZE]; 
static com_mac_send_pool_item_t *p_mac_vacancy_pool_header;
com_mac_port_t s_mac_port;

link_info_t   link_info;
frame_info_t  frame_info;

uint8_t ver_buf[CFG_COM_MAC_PACKET_SIZE];

ANTI_COPY_CMD_EVT_E anti_copy_evt;
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/


/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
extern uint8_t anti_copy_info[20];
extern rs485_frame_msg *queue_frame_head;
extern com_mac_send_packet_t *com_mac_send(void);

static void set_link_time(void)
{
     link_info.link_sta_timer = 0;        
}

static uint8_t get_link_sta(void)
{
	if(link_info.link_sta_timer > 23)   // 23 * 128ms = 3s
	{
        return CLOSE;
	}
	return OPEN;   
}


static void check_frame_id_valid_time(void)
{

    static uint16_t time = 0;

    if(!IS_TIMER_EVT(TIME_EVT_1)) return;
    
    if(time++ < 1000)   return;  //id无效时间
        time = 0;
        
    if(frame_info.timer > 0)
    {
        ENTER_CRITICAL() ;
        frame_info.timer--;                     
        if(frame_info.timer == 0)
        {
            frame_info.old_dec_addr = 0x10;
            frame_info.old_frame_id = 0;
        }
        EXIT_CRITICAL();
    }
    
}


void free_frame_queue(void)
{
     rs485_frame_msg *move_point;
     
     ENTER_CRITICAL() ;
     while(queue_frame_head != NULL)   
     {                               
        move_point =  queue_frame_head;
        queue_frame_head =  queue_frame_head->next;                  
        move_point->next = NULL;  
        del_frame_queue(move_point);
     }
     EXIT_CRITICAL();
}
/*****************************************************************************
 函 数 名  : check_rs485_link_real_time
 功能描述  : 查询rs485通信链路
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月27日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void check_rs485_link_real_time(void)
{
    link_status_frame_t *link_status_data;
    frame_head_t *frame_head;
    
    com_mac_send_packet_t *p_packet;
    
    if(!IS_TIMER_EVT(TIME_EVT_128)) return;       


    if(link_info.link_sta_timer < 39)       //39 * 128 = 5s
    {     
        ENTER_CRITICAL();
        link_info.link_sta_timer++;
        EXIT_CRITICAL();
    }
    
    if(link_info.link_sta_timer >= 39)     //通信断路5s释放数据
    {                       
        free_frame_queue();
    }       

    if(link_info.s_link_sta_timer < 78)    //每10s发送一次数据 78 *128ms = 10s
    {
        link_info.s_link_sta_timer++;
    }
    
    if(link_info.s_link_sta_timer >= 78)     //10s
    {
        link_info.s_link_sta_timer = 0;   

        ENTER_CRITICAL() ;
        p_packet = com_mac_send();
        EXIT_CRITICAL();

        if(p_packet == NULL) return;
        
        p_packet->flag.all = 0;
        p_packet->len = 8;
        p_packet->times = 1;                   
        frame_head = (frame_head_t *)p_packet->data;
        
        frame_head->head = 0xaa;
        frame_head->len   = 7;
        frame_head->dest_addr = 0x8f;
        frame_head->src_addr  = get_reader_addr();
        frame_head->frame_id  = set_frame_id(RS485_SIM);
        frame_head->cmd = LINK_CMD;
        
        link_status_data = (link_status_frame_t *)&frame_head->payload;       
        link_status_data->link_status = get_link_sta();
        p_packet->data[p_packet->len-1] = get_check_sum(p_packet->data, p_packet->len);
    } 
}

/*****************************************************************************
 函 数 名  : com_mac_push_vacancy_pool
 功能描述  : 回收已发送节点
 输入参数  : com_mac_send_pool_item_t * p_recycle  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static  void com_mac_push_vacancy_pool(com_mac_send_pool_item_t * p_recycle)
{

    if(!p_recycle) return;

    p_recycle->packet.times = 0;
    p_recycle->next = p_mac_vacancy_pool_header;
    p_mac_vacancy_pool_header = p_recycle;

    return;
}

/*****************************************************************************
 函 数 名  : com_mac_pop_vacancy_pool
 功能描述  : 获取节点
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static com_mac_send_pool_item_t * com_mac_pop_vacancy_pool()
{
   
    com_mac_send_pool_item_t * p;

    if(p_mac_vacancy_pool_header == NULL)       
    {
        while(1)
        {  
             led_off();      
        }       
      //  return NULL; /* pool is empty */
    }
    p = p_mac_vacancy_pool_header;
    p_mac_vacancy_pool_header = p->next;
        
    return p;
}


/*****************************************************************************
 函 数 名  : com_mac_assign_send_packet
 功能描述  : 将节点加入待发送队列
 输入参数  : com_mac_send_pool_item_t * p  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void com_mac_assign_send_packet(com_mac_send_pool_item_t * p)
{

    p->next = NULL;
    
    if((s_mac_port.p_send_head == NULL) ||(s_mac_port.p_send_tail == NULL))
    {
        /*!< Send queue is empty */
        s_mac_port.p_send_head = p;
        s_mac_port.p_send_tail = p;         
    }
    else
    {
        
        s_mac_port.p_send_tail->next = p;
        s_mac_port.p_send_tail = p;
    }

}

com_mac_send_packet_t *com_mac_send(void)
{
    com_mac_send_pool_item_t * p;

    p = com_mac_pop_vacancy_pool();
    if(!p) return 0;	/*!< Pool empty, return fail */

   	/*!< Add to the tail of spec port's send queue */
    com_mac_assign_send_packet(p);

    return &(p->packet);
}


static void set_rev_evt(ENUM_COM_MAC_EVT_T evt)
{
    s_mac_port.evt = evt;
}

static ENUM_COM_MAC_EVT_T get_rev_evt(void)
{
    return s_mac_port.evt;
}


static void ver_frame_to_queue(void)
{

    rs485_frame_msg *p_msg;
    frame_head_t *frame_head;
    
    /* 分配一个链表节点 */
	frame_head = (frame_head_t *)ver_buf;
    p_msg = append_frame_queue((frame_head->len+1)); 

    if(p_msg == NULL) return;

   // if((frame_head->len+1) > MALLOC_LEN) return;

    memcpy(p_msg->pbuf, ver_buf, (frame_head->len+1));
    /* 获取数据总长度 */
    p_msg->len = (frame_head->len+1);      
}

void rs485_send_real_time(void)
{
    com_mac_port_t * ptr;

    /* mac layer process*/
    ptr = &s_mac_port;

    
    if(get_rev_evt() == EVT_COM_MAC_N_FRAME)
    {
				ENTER_CRITICAL();
        ptr->sending.packet.flag.all = 0;
        ptr->sending.packet.times = 0;
        ver_frame_to_queue();
				EXIT_CRITICAL();
    }
    /* Clear event */
    ptr->evt = EVT_COM_MAC_NONE;
    
    
    /*!< Sending... */
    if(ptr->sending.packet.times == 0)
    {
       	/*!< the sending queue is not empty */
        if(ptr->p_send_head != NULL)
       	{      
            ptr->sending = *ptr->p_send_head;	/*!< copy the packet */
    	    /*!< recycle the send pool */
       	    com_mac_push_vacancy_pool(ptr->p_send_head);
   	        ptr->p_send_head = ptr->sending.next;
   	        ptr->sending.packet.times++; 	/*!< Increate the send times  */
       	}

    }
    else if(ptr->sending.packet.times == 1)	/*!< the last time */
    {
        if(ptr->sending.packet.flag.f.need_ack)
        {
            /*!< check for ACK timeout */
          if(com_hal_get_idle_time(RS485_SIM, ENUM_HAL_IDLE_SEND) >= 30    \
             && (!com_hal_is_sending(RS485_SIM)))
           {      
                ptr->evt = EVT_COM_MAC_TIMEOUT;
                ptr->sending.packet.flag.all = 0;
                ptr->sending.packet.times = 0;                                  
           }       
           
        }
        else
        {	/*!< don't need wait for ack, just clear everything */
            ptr->sending.packet.flag.all = 0;
            ptr->sending.packet.times = 0;
        }
    }
    else
    {           
       
        if((!com_hal_is_sending(RS485_SIM)) \
        && (com_hal_get_idle_time(RS485_SIM, ENUM_HAL_IDLE_ALL) >= SEND_FRAME_INTERVAL_TIME))
        {    
            rs485_send_frame(RS485_SIM, ptr->sending.packet.data, ptr->sending.packet.len);   
            ptr->sending.packet.times--;
        }           
    }
}


/*****************************************************************************
 函 数 名  : rs485_mode_init
 功能描述  : 485模块初始化
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年3月28日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void rs485_mode_init(void)
{
    uint8_t i;

    frame_info.old_dec_addr = 0x10;
    frame_info.old_frame_id = 0;
    memset(&link_info,0,sizeof(link_info_t));

    for(i = 0; i < sizeof(s_mac_send_pool)/sizeof(s_mac_send_pool[0]); i++)
    {
        s_mac_send_pool[i].packet.times = 0;
        s_mac_send_pool[i].next = &s_mac_send_pool[i+1];
    }
    s_mac_send_pool[i - 1].next = NULL;
    p_mac_vacancy_pool_header = s_mac_send_pool;
}

static void check_anti_copy_evt_real_time(void)
{
    static uint8_t times = 0;
    if(anti_copy_evt == SET_PARAM_EVT)
    {
        if(flash_write(FLASH_USER_START_ADDR, anti_copy_info, sizeof(anti_copy_info)))
        {
            times = 0;
            anti_copy_evt = NONE_EVT;
        }
        else
        {
            times++;
            if(times > 3)
            {
                times = 0;
                anti_copy_evt = NONE_EVT;
            }
        }
    }
}

void rs485_mode_real_time(void)
{
    check_frame_id_valid_time();
	#if (!DEBUG_COM_CONFIG)
    check_rs485_link_real_time();
    #endif
    rs485_send_real_time();
    rs485_deal_elev_frame();
    check_anti_copy_evt_real_time();
}
/*****************************************************************************
 函 数 名  : rs485_deal_frame
 功能描述  : 对接收帧进行解析
 输入参数  : RS485_ID_E id  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月27日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void rs485_deal_access_frame(void)
{
    uint8_t *pbuf;    
    uint8_t len;
    uint8_t s_buf[40];  //临时组帧发送
    
    frame_head_t *frame_head;
    frame_head_t *frame_head_move;    
    rs485_frame_msg *move_point;
	rs485_frame_msg *head_point;

    lock_frame_t *lock_frame_data;
    check_frame_t *check_frame_data;
    ack_check_frame_t *ack_check_frame_data;
    reader_status_frame_t *reader_sta_frame_data;
    reader_ver_frame_t *reader_ver_frame_data;

    anti_copy_info_t *anti_copy_info_frame;
    
    com_mac_send_packet_t *p_packet;
    
    if(get_rs485_frame(RS485_ID1, &pbuf, &len) == 1)
    {
       
       frame_head = (frame_head_t *)pbuf;
        
       if(get_reader_addr() == 0) return; //读头初始化未完成    
       if(frame_info.old_frame_id == frame_head->frame_id && frame_head->cmd != ACK_CARD_DATA_CMD && frame_head->cmd != ACK_EVT_DATA_CMD
       && frame_head->src_addr == frame_info.old_dec_addr && frame_head->dest_addr != 0x8f && frame_head->dest_addr < 0x80)   return;

        if(frame_head->cmd != ACK_CARD_DATA_CMD && frame_head->cmd != ACK_EVT_DATA_CMD && frame_head->src_addr <= 0x0f)
        {
            frame_info.old_frame_id = frame_head->frame_id;
    		frame_info.old_dec_addr = frame_head->src_addr;		
            frame_info.timer = UPDATE_TIME; //更新id时间
        }

        if((frame_head->dest_addr != 0x1f) && (frame_head->dest_addr >  0x83) && frame_head->dest_addr != 0x8f && \
        (frame_head->dest_addr != get_reader_addr())) return;   
        
        set_link_time();

                      
        switch(frame_head->cmd)
        {
            case CHECK_READER_CMD:
            
                if(frame_head->dest_addr != get_reader_addr()) return;
                
				head_point = (rs485_frame_msg *)queue_frame_head;
				check_frame_data = (check_frame_t*)&frame_head->payload;
				set_sys_param(UNLOCK_TIME_PARAM, check_frame_data->unlock_time);
				set_sys_param(ANTI_COPY_PARAM, check_frame_data->anti_copy);
                 
                if(head_point == NULL)
                {      
                    frame_head_move = (frame_head_t *)s_buf;
                    frame_head_move->head = 0xaa;
                    frame_head_move->len   = 6;
                    frame_head_move->dest_addr = 0;
                    frame_head_move->src_addr  = get_reader_addr();
                    frame_head_move->frame_id  = frame_head->frame_id;
                    frame_head_move->cmd = (frame_head->cmd | 0x80);
                    
                    ack_check_frame_data = (ack_check_frame_t *)&frame_head_move->payload;
										ack_check_frame_data->sum = get_check_sum(s_buf, 7); 

                    //轮询帧空闲应答
                    rs485_send_frame(RS485_ID1,s_buf,7);    
                }
                else
                {    
                    //卡数据上传
                    rs485_send_frame(RS485_ID1,head_point->pbuf, head_point->len);
                    frame_head_move = (frame_head_t *)head_point->pbuf;
                    
                    if(frame_head_move->cmd == ACK_CHECK_VER_CMD)
                    {
                        move_point =  queue_frame_head;
                        queue_frame_head =  queue_frame_head->next;                  
                        move_point->next = NULL;                 
                        del_frame_queue(move_point);
                    }
                }
            break;
            
            case ACK_CARD_DATA_CMD: 

                if(frame_head->dest_addr != get_reader_addr()) return;
                //主机对上传的队列卡数据应答
                if(queue_frame_head == NULL) return;

              //  frame_head_move = (frame_head_t *)queue_frame_head->pbuf;
              //  if(frame_head_move->cmd != CAR_UPDATA_CMD || frame_head_move->frame_id != frame_head->frame_id) return;
                
                move_point =  queue_frame_head;
                queue_frame_head =  queue_frame_head->next;                  
                move_point->next = NULL;                 
                del_frame_queue(move_point);
            break;
            
            case ACK_EVT_DATA_CMD: 

                if(frame_head->dest_addr != get_reader_addr()) return;
                //主机对上传的队列事件数据应答
                if(queue_frame_head == NULL) return;

             //   frame_head_move = (frame_head_t *)queue_frame_head->pbuf;
             //   if(frame_head_move->cmd != EVT_UPDATA_CMD || frame_head_move->frame_id != frame_head->frame_id) return;
                                
                move_point =  queue_frame_head;
                queue_frame_head =  queue_frame_head->next;                  
                move_point->next = NULL;                 
                del_frame_queue(move_point);
            break;
            
            case ACK_CARD_RESULT_UPDATA_CMD: 

                if(frame_head->dest_addr != get_reader_addr()) return;
            
                if(queue_frame_head == NULL) return;

               // frame_head_move = (frame_head_t *)queue_frame_head->pbuf;
               // if(frame_head_move->cmd != CARD_RESULT_UPDATA_CMD || frame_head_move->frame_id != frame_head->frame_id) return;
                
                move_point =  queue_frame_head;
                queue_frame_head =  queue_frame_head->next;                  
                move_point->next = NULL;                 
                del_frame_queue(move_point);
            break;
            
            case CHECK_VER_CMD:

            if(frame_head->dest_addr == get_reader_addr())
            {
                 //查询读头版本
                frame_head_move = (frame_head_t *)s_buf;
                frame_head_move->head = 0xaa;
                frame_head_move->len   = 8;
                frame_head_move->dest_addr = 0;
                frame_head_move->src_addr  = get_reader_addr();
                frame_head_move->frame_id  = frame_head->frame_id;
                frame_head_move->cmd = (frame_head->cmd | 0x80);
                
                reader_ver_frame_data = (reader_ver_frame_t *)&frame_head_move->payload;
                reader_ver_frame_data->ver1 = VER1;
                reader_ver_frame_data->ver2 = VER2;
		        reader_ver_frame_data->sum = get_check_sum(s_buf, 9); 
	
                rs485_send_frame(RS485_ID1, s_buf, 9);        
            }
            else
            {
                if(frame_head->dest_addr > 0x83 && frame_head->dest_addr != 0x8f) return;
                 //转发查询干接点  
                p_packet = com_mac_send();  
                if(p_packet == NULL) return;
                
                p_packet->flag.all = 1;
                p_packet->len = 0x07;
                p_packet->times = 1;                   
                frame_head_move = (frame_head_t *)p_packet->data;
                
                memcpy(p_packet->data,pbuf,(frame_head->len+1));

                frame_head_move->frame_id = set_frame_id(RS485_SIM);
                p_packet->data[p_packet->len-1] = get_check_sum(p_packet->data, p_packet->len);
            }
            break;

            case CHECK_DOOR_LOCK_CMD:
            
                if(frame_head->dest_addr != get_reader_addr()) return;
              //读头状态查询
                frame_head_move = (frame_head_t *)s_buf;
                frame_head_move->head = 0xaa;
                frame_head_move->len   = 8;
                frame_head_move->dest_addr = 0;
                frame_head_move->src_addr  = get_reader_addr();
                frame_head_move->frame_id  = frame_head->frame_id;
                frame_head_move->cmd = (frame_head->cmd | 0x80);
                
                reader_sta_frame_data = (reader_status_frame_t *)&frame_head_move->payload;
                reader_sta_frame_data->lock_status = get_door_lock_sta(LOCK_STA);
                reader_sta_frame_data->door_status = get_door_lock_sta(DOOR_STA);
		        reader_sta_frame_data->sum = get_check_sum(s_buf, 9);
		        
                rs485_send_frame(RS485_ID1, s_buf, 9);
                
            break;
                                             
            case OPEN_LOCK_CMD: 

                if(frame_head->dest_addr != get_reader_addr()) return;                         
                //接收到开锁命令应答  
                frame_head_move = (frame_head_t *)s_buf;
                frame_head_move->head = 0xaa;
                frame_head_move->len   = 6;
                frame_head_move->dest_addr = 0;
                frame_head_move->src_addr  = get_reader_addr();
                frame_head_move->frame_id  = frame_head->frame_id;
                frame_head_move->cmd = (frame_head->cmd | 0x80);
                
                ack_check_frame_data = (ack_check_frame_t *)&frame_head_move->payload;
			    ack_check_frame_data->sum = get_check_sum(s_buf, 7);              		                           
                
                //接收到开锁数据
                lock_frame_data = (lock_frame_t*)&frame_head->payload;
                
                beep_set_mode((BEEP_MODE_E)lock_frame_data->beep_mode);                
                
                led_set_mode((LED_MODE_E)lock_frame_data->led_mode);
                
                if(lock_frame_data->unlock_time > 0 && lock_frame_data->unlock_time < 0xfe)
                {                
                    lock_set_mode(CARD_UNLOCK_MODE,lock_frame_data->unlock_time);
                    set_sys_param(UNLOCK_TIME_PARAM,lock_frame_data->unlock_time);
                }
                else if(lock_frame_data->unlock_time == 0xff)
                {
                    lock_set_mode(CMD_UNLOCK_MODE,lock_frame_data->unlock_time);
                }
                else if(lock_frame_data->unlock_time == 0xfe)
                {
                    lock_set_mode(CMD_UNLOCK_MODE,lock_frame_data->unlock_time);
                }   

                if(frame_head->dest_addr != 0x1f) //广播帧不应答
                {
                    rs485_send_frame(RS485_ID1,s_buf, 7);   
                }
                
            break;
            
            case OPEN_FLOOR_CMD:                             
                
                //楼层开放转发       		
                if(frame_head->dest_addr == 0x8f)
                {   
                    p_packet = com_mac_send(); 
                    if(p_packet == NULL) return;
                    
                    p_packet->flag.all = 0;
                    p_packet->len = 0x1a;
                    p_packet->times = 1;                   
                    frame_head_move = (frame_head_t *)p_packet->data;
                    
                    memcpy(p_packet->data,pbuf,(frame_head->len+1));

                    frame_head_move->frame_id = set_frame_id(RS485_SIM);
                    p_packet->data[p_packet->len-1] = get_check_sum(p_packet->data, p_packet->len);
                }
                else if(frame_head->dest_addr == get_reader_addr())
                {    
                    
                    frame_head_move = (frame_head_t *)s_buf;
                    frame_head_move->head = 0xaa;
                    frame_head_move->len   = 6;
                    frame_head_move->dest_addr = 0;
                    frame_head_move->src_addr  = get_reader_addr();
                    frame_head_move->frame_id  = frame_head->frame_id;
                    frame_head_move->cmd = (frame_head->cmd | 0x80);
                    
                    ack_check_frame_data = (ack_check_frame_t *)&frame_head_move->payload;
			        ack_check_frame_data->sum = get_check_sum(s_buf, 7);  			        					
                    rs485_send_frame(RS485_ID1, s_buf, 7); //应答帧 
                     					


                    p_packet = com_mac_send();   
                    if(p_packet == NULL) return;
                    
                    p_packet->flag.all = 0;
                    p_packet->len = 0x1a;
                    p_packet->times = 2;                   
                    frame_head_move = (frame_head_t *)p_packet->data;
                    
                    memcpy(p_packet->data,pbuf,p_packet->len);
                    
                    frame_head_move->src_addr = get_reader_addr();
                    frame_head_move->dest_addr = 0x8f;
                    frame_head_move->frame_id = set_frame_id(RS485_SIM);
                    p_packet->data[p_packet->len-1] = get_check_sum(p_packet->data, p_packet->len);
                }                
            break;
            case ACCESS_CMD:
            
            if(frame_head->dest_addr != get_reader_addr()) return;
            
            if(frame_head->payload != 0x02)
            {                   
                if(frame_head->payload == 0x01)
                {
                    anti_copy_info_frame = (anti_copy_info_t*)&frame_head->payload;
                    if(memcmp(anti_copy_info, anti_copy_info_frame->project_num,19))
                    {
                        memcpy(anti_copy_info,anti_copy_info_frame->project_num,19);
                        anti_copy_evt = SET_PARAM_EVT;
                    }
                    
                }
                else
                {
                    card_rw_buffer(frame_head->len+1,&frame_head->head);    
                }
                frame_head_move = (frame_head_t *)s_buf;
                frame_head_move->head = 0xaa;
                frame_head_move->len   = 6;
                frame_head_move->dest_addr = 0;
                frame_head_move->src_addr  = get_reader_addr();
                frame_head_move->frame_id  = frame_head->frame_id;
                frame_head_move->cmd = (frame_head->cmd | 0x80);
                frame_head_move->payload = get_check_sum(s_buf, 7);		
		        
                rs485_send_frame(RS485_ID1, s_buf, 7);
            }
            else
            {              
                frame_head_move = (frame_head_t *)s_buf;
                frame_head_move->head = 0xaa;
                frame_head_move->len   = 26;
                frame_head_move->dest_addr = 0;
                frame_head_move->src_addr  = get_reader_addr();
                frame_head_move->frame_id  = frame_head->frame_id;
                frame_head_move->cmd = (frame_head->cmd | 0x80);  
                
                anti_copy_info_frame = (anti_copy_info_t*)&frame_head_move->payload;
                anti_copy_info_frame->exp_cmd = 0x02;
                memcpy(anti_copy_info_frame->project_num,anti_copy_info,16); 
                anti_copy_info_frame->adf[0] = anti_copy_info[16];
                anti_copy_info_frame->adf[1] = anti_copy_info[17];
                anti_copy_info_frame->inter_num = anti_copy_info[18];
                
		        anti_copy_info_frame->sum = get_check_sum(s_buf, 27);
		        
                rs485_send_frame(RS485_ID1, s_buf, 27);
            }
            break;
						case BT_DATA_PASSTHROUGHT_CMD:
							if(frame_head->dest_addr != get_reader_addr()) return;
							bt_frame_transmit_buffer(frame_head->len - 6,&frame_head->payload);

							//发送应答帧
							frame_head_move = (frame_head_t *)s_buf;
							frame_head_move->head = 0xaa;
							frame_head_move->len   = 6;
							frame_head_move->dest_addr = 0;
							frame_head_move->src_addr  = get_reader_addr();
							frame_head_move->frame_id  = frame_head->frame_id;
							frame_head_move->cmd = (frame_head->cmd | 0x80);
							ack_check_frame_data = (ack_check_frame_t *)&frame_head_move->payload;
							ack_check_frame_data->sum = get_check_sum(s_buf, 7); 
							rs485_send_frame(RS485_ID1,s_buf,7);    
						break;
						case ACK_BT_DATA_CMD:
							if(frame_head->dest_addr != get_reader_addr()) return;
							if(queue_frame_head == NULL) return;
							move_point =  queue_frame_head;
							queue_frame_head =  queue_frame_head->next;  
							move_point->next = NULL;
							del_frame_queue(move_point);
						break;
        }
         
    }
}


void rs485_deal_elev_frame(void)
{

    uint8_t *pbuf;    
    uint8_t len;
   
    frame_head_t *frame_head;

    if(get_rs485_frame(RS485_SIM, &pbuf, &len) == 1)
    {     
        
       frame_head = (frame_head_t *)pbuf;
        
       if(get_reader_addr() == 0) return; //读头初始化未完成      
       
  
        if((frame_head->dest_addr != 0x0) && (frame_head->dest_addr != get_reader_addr())) return;           
                      
        switch(frame_head->cmd)
        {
            
            case ACK_CHECK_VER_CMD:
                 memcpy(ver_buf, pbuf, (frame_head->len+1));
                 set_rev_evt(EVT_COM_MAC_N_FRAME);
            break;            
          
        }
         
    }
     
}



