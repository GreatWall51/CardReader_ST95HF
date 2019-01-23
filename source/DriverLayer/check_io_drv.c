/******************************************************************************

                  版权所有 (C), 2015-2025, 立林科技有限公司

 ******************************************************************************
  文 件 名   : check_io_drv.c
  版 本 号   : 初稿
  作    者   : wzh
  生成日期   : 2016年2月23日
  最近修改   :
  功能描述   : check_io
  函数列表   :
  修改历史   :
  1.日    期   : 2016年2月23日
    作    者   : wzh
    修改内容   : 创建文件

******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "timer_drv.h"
#include "define.h" 
#include "check_io_drv.h"
/*----------------------- Constant / Macro Definitions -----------------------*/


/*------------------------ Variable Define/Declarations ----------------------*/
#define OLD_CODE    0

uint8_t now_addr = 0xff;
typedef struct
{
    uint8_t io_down;
    uint8_t io_old;
    uint8_t io_check_ok;
}io_info_t;

static io_info_t io_info;
/*----------------------------- External Variables ---------------------------*/
 

/*------------------------ Function Prototype --------------------------------*/
 
 
/*------------------------ Variable Define/Declarations ----------------------*/
 
 
/*------------------------ Function Implement --------------------------------*/
 
 
/*---------------------------------------------------------------------------*/

extern void uart_putc(unsigned char c);


static uint16_t get_gpio_sta(void)
{
	uint16_t pb, pc;

	pb = (GPIO_ReadInputData(GPIOA) >> 10) & SET_BIT5;
	pb |= (GPIO_ReadInputData(GPIOB) << 3) & (SET_BIT6 | SET_BIT7);
	pb |= (GPIO_ReadInputData(GPIOB) >> 11) & SET_BIT0; 
	pb |= (GPIO_ReadInputData(GPIOB) >> 14) & SET_BIT1;
	pc = GPIO_ReadOutputData(GPIOB) & SET_BIT2;

	return (uint8_t)(pb | pc);
}

static void io_info_init(void)
{
    io_info.io_check_ok = 0;
    io_info.io_down = 0;
    io_info.io_old  = get_gpio_sta();
}


void check_io_drv_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOC, ENABLE);	 //13,14,15地址				  											 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 

	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOB, ENABLE);	 	
											 //防拆防移离  //门状态     //火警
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_15;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);		
												 //出门按钮
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;                 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	io_info_init();
}


/*****************************************************************************
 函 数 名  : check_io_real_time
 功能描述  : 检测io口
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void check_io_real_time(void)
{
	uint8_t	l1 ;
	static uint8_t l2 = 0;

	
  if(!IS_TIMER_EVT(TIME_EVT_128)) return;   
	
	l1 = get_gpio_sta();
	
	if ( l1 != l2 )
	{
		l2 = l1 ;
		return;
	}
			
	//io_down |= ( io_old ^ l1 ) & io_old ;
	io_info.io_down |= ( io_info.io_old ^ l1 ) ; //检测io口是否发生变化
	io_info.io_old = l1 ;
	io_info.io_check_ok = 1;

}



uint8_t get_io_evt( uint8_t no )
{
	uint8_t key = 1ul << no ;

	if ( io_info.io_down & key )
	{	
		io_info.io_down &= ~key ;
		return 1 ;
	}
	return 0 ;
}


uint8_t get_io_level( uint8_t no )
{
	uint16_t key = 1ul << no ;

    
	if ( io_info.io_old & key )
	{		
		return 1 ;
	}
	return 0 ;
}


/*****************************************************************************
 函 数 名  : check_addr_real_time
 功能描述  : 检测读头地址
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
void check_addr_real_time(void)
{
    uint8_t addr;
    static uint8_t addr_buf = 0xff;
    	
    if(!IS_TIMER_EVT(TIME_EVT_128))	return ;

    addr =  (uint8_t)(GPIO_ReadInputData(GPIOC) >> 11) & SET_BIT2;
    addr |= (uint8_t)(GPIO_ReadInputData(GPIOC) >> 13) & SET_BIT1;
    addr |= (uint8_t)(GPIO_ReadInputData(GPIOC) >> 15) & SET_BIT0;
    
    addr = ~addr;
    addr &= 0x07;  //读头地址只有低三位
    if(addr != addr_buf)
    {
        addr_buf = addr;
        return ;
    }
    now_addr = addr_buf | 0x10;  //读头地址只能为0x1x
    
}

/*****************************************************************************
 函 数 名  : get_reader_addr
 功能描述  : 获取读头地址
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2016年4月13日
    作    者   : wzh
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t get_reader_addr(void)
{
    if(now_addr > 0x17)
    {
        return 0;
    }
    
    return now_addr;
}
