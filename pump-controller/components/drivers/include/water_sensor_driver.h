#pragma once

#include "driver/gpio.h"

#include "app_config.h"

void water_sensor_config(
    gpio_num_t signal_pin,
    gpio_num_t mode_pin);
