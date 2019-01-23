/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : rs485_drv.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月24日
  最近修改   :
  功能描述   : rs485
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "timer_drv.h"
#include "define.h" 
#include "rs485_drv.h"
#include "rs485_mode.h"
#include <string.h>
#include <stdlib.h>
#include "check_io_drv.h"
#include "rs485_mode.h" 
#include "define_platform.h" 
#include "sim_uart_drv.h"
#include "debug.h"
/*----------------------- Constant / Macro Definitions -----------------------*/
//TEST
#include "led_drv.h"

typedef enum
{
    TX,
    RX,
    PIN_DIR_NUM,
}PIN_DIR;

typedef enum
{
    COM1,
    COM2,
    COMn,
}COM_TypeDef;

USART_TypeDef* RS485_USART[COMn] = {USART1,USART1}; 

GPIO_TypeDef* RS485_TX_PORT[COMn] = {GPIOA,GPIOA};
 
GPIO_TypeDef* RS485_RX_PORT[COMn] = {GPIOA,GPIOA};

const uint32_t RS485_USART_CLK[COMn] = {RCC_APB2Periph_USART1,RCC_APB2Periph_USART1};

const uint32_t RS485_TX_PORT_CLK[COMn] = {RCC_AHBPeriph_GPIOA,RCC_AHBPeriph_GPIOA};
  
const uint32_t RS485_RX_PORT_CLK[COMn] = {RCC_AHBPeriph_GPIOA,RCC_AHBPeriph_GPIOA};

const uint16_t RS485_TX_PIN[COMn] = {GPIO_Pin_9,GPIO_Pin_9};

const uint16_t RS485_RX_PIN[COMn] = {GPIO_Pin_10,GPIO_Pin_10};
 
const uint8_t RS485_TX_PIN_SOURCE[COMn] = {GPIO_PinSource9,GPIO_PinSource9};

const uint8_t RS485_RX_PIN_SOURCE[COMn] = {GPIO_PinSource10,GPIO_PinSource10};
 
const uint8_t RS485_TX_AF[COMn] = {GPIO_AF_1,GPIO_AF_1};
 
const uint8_t RS485_RX_AF[COMn] = {GPIO_AF_1,GPIO_AF_1};


const uint16_t RS485_DIR[RS485_IDNUM] = {GPIO_Pin_11,GPIO_Pin_1};
/*------------------------ Variable Define/Declarations ----------------------*/


rs485_frame_msg *queue_frame_head = NULL;
rs485_frame_msg *queue_free_head = NULL;

uint8_t queue_cnt = 0;

rs485_port_t rs485_port[RS485_IDNUM];

extern uint8_t receive_data;
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
extern void uart_putc(unsigned char c);

static void rs485_port_t_init(void)
{
    uint8_t i;
    
    for(i=0; i<RS485_IDNUM; i++)
    {
        memset(&rs485_port[i],0,sizeof(rs485_port[0]));
    }
}


/*****************************************************************************
 函 数 名  : rs485_send_delay
 功能描述  : 用于发送延时，放在定时中断中
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年3月23日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void rs485_send_delay(void)
{
    
    if(rs485_port[RS485_ID1].s_delay > 0)
    {
        ENTER_CRITICAL() ;
        rs485_port[RS485_ID1].s_delay--;
        EXIT_CRITICAL();
        
        if(rs485_port[RS485_ID1].s_delay == 0)
        {
             USART_SendData(RS485_USART[RS485_ID1], rs485_port[RS485_ID1].rs485_send_buf[0]);   
        }
    }
    
}

static void rs485_set_pin(RS485_ID_E id, PIN_DIR dir)
{	
	if(dir == RX)   
	{		
		cbi(GPIOA,RS485_DIR[id]); 
	}   
	else   
	{
		sbi(GPIOA,RS485_DIR[id]); 
	}			 
}

static void rs485_pin_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA, ENABLE);					  											 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;//GPIO_Pin_11;//                
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	rs485_set_pin(RS485_ID1,RX);
	rs485_set_pin(RS485_SIM,RX);
}




static void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(RS485_TX_PORT_CLK[COM] | RS485_RX_PORT_CLK[COM], ENABLE);

  /* Enable USART clock */
  if(COM == COM1)
  {
    RCC_APB2PeriphClockCmd(RS485_USART_CLK[COM1] , ENABLE); 
  }
  else if(COM == COM2)
  {
    RCC_APB1PeriphClockCmd(RS485_USART_CLK[COM2], ENABLE);
  }
  /* Connect PXx to USARTx_Tx */
  GPIO_PinAFConfig(RS485_TX_PORT[COM], RS485_TX_PIN_SOURCE[COM], RS485_TX_AF[COM]);

  /* Connect PXx to USARTx_Rx */
  GPIO_PinAFConfig(RS485_RX_PORT[COM], RS485_RX_PIN_SOURCE[COM], RS485_RX_AF[COM]);
  
  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = RS485_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(RS485_TX_PORT[COM], &GPIO_InitStructure);
    
  /* Configure USART Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = RS485_RX_PIN[COM];
  GPIO_Init(RS485_RX_PORT[COM], &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(RS485_USART[COM], USART_InitStruct);
    
  /* Enable USART */
  USART_Cmd(RS485_USART[COM], ENABLE);
}


