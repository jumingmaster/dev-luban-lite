#include "cooker_drv.h"
#include "cooker_protocol.h"
#include "cooker_task.h"

#define THREAD_PRIORITY         16
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static char cook_com_tx_stack[1024];
static char cook_com_rx_stack[1024];

static struct rt_thread cook_com_tx_thread;
static struct rt_thread cook_com_rx_thread;

// static rt_sem_t rx_sem = RT_NULL;
// static rt_sem_t tx_sem = RT_NULL;


static uint8_t rx_buffer[4];

static uint8_t tx_buffer[4];

static struct rt_messagequeue tx_mq = {0};

static rt_uint8_t tx_msg_pool[1024] = {0};


// static void cooker_receive_irq(void *args)
// {
//     rt_sem_release(rx_sem);
// }




static void cook_com_tx_thread_entry(void *param)
{
    uint8_t msg = 0;
    while (1)
    {
        rt_thread_mdelay(100);

        if (rt_mq_recv(&tx_mq, &msg, sizeof(msg), RT_WAITING_NO) == RT_EOK)
        {
            memset(tx_buffer, 0x00, sizeof(tx_buffer));
            tx_buffer[0] = msg;
            // cooker_send_bytes(tx_buffer, sizeof(tx_buffer));
                // rt_kprintf("Cooker sent: %02x %02x %02x %02x\r\n", tx_buffer[0], tx_buffer[1], tx_buffer[2], tx_buffer[3]);
            // rt_kprintf("Cooker sent: %02x\r\n", msg);
        }
    }
}


static void cook_com_rx_thread_entry(void *param)
{ 
    while (1)
    {
        
        // rt_sem_take(rx_sem, RT_WAITING_FOREVER);

        // rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_DISABLE);

        // rt_kprintf("cooker_read irq sem\r\n");
        

        // if (cooker_read_bytes(rx_buffer, sizeof(rx_buffer)) == 0)
        // {
        //     rt_kprintf("Cooker received: %02x %02x %02x %02x\r\n", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3]);
        // }
        // else
        // {
        //     rt_kprintf("Cooker received failed.\r\n");
        // }

        // rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_ENABLE);

        rt_thread_mdelay(30);
    }
}




static int cook_com_thread_init(void)
{
    // rt_err_t result;

    // rx_sem = rt_sem_create("rx_sem", 0, RT_IPC_FLAG_PRIO);
    
    cooker_drv_init(NULL);

    // if (rx_sem == RT_NULL)
    // {
    //     rt_kprintf("create cooker rx semaphore failed.\n");
    //     return -1;
    // }

        

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
