#include "mqtt_init.h"

static const char *TAG = "mqtt_init";

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);

esp_mqtt_client_config_t mqtt_cfg = {
    .uri = MQTT_BROKER_ADDR,
    .event_handle = mqtt_event_handler,
    .task_prio = TASK_MQTT_CLIENT_PRIORITY,
    .client_cert_pem = (const char *)client_cert_pem_start,
    .client_key_pem = (const char *)client_key_pem_start,
    .cert_pem = (const char *)ca_cert_pem_start,
    // .lwt_topic = "TODO!",
    // .lwt_msg = "TODO!",
};

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, SUBSCRIBE_TOPIC, 2);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOGD(TAG, "{ TOPIC=\"%.*s\", DATA=\"%.*s\" }", event->topic_len, event->topic, event->data_len, event->data);

        pump_controller_msg_t *msg = mqtt_payload_to_pump_controller_msg_t(event->data, event->data_len);
        if (msg != NULL)
        {
            taskENTER_CRITICAL();
            xQueueSend(pump_controller_msg_queue, msg, pdMS_TO_TICKS(100));
            taskEXIT_CRITICAL();
        }
        else
        {
            ESP_LOGE(TAG, "Error parsing mqtt message!");
        }
        heap_caps_free(msg);

        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

/* -------------------------------------------------------------------------- */
/*                 Connect to WiFi and start MQTT client task                 */
/* -------------------------------------------------------------------------- */
void mqtt_init(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());

    esp_mqtt_client_start(client);
}
