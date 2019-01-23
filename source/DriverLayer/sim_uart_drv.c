/******************************************************************************
* Copyright 2017-2022 liuweiqiang@leelen.cn
* FileName:      sim_uart_drv.c
* Desc:
*
*
* Author:    liuwq
* Date:      2017/06/19
* Notes:
*
* ----------------------------------------------------------------------------
* Histroy: v1.0   2017/06/19, liuwq create this file
*
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "stm32f0xx.h"
#include "stm32f0xx_exti.h"
#include "sim_uart_drv.h"
#include "define.h"
#include <string.h>
#include "rs485_drv.h"
#include "debug.h"
/*---------------------- Constant / Macro Definitions -----------------------*/
#define RECV_BUF_MAX_LEN    50
#define SEND_BUF_MAX_LEN    50

#define SIM_UART_RX_TIMER               TIM17
#define SIM_UART_TIMET_CLOCK         RCC_APB2Periph_TIM17

#define SIM_UART_TX_TIMER            TIM16
#define SIM_UART_TX_TIMET_CLOCK      RCC_APB2Periph_TIM16

#define TX_PIN_GROP                  GPIOB
#define TX_PIN                       GPIO_Pin_6
#define TX_EXTI_PinSource            EXTI_PinSource6
#define TX_EXTI_Line                 EXTI_Line6

#define RX_PIN_GROP                  GPIOB
#define RX_PIN                       GPIO_Pin_7
#define RX_EXTI_PinSource_GROP       EXTI_PortSourceGPIOB
#define RX_EXTI_PinSource            EXTI_PinSource7
#define RX_EXTI_Line                 EXTI_Line7
#define RX_IRQChannel                EXTI4_15_IRQn

#define SET_TX_1() sbi(TX_PIN_GROP,TX_PIN)
#define SET_TX_0() cbi(TX_PIN_GROP,TX_PIN)
#define GET_RX_VALUE() bis(RX_PIN_GROP,RX_PIN)

#define STOP_TX_TIMER() TIM_Cmd(SIM_UART_TX_TIMER, DISABLE)
#define STOP_RX_TIMER() TIM_Cmd(SIM_UART_RX_TIMER, DISABLE)

#define SML_USART_BaudRate           4800  //定时器精度有限,最大支持57600(亲测)
/*可选波特率：
4800
9600
14400
28800
19200
34800
56000
57600
*/

#define Baud_TO_Time                 1000000/SML_USART_BaudRate   //us
#define TIMER_START(TIMER,time) do{\
  TIMER->ARR = time - 1;\
  TIMER->CNT = 0;\
  TIMER->DIER = 0x0001;\
  TIMER->CR1 |= 0x0001;\
}while(0)
/*----------------------- Type Declarations ---------------------------------*/
typedef enum
{
    DATA_DIR_IDLE = 0,
    DATA_DIR_RECEIVE,
    DATA_DIR_SEND,
} DATA_DIR_E; //数据传输方向

/*------------------- Global Definitions and Declarations -------------------*/
static DATA_DIR_E data_dir = DATA_DIR_IDLE;


static uint8_t sending_data = 0;                   //正在发送的字节
static uint8_t send_bit_index = 0;                 //发送位索引
static uint8_t send_byte_index = 0;                //发送字节索引
static uint8_t send_cnt = 0;                       //待发送数据总个数
static uint8_t send_buf[SEND_BUF_MAX_LEN] = {0};  //发送缓冲区

uint8_t receive_data = 0;                       //正在接收的字节
static uint8_t recv_bit_index = 0;              //已接收位索引
static uint8_t recv_byte_index=0;               //接收字节索引
static uint8_t recv_cnt=0;                      //接收个数
static uint8_t recv_buf[RECV_BUF_MAX_LEN]={0}; //接收缓冲区
static uint8_t RX_capt_flag = 0;                //0->空闲,1->正在捕捉
extern rs485_port_t rs485_port[RS485_IDNUM];
/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/

