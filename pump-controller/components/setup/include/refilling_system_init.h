#pragma once

#include "driver/gpio.h"
#include "esp_log.h"

#include "pump_driver.h"
#include "water_sensor_driver.h"

#include "task_pump_controller.h"

#include "app_config.h"

extern volatile uint8_t REFILLING_FLAG;

void refilling_system_init();