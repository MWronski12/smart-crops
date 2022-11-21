#include "water_sesnor_driver.h"

void water_sensor_config(
    gpio_num_t signal_pin,
    gpio_num_t mode_pin,
    water_sensor_intr_edge edge,
    gpio_isr_t isr_callback, void *isr_callback_args)
{
    gpio_config_t water_sensor_signal_pin_config = {
        .intr_type = edge,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1UL << signal_pin),
        .pull_down_en = 1,
        .pull_up_en = 0};

    gpio_config(&water_sensor_signal_pin_config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(signal_pin, isr_callback, isr_callback_args);

    gpio_config_t water_sensor_mode_pin_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1UL << mode_pin),
        .pull_down_en = 0,
        .pull_up_en = 0};

    gpio_config(&water_sensor_mode_pin_config);
    gpio_set_level(mode_pin, 1);
}