/******************************************************************************
* Copyright 2010-2011 Renyucjs@163.com
* FileName: 	 com_hal_layer.c 
* Desc:
* 
* 
* Author: 	 Kimson
* Date: 	 2015/03/02
* Notes: 
* 
* -----------------------------------------------------------------
* Histroy: v1.0   2015/03/02, Kimson create this file
* 
******************************************************************************/
 
 
/*------------------------------- Includes ----------------------------------*/
#include "com_hal_layer.h" 
#include "timer_drv.h"
#include <string.h>
#include "com_func_config.h" 



#include "debug.h"
/*------------------- Global Definitions and Declarations -------------------*/

 
/*---------------------- Constant / Macro Definitions -----------------------*/
 
 
/*----------------------- Type Declarations ---------------------------------*/

 
/*----------------------- Variable Declarations -----------------------------*/

const com_hal_fun_t c_hal_fun_config[CFG_COM_PORT_CNT] = 
{
    {uart2_init, uart2_start_send, uart2_stop_send, uart2_send},
//		{rs485_id1_init,rs485_id1_start_send,rs485_id1_stop_send,rs485_id1_send},
};

com_hal_port_t s_hal_port[CFG_COM_PORT_CNT];

/*----------------------- Function Prototype --------------------------------*/
 
 
/*----------------------- Function Implement --------------------------------*/

/******************************************************************************
* Name: 	 com_hal_init 
*
* Desc: 	 
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2015/03/03, Create this function by Kimson
 ******************************************************************************/
void com_hal_init()
{
    uint32_t i;
    
    /* Port's data initial */
    for(i = 0; i < CFG_COM_PORT_CNT; i++)
    {
				/*!< Clear all things */
        memset(&s_hal_port[i], 0, sizeof(s_hal_port[0]));	
       	/*!< initial hal portable funs */
       	s_hal_port[i].hal_funs =  c_hal_fun_config[i];
        /* init the next hw layer */
       	if(s_hal_port[i].hal_funs.p_fn_hal_init != NULL) s_hal_port[i].hal_funs.p_fn_hal_init();
    }
} 

/******************************************************************************
* Name: 	 com_hal_real_time 
*
* Desc: 	 run in main runtime
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2015/03/03, Create this function by Kimson
 ******************************************************************************/
void com_hal_timer_real_time(void)
{
    uint32_t i;
    for(i = 0; i < CFG_COM_PORT_CNT; i++)
    {
        if(s_hal_port[i].s_timer)
        {
            s_hal_port[i].s_timer--;
            if(s_hal_port[i].s_timer == 0)
            {	/*!< Something wrong, restart */
                s_hal_port[i].s_status = HAL_SEND_STA_FINISH;
            }
        }
        ENTER_CRITICAL();
        if(s_hal_port[i].r_interval_timer < CFG_COM_HAL_MAX_INTERVAL_CNT) s_hal_port[i].r_interval_timer++;
        if(s_hal_port[i].s_interval_timer < CFG_COM_HAL_MAX_INTERVAL_CNT) s_hal_port[i].s_interval_timer++;
        EXIT_CRITICAL();        
    }
}
void com_hal_real_time()
{		 
    uint32_t i;
    for(i = 0; i < CFG_COM_PORT_CNT; i++)
    {
        if(s_hal_port[i].s_status == HAL_SEND_STA_PRE_SEND)
        {
            s_hal_port[i].s_status = HAL_SEND_STA_SENDING;
            
            ENTER_CRITICAL();
            s_hal_port[i].s_timer = CFG_COM_HAL_MAX_SEND_TIME;
            EXIT_CRITICAL();
            
            if(s_hal_port[i].hal_funs.p_fn_hal_start_send != NULL)
						{
              s_hal_port[i].hal_funs.p_fn_hal_start_send();
						}
        }
        else if(s_hal_port[i].s_status == HAL_SEND_STA_FINISH)
        {
            if(s_hal_port[i].hal_funs.p_fn_hal_stop_send != NULL)
               s_hal_port[i].hal_funs.p_fn_hal_stop_send();
            s_hal_port[i].s_status = HAL_SEND_STA_IDLE;
            ENTER_CRITICAL();
            s_hal_port[i].s_timer = 0;
            EXIT_CRITICAL();
            s_hal_port[i].s_len = 0;
            s_hal_port[i].s_sent_point = 0;
        }
        if(IS_TIMER_EVT(TIME_EVT_1))
        {
            if(s_hal_port[i].s_timer)
            {
                s_hal_port[i].s_timer--;
                if(s_hal_port[i].s_timer == 0)
                {	/*!< Something wrong, restart */
                    s_hal_port[i].s_status = HAL_SEND_STA_FINISH;
                }
            }
            ENTER_CRITICAL();
            if(s_hal_port[i].r_interval_timer < CFG_COM_HAL_MAX_INTERVAL_CNT) s_hal_port[i].r_interval_timer++;
            if(s_hal_port[i].s_interval_timer < CFG_COM_HAL_MAX_INTERVAL_CNT) s_hal_port[i].s_interval_timer++;
            EXIT_CRITICAL();
        }
    }
    return;
}

