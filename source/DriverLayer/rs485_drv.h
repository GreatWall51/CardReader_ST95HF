/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : rs485_drv.h
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月24日
  最近修改   :
  功能描述   : rs485_drv.h
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月24日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/
#ifndef _RS485_DRV_H_
#define  _RS485_DRV_H_
/*-------------------------------- Includes ----------------------------------*/
#include <stdint.h>
 
/*----------------------- Constant / Macro Definitions -----------------------*/
//#define NULL        ((void*)0)

//波特率定义
#define BAUDRATE_ACCESS       19200
#define BAUDRATE_ELEVATOR     4800

//读头版本号
#define VER1   0X02
#define VER2   0X00

//door & lock status
#define OPEN     0x01
#define CLOSE    0x00

//cmd
#define CHECK_READER_CMD       0x01
#define OPEN_LOCK_CMD          0x02      //下发开锁数帧(卡验证)
#define CHECK_DOOR_LOCK_CMD    0x03
#define CHECK_VER_CMD          0x04

#define CARD_RESULT_UPDATA_CMD           0x15    //上传指令
#define ACK_CARD_RESULT_UPDATA_CMD        0x95

#define ACK_CHECK_CMD          0x81
#define ACK_DOOR_LOCK_CMD      0x83
#define ACK_CHECK_VER_CMD      0x84

#define LINK_CMD               0x23    //通知干接点通信断路

#define OPEN_FLOOR_CMD         0x21    //楼层权限开放
#define CAR_UPDATA_CMD         0x11    //刷卡上传
#define CARD_RESULT_UPDATA_CMD         0x15    //卡操作上传

#define EVT_UPDATA_CMD         0x12    //事件上传

#define ACK_CARD_DATA_CMD      0x91
#define ACK_EVT_DATA_CMD       0x92

#define ACCESS_CMD             0x05 //门禁主机卡防复制
#define BT_DATA_PASSTHROUGHT_CMD            0X07      //主机下发蓝牙数据透传
#define BT_REPORD_DATA_CMD                  0X17      //蓝牙上报数据到主机
#define ACK_BT_DATA_CMD        0x97      //主机应答蓝牙数据

//队列最大值
#define QUEUE_MAX_NUM         40
//帧最大发送时间
#define FRAME_MAX_SEND_TIME    200
//帧间隔
#define FRAME_INTERVAL_TIME     20


#define SEND_FRAME_INTERVAL_TIME     20

#define FEAME_HEAD_DATA    0xaa

#define FRAME_MAX_SIZE     250
#define FRAME_MIN_SIZE     3
#define FRAME_OFFSET_LEN   1

#define  MALLOC_LEN   20

//帧发送延时
#define SEND_DELAY   4  // > 3ms

typedef enum
{
    RS485_ID1,
    RS485_SIM,
    RS485_IDNUM,
}RS485_ID_E;

typedef enum
{
    QUEUE_HEAD,
    READER_STATUS,
    READER_VER,
    CHECK_ACK,
}QUEUE_FRAME_E;

typedef enum
{
    ENUM_HAL_IDLE_SEND,
    ENUM_HAL_IDLE_REV,
    ENUM_HAL_IDLE_ALL
}ENUM_HAL_IDLE_TIME_T;

enum {
    ENUM_HAL_SEND_STA_IDLE,
    ENUM_HAL_SEND_STA_SENDING,             
};
/*------------------------ Variable Define/Declarations ----------------------*/
typedef struct rs485_frame
{
    uint8_t len;
    uint8_t *pbuf;           
    struct rs485_frame  *next;
    
}rs485_frame_msg;


typedef struct   
{
    uint8_t head;
    uint8_t len;
    uint8_t dest_addr;
    uint8_t src_addr;
    uint8_t frame_id;
    uint8_t cmd;
    uint8_t payload;
}frame_head_t;

typedef struct        //空闲应答帧
{   
    uint8_t sum;
}ack_check_frame_t;