/*****************************************************************************
 函 数 名  : frame_queue_free
 功能描述  : 
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void frame_queue_free(void)
{
    rs485_frame_msg *free_point = NULL;
    
    if(queue_free_head == NULL) 
    {
        return;
    }
    
    while(queue_free_head != NULL)
    {   
        ENTER_CRITICAL() ;
        free_point = queue_free_head->next;       
        if(queue_free_head->pbuf != NULL)    free(queue_free_head->pbuf);       
        free(queue_free_head);       
        queue_free_head = free_point;
        EXIT_CRITICAL() ;
    }
    
}


uint8_t com_hal_is_sending(RS485_ID_E id)
{
    return (rs485_port[id].s_status != ENUM_HAL_SEND_STA_IDLE);
}

/******************************************************************************
* Name: 	 com_hal_get_idle_time 
*
* Desc: 	 
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2015/03/19, Create this function by Kimson
 ******************************************************************************/
uint8_t com_hal_get_idle_time(RS485_ID_E id, ENUM_HAL_IDLE_TIME_T type)
{
    uint8_t s_time, r_time;
    uint8_t time = 0;


    s_time = rs485_port[id].s_interval_time;
    r_time = rs485_port[id].r_interval_time;

    if(type == ENUM_HAL_IDLE_SEND)
    {
        time = s_time*TIME_BASE;
    }
    else if(type == ENUM_HAL_IDLE_REV)
    {
        time = r_time*TIME_BASE;
    }
    else
    {
        if(s_time > r_time)
        {
            time = r_time;
        }
        else
        {
            time = s_time;
        }

        time *= TIME_BASE;
    }

    return time;
    
}

/*****************************************************************************
 函 数 名  : rs485_time_out
 功能描述  : 接收数据超时
 输入参数  : void  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月26日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
static void rs485_idle_time_real_time(RS485_ID_E id_num)
{
    uint8_t i;
    
    if(!IS_TIMER_EVT(TIME_EVT_1)) return;

    ENTER_CRITICAL() ;
    for(i=0; i<id_num; i++)
    {
        if(rs485_port[i].r_interval_time < FRAME_MAX_SEND_TIME)  rs485_port[i].r_interval_time++;
        if(rs485_port[i].s_interval_time < FRAME_MAX_SEND_TIME)  rs485_port[i].s_interval_time++;
    }
    EXIT_CRITICAL() ;
}
/*****************************************************************************
 函 数 名  : rs485_rev_real_time
 功能描述  : 数据接收
 输入参数  : RS485_ID_E id  
 输出参数  : 无
 返 回 值  : static
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月29日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void rs485_rev_irq_callback(RS485_ID_E id)
{

    uint8_t rev_data;    
    
    if(id == RS485_SIM) 
    {
        rev_data = receive_data;
    }
    else
    {
        rev_data = USART_ReceiveData(RS485_USART[id]);
    }

    if(rs485_port[id].r_interval_time > FRAME_INTERVAL_TIME)   //接收总线空闲5ms认为是一个新帧
    {
        rs485_port[id].rev_cnt = 0;        
    }     
    
    rs485_port[id].r_interval_time = 0;
    
    switch(rs485_port[id].rev_cnt)
    {
        case 0: /* waite for header */
        {
            if(rev_data == FEAME_HEAD_DATA)
            {   
                rs485_port[id].rs485_rev_buf[rs485_port[id].rev_cnt] = rev_data;
                rs485_port[id].rev_cnt++;
            }
        }break;
        case FRAME_OFFSET_LEN: /* we get len  */
        {
            if((rev_data > FRAME_MAX_SIZE) || (rev_data < FRAME_MIN_SIZE)) /* overflow*/
            {
                rs485_port[id].rev_cnt = 0;
            }
            else
            {    
                rs485_port[id].rs485_rev_buf[rs485_port[id].rev_cnt] = rev_data; 
                rs485_port[id].rev_cnt++;
            }
        }break;
        default: /* receive data untill the end  */
        {
            
            rs485_port[id].rs485_rev_buf[rs485_port[id].rev_cnt] = rev_data;
            rs485_port[id].rev_cnt++;
            if(rs485_port[id].rev_cnt >= (rs485_port[id].rs485_rev_buf[FRAME_OFFSET_LEN]+1))
            {
                if(check_sum(rs485_port[id].rs485_rev_buf, (rs485_port[id].rs485_rev_buf[FRAME_OFFSET_LEN]+1)) == 0)
                {
                   rs485_port[id].rev_cnt = 0;  
                   return;
                }
                rs485_port[id].rev_buff_full = 1;
                rs485_port[id].rev_cnt = 0;   
            }
        }break;
        
    }    
                
    
}

