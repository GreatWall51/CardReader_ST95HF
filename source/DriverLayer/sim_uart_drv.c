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

#define SML_USART_BaudRate           4800  //��ʱ����������,���֧��57600(�ײ�)
/*��ѡ�����ʣ�
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
} DATA_DIR_E; //���ݴ��䷽��

/*------------------- Global Definitions and Declarations -------------------*/
static DATA_DIR_E data_dir = DATA_DIR_IDLE;


static uint8_t sending_data = 0;                   //���ڷ��͵��ֽ�
static uint8_t send_bit_index = 0;                 //����λ����
static uint8_t send_byte_index = 0;                //�����ֽ�����
static uint8_t send_cnt = 0;                       //�����������ܸ���
static uint8_t send_buf[SEND_BUF_MAX_LEN] = {0};  //���ͻ�����

uint8_t receive_data = 0;                       //���ڽ��յ��ֽ�
static uint8_t recv_bit_index = 0;              //�ѽ���λ����
static uint8_t recv_byte_index=0;               //�����ֽ�����
static uint8_t recv_cnt=0;                      //���ո���
static uint8_t recv_buf[RECV_BUF_MAX_LEN]={0}; //���ջ�����
static uint8_t RX_capt_flag = 0;                //0->����,1->���ڲ�׽
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
    //ȫ���������,��Ϊ����״̬
    cbi(GPIOA, GPIO_Pin_1);
    rs485_port[RS485_SIM].s_status = ENUM_HAL_SEND_STA_IDLE;
}
/******************************************************************************
* Name:      get_data_dir
*
* Function Desc:
* Input Param:
* Output Param:
* Return Value:     0:����,1:����
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
* Input Param:   dir:0->����;1->����
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
* Function Desc:     ģ�⴮��IO�ڳ�ʼ��
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
    SET_TX_1();//���ݴ���ͨ�Ÿ�ʽ,����Ҫ����

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
* Function Desc:     ģ�⴮�ڵ�ʱ�ӳ�ʼ��,�����ڲ�����4800bit/s
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
    TIM_TimeBaseStructure.TIM_Period = Baud_TO_Time - 1; //208us   //������4800
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(SIM_UART_RX_TIMER, &TIM_TimeBaseStructure);


//  TIM_ARRPreloadConfig(SIM_UART_RX_TIMER,ENABLE);//�ֶ�����
    TIM_ClearITPendingBit(SIM_UART_RX_TIMER, TIM_IT_Update);
    TIM_ITConfig(SIM_UART_RX_TIMER, TIM_IT_Update, ENABLE);
    STOP_RX_TIMER(); //��ʼ״̬Ϊ������
    NVIC_EnableIRQ(TIM17_IRQn);

    TIM_TimeBaseInit(SIM_UART_TX_TIMER, &TIM_TimeBaseStructure);
    TIM_ClearITPendingBit(SIM_UART_TX_TIMER, TIM_IT_Update);
    TIM_ITConfig(SIM_UART_TX_TIMER, TIM_IT_Update, ENABLE);
    STOP_TX_TIMER(); //��ʼ״̬Ϊ������
    NVIC_EnableIRQ(TIM16_IRQn);

}

void set_timer_period(uint8_t time)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Prescaler = 47;  // 1MHz
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = time - 1; //208us   //������4800
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(SIM_UART_RX_TIMER, &TIM_TimeBaseStructure);
}
/******************************************************************************
* Name:      SimUART_TxBytes
*
* Function Desc:     ģ�⴮�ڷ�������
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
    //�������ֽ�
    sending_data = send_buf[send_byte_index++];
    //�������ݷ���
    set_data_dir(DATA_DIR_SEND);
    //�������� ��ص��ж�
    TIMER_START(SIM_UART_TX_TIMER, Baud_TO_Time);
}
/******************************************************************************
* Name:      SimUART_RxBytes
*
* Function Desc:     ģ�⴮�ڽ�������(ע�����ݿ��ܽ���δ��ɾ�����ȡ)
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
  //���ݻ�û���������ȡ
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
* Function Desc:     �����������ⲿ�жϴ�����
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
            RX_capt_flag = 1;   //��RX����״̬Ϊ���ڲ���
            //�������ݷ���
            set_data_dir(DATA_DIR_RECEIVE);
            TIMER_START(SIM_UART_RX_TIMER, (Baud_TO_Time >> 1)); //���ղ�����λ��Ϊ��ƽ���м�ʱ��
        }
    }
}
//�������ݶ�ʱ���жϴ�����
void TIM17_IRQHandler(void)
{
    uint8_t recv_bit_val;            //��ǰ����λ��ֵ

    TIM_ClearITPendingBit(SIM_UART_RX_TIMER, TIM_IT_Update);//�嶨ʱ���жϱ�־
		ENTER_CRITICAL() ;
    if (get_data_dir() == DATA_DIR_RECEIVE)
    {
        if ((recv_bit_index > 0) && (recv_bit_index < 9)) //��������
        {
            receive_data |= (GET_RX_VALUE() << (recv_bit_index - 1));
            recv_bit_index++;//�Ƚ�����һλ�����Լ�
        }
        else if (recv_bit_index == 9) //���ս���
        {
            recv_bit_val = GET_RX_VALUE();
            recv_bit_index = 0;
            STOP_RX_TIMER();       //ֹͣ��ʱ
            RX_capt_flag = 0;                       //��RX����״̬Ϊ����
            set_data_dir(DATA_DIR_IDLE);
            if (recv_bit_val != 1) //���մ���
            {
                receive_data = 0;
							  EXIT_CRITICAL();
                return;//����λ����
            }
            else//��ȷ����
            {
								recv_cnt++;
                if(recv_cnt > RECV_BUF_MAX_LEN) return ;
                recv_buf[recv_byte_index++] = receive_data;
                rs485_rev_irq_callback(RS485_SIM);
                receive_data = 0;
            }
        }
        else if (recv_bit_index == 0) //������ʼλ
        {
            recv_bit_val = GET_RX_VALUE();
            if (recv_bit_val != 0)
            {
                recv_bit_index = 0;
                STOP_RX_TIMER(); //ֹͣ��ʱ
                RX_capt_flag = 0;                 //��RX����״̬Ϊ����
                set_data_dir(DATA_DIR_IDLE);
								EXIT_CRITICAL();
                return;//������ʼλ����
            }
            recv_bit_index++;
            TIMER_START(SIM_UART_RX_TIMER, Baud_TO_Time);
        }
    }
		EXIT_CRITICAL();
}

//�������ݶ�ʱ���жϴ�����
void TIM16_IRQHandler(void)
{
    uint8_t send_bit_val;            //��ǰ����λ��ֵ
    TIM_ClearITPendingBit(SIM_UART_TX_TIMER, TIM_IT_Update);//�嶨ʱ���жϱ�־
	ENTER_CRITICAL() ;
    if (get_data_dir() == DATA_DIR_SEND) //����
    {
        if (send_bit_index < 1) //������ʼλ
        {
            SET_TX_0();
            send_bit_index++;
        }
        else
        {
            if (send_bit_index == 9)//���ͽ���λ
            {
                SET_TX_1();
                send_bit_index++;
            }
            else if (send_bit_index == 10) //����һ�ֽ����
            {
                send_bit_index = 0;     //�巢��λ����
                STOP_TX_TIMER(); //ֹͣ��ʱ
                set_data_dir(DATA_DIR_IDLE);
                SET_TX_1();//���ͽ���λ
                send_one_byte_finish_cb();
                if (send_byte_index < send_cnt)
                {
                    //��������дһ���ֽ�����
                    sending_data = send_buf[send_byte_index++];
                    //�������ݷ���
                    set_data_dir(DATA_DIR_SEND);
                    //����������ص��ж�
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
* Function Desc:     ģ�⴮�ڳ�ʼ��
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

