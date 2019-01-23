/********************************************************************************
**
** �ļ���:     	 card_rw_drv.h 
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
#ifndef _CARD_RW_DRV_H_     
#define _CARD_RW_DRV_H_    
 
 
/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "card_read_drv.h "
/*------------------------------ Global Defines ------------------------------*/

#define DATE_BUFF_MAX    128 
#define CPU_WRITE_MAX (DATE_BUFF_MAX - 31)   
#define CPU_READ_MAX (DATE_BUFF_MAX - 12) 

enum CMD_AUX_E{
    PARA_SET = 0x01,
    PARA_REQ = 0x02,
    IC_BLOCK_WRITE = 0x11,
    IC_BLOCK_READ = 0x12,
    CPU_IN_AUTH = 0x20,
    CPU_EF_WRITE = 0x21,
    CPU_EF_READ = 0x22,
};

/* CPU�������Ʋ���  ...added by yeqizhi, 2016-04-21 */
typedef struct   CPU_PARA_S{
    uint8_t cmd_aux;      //����ָ��  0x01
    uint8_t project_num[16];      //��Ŀ��
    uint8_t adf[2];         //ADF��ʾ
    uint8_t key_inter;     //�ڲ���Կ��
    uint8_t check_sum;
}CPU_PARA_STR;


/* CPU��������ָ��ṹ ...added by yeqizhi, 2016-04-21 */
typedef struct   CPU_CMD_S{
    uint8_t CPU_PCB;
    uint8_t card_id;
    uint8_t CLA;
    uint8_t INS;
    uint8_t P1;
    uint8_t P2;
    uint8_t LC;
    uint8_t data_buff[CPU_WRITE_MAX];
    uint8_t LE;
}CPU_CMD_STR;

typedef struct 
{
  uint8_t CPU_PCB;
  uint8_t card_id;
  uint8_t CLA;
  uint8_t INS;
  uint8_t P1;
  uint8_t P2;
  uint8_t LE;
}CPU_CMD_ND_STR;




/* IC������д��ͨ�����ݽṹ ...added by yeqizhi, 2016-04-21 */
typedef struct{
    uint8_t cmd_aux;      //����ָ��  0x11
    uint8_t uid[4];       //����
    uint8_t block_cn;     //���   /4��������  %4��������block��
    uint8_t key_cn;       //��Կ��:0x60 KEYA  0X61 KEYB
    uint8_t key[6];       //��Կvalue
    uint8_t block_buffer[16];      //block����value
}IC_BLOCK_RW_STR;

/* IC������д���������ϴ�ͨ�����ݽṹ ...added by yeqizhi, 2016-04-21 */
typedef struct{
    uint8_t cmd_aux;      //����ָ��0X15
    uint8_t uid[4];
    uint8_t block_cn;
    uint8_t result;     //IC�����������0x01�ɹ� 0x02ʧ��
    uint8_t payload[16];
} IC_RESULT_STR;

/* CPU���ڲ���֤ͨ�����ݽṹ*/
typedef struct   CPU_IN_AUTH_S
{
	uint8_t cmd_aux;      //����ָ��0X20
	uint8_t uid[4];
	uint8_t adf[2];
	uint8_t key_cn;
	uint8_t key_len;
	uint8_t key[16];
	uint8_t result;
	uint8_t result_expand;
} CPU_IN_AUTH_T;

/* CPU���ڲ���֤��� ...added by yeqizhi, 2016-04-21 */
typedef struct   CPU_IN_AUTH_RESULT_S{
    uint8_t cmd_aux;      //����ָ��0X20
    uint8_t uid[4];
    uint8_t adf[2];
    uint8_t result;
    uint8_t result_expand;
}CPU_IN_AUTH_RESULT_STR;

