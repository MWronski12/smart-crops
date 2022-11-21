#ifndef WATER_SENSOR_DRIVER_H
#define WATER_SENSOR_DRIVER_H

#include "driver/gpio.h"

typedef enum
{
    RISING = GPIO_INTR_POSEDGE,
    FALLING = GPIO_INTR_NEGEDGE
} water_sensor_intr_edge;

void water_sensor_config(
    gpio_num_t signal_pin,
    gpio_num_t mode_pin,
    water_sensor_intr_edge edge,
    gpio_isr_t isr_callback,
    void *isr_callback_args);

#endif