uint8_t set_frame_id(RS485_ID_E id)
{
    static uint8_t frame_id[RS485_IDNUM] = {0};
    
    frame_id[id]++;
    if(frame_id[id] == 0)  frame_id[id]++;
    return frame_id[id];
}

void rs485_drv_init(void)
{    
	USART_InitTypeDef USART_InitStructure;		
	NVIC_InitTypeDef 	NVIC_InitStructure;

    
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

//    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
  
#if (WORKMODE == READER_ACCESS) 
	USART_InitStructure.USART_BaudRate = BAUDRATE_ACCESS;
#else
	USART_InitStructure.USART_BaudRate = BAUDRATE_ELEVATOR;
#endif
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//    STM_EVAL_COMInit(COM2, &USART_InitStructure);
		
#if (WORKMODE == READER_ACCESS) 
	USART_InitStructure.USART_BaudRate = BAUDRATE_ACCESS;
#else
	USART_InitStructure.USART_BaudRate = BAUDRATE_ELEVATOR;
#endif
	STM_EVAL_COMInit(COM1, &USART_InitStructure);
    

	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	USART_ClearITPendingBit(USART1, USART_IT_TC);
	USART_ITConfig(USART1,USART_IT_TC,ENABLE);
    
//#if DEBUG_COM
//	USART_ITConfig(USART2,USART_IT_RXNE,DISABLE);
//#else		
//	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
//	USART_ClearITPendingBit(USART2, USART_IT_TC);     
//	USART_ITConfig(USART2,USART_IT_TC,ENABLE);
//#endif
    
	rs485_pin_init();
	rs485_port_t_init();
}


