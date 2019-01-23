#include "stm32f0xx_conf.h"
#include "define.h"
#include "wdg_drv.h"
#include "timer_drv.h"
#include "beep_drv.h"
#include "led_drv.h"
#include "lock_drv.h"
#include "card_read_drv.h"
#include "check_io_drv.h"
#include "check_mode.h"
#include "param_mode.h"
#include "check_card_mode.h"
#include "card_rw_app.h"
#include "rs485_drv.h"
#include "rs485_mode.h"
#include "com_hal_layer.h"
#include "hal_BT.h"
#include "BT_mode.h"
#include "debug.h"  
#include "sim_uart_drv.h"

void flash_read_pro(void)
{
   if(SET != FLASH_OB_GetRDP())
   {
      FLASH_Unlock();
      FLASH_OB_Unlock();
      FLASH_OB_RDPConfig(OB_RDP_Level_1);
      FLASH_OB_Launch();
      FLASH_OB_Lock();    
      FLASH_Lock();
   }
}

void drv_init(void)
{
#if DEBUG_COM_CONFIG
	debug_init();
#endif
	init_timer();
	beep_drv_init();
	led_drv_init();
	lock_drv_init();
	check_io_drv_init();
	param_mode_init();
	card_read_init();
	rs485_drv_init();
#if (!DEBUG_COM_CONFIG)
	com_hal_init();
	wdg_drv_init();
#endif
	sim_uart_init();

}

void mode_init(void)
{
	rs485_mode_init();
	card_rw_init();
#if (!DEBUG_COM_CONFIG)
	bt_mode_init();
#endif
}

void drv_run_real_time(void)
{
	timer_real_time();
	beep_real_time();
	led_real_time();
	lock_real_time();
	check_io_real_time();
	check_addr_real_time();
	card_read_real_time();
	card_rw_real_time();
	rs485_drv_real_time();
	com_hal_real_time();
#if (!DEBUG_COM_CONFIG)
	recv_bt_fram_real_time();
#endif
}

void test_io_init(void)
{
   GPIO_InitTypeDef  GPIO_InitStructure;
	
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);					  											 
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;                 
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOB, &GPIO_InitStructure);
	 cbi(GPIOB,GPIO_Pin_12);
}
//void test_sim_uart_send(void)
//{
//	uint8_t data_buf[5] = {0xff,0xef,0xcd,0x00,0xaa};
////	uint8_t data_buf[5] = {0x01,0x02,0x03,0x04,0x05};
//	if(!IS_TIMER_EVT(TIME_EVT_512)) return ;
//	SimUART_TxBytes(data_buf,5);
//}

//void test_sim_uart_recv(void)
//{
//	uint8_t data_buf[20] = {0},len;
//	if(!IS_TIMER_EVT(TIME_EVT_64)) return ;
//	SimUART_RxBytes(data_buf,&len);
//	if(len>0)
//	{
//	SimUART_TxBytes(data_buf,len);
//	}
//}

//void test_bt_send()
//{
//	uint8_t len=0;
//	uint8_t data[20]={0};//{0x01,0x02,0x03,0x04,0x05};
//	if(!IS_TIMER_EVT(TIME_EVT_1024)) return;
//	if(bt_get_fram(data,&len))
//	{
//		bt_send_fram(data,len);
//	}
//}
void test_sim_rs485_send(void)
{
    uint8_t data[20]={0xaa,0x02,0x03,0x04,0x05};
    if(!IS_TIMER_EVT(TIME_EVT_1024)) return;
    rs485_send_frame(RS485_SIM,data,5);
}

void test_sim_rs485_recv(void)
{
	uint8_t *pbuf;
	frame_head_t *frame_head;
	uint8_t len = 0;
	uint8_t buf[30] = {0};
	
	if(get_rs485_frame(RS485_SIM, &pbuf, &len) == 1)
	{
		frame_head = (frame_head_t *)pbuf;
		memcpy(buf,(uint8_t *)frame_head,len);
		rs485_send_frame(RS485_SIM,(uint8_t*)frame_head,len);
	}	
}
void app_run_real_time(void)
{
    check_evt_real_time();
    check_card_real_time();
    rs485_mode_real_time();
#if (!DEBUG_COM_CONFIG)
		bt_mode_real_time();
#endif
		
//	test_sim_uart_send();
//	test_sim_uart_recv();
//		test_bt_send();
//    test_sim_rs485_send();
//	test_sim_rs485_recv();
//    debug_real_time();
}


int main(void)
{
//		flash_read_pro();
    drv_init();
    mode_init();
    while (1)
    {
        drv_run_real_time();
        app_run_real_time();
#if (!DEBUG_COM_CONFIG)
        wdg_feed_real_time();
#endif
    }
}

