#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "driver/gpio.h"

#include "app_config.h"

#define START_WATER 1
#define STOP_WATER 0

gpio_num_t pump_config(gpio_num_t gpio_pin);
void pump_on(gpio_num_t gpio_pin);
void pump_off(gpio_num_t gpio_pin);