/*****************************************************************************
 函 数 名  : append_frame_queue
 功能描述  : 加入链表
 输入参数  : rs485_frame_msg *point  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
rs485_frame_msg *append_frame_queue(uint16_t len)
{

    uint8_t *pbuf;
    rs485_frame_msg *move_point;
    rs485_frame_msg *add_point;

    if(queue_cnt  > QUEUE_MAX_NUM)  return NULL;

    pbuf = (uint8_t *)malloc(len);
	  if(pbuf == NULL) return NULL;
    add_point = (rs485_frame_msg *)malloc(sizeof(rs485_frame_msg));
	  if(add_point == NULL) return NULL;
    add_point->pbuf = pbuf;
    add_point->next = NULL;

    ENTER_CRITICAL() ;   
    queue_cnt++;  
    
    if(queue_frame_head == NULL)
    {
        queue_frame_head = add_point;
        queue_frame_head->next = NULL;
        EXIT_CRITICAL();
        return add_point;
    }      
    move_point = queue_frame_head;
    while(move_point->next != NULL)
    {
        move_point = move_point->next;
    }
    move_point->next = add_point;    
    EXIT_CRITICAL();

    return add_point;
}


/*****************************************************************************
 函 数 名  : del_frame_queue
 功能描述  : 
 输入参数  : rs485_frame_msg *point  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void del_frame_queue(rs485_frame_msg *point)
{
    rs485_frame_msg *move_point;


    ENTER_CRITICAL() ;     
    if(queue_cnt > 0)   //队列个数
        queue_cnt--;
        
    if(queue_free_head == NULL)
    {
        queue_free_head =  point;
        queue_free_head->next = NULL;
    }
    else
    {
        move_point = queue_free_head;   
        while(move_point->next != NULL)
        {
            move_point = move_point->next;   
        }
        move_point->next = point;
        point->next = NULL;
    }
    EXIT_CRITICAL();
}


/*****************************************************************************
 函 数 名  : set_frame_head
 功能描述  : 设置帧头
 输入参数  : uint8_t *addr      
             uint8_t cmd        
             uint8_t len        
             uint8_t dest_addr  
 输出参数  : 无
 返 回 值  : uint8_t     -返回帧头 paylaod地址
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月26日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t *set_frame_head(uint8_t *addr, uint8_t cmd, uint8_t len, uint8_t dest_addr)
{
    uint8_t *paddr;
   
    frame_head_t *frame_head; 
    frame_head = (frame_head_t*) addr;
    frame_head->head = 0xaa;
    frame_head->len   = len;
    frame_head->dest_addr = dest_addr;
    frame_head->src_addr   = get_reader_addr();
    frame_head->frame_id  = set_frame_id(RS485_ID1);
    frame_head->cmd   = cmd; 
    
    paddr = &(frame_head->payload);
    return paddr;
}

uint8_t  get_check_sum(uint8_t *point, uint8_t len)
{
    uint8_t *pbuf;
    uint8_t i;
    uint8_t sum = 0;

    if(point == NULL || len  < 1) return 0;
    
    pbuf = (uint8_t *)point;
    
    for(i=1; i<(len-1); i++)
    {
        sum += *(pbuf+i);
    }
    return (0 - sum);
}

uint8_t check_sum(uint8_t *point, uint8_t len)
{
    uint8_t *pbuf;
    uint8_t i;
    uint8_t sum = 0;

    if(point == NULL || len  < 1) return 0;
    
    pbuf = point;
    
    for(i=1; i<len; i++)
    {
        sum += *(pbuf+i);
    }
    if(sum == 0)    return 1;     
    return 0;    
}
/*****************************************************************************
 函 数 名  : rs485_send_frame
 功能描述  : 启动发送
 输入参数  : RS485_ID_E id     
             uint8_t *paddr  
             uint8_t len     
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t rs485_send_frame(RS485_ID_E id, uint8_t *paddr, uint8_t len)
{
    uint8_t *pbuf;

    if(paddr == NULL || len  < 1) return 0;
    
    rs485_port[id].s_status = ENUM_HAL_SEND_STA_SENDING;
    rs485_port[id].s_interval_time = 0;
    rs485_set_pin(id, TX);
    
    pbuf = paddr;
    rs485_port[id].rs485_send_len = len;
    memset(rs485_port[id].rs485_send_buf,0,sizeof(rs485_port[id].rs485_send_buf));
    memcpy(rs485_port[id].rs485_send_buf, pbuf,len);

    if(id == RS485_ID1)  //延时4ms发送
    {        
        rs485_port[id].s_delay = SEND_DELAY;   
    }
    else if(id == RS485_SIM) //直接发送
    {
			
        SimUART_TxBytes(paddr,len);
//        USART_SendData(COM_USART[id], rs485_port[id].rs485_send_buf[0]);   
    }
    return 1;
}

uint8_t get_rs485_frame(RS485_ID_E id, uint8_t **pbuf, uint8_t *len)
{   
    if(rs485_port[id].rev_buff_full == 1)
    {      
        rs485_port[id].rev_buff_full = 0;
        *pbuf = rs485_port[id].rs485_rev_buf; 
        *len  = rs485_port[id].rs485_rev_buf[1]+1;
        return 1;
    }
    *pbuf = NULL;
    return 0;
}


void rs485_drv_real_time(void)
{
   frame_queue_free();
   rs485_idle_time_real_time(RS485_IDNUM);
}


void USART1_IRQHandler(void)
{
     static uint8_t send_cnt = 0;
     //注意！不能使用if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)来判断
     if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)    
     {
          USART_ClearITPendingBit(USART1, USART_IT_ORE);               //清除中断标志  
         //USART_ReceiveData(USART1);
     }   
     
     if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
     {
          rs485_port[RS485_ID1].s_interval_time = 0;                 //清除发送总线空闲定时器
          
          USART_ClearITPendingBit(USART1, USART_IT_TC);              //清除中断标志
          
          if(send_cnt < (rs485_port[RS485_ID1].rs485_send_len - 1))   //是否发送完成
          {
   
              send_cnt++;
              USART_SendData(RS485_USART[RS485_ID1],rs485_port[RS485_ID1].rs485_send_buf[send_cnt]); 
          }
          else
          {                              
              send_cnt = 0;
              rs485_set_pin(RS485_ID1, RX);                             //485总线置为接收
              rs485_port[RS485_ID1].s_status = ENUM_HAL_SEND_STA_IDLE; //置发送空闲
          }
     }
     /* 接收中断 */
     if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
     {   
         rs485_rev_irq_callback(RS485_ID1)  ; 
     }
     rs485_deal_access_frame();
     
}

