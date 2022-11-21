#include <stdio.h>

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "water_sesnor_driver.h"
#include "pump_driver.h"

#define START_WATER_SENSOR_SIGNAL_PIN GPIO_NUM_14
#define START_WATER_SENSOR_MODE_PIN GPIO_NUM_12
#define STOP_WATER_SENSOR_SIGNAL_PIN GPIO_NUM_14
#define STOP_WATER_SENSOR_MODE_PIN GPIO_NUM_12
#define PUMP_PIN GPIO_NUM_4
#define START_WATER 1
#define STOP_WATER 0

static xQueueHandle event_queue = NULL;

static void water_sensor_isr_handler(void *arg)
{
    uint32_t pump_output = (uint32_t)arg;
    xQueueSendFromISR(event_queue, &pump_output, NULL);
}

static void thread_tank_refilling_system(void *arg)
{
    uint32_t pump_output;

    for (;;)
    {
        if (xQueueReceive(event_queue, &pump_output, portMAX_DELAY))
        {
            gpio_set_level(PUMP_PIN, pump_output);
        }
    }
}

void app_main()
{
    event_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(thread_tank_refilling_system, "thread_tank_refilling_system", 2048, NULL, 10, NULL);

    pump_config(PUMP_PIN);

    water_sensor_config(START_WATER_SENSOR_SIGNAL_PIN,
                        START_WATER_SENSOR_MODE_PIN,
                        FALLING,
                        water_sensor_isr_handler,
                        (void *)START_WATER);

    water_sensor_config(STOP_WATER_SENSOR_SIGNAL_PIN,
                        STOP_WATER_SENSOR_MODE_PIN,
                        RISING,
                        water_sensor_isr_handler,
                        (void *)STOP_WATER);
}
