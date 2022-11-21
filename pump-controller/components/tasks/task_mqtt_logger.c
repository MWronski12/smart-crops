#include "task_mqtt_logger.h"

static const char *TAG = "task_mqtt_logger";

/* -------------------------------------------------------------------------- */
/*    Sends log messages over MQTT to PUBLISH_TOPIC to allow system control   */
/* -------------------------------------------------------------------------- */
void task_mqtt_logger(void *arg)
{
    (void)arg;
    char payload[MQTT_MAX_PAYLOAD_LENGTH];

    int msg_id;
    int event;

    for (;;)
    {
        event = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        switch (event)
        {
        case REFILLING_START:
            strcpy(payload, "Refilling of the tank started!");
            break;

        case REFILLING_FINISH:
            strcpy(payload, "Refilling of the tank finished!");
            break;

        case TASK_START:
            strcpy(payload, "New task started!");
            break;

        case TASK_FINISH:
            strcpy(payload, "Task finished!");
            break;

        case TASK_BAD_REQUEST:
            strcpy(payload, "Bad request for a new task!");
            break;

        default:
            ESP_LOGE(TAG, "Unknown event!");
            break;
        }

        msg_id = esp_mqtt_client_publish(client, PUBLISH_TOPIC, payload, strlen(payload), 2, 0);

        if (msg_id == -1)
        {
            ESP_LOGE(TAG, "Failed to send log message! Payload: %s", payload);
        }
    }
}