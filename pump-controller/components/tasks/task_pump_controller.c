#include "task_pump_controller.h"

static const char *TAG = "task_pump_controller";

static pump_t pumps_config[] = {
    {.id = 0, .gpio = PUMP_0_PIN, .has_active_task = 0, .active_task_ticks_left = 0, .timer = NULL},
    // {.id = 1, .gpio = PUMP_1_PIN, .has_active_task = 0, .timer = NULL},
    // {.id = 2, .gpio = PUMP_2_PIN, .has_active_task = 0, .timer = NULL},
};

static pump_t *get_pump_by_id(uint8_t pump_id)
{
    for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
    {
        if (pumps_config[i].id == pump_id)
        {
            return &pumps_config[i];
        }
    }
    ESP_LOGW(TAG, "Could not get pump with id=%d", pump_id);
    return NULL;
}

static void timer_callback(TimerHandle_t timer)
{
    const uint8_t pump_id = *(uint8_t *)pvTimerGetTimerID(timer);
    xTaskNotify(task_mqtt_logger_handle, TASK_FINISH, eSetValueWithoutOverwrite);
    ESP_LOGI(TAG, "Timer %d expired", pump_id);
    pump_t *pump = get_pump_by_id(pump_id);
    pump->has_active_task = 0;
    pump_off(pump_id);
}

static void pump_timers_config()
{
    pump_t *pump;

    for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
    {
        pump = &pumps_config[i];

        pump->timer = xTimerCreate(
            "pump_timer",
            pdMS_TO_TICKS(5000),
            pdFALSE,
            (void *)&pump->id,
            timer_callback);

        if (pump->timer == NULL)
        {
            ESP_LOGE(TAG, "Timer for pump with id=%d was not created!", pump->id);
        }
        else
        {
            ESP_LOGI(TAG, "Timer for pump with id=%d created succesfully!", pump->id);
        }
    }
}

static void on_new_task_msg(pump_controller_msg_t *msg)
{
    pump_t *pump;
    TickType_t duration_ticks;

    pump = get_pump_by_id(msg->pump_id);

    if (pump == NULL)
    {
        ESP_LOGE(TAG, "Pump with id=%d does not exist! Aborting new task request...", msg->pump_id);
        return;
    }

    if (pump->has_active_task)
    {
        ESP_LOGW(TAG, "Previous task of pump with id=%d was not finished yet! Aborting new task request...", pump->id);
        return;
    }

    pump->has_active_task = 1;
    duration_ticks = pdMS_TO_TICKS(msg->duration_s * 1000);
    pump->active_task_ticks_left = duration_ticks;
    xTimerChangePeriod(pump->timer, duration_ticks, pdMS_TO_TICKS(100));

    if (REFILLING_FLAG == 0)
    {
        xTimerStart(pump->timer, pdMS_TO_TICKS(100));
        pump_on(pump->id);
        ESP_LOGI(TAG, "Timer %d started!", pump->id);
    }
    else if (REFILLING_FLAG == 1)
    {
        pump_off(pump->gpio);
        xTimerStop(pump->timer, pdMS_TO_TICKS(100));
        ESP_LOGW(TAG, "Timer %d stopped! New task request during refilling the tank", pump->id);
    }
}

static void on_pause_tasks_msg()
{
    pump_t *pump;

    if (REFILLING_FLAG == 1)
    {
        for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
        {
            pump = &pumps_config[i];

            if (pump->has_active_task && xTimerIsTimerActive(pump->timer))
            {
                TickType_t expiry_time = xTimerGetExpiryTime(pump->timer);
                TickType_t now = xTaskGetTickCount();
                uint8_t overflow_happened = expiry_time < now;
                if (overflow_happened)
                {
                    pump->active_task_ticks_left = 0xffffffff - now + expiry_time;
                }
                else
                {
                    pump->active_task_ticks_left = expiry_time - now;
                }

                pump_off(pump->gpio);
                if (xTimerStop(pump->timer, pdMS_TO_TICKS(100)) == pdPASS)
                {
                    ESP_LOGI(TAG, "Timer %d paused successfully! Ticks left to finish the task: %d", pump->id, pump->active_task_ticks_left);
                }
                else
                {
                    ESP_LOGE(TAG, "Pause command was not sent to Timer %d!", pump->id);
                }
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "PAUSE_TASKS message received, but tank refilling was not happening!");
    }
}

static void on_start_tasks_msg()
{
    pump_t *pump;

    if (REFILLING_FLAG == 0)
    {
        for (int i = 0; i < sizeof(pumps_config) / sizeof(pump_t); i++)
        {
            pump = &pumps_config[i];

            if (pump->has_active_task && !xTimerIsTimerActive(pump->timer))
            {

                if (xTimerChangePeriod(pump->timer, pump->active_task_ticks_left, pdMS_TO_TICKS(100)) == pdPASS)
                {
                    pump_on(pump->gpio);
                    ESP_LOGI(TAG, "Timer %d started successfully! Ticks left to finished the task: %d", pump->id, pump->active_task_ticks_left);
                }
                else
                {
                    ESP_LOGE(TAG, "Start command was not sent to Timer %d!", pump->id);
                }
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "START_TASKS message received, but tank refilling was happening!");
    }
}

/* -------------------------------------------------------------------------- */
/*             Handle commands "turn on pump <id> for <n> seconds"            */
/* -------------------------------------------------------------------------- */
void task_pump_controller(void *arg)
{
    (void)arg;

    pump_timers_config();

    pump_controller_msg_t msg;

    for (;;)
    {
        if (xQueueReceive(pump_controller_msg_queue, &msg, portMAX_DELAY) == pdTRUE)
        {
            switch (msg.type)
            {

            case NEW_TASK_MSG:

                xTaskNotify(task_mqtt_logger_handle, TASK_START, eSetValueWithoutOverwrite);
                ESP_LOGI(TAG, "NEW_TASK msg received! duration_s=%d, pump_id=%d", msg.duration_s, msg.pump_id);
                on_new_task_msg(&msg);
                break;

            case PAUSE_TASKS_MSG:

                ESP_LOGI(TAG, "PAUSE_TASKS msg received!");
                on_pause_tasks_msg();
                break;

            case START_TASKS_MSG:

                ESP_LOGI(TAG, "START_TASKS msg reveiced!");
                on_start_tasks_msg();
                break;

            default:
                ESP_LOGW(TAG, "Unknown message received!");
                break;
            }
        }
        else
        {
            ESP_LOGE(TAG, "Error receiving message from the queue!");
        }
    }
}