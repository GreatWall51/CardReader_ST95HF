/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : rs485_mode.h
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月25日
  最近修改   :
  功能描述   : 
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月25日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/
#ifndef _RS485_MODE_H_
#define _RS485_MODE_H_
/*-------------------------------- Includes ----------------------------------*/
 
 
/*----------------------- Constant / Macro Definitions -----------------------*/

#define UPDATE_TIME  5
#define CFG_COM_MAC_PACKET_SIZE     40
#define CFG_COM_MAC_POOL_SIZE       10

typedef enum {
   NONE_EVT = 0,
   SET_PARAM_EVT,
   NEW_EVT,
}ANTI_COPY_CMD_EVT_E;

typedef enum {
   APPEND_QUEUE_HEAD, //插入队列头
   APPEND_QUEUE_TAIL, //插入队列尾
}ENUM_QUEUE_DIR_T;

typedef enum {
   EVT_COM_MAC_NONE, 	/*!< nothing happend */
   EVT_COM_MAC_N_FRAME,	/*!< receive new frame */
   EVT_COM_MAC_ACK, 	/*!< get a ACK */
   EVT_COM_MAC_TIMEOUT 	/*!< timeout without ACK */
}ENUM_COM_MAC_EVT_T;

/*------------------------ Variable Define/Declarations ----------------------*/

typedef struct com_mac_send_packet_str {
    union {
        uint16_t all;
        struct {
        uint16_t need_ack: 1;
        uint16_t reserved: 15;
        }f;
    }flag;
    uint16_t len; 	/*!< len of data */ 
    uint8_t data[CFG_COM_MAC_PACKET_SIZE];
    uint8_t times;	/*!< times to be sent, if this count down to 0, means this packet is empty */
}com_mac_send_packet_t;


typedef struct com_mac_send_pool_item_str {
    com_mac_send_packet_t packet;
    struct com_mac_send_pool_item_str *next;
}com_mac_send_pool_item_t;


typedef struct com_mac_port_str {
    ENUM_COM_MAC_EVT_T evt;
    /* send */
    com_mac_send_pool_item_t *p_send_head;
    com_mac_send_pool_item_t *p_send_tail;
    com_mac_send_pool_item_t sending;

}com_mac_port_t;  



/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
void rs485_deal_access_frame(void);
void rs485_deal_elev_frame(void);
void check_rs485_link(void);
void rs485_mode_init(void);
void rs485_mode_real_time(void);
com_mac_send_packet_t *com_mac_send(void);
void free_frame_queue(void);
#endif

