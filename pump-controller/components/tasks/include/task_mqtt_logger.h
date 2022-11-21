#pragma once

#include "string.h"

#include "FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "mqtt_init.h"

#include "app_config.h"

extern TaskHandle_t task_mqtt_logger_handle;

enum events
{
    REFILLING_START,
    REFILLING_FINISH,
    TASK_START,
    TASK_FINISH,
    TASK_BAD_REQUEST,
};

void task_mqtt_logger(void *arg);
