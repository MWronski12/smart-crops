// Freertos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Espressif
#include "esp_log.h"

// System Init
#include "refilling_system_init.h"
#include "mqtt_init.h"

// Tasks
#include "task_pump_controller.h"
#include "task_mqtt_logger.h"

// App config file
#include "app_config.h"

// Global variables
QueueHandle_t pump_controller_msg_queue = NULL;
TaskHandle_t task_mqtt_logger_handle = NULL;
esp_mqtt_client_handle_t client = NULL;
volatile uint8_t REFILLING_FLAG = 0;

void app_main()
{
    const char *TAG = "main";

    // Global variables
    pump_controller_msg_queue = xQueueCreate(10, sizeof(pump_controller_msg_t));
    client = esp_mqtt_client_init(&mqtt_cfg);

    // Setup
    refilling_system_init();
    mqtt_init();

    // Tasks
    xTaskCreate(task_mqtt_logger, "task_mqtt_logger", 2048, NULL, TASK_MQTT_LOGGER_PRIORITY, &task_mqtt_logger_handle);
    xTaskCreate(task_pump_controller, "task_pump_controller", 2048, NULL, TASK_PUMP_CONTROLLER_PRIORITY, NULL);

    ESP_LOGI(TAG, "System initialized!");
}
