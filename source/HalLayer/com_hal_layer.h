/******************************************************************************
* Copyright 2010-2011 Renyucjs@163.com
* FileName: 	 com_hal_layer.h 
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
#ifndef _COM_HAL_LAYER_H_     
#define _COM_HAL_LAYER_H_    
 
#include "com_config.h" 
#include <stdint.h> 

/*----------------------------- Global Defines ------------------------------*/
enum {
    HAL_SEND_STA_IDLE,
    HAL_SEND_STA_PRE_SEND,    
    HAL_SEND_STA_SENDING,       
    HAL_SEND_STA_FINISH,        
};

typedef enum {
    HAL_IDLE_SEND, 
    HAL_IDLE_REV,
    HAL_IDLE_ALL,
}HAL_IDLE_TIME_E; 
/*----------------------------- Global Typedefs -----------------------------*/
typedef void fn_hal_void_fun_t(void);
//typedef void fn_hal_send_fun_t(uint8_t *data, uint8_t len);
typedef void fn_hal_send_fun_t(uint8_t data);

typedef struct com_hal_fun_str {
    fn_hal_void_fun_t *p_fn_hal_init;
    fn_hal_void_fun_t *p_fn_hal_start_send;
    fn_hal_void_fun_t *p_fn_hal_stop_send;
    fn_hal_send_fun_t *p_fn_hal_send;
}com_hal_fun_t;  

typedef struct com_hal_port_str {
    /* receive */
    uint8_t r_cyc_buffer[CFG_COM_HAL_RECEIVE_CYC_SIZE];
    uint8_t r_cyc_interval[CFG_COM_HAL_RECEIVE_CYC_SIZE];
    uint8_t r_cyc_head;
    uint8_t r_cyc_tail;
    uint8_t r_cyc_number;
    uint8_t r_interval_timer;	/*!< time between two rec char */
    //uint16_t r_interval_timer;	/*!< time between two rec char */
    /* send */
    uint16_t s_len;
    uint16_t s_timer; 
    uint32_t s_status;
    uint8_t s_sent_point;
    uint8_t s_frame[CFG_COM_HAL_SEND_BUF_SIZE];
    uint8_t s_interval_timer;
    //uint16_t s_interval_timer;
    /* hal portable functions */
    com_hal_fun_t hal_funs;
}com_hal_port_t;   
 
/*----------------------------- External Variables --------------------------*/
 
/*------------------------ Global Function Prototypes -----------------------*/
extern void com_hal_init(void); 
void com_hal_timer_real_time(void);
extern void com_hal_real_time(void); 
uint8_t com_hal_have_new_data(ENUM_COM_PORT_T port);
extern uint8_t com_hal_receive_data(ENUM_COM_PORT_T port, uint8_t *p_interval);
extern void com_hal_receive_irq_callback(ENUM_COM_PORT_T port, uint8_t data);
extern void com_hal_send_irq_callback(ENUM_COM_PORT_T port);
extern uint8_t com_hal_send(ENUM_COM_PORT_T port, uint8_t *p_buf, uint16_t len);
com_hal_port_t * com_hal_get_port_info(ENUM_COM_PORT_T port);
#endif //_COM_HAL_LAYER_H_
