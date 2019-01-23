/********************************************************************************
**
** �ļ���:     	 Card_read_drv.h 
** ��Ȩ����:   (c) 	 2016 �������ֿƼ����޹�˾
** �ļ�����:   
**
*********************************************************************************
**             �޸���ʷ��¼
**===============================================================================
**| ����        | ����     |  �޸ļ�¼
**===============================================================================
**| 2016/02/24  | yeqizhi  |  �������ļ�
**
*********************************************************************************/
#ifndef _CARD_READ_DRV_H_     
#define _CARD_READ_DRV_H_    
 
/*-------------------------------- Includes ----------------------------------*/
 #include <string.h>
 #include "stm32f0xx.h"
 
/*------------------------------ Global Defines ------------------------------*/

#define    PCD_FM1702    1
#define    PCD_FM1722    2
#define    CFG_PCD_TYPE    PCD_FM1722      /*!< Choose one pcd dev from above PCD_DEV_LIST */

#define    Type_KEYA   0
#define    Type_KEYB   1

#define    IC_SCAN_TIMES_START  0x01       //��ʼɨ��IC��
#define    IC_SCAN_TIMES        0x01       //IC��ɨ�����
#define    SFZ_SCAN_TIMES       0x01       //���֤ɨ�����
#define	   NFC_SCAN_TIMES       0x01       //NFC��ɨ�����




//���ʱ��������ɨ�迨������,����ɨ��3�ֿ�,ʱ�����������3
#define IC_INTERVAL_TIME    5 
#define SFZ_INTERVAL_TIME   5
#define NFC_INTERVAL_TIME   5
 
/*------------------------------ Global Typedefs -----------------------------*/
typedef struct   CARD_PARA_S{
    uint8_t card_ab_type;       //������  0:ֻ��IC��  1:֧�ֶ����֤��IC��
    uint8_t anti_copy;         //IC�����Ʋ�����81-8F
    uint8_t project_num[16];      //��Ŀ��
    uint8_t adf[2];         //ADF��ʾ
    uint8_t key_inter;     //�ڲ���Կ��
}CARD_PARA_STR;

enum card_type{
    CARD_NONE = 0x00,
    TYPE_ID=0x01,
    TYPE_IC=0x11,
    TYPE_IC_NO_REG=0x91,
    TYPE_CPU=0x12,
    TYPE_CPU_NO_REG=0x92,
    TYPE_SFZ=0X21,
    TYPE_IC_B=0X22,  
    TYPE_NFC_NO_REG=0x93,
    TYPE_NFC = 0x31,
};

enum card_uid_len{
    CARD_IC_LEN=0x04,
    CARD_SFZ_LEN=0x08, 
	  CARD_NFC_LEN=0x08, 
};

enum work_mode{
    IC_MODE = 0x00,
    SFZ_MODE =0x01, 
	  NFC_MODE = 0X02,
};

enum ANTI_COPY_PARAM_E{
    ANTI_COPY_NONE = 0x00,
    ANTI_COPY_IC = 0x8F,       //IC��������
    ANTI_COPY_CPU = 0xC0,      //CPU��������
};


/*----------------------------- External Variables ---------------------------*/
typedef struct
{
    uint8_t  card_type;      //������
    uint8_t  card_len;       //��uid����
    uint8_t  card_uid[8];    //��uidֵ
}CARD_INF0_T;  
 
/*------------------------- Global Function Prototypes -----------------------*/
void card_read_set_para(CARD_PARA_STR *card_para);
void card_read_init(void);
void card_read_real_time(void);
uint8_t card_get_event(CARD_INF0_T *card_data_str);
uint8_t cehck_have_card(void);
uint8_t get_reader_mode(void);

#endif //_CARD_READ_APP_H_
