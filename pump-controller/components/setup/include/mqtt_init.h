#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Espressif
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "protocol_examples_common.h"

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_heap_caps.h"

// Components
#include "mqtt_payload_parser.h"
#include "task_pump_controller.h"

// App config file
#include "app_config.h"

extern esp_mqtt_client_handle_t client;
esp_mqtt_client_config_t mqtt_cfg;

/* ----------------------------- CA certificate ----------------------------- */
extern const uint8_t ca_cert_pem_start[] asm("_binary_ca_crt_start");
extern const uint8_t ca_cert_pem_end[] asm("_binary_ca_cert_end");

/* --------------------------- Client certificate --------------------------- */
extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");

/* ------------------------------- Client key ------------------------------- */
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");

void mqtt_init(void);
