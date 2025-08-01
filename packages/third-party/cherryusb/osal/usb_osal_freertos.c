/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usb_osal.h"
#include "usb_errno.h"
#include <FreeRTOS.h>
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include <aic_osal.h>

usb_osal_thread_t usb_osal_thread_create(const char *name, uint32_t stack_size, uint32_t prio, usb_thread_entry_t entry, void *args)
{
    TaskHandle_t htask = NULL;
    BaseType_t ret;

    stack_size /= sizeof(StackType_t);
    ret = xTaskCreate(entry, name, stack_size, args, configMAX_PRIORITIES - 1 - prio, &htask);
    if (ret != pdPASS)
        return NULL;

    return (usb_osal_thread_t)htask;
}

void usb_osal_thread_delete(usb_osal_thread_t thread)
{
    vTaskDelete(thread);
}

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count)
{
    return (usb_osal_sem_t)xSemaphoreCreateCounting(1, initial_count);
}

void usb_osal_sem_delete(usb_osal_sem_t sem)
{
    vSemaphoreDelete((SemaphoreHandle_t)sem);
}

int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout)
{
    if (timeout == USB_OSAL_WAITING_FOREVER) {
        return (xSemaphoreTake((SemaphoreHandle_t)sem, portMAX_DELAY) == pdPASS) ? 0 : -USB_ERR_TIMEOUT;
    } else {
        return (xSemaphoreTake((SemaphoreHandle_t)sem, pdMS_TO_TICKS(timeout)) == pdPASS) ? 0 : -USB_ERR_TIMEOUT;
    }
}

int usb_osal_sem_give(usb_osal_sem_t sem)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int ret;

    if (xPortIsInsideInterrupt()) {
        ret = xSemaphoreGiveFromISR((SemaphoreHandle_t)sem, &xHigherPriorityTaskWoken);
        if (ret == pdPASS) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        ret = xSemaphoreGive((SemaphoreHandle_t)sem);
    }

    return (ret == pdPASS) ? 0 : -USB_ERR_TIMEOUT;
}

void usb_osal_sem_reset(usb_osal_sem_t sem)
{
    xQueueReset((QueueHandle_t)sem);
}

usb_osal_mutex_t usb_osal_mutex_create(void)
{
    return (usb_osal_mutex_t)xSemaphoreCreateMutex();
}

void usb_osal_mutex_delete(usb_osal_mutex_t mutex)
{
    vSemaphoreDelete((SemaphoreHandle_t)mutex);
}

int usb_osal_mutex_take(usb_osal_mutex_t mutex)
{
    return (xSemaphoreTake((SemaphoreHandle_t)mutex, portMAX_DELAY) == pdPASS) ? 0 : -USB_ERR_TIMEOUT;
}

int usb_osal_mutex_give(usb_osal_mutex_t mutex)
{
    return (xSemaphoreGive((SemaphoreHandle_t)mutex) == pdPASS) ? 0 : -USB_ERR_TIMEOUT;
}

usb_osal_mq_t usb_osal_mq_create(uint32_t max_msgs)
{
    return (usb_osal_mq_t)xQueueCreate(max_msgs, sizeof(uintptr_t));
}

void usb_osal_mq_delete(usb_osal_mq_t mq)
{
    vQueueDelete((QueueHandle_t)mq);
}

int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int ret;

    ret = xQueueSendFromISR((usb_osal_mq_t)mq, &addr, &xHigherPriorityTaskWoken);
    if (ret == pdPASS) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    return (ret == pdPASS) ? 0 : -USB_ERR_TIMEOUT;
}

int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout)
{
    if (timeout == USB_OSAL_WAITING_FOREVER) {
        return (xQueueReceive((usb_osal_mq_t)mq, addr, portMAX_DELAY) == pdPASS) ? 0 : -USB_ERR_TIMEOUT;
    } else {
        return (xQueueReceive((usb_osal_mq_t)mq, addr, pdMS_TO_TICKS(timeout)) == pdPASS) ? 0 : -USB_ERR_TIMEOUT;
    }
}

usb_osal_event_t usb_osal_event_creat(const char *name)
{
    return 0;
}

void usb_osal_event_delete(usb_osal_event_t event)
{
    return;
}

int usb_osal_event_send(usb_osal_event_t event, uint32_t set)
{
    return 0;
}

int usb_osal_event_recv(usb_osal_event_t event, uint32_t  set, uint8_t option,
                        int32_t timeout, uint32_t *recved)
{
    return 0;
}

static void __usb_timeout(TimerHandle_t *handle)
{
    struct usb_osal_timer *timer = (struct usb_osal_timer *)pvTimerGetTimerID((TimerHandle_t)handle);

    timer->handler(timer->argument);
}

struct usb_osal_timer *usb_osal_timer_create(const char *name, uint32_t timeout_ms, usb_timer_handler_t handler, void *argument, bool is_period)
{
    struct usb_osal_timer *timer;
    (void)name;

    timer = pvPortMalloc(sizeof(struct usb_osal_timer));

    if (timer == NULL) {
        USB_LOG_ERR("Create usb_osal_timer failed\r\n");
        while (1) {
        }
    }
    memset(timer, 0, sizeof(struct usb_osal_timer));

    timer->handler = handler;
    timer->argument = argument;

    timer->timer = (void *)xTimerCreate("usb_tim", pdMS_TO_TICKS(timeout_ms), is_period, timer, (TimerCallbackFunction_t)__usb_timeout);
    if (timer->timer == NULL) {
        USB_LOG_ERR("Create timer failed\r\n");
        while (1) {
        }
    }
    return timer;
}

void usb_osal_timer_delete(struct usb_osal_timer *timer)
{
    xTimerStop(timer->timer, 0);
    xTimerDelete(timer->timer, 0);
    vPortFree(timer);
}

void usb_osal_timer_start(struct usb_osal_timer *timer)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int ret;

    if (xPortIsInsideInterrupt()) {
        ret = xTimerStartFromISR(timer->timer, &xHigherPriorityTaskWoken);
        if (ret == pdPASS) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        xTimerStart(timer->timer, 0);
    }
}

void usb_osal_timer_stop(struct usb_osal_timer *timer)
{
    xTimerStop(timer->timer, 0);
}

size_t usb_osal_enter_critical_section(void)
{
    taskDISABLE_INTERRUPTS();
    return 1;
}

void usb_osal_leave_critical_section(size_t flag)
{
    taskENABLE_INTERRUPTS();
}

void usb_osal_msleep(uint32_t delay)
{
    vTaskDelay(pdMS_TO_TICKS(delay));
}

void *usb_osal_malloc_align(uint32_t mem_type, size_t size, size_t align)
{
    return aicos_malloc_align(mem_type, size, align);
}

void usb_osal_free_align(uint32_t mem_type, void *mem)
{
    aicos_free_align(mem_type, mem);
}