void send_one_byte_finish_cb(void)
{
    rs485_port[RS485_SIM].s_interval_time = 0;
}
void send_all_byte_finish_cb(void)
{
    //全部发送完成,置为接收状态
    cbi(GPIOA, GPIO_Pin_1);
    rs485_port[RS485_SIM].s_status = ENUM_HAL_SEND_STA_IDLE;
}
/******************************************************************************
* Name:      get_data_dir
*
* Function Desc:
* Input Param:
* Output Param:
* Return Value:     0:接收,1:发送
* Global:
* Note:
* Author:    liuwq
* ----------------------------------------------------------------------------
* Log:   2017/06/20, Create this function by liuwq
******************************************************************************/
static DATA_DIR_E get_data_dir(void)
{
    return data_dir;
}
/******************************************************************************
* Name:      set_data_dir
*
* Function Desc:
* Input Param:   dir:0->接收;1->发送
* Output Param:
* Return Value:
* Global:
* Note:
* Author:    liuwq
* ----------------------------------------------------------------------------
* Log:   2017/06/20, Create this function by liuwq
******************************************************************************/
static void set_data_dir(DATA_DIR_E dir)
{
    data_dir = dir;
}
/******************************************************************************
* Name:      sim_uart_io_init
*
* Function Desc:     模拟串口IO口初始化
* Input Param:
* Output Param:
* Return Value:
* Global:
* Note:
* Author:    liuwq
* ----------------------------------------------------------------------------
* Log:   2017/06/21, Create this function by liuwq
******************************************************************************/
static void sim_uart_io_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable GPIOB clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Configure PB6 pin as input floating */
    GPIO_InitStructure.GPIO_Pin = TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(TX_PIN_GROP, &GPIO_InitStructure);
    SET_TX_1();//根据串口通信格式,必须要拉高

    /* Configure PB7 pin as input UP */
    GPIO_InitStructure.GPIO_Pin = RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(RX_PIN_GROP, &GPIO_InitStructure);

    /* Connect EXTI7 Line to PB pin */
    SYSCFG_EXTILineConfig(RX_EXTI_PinSource_GROP, RX_EXTI_PinSource);

    /* Configure EXTI7 line */
    EXTI_InitStructure.EXTI_Line = RX_EXTI_Line;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI7 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RX_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/******************************************************************************
* Name:      sim_uart_timer_init
*
* Function Desc:     模拟串口的时钟初始化,适用于波特率4800bit/s
* Input Param:
* Output Param:
* Return Value:
* Global:
* Note:
* Author:    liuwq
* ----------------------------------------------------------------------------
* Log:   2017/06/21, Create this function by liuwq
******************************************************************************/
static void sim_uart_timer_init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    /* TIM1 clock enable */
    /* TIM1 clock enable */
    if (SIM_UART_RX_TIMER == TIM1 || SIM_UART_RX_TIMER == TIM15\
            || SIM_UART_RX_TIMER == TIM16 || SIM_UART_RX_TIMER == TIM17)
    {
        RCC_APB2PeriphClockCmd(SIM_UART_TIMET_CLOCK | SIM_UART_TX_TIMET_CLOCK, ENABLE);
    }
    else
    {
        RCC_APB1PeriphClockCmd(SIM_UART_TIMET_CLOCK | SIM_UART_TX_TIMET_CLOCK, ENABLE);
    }

    /* Time Base configuration */
    TIM_TimeBaseStructure.TIM_Prescaler = 48 - 1;  // 1MHz
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = Baud_TO_Time - 1; //208us   //波特率4800
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(SIM_UART_RX_TIMER, &TIM_TimeBaseStructure);


