#pragma once

#include "FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "pump_driver.h"
#include "refilling_system_init.h"

#include "task_mqtt_logger.h"

#include "app_config.h"

extern QueueHandle_t pump_controller_msg_queue;

enum pump_controller_msg_type
{
    NEW_TASK_MSG,
    PAUSE_TASKS_MSG,
    START_TASKS_MSG,
} typedef pump_controller_msg_type;

struct pump_controller_msg_t
{
    pump_controller_msg_type type;
    uint8_t pump_id;
    uint16_t duration_s;
} typedef pump_controller_msg_t;

struct pump_t
{
    uint8_t id;
    gpio_num_t gpio;
    uint8_t has_active_task;
    TimerHandle_t timer;
    TickType_t active_task_ticks_left;
} typedef pump_t;

void task_pump_controller(void *arg);