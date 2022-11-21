#include "pump_driver.h"

void pump_config(gpio_num_t gpio)
{
    gpio_config_t pump_pin_config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1UL << gpio),
        .pull_down_en = 1,
        .pull_up_en = 0};

    gpio_config(&pump_pin_config);
    gpio_set_level(gpio, 0);
}