//  TIM_ARRPreloadConfig(SIM_UART_RX_TIMER,ENABLE);//手动设置
    TIM_ClearITPendingBit(SIM_UART_RX_TIMER, TIM_IT_Update);
    TIM_ITConfig(SIM_UART_RX_TIMER, TIM_IT_Update, ENABLE);
    STOP_RX_TIMER(); //初始状态为不启动
    NVIC_EnableIRQ(TIM17_IRQn);

    TIM_TimeBaseInit(SIM_UART_TX_TIMER, &TIM_TimeBaseStructure);
    TIM_ClearITPendingBit(SIM_UART_TX_TIMER, TIM_IT_Update);
    TIM_ITConfig(SIM_UART_TX_TIMER, TIM_IT_Update, ENABLE);
    STOP_TX_TIMER(); //初始状态为不启动
    NVIC_EnableIRQ(TIM16_IRQn);

}

void set_timer_period(uint8_t time)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Prescaler = 47;  // 1MHz
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = time - 1; //208us   //波特率4800
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(SIM_UART_RX_TIMER, &TIM_TimeBaseStructure);
}
/******************************************************************************
* Name:      SimUART_TxBytes
*
* Function Desc:     模拟串口发送数据
* Input Param:
* Output Param:
* Return Value:
* Global:
* Note:
* Author:    liuwq
* ----------------------------------------------------------------------------
* Log:   2017/06/21, Create this function by liuwq
******************************************************************************/
void SimUART_TxBytes(uint8_t *SendData, uint8_t len)
{
    if ((len > SEND_BUF_MAX_LEN) || (len < 1) || (get_data_dir() != DATA_DIR_IDLE)) return;
    memcpy(send_buf, SendData, len);
    send_cnt = len;
    send_byte_index = 0;
    send_bit_index = 0;
    //发送首字节
    sending_data = send_buf[send_byte_index++];
    //设置数据方向
    set_data_dir(DATA_DIR_SEND);
    //开启发送 相关的中断
    TIMER_START(SIM_UART_TX_TIMER, Baud_TO_Time);
}
/******************************************************************************
* Name:      SimUART_RxBytes
*
* Function Desc:     模拟串口接收数据(注意数据可能接收未完成就来读取)
* Input Param:
* Output Param:
* Return Value:
* Global:
* Note:
* Author:    liuwq
* ----------------------------------------------------------------------------
* Log:   2017/06/21, Create this function by liuwq
******************************************************************************/
void SimUART_RxBytes(uint8_t *RecvData, uint8_t *len)
{
  if(get_data_dir() != DATA_DIR_IDLE)
  {
      *len = 0;
      return ;
  }
  //数据还没接受完就来取
  if((recv_cnt > 0) && (RecvData[1]==recv_cnt-1))
  {
        *len = recv_cnt;
        memcpy(RecvData,recv_buf,*len);
        recv_cnt=0;
        recv_byte_index=0;
  }
  else
  {
      recv_cnt=0;
      recv_byte_index = 0;
      recv_bit_index = 0;
  }
}
/******************************************************************************
* Name:      EXTI4_15_IRQHandler
*
* Function Desc:     检测接收数据外部中断处理函数
* Input Param:
* Output Param:
* Return Value:
* Global:
* Note:
* Author:    liuwq
* ----------------------------------------------------------------------------
* Log:   2017/06/21, Create this function by liuwq
******************************************************************************/
void EXTI4_15_IRQHandler(void)
{
    if (EXTI_GetITStatus(RX_EXTI_Line) != RESET)
    {
        EXTI_ClearITPendingBit(RX_EXTI_Line);
        if ((RX_capt_flag == 0) && (get_data_dir() == DATA_DIR_IDLE))
        {
            RX_capt_flag = 1;   //置RX捕获状态为正在捕获
            //设置数据方向
            set_data_dir(DATA_DIR_RECEIVE);
            TIMER_START(SIM_UART_RX_TIMER, (Baud_TO_Time >> 1)); //接收采样的位置为电平的中间时刻
        }
    }
}
//接收数据定时器中断处理函数
void TIM17_IRQHandler(void)
{
    uint8_t recv_bit_val;            //当前接收位的值

    TIM_ClearITPendingBit(SIM_UART_RX_TIMER, TIM_IT_Update);//清定时器中断标志
		ENTER_CRITICAL() ;
    if (get_data_dir() == DATA_DIR_RECEIVE)
    {
        if ((recv_bit_index > 0) && (recv_bit_index < 9)) //接收数据
        {
            receive_data |= (GET_RX_VALUE() << (recv_bit_index - 1));
            recv_bit_index++;//先接收完一位才能自加
        }
        else if (recv_bit_index == 9) //接收结束
        {
            recv_bit_val = GET_RX_VALUE();
            recv_bit_index = 0;
            STOP_RX_TIMER();       //停止计时
            RX_capt_flag = 0;                       //置RX捕获状态为空闲
            set_data_dir(DATA_DIR_IDLE);
            if (recv_bit_val != 1) //接收错误
            {
                receive_data = 0;
							  EXIT_CRITICAL();
                return;//结束位错误
            }
            else//正确接收
            {
								recv_cnt++;
                if(recv_cnt > RECV_BUF_MAX_LEN) return ;
                recv_buf[recv_byte_index++] = receive_data;
                rs485_rev_irq_callback(RS485_SIM);
                receive_data = 0;
            }
        }
        else if (recv_bit_index == 0) //接收起始位
        {
            recv_bit_val = GET_RX_VALUE();
            if (recv_bit_val != 0)
            {
                recv_bit_index = 0;
                STOP_RX_TIMER(); //停止计时
                RX_capt_flag = 0;                 //置RX捕获状态为空闲
                set_data_dir(DATA_DIR_IDLE);
								EXIT_CRITICAL();
                return;//接收起始位错误
            }
            recv_bit_index++;
            TIMER_START(SIM_UART_RX_TIMER, Baud_TO_Time);
        }
    }
		EXIT_CRITICAL();
}