/* CPU��д��¼ͨ�����ݽṹ ...added by yeqizhi, 2016-04-21 */
typedef struct   CPU_WRITE_S{
    uint8_t cmd_aux;             //����ָ��0X21
    uint8_t uid[4];              //cpu������
    uint8_t adf[2];              //adf�ļ��б�ʶ
    uint8_t ef[2];               //ef�ļ���ʶ
    uint8_t key_cn;              //��Կ��
    uint8_t key_len;             //��Կ����
    uint8_t key[16];             //��Կ����
    uint8_t ef_type[2];          //ef_type[0]:������¼�ļ���������ļ�,ef_type[1]ÿ����¼��С
    uint8_t addr_start;          //��ʼд��λ��,��С���Ϊ1,������0   ��¼Ϊ��¼�ţ�������Ϊƫ����
    uint8_t date_len;            //д���ݵĳ���
    uint8_t date_payload[CPU_WRITE_MAX];   //���֧��200���ֽ�д��
}CPU_WRITE_STR;
/* CPU������¼ͨ�����ݽṹ ...added by yeqizhi, 2016-04-21 */
typedef struct   CPU_READ_S{
    uint8_t cmd_aux;      //����ָ��0X22
    uint8_t uid[4];
    uint8_t adf[2];
    uint8_t ef[2];
    uint8_t key_cn;
    uint8_t key_len;
    uint8_t key[16];
    uint8_t ef_type[2];
    uint8_t addr_start;      //��ʼд��λ��   ��¼Ϊ��¼�ţ�������Ϊƫ����
    uint8_t date_len;
}CPU_READ_STR;

/* CPU������¼ͨ�����ݽṹ ...added by yeqizhi, 2016-04-21 */
typedef struct   CPU_RESULT_READ_S{
    uint8_t cmd_aux;      //����ָ��0X22
    uint8_t uid[4];
    uint8_t adf[2];
    uint8_t ef[2];
    uint8_t result;
    uint8_t addr_start;      //��ʼ����λ��   ��¼Ϊ��¼�ţ�������Ϊƫ����
    uint8_t date_len;
    uint8_t date_payload[CPU_READ_MAX];   //���֧��200���ֽڶ���
}CPU_RESULT_READ_STR;

/* CPU��д��¼ͨ�����ݽṹ ...added by yeqizhi, 2016-04-21 */
typedef struct   CPU_RESULT_S{
    uint8_t cmd_aux;      //����ָ��0X15
    uint8_t uid[4];
    uint8_t adf[2];
    uint8_t ef[2];
    uint8_t result;
    uint8_t result_expand;
}CPU_RESULT_STR;

typedef struct   UNI_CMD_REPEAT_S{
    uint8_t cmd_aux;      
    uint8_t payload;
}UNI_CMD_REPEAT_STR;


#define  SUCCESS  0x01
#define  FAIL     0x02

typedef enum
{
    NONE = 0X00,  
    CARD_NONE_ERR=0x01, 
    CARD_ERR =0x02,
    IC_AUTH_ERR = 0x03,
    MF_ERR = 0x03,
    BLOCK_ERR =0x04,
    ADF_ERR =0x04,
    CPU_AUTH_ERR =0x05,
    EF_ERR =0x06,
    EF_OPERATE_ERR =0x07,
    IC_WRITE_SUCCESS = 0x08,
    IC_READ_SUCCESS = 0x09,
    CPU_OPERATE_SUCCESS =0x0a,
}ADDR_CR_ENUM; 

typedef enum
{
    DATA_USER,  
    DATA_KEY, 
}CARD_DATA_TYPE; 

/*------------------------------ Global Typedefs -----------------------------*/
 
 
/*----------------------------- External Variables ---------------------------*/
 
 
/*------------------------- Global Function Prototypes -----------------------*/
void card_rw_set_para(CARD_PARA_STR *card_para);    
uint8_t card_ic_write(IC_BLOCK_RW_STR       *date_buff,CARD_DATA_TYPE date_type);
uint8_t card_ic_read(IC_BLOCK_RW_STR       *date_buff);
uint8_t cpu_internal_auth ( CPU_IN_AUTH_T* date_buff );
#endif //_CARD_CPU_EXE_H_
