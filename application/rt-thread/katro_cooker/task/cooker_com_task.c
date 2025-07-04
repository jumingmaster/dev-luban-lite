#include "cooker_protocol.h"
#include "cooker_task.h"
#include <rtthread.h>
#include "rtdevice.h"
#include <string.h>

#define THREAD_PRIORITY         26
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

#define UART_MSG_LENGTH 0x05
#define UART_PAYLOAD_LENGTH 0x03
#define UART_MSG_HEAD   0xFC

#pragma pack(push, 1)
typedef struct
{
    uint8_t     head;
    uint8_t     payload;
    uint8_t     reserved1;
    uint8_t     reserved2;
    uint8_t     checksum;
} sc_uart_msg_t;
#pragma pack(pop)

static char cook_com_tx_stack[1024];
static char cook_com_rx_stack[1024];

static struct rt_thread cook_com_tx_thread;
static struct rt_thread cook_com_rx_thread;

static uint8_t rx_buffer[UART_MSG_LENGTH];

static uint8_t tx_buffer[UART_MSG_LENGTH];

static struct rt_messagequeue tx_mq = {0};

static rt_uint8_t tx_msg_pool[1024] = {0};


static rt_device_t serial;


static inline uint8_t cal_checksum(uint8_t * data, uint8_t length)
{
    uint8_t sum = 0;
    for (int i = 0; i < length; i++)
    {
        sum += data[i];
    }
    sum = ~sum;
}


static inline void gen_packet(uint8_t msg, uint8_t * buf)
{
    sc_uart_msg_t * packet = (sc_uart_msg_t *)buf;
    packet->head = UART_MSG_HEAD;
    packet->payload = msg;
    packet->checksum = cal_checksum(packet->payload, UART_PAYLOAD_LENGTH);
}



static void cook_com_tx_thread_entry(void *param)
{
    uint8_t msg = 0;
    while (1)
    {
        if (rt_mq_recv(&tx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK)
        {
            memset(tx_buffer, 0x00, sizeof(tx_buffer));
            gen_packet(msg, tx_buffer);

        }
        rt_thread_mdelay(5);
    }
}


static void cook_com_rx_thread_entry(void *param)
{ 
    while (1)
    {
        rt_thread_mdelay(100);
    }
}




static int cook_com_thread_init(void)
{
    

    /* 初始化消息队列 */
    rt_mq_init(&tx_mq,
                    "tx_mqt",
                    &tx_msg_pool[0],             /* 内存池指向 msg_pool */
                    1,                          /* 每个消息的大小是 1 字节 */
                    sizeof(tx_msg_pool),        /* 内存池的大小是 msg_pool 的大小 */
                    RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待，优先级大小的方法分配消息 */

    rt_thread_init(&cook_com_rx_thread,
                    "cooker_com_rx",
                    cook_com_rx_thread_entry,
                    RT_NULL,
                    &cook_com_rx_stack[0],
                    sizeof(cook_com_rx_stack),
                    THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&cook_com_rx_thread);

    rt_thread_init(&cook_com_tx_thread,
                    "cooker_com_tx",
                    cook_com_tx_thread_entry,
                    RT_NULL,
                    &cook_com_tx_stack[0],
                    sizeof(cook_com_tx_stack),
                    THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&cook_com_tx_thread);
    
    return 0;
}
INIT_ENV_EXPORT(cook_com_thread_init);




void cooker_msg_send(cooker_resp_t * msg)
{
    rt_mq_send(&tx_mq, msg, 1);
}