/******************************************************************************
* Name: 	 com_hal_receive_irq_callback 
*
* Desc: 	 this function must be call by user hal data receive IRQ
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2015/03/03, Create this function by Kimson
 ******************************************************************************/
void com_hal_receive_irq_callback(ENUM_COM_PORT_T port, uint8_t data)
{
	
    com_hal_port_t *p_port;
    p_port = &s_hal_port[port];
    p_port->r_cyc_buffer[p_port->r_cyc_tail & COM_HAL_CYC_MASK] = data;
    p_port->r_cyc_interval[p_port->r_cyc_tail & COM_HAL_CYC_MASK] = p_port->r_interval_timer;

    ENTER_CRITICAL();
    p_port->r_cyc_tail++;
    p_port->r_cyc_number++;
    p_port->r_interval_timer = 0;
    EXIT_CRITICAL();
}
/******************************************************************************
* Name: 	 com_hal_send_irq_callback 
*
* Desc: 	 
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2015/03/03, Create this function by Kimson
 ******************************************************************************/
void com_hal_send_irq_callback(ENUM_COM_PORT_T port)
{
    com_hal_port_t *p_port;

    p_port = &s_hal_port[port];

		if(p_port->s_len > p_port->s_sent_point)
		{    
				p_port->hal_funs.p_fn_hal_send(p_port->s_frame[p_port->s_sent_point]);
				ENTER_CRITICAL();
				p_port->s_sent_point++;
				p_port->s_interval_timer = 0;
				EXIT_CRITICAL();            
		}
		else
		{
				p_port->s_status = HAL_SEND_STA_FINISH;
				if(p_port->hal_funs.p_fn_hal_stop_send != NULL)
						p_port->hal_funs.p_fn_hal_stop_send();
		}
}

/******************************************************************************
* Name: 	 com_hal_receive_data 
*
* Desc: 	 com_hal接收来自drv的数据
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2015/03/03, Create this function by Kimson
 ******************************************************************************/
uint8_t com_hal_receive_data(ENUM_COM_PORT_T port, uint8_t *p_interval)
{
    com_hal_port_t *p_port;
    uint8_t data = 0;
    
    p_port = &s_hal_port[port];
    
    if(p_port->r_cyc_number > 0)
    {
				
        data = p_port->r_cyc_buffer[p_port->r_cyc_head & COM_HAL_CYC_MASK]; //从环形缓冲区读入数据
        *p_interval = p_port->r_cyc_interval[p_port->r_cyc_head & COM_HAL_CYC_MASK];
        ENTER_CRITICAL();
        p_port->r_cyc_head++; //接收处理指令前移
        p_port->r_cyc_number--; //未处理计数减1
        EXIT_CRITICAL();
    }
    return data;
}
/******************************************************************************
* Name: 	 com_hal_have_new_data 
*
* Desc: 	 
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2015/03/03, Create this function by Kimson
 ******************************************************************************/
uint8_t com_hal_have_new_data(ENUM_COM_PORT_T port)
{
    return s_hal_port[port].r_cyc_number;
}

/******************************************************************************
* Name: 	 com_hal_send 
*
* Desc: 	 
* Param: 	 
* Return: 	 
* Global: 	 
* Note: 	 
* Author: 	 Kimson
* -------------------------------------
* Log: 	 2015/03/03, Create this function by Kimson
 ******************************************************************************/
 uint8_t com_hal_send(ENUM_COM_PORT_T port, uint8_t *p_buf, uint16_t len)
{
    com_hal_port_t *p_port;

    if((port >= CFG_COM_PORT_CNT) || (len > CFG_COM_HAL_SEND_BUF_SIZE) || (p_buf == NULL)) return 0;
    
    p_port = s_hal_port + port;
    if(p_port->s_status != HAL_SEND_STA_IDLE) return 0;
    memcpy(p_port->s_frame, p_buf, len);
    p_port->s_sent_point = 0;
    p_port->s_len = len;
    p_port->s_status = HAL_SEND_STA_PRE_SEND;
    return 1;
}

com_hal_port_t * com_hal_get_port_info(ENUM_COM_PORT_T port)
{
     com_hal_port_t *p_port;
     p_port = &s_hal_port[port];

     return p_port;
} 
/*---------------------------------------------------------------------------*/

