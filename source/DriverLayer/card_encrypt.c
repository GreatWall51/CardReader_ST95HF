/********************************************************************************
**
** 文件名:     	 encrypt.c
** 版权所有:   (c) 	 2016 厦门立林科技有限公司
** 文件描述:
**
*********************************************************************************
**             修改历史记录
**===============================================================================
**| 日期        | 作者     |  修改记录
**===============================================================================
**| 2016/04/22  | Kimson  |  创建该文件
**
*********************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "card_encrypt.h"

/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
static unsigned char const shift_data[] =
{
        0x00,0xFE,0xFF,0x01,0xFD,0x03,0x02,0xFC,0xF9,0x07,0x06,0xF8,0x04,0xFA,0xFB,0x05,
        0xF1,0x0F,0x0E,0xF0,0x0C,0xF2,0xF3,0x0D,0x08,0xF6,0xF7,0x09,0xF5,0x0B,0x0A,0xF4,
        0xE1,0x1F,0x1E,0xE0,0x1C,0xE2,0xE3,0x1D,0x18,0xE6,0xE7,0x19,0xE5,0x1B,0x1A,0xE4,
        0x10,0xEE,0xEF,0x11,0xED,0x13,0x12,0xEC,0xE9,0x17,0x16,0xE8,0x14,0xEA,0xEB,0x15,
        0xC1,0x3F,0x3E,0xC0,0x3C,0xC2,0xC3,0x3D,0x38,0xC6,0xC7,0x39,0xC5,0x3B,0x3A,0xC4,
        0x30,0xCE,0xCF,0x31,0xCD,0x33,0x32,0xCC,0xC9,0x37,0x36,0xC8,0x34,0xCA,0xCB,0x35,
        0x20,0xDE,0xDF,0x21,0xDD,0x23,0x22,0xDC,0xD9,0x27,0x26,0xD8,0x24,0xDA,0xDB,0x25,
        0xD1,0x2F,0x2E,0xD0,0x2C,0xD2,0xD3,0x2D,0x28,0xD6,0xD7,0x29,0xD5,0x2B,0x2A,0xD4,
        0x81,0x7F,0x7E,0x80,0x7C,0x82,0x83,0x7D,0x78,0x86,0x87,0x79,0x85,0x7B,0x7A,0x84,
        0x70,0x8E,0x8F,0x71,0x8D,0x73,0x72,0x8C,0x89,0x77,0x76,0x88,0x74,0x8A,0x8B,0x75,
        0x60,0x9E,0x9F,0x61,0x9D,0x63,0x62,0x9C,0x99,0x67,0x66,0x98,0x64,0x9A,0x9B,0x65,
        0x91,0x6F,0x6E,0x90,0x6C,0x92,0x93,0x6D,0x68,0x96,0x97,0x69,0x95,0x6B,0x6A,0x94,
        0x40,0xBE,0xBF,0x41,0xBD,0x43,0x42,0xBC,0xB9,0x47,0x46,0xB8,0x44,0xBA,0xBB,0x45,
        0xB1,0x4F,0x4E,0xB0,0x4C,0xB2,0xB3,0x4D,0x48,0xB6,0xB7,0x49,0xB5,0x4B,0x4A,0xB4,
        0xA1,0x5F,0x5E,0xA0,0x5C,0xA2,0xA3,0x5D,0x58,0xA6,0xA7,0x59,0xA5,0x5B,0x5A,0xA4,
        0x50,0xAE,0xAF,0x51,0xAD,0x53,0x52,0xAC,0xA9,0x57,0x56,0xA8,0x54,0xAA,0xAB,0x55
};

static unsigned char get_shift_val(unsigned char data, unsigned char shift_val)
{
    return(shift_data[data ^ shift_val]);
}

static unsigned char calc_shift_val(const unsigned char *pBuff, unsigned int len)
{
    unsigned char val = 0;

    while(len--)
    {
        val = get_shift_val(val,*pBuff);
        pBuff++;
    }

    return val;
}

static unsigned char sum(const unsigned char *pBuff, unsigned int len)
{
    unsigned char sum = 0;
    while(len--)
    {
        sum += *pBuff;
        pBuff++;
    }

    return sum;
}

static void genKeyA(const unsigned char *id_4b, unsigned char *key_6b)
{
    key_6b[0] = calc_shift_val(id_4b,4) ^ 0xa5;
    key_6b[1] = sum(id_4b,4) ^ 0x5a;
    key_6b[2] = (id_4b[0]^0x03) + (id_4b[1]^0x05)+ (id_4b[2]^0x30)+ (id_4b[3]^0x50)+ (id_4b[2]^0x35)+ (id_4b[1]^0x53);
    key_6b[3] = ~calc_shift_val(key_6b,3);
    key_6b[4] = sum(key_6b,4);
    key_6b[5] = calc_shift_val(key_6b,5);
}

static void genKeyB(const unsigned char *id_4b, unsigned char *key_6b)
{
    key_6b[0] = (id_4b[0]^0x0C) + (id_4b[1]^0x0A)+ (id_4b[2]^0xC0)+ (id_4b[3]^0xA0)+ (id_4b[2]^0xCA)+ (id_4b[1]^0xAC);
    key_6b[1] = calc_shift_val(id_4b,4)^0x5a;
    key_6b[2] = sum(id_4b,4) ^ 0xa5;
    key_6b[3] = ~calc_shift_val(key_6b,3);
    key_6b[4] = sum(key_6b,4);
    key_6b[5] = calc_shift_val(key_6b,5);
}



/*******************************************************************
** 函数名:      gen_key_ic
** 函数描述:    根据输入信息，生成IC卡密钥
** 参数:       	输入:
                     type - 0: KeyA, 1:KeyB
                     id_4b - 4Bytes 卡号
                     sec_1b - 1Bytes 扇区编号
                     project_16b - 16Bytes 项目名
               输出:
                     key_6b - 6Bytes 密钥
** 返回:
** 注意:
** 记录: 	 2016/04/22,  Kimson创建
********************************************************************/
void  gen_key_ic(const unsigned char type, const unsigned char *id_4b, const unsigned char sec_1b, const unsigned char *project_16b, unsigned char *key_6b)
{
    unsigned char key[6];
    unsigned char tmp;
    unsigned char sum_val;

    if(type)
		{
			  genKeyB(id_4b,key);
		}
    else
		{
			  genKeyA(id_4b,key);
		}

    tmp = calc_shift_val(project_16b,16);
    tmp = get_shift_val(tmp,sec_1b);
    sum_val = sum(project_16b,16);

    key_6b[0] = key[3] + project_16b[0] + tmp + key[4];
    key_6b[1] = key[2] + (sum_val ^ tmp);
    key_6b[2] = ~calc_shift_val(key_6b,2) + sum_val;
    key_6b[3] = key[0] + (project_16b[3]^sum_val) + project_16b[13] + key[1] + (project_16b[10] ^ tmp)+ project_16b[5];
    key_6b[4] = key[5] + (sum_val ^ tmp);
    key_6b[5] = calc_shift_val(key_6b,5);

}

