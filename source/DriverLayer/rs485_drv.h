/******************************************************************************

                  ��Ȩ���� (C), 2015-2025, ���ֿƼ����޹�˾

 ******************************************************************************
  �� �� ��   : rs485_drv.h
  �� �� ��   : ����
  ��    ��   : wzh
  ��������   : 2016��2��24��
  ����޸�   :
  ��������   : rs485_drv.h
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��2��24��
    ��    ��   : wzh
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef _RS485_DRV_H_
#define  _RS485_DRV_H_
/*-------------------------------- Includes ----------------------------------*/
#include <stdint.h>
 
/*----------------------- Constant / Macro Definitions -----------------------*/
//#define NULL        ((void*)0)

//�����ʶ���
#define BAUDRATE_ACCESS       19200
#define BAUDRATE_ELEVATOR     4800

//��ͷ�汾��
#define VER1   0X02
#define VER2   0X00

//door & lock status
#define OPEN     0x01
#define CLOSE    0x00

//cmd
#define CHECK_READER_CMD       0x01
#define OPEN_LOCK_CMD          0x02      //�·�������֡(����֤)
#define CHECK_DOOR_LOCK_CMD    0x03
#define CHECK_VER_CMD          0x04

#define CARD_RESULT_UPDATA_CMD           0x15    //�ϴ�ָ��
#define ACK_CARD_RESULT_UPDATA_CMD        0x95

#define ACK_CHECK_CMD          0x81
#define ACK_DOOR_LOCK_CMD      0x83
#define ACK_CHECK_VER_CMD      0x84

#define LINK_CMD               0x23    //֪ͨ�ɽӵ�ͨ�Ŷ�·

#define OPEN_FLOOR_CMD         0x21    //¥��Ȩ�޿���
#define CAR_UPDATA_CMD         0x11    //ˢ���ϴ�
#define CARD_RESULT_UPDATA_CMD         0x15    //�������ϴ�

#define EVT_UPDATA_CMD         0x12    //�¼��ϴ�

#define ACK_CARD_DATA_CMD      0x91
#define ACK_EVT_DATA_CMD       0x92

#define ACCESS_CMD             0x05 //�Ž�������������
#define BT_DATA_PASSTHROUGHT_CMD            0X07      //�����·���������͸��
#define BT_REPORD_DATA_CMD                  0X17      //�����ϱ����ݵ�����
#define ACK_BT_DATA_CMD        0x97      //����Ӧ����������

//�������ֵ
#define QUEUE_MAX_NUM         40
//֡�����ʱ��
#define FRAME_MAX_SEND_TIME    200
//֡���
#define FRAME_INTERVAL_TIME     20


#define SEND_FRAME_INTERVAL_TIME     20

#define FEAME_HEAD_DATA    0xaa

#define FRAME_MAX_SIZE     250
#define FRAME_MIN_SIZE     3
#define FRAME_OFFSET_LEN   1

#define  MALLOC_LEN   20

//֡������ʱ
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

typedef struct        //����Ӧ��֡
{   
    uint8_t sum;
}ack_check_frame_t;

typedef struct        //��ѯ֡  9byte
{   
    uint8_t unlock_time;
    uint8_t anti_copy;  
    uint8_t sum;
}check_frame_t;


typedef struct        //�����·�����֡
{
    uint8_t unlock_time;
    uint8_t beep_mode;
    uint8_t led_mode;
    uint8_t sum;
}lock_frame_t;


typedef struct        //Ӧ��֡,״̬��ѯ֡  7byte
{
    uint8_t sum;
}ack_frame_t;


typedef struct        //������֡ 
{
    uint8_t card_type;
    uint8_t card_len;
    uint8_t card_uid[8]; 
    uint8_t sum;
}card_data_frame_t;

typedef struct           //��ͷ�¼�Ӧ��
{
    uint8_t evt_type;    //�¼�����
    uint8_t expand;      //�����¼�
    uint8_t sum;
    
}reader_evt_frame_t;


typedef struct        //��ͷ״̬Ӧ��
{
    uint8_t door_status;   //0x00 �أ�0x01��
    uint8_t lock_status;   //0x00�أ�0x01��
    uint8_t sum;
    
}reader_status_frame_t;

typedef struct        //��ͷ�汾��Ӧ��֡
{
    uint8_t ver1;  
    uint8_t ver2; 
    uint8_t sum;
    
}reader_ver_frame_t;


typedef struct        //ת���ݿ��ź�
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


typedef struct        //��ͷ->�ɽӵ� ͨ��״̬��ѯ
{
    uint8_t link_status;  
    uint8_t sum;
    
}link_status_frame_t;

typedef struct    //��ѯ�ɽӵ�汾    
{
    uint8_t type; 
    uint8_t sum;

}elevator_ver_frame_t;


typedef struct       //�ɽӵ�Ӧ�� 
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
    uint8_t s_status;                         //����״̬
    uint8_t rs485_send_len;                   //�������ݳ���
    uint8_t rs485_send_buf[FRAME_MAX_SIZE];   //���ͻ�����

    uint8_t rev_buff_full;                    // 1���յ�1֡���ݣ�0û������
    uint8_t rs485_rev_buf[FRAME_MAX_SIZE];
    uint8_t rev_cnt;
    uint8_t rev_len;                          //���ճ���
     
    uint8_t s_interval_time;                  //�������߿���ʱ��
    uint8_t r_interval_time;                  //�������߿���ʱ��

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

