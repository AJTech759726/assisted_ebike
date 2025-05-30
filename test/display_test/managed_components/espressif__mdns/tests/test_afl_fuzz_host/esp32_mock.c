/*
 * SPDX-FileCopyrightText: 2022-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "esp32_mock.h"
#include "esp_log.h"

void     *g_queue;
int       g_queue_send_shall_fail = 0;
int       g_size = 0;

const char *WIFI_EVENT = "wifi_event";
const char *ETH_EVENT = "eth_event";

esp_err_t esp_event_handler_register(const char *event_base,
                                     int32_t event_id,
                                     void *event_handler,
                                     void *event_handler_arg)
{
    return ESP_OK;
}

esp_err_t esp_event_handler_unregister(const char *event_base, int32_t event_id, void *event_handler)
{
    return ESP_OK;
}

esp_err_t esp_timer_delete(esp_timer_handle_t timer)
{
    return ESP_OK;
}

esp_err_t esp_timer_stop(esp_timer_handle_t timer)
{
    return ESP_OK;
}

esp_err_t esp_timer_start_periodic(esp_timer_handle_t timer, uint64_t period)
{
    return ESP_OK;
}

esp_err_t esp_timer_create(const esp_timer_create_args_t *create_args,
                           esp_timer_handle_t *out_handle)
{
    return ESP_OK;
}

uint32_t xTaskGetTickCount(void)
{
    static uint32_t tick = 0;
    return tick++;
}

/// Queue mock
QueueHandle_t xQueueCreate(uint32_t uxQueueLength, uint32_t uxItemSize)
{
    g_size = uxItemSize;
    g_queue = malloc((uxQueueLength) * (uxItemSize));
    return g_queue;
}


void vQueueDelete(QueueHandle_t xQueue)
{
    free(xQueue);
}

uint32_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait)
{
    if (g_queue_send_shall_fail) {
        return pdFALSE;
    } else {
        memcpy(xQueue, pvItemToQueue, g_size);
        return pdPASS;
    }
}


uint32_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait)
{
    return pdFALSE;
}

void GetLastItem(void *pvBuffer)
{
    memcpy(pvBuffer, g_queue, g_size);
}

void ForceTaskDelete(void)
{
    g_queue_send_shall_fail = 1;
}

TaskHandle_t xTaskGetCurrentTaskHandle(void)
{
    return NULL;
}

void xTaskNotifyGive(TaskHandle_t task)
{
    return;
}

BaseType_t xTaskNotifyWait(uint32_t bits_entry_clear, uint32_t bits_exit_clear, uint32_t *value, TickType_t wait_time)
{
    return pdTRUE;
}

void esp_log_write(esp_log_level_t level, const char *tag, const char *format, ...)
{
}

void esp_log(esp_log_config_t config, const char *tag, const char *format, ...)
{
}

uint32_t esp_log_timestamp(void)
{
    return 0;
}

void *mdns_mem_malloc(size_t size)
{
    return malloc(size);
}

void *mdns_mem_calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void mdns_mem_free(void *ptr)
{
    free(ptr);
}

char *mdns_mem_strdup(const char *s)
{
    return strdup(s);
}

char *mdns_mem_strndup(const char *s, size_t n)
{
    return strndup(s, n);
}

void *mdns_mem_task_malloc(size_t size)
{
    return malloc(size);
}

void mdns_mem_task_free(void *ptr)
{
    free(ptr);
}