/*******************************************************************
** 函数名:      gen_key_cpu
** 函数描述:    生成CPU卡密钥
** 参数:       	输入:
                     id_4b - 4Bytes 卡号
                     adf_2b - 2Bytes ADF编号
                     keyid_1b - 1Byte 目标KEY ID号
                     project_16b - 16Bytes 项目名
               输出:
                     key_16b - 16Bytes 密钥

** 返回:
** 注意:
** 记录: 	 2016/04/22,  Kimson创建

********************************************************************/
void  gen_key_cpu(const unsigned char *id_4b, const unsigned char *adf_2b, const unsigned char keyid_1b, const unsigned char *project_16b, unsigned char *key_16b)
{
    unsigned char buf[4];

    buf[0] = adf_2b[0] + sum(id_4b,4);
    buf[1] = adf_2b[1];
    buf[2] = keyid_1b;
    buf[3] = calc_shift_val(project_16b,16);

    genKeyB(buf,key_16b);

    buf[0] = id_4b[0] + sum(id_4b,4) + 0x5A;
    buf[1] = id_4b[1] + calc_shift_val(id_4b,4);
    buf[2] = id_4b[2] + keyid_1b;
    buf[3] = id_4b[3] + 0xA5;

    gen_key_ic(0,buf,keyid_1b+adf_2b[0],project_16b,key_16b+6);

    key_16b[12] = sum(key_16b,12) + 0x85 + keyid_1b;
    key_16b[13] = sum(adf_2b,2) + keyid_1b + calc_shift_val(key_16b + 6, 6);
    key_16b[14] = (id_4b[0]^ 0x5a) + (project_16b[10] ^ 0x65)+sum(key_16b, 6);
    key_16b[15] = calc_shift_val(key_16b,15)+0x55;
}


/*---------------------------------------------------------------------------*/