//发送数据定时器中断处理函数
void TIM16_IRQHandler(void)
{
    uint8_t send_bit_val;            //当前发送位的值
    TIM_ClearITPendingBit(SIM_UART_TX_TIMER, TIM_IT_Update);//清定时器中断标志
	ENTER_CRITICAL() ;
    if (get_data_dir() == DATA_DIR_SEND) //发送
    {
        if (send_bit_index < 1) //发送起始位
        {
            SET_TX_0();
            send_bit_index++;
        }
        else
        {
            if (send_bit_index == 9)//发送结束位
            {
                SET_TX_1();
                send_bit_index++;
            }
            else if (send_bit_index == 10) //发送一字节完成
            {
                send_bit_index = 0;     //清发送位索引
                STOP_TX_TIMER(); //停止计时
                set_data_dir(DATA_DIR_IDLE);
                SET_TX_1();//发送结束位
                send_one_byte_finish_cb();
                if (send_byte_index < send_cnt)
                {
                    //继续发送写一个字节数据
                    sending_data = send_buf[send_byte_index++];
                    //设置数据方向
                    set_data_dir(DATA_DIR_SEND);
                    //开启发送相关的中断
                    TIMER_START(SIM_UART_TX_TIMER, Baud_TO_Time);
                }
                else
                {
                    send_byte_index = 0;
                    send_all_byte_finish_cb();
                }
            }
            else
            {
                send_bit_val = ((sending_data >> (send_bit_index - 1)) & 0x01);
                if (send_bit_val) SET_TX_1();
                else SET_TX_0();
                send_bit_index++;
            }
        }
    }
		EXIT_CRITICAL();
}
/******************************************************************************
* Name:      sim_uart_init
*
* Function Desc:     模拟串口初始化
* Input Param:
* Output Param:
* Return Value:
* Global:
* Note:
* Author:    liuwq
* ----------------------------------------------------------------------------
* Log:   2017/06/21, Create this function by liuwq
******************************************************************************/

void sim_uart_init(void)
{
    memset(recv_buf,0,RECV_BUF_MAX_LEN);
    memset(send_buf,0,SEND_BUF_MAX_LEN);
    sim_uart_io_init();
    sim_uart_timer_init();
}
/*---------------------------------------------------------------------------*/