typedef struct        //轮询帧  9byte
{   
    uint8_t unlock_time;
    uint8_t anti_copy;  
    uint8_t sum;
}check_frame_t;


typedef struct        //主机下发数据帧
{
    uint8_t unlock_time;
    uint8_t beep_mode;
    uint8_t led_mode;
    uint8_t sum;
}lock_frame_t;


typedef struct        //应答帧,状态查询帧  7byte
{
    uint8_t sum;
}ack_frame_t;


typedef struct        //卡数据帧 
{
    uint8_t card_type;
    uint8_t card_len;
    uint8_t card_uid[8]; 
    uint8_t sum;
}card_data_frame_t;

typedef struct           //读头事件应答
{
    uint8_t evt_type;    //事件类型
    uint8_t expand;      //具体事件
    uint8_t sum;
    
}reader_evt_frame_t;


typedef struct        //读头状态应答
{
    uint8_t door_status;   //0x00 关，0x01开
    uint8_t lock_status;   //0x00关，0x01开
    uint8_t sum;
    
}reader_status_frame_t;

typedef struct        //读头版本号应答帧
{
    uint8_t ver1;  
    uint8_t ver2; 
    uint8_t sum;
    
}reader_ver_frame_t;


typedef struct        //转发梯控信号
{
    uint8_t option_type;  
    uint8_t dir;
    uint8_t src_room[2]; 
    uint8_t src_floor; 
    uint8_t des_room[2]; 
    uint8_t get_time[2];
    uint8_t floor[10];
    uint8_t sum;
    
}elevator_ctl_frame_t;


typedef struct        //读头->干接点 通信状态查询
{
    uint8_t link_status;  
    uint8_t sum;
    
}link_status_frame_t;

typedef struct    //查询干接点版本    
{
    uint8_t type; 
    uint8_t sum;

}elevator_ver_frame_t;


typedef struct       //干接点应答 
{
    uint8_t type; 
    uint8_t ver1;
    uint8_t ver2;
    uint8_t sum;
    
}ver_ack_frame_t;


typedef struct   
{
    uint8_t exp_cmd;
    uint8_t project_num[16];
    uint8_t adf[2];
    uint8_t inter_num;
    uint8_t sum;
}anti_copy_info_t;

typedef struct
{
    uint8_t s_status;                         //发送状态
    uint8_t rs485_send_len;                   //发送数据长度
    uint8_t rs485_send_buf[FRAME_MAX_SIZE];   //发送缓冲区

    uint8_t rev_buff_full;                    // 1接收到1帧数据，0没有数据
    uint8_t rs485_rev_buf[FRAME_MAX_SIZE];
    uint8_t rev_cnt;
    uint8_t rev_len;                          //接收长度
     
    uint8_t s_interval_time;                  //发送总线空闲时间
    uint8_t r_interval_time;                  //接收总线空闲时间

    uint8_t s_delay;
}rs485_port_t;

/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/
uint8_t  check_sum(uint8_t *point, uint8_t len);
uint8_t  get_check_sum(uint8_t *point, uint8_t len);
uint8_t get_rs485_frame(RS485_ID_E id, uint8_t **pbuf, uint8_t *len);
uint8_t rs485_send_frame(RS485_ID_E id, uint8_t *paddr, uint8_t len);

void del_frame_queue(rs485_frame_msg *point);
rs485_frame_msg *append_frame_queue(uint16_t len);
void rs485_drv_init(void);
uint8_t *set_frame_head(uint8_t *addr, uint8_t cmd, uint8_t len, uint8_t dest_addr);
void rs485_drv_real_time(void);

uint8_t com_hal_get_idle_time(RS485_ID_E id, ENUM_HAL_IDLE_TIME_T type);
void rs485_send_delay(void);
void rs485_rev_irq_callback(RS485_ID_E id);

uint8_t set_frame_id(RS485_ID_E id);
uint8_t com_hal_is_sending(RS485_ID_E id);
#endif

