#include "refilling_system_init.h"

const char *TAG = "refilling_system_init";

static void top_water_sensor_isr_handler()
{
    static pump_controller_msg_t msg = {.type = START_TASKS_MSG, .duration_s = 0, .pump_id = 0};
    if (REFILLING_FLAG == 1)
    {
        REFILLING_FLAG = 0;
        xTaskNotify(task_mqtt_logger_handle, REFILLING_FINISH, eSetValueWithoutOverwrite);
        xQueueSendFromISR(pump_controller_msg_queue, &msg, NULL);
        pump_off(PUMP_MAIN_PIN);
    }
}

static void bottom_water_sensor_isr_handler()
{
    REFILLING_FLAG = 1;
    static pump_controller_msg_t msg = {.type = PAUSE_TASKS_MSG, .duration_s = 0, .pump_id = 0};
    xTaskNotify(task_mqtt_logger_handle, REFILLING_START, eSetValueWithoutOverwrite);
    xQueueSendFromISR(pump_controller_msg_queue, &msg, NULL);
    pump_on(PUMP_MAIN_PIN);
}

/* -------------------------------------------------------------------------- */
/*      Configure water sensors and pumps gpio and fill the tank if empty     */
/* -------------------------------------------------------------------------- */
void refilling_system_init()
{
    // Bottom sensor config
    water_sensor_config(SENSOR_LOW_SIGNAL_PIN, SENSOR_LOW_MODE_PIN);
    gpio_set_intr_type(SENSOR_LOW_SIGNAL_PIN, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(SENSOR_LOW_SIGNAL_PIN, bottom_water_sensor_isr_handler, NULL);

    // Top sensor config
    water_sensor_config(SENSOR_HIGH_SIGNAL_PIN, SENSOR_HIGH_MODE_PIN);
    gpio_set_intr_type(SENSOR_HIGH_SIGNAL_PIN, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(SENSOR_HIGH_SIGNAL_PIN, top_water_sensor_isr_handler, NULL);

    // Pumps config
    pump_config(PUMP_0_PIN);
    // pump_config(PUMP_1_PIN);
    // pump_config(PUMP_2_PIN);
    pump_config(PUMP_MAIN_PIN);

    // Fill the water for the first time
    if (gpio_get_level(SENSOR_LOW_SIGNAL_PIN) == 0)
    {
        ESP_LOGI(TAG, "Tank was empty during system init, filling the tank...");

        REFILLING_FLAG = 1;
        pump_on(PUMP_MAIN_PIN);
    }
    else
    {
        ESP_LOGW(TAG, "Tank was already full during system init!");
    }

    ESP_LOGI(TAG, "Initialization completed!");
}