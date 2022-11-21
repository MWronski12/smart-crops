/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example demonstrating the use of LoRaWAN with RIOT
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
// BMP280
#include "bmx280.h"
#include "bmx280_params.h"
#include "xtimer.h"

#include "msg.h"
#include "thread.h"
#include "fmt.h"

#include "periph/pm.h"
#if IS_USED(MODULE_PERIPH_RTC)
#include "periph/rtc.h"
#else
#include "timex.h"
#include "ztimer.h"
#endif

#include "net/loramac.h"
#include "semtech_loramac.h"

/* By default, messages are sent every 20s to respect the duty cycle
   on each channel */
#ifndef SEND_PERIOD_S
#define SEND_PERIOD_S (20U)
#endif

/* Low-power mode level */
#define PM_LOCK_LEVEL (1)

#define SENDER_PRIO (THREAD_PRIORITY_MAIN - 1)
static kernel_pid_t sender_pid;
static char sender_stack[THREAD_STACKSIZE_MAIN / 2];

extern semtech_loramac_t loramac;
#if !IS_USED(MODULE_PERIPH_RTC)
static ztimer_t timer;
#endif

// Stack for thread
char stack_thread_update_msg[THREAD_STACKSIZE_MAIN];
static kernel_pid_t update_msg_pid;
// TEMP INIT
bmx280_t dev;

// MUTEX for msg block

#define MSG_LENGTH 6
static uint8_t message[MSG_LENGTH] = {67, 68, 69, 70, 71, 72};

#ifdef USE_OTAA
static uint8_t deveui[LORAMAC_DEVEUI_LEN];
static uint8_t appeui[LORAMAC_APPEUI_LEN];
static uint8_t appkey[LORAMAC_APPKEY_LEN];
#endif

#ifdef USE_ABP
static uint8_t devaddr[LORAMAC_DEVADDR_LEN];
static uint8_t nwkskey[LORAMAC_NWKSKEY_LEN];
static uint8_t appskey[LORAMAC_APPSKEY_LEN];
#endif

static void print_message(uint8_t *message, int length)
{
    printf("Message: ");
    for (int i = 0; i < length; i++)
    {
        printf("%u ", message[i]);
    }
    printf("\r\n");
}

static void *thread_update_msg(void *arg)
{
    (void)arg;
    int bmp280_init_code = bmx280_init(&dev, bmx280_params);
    if (bmp280_init_code != BMX280_OK)
    {
        printf("bmp280 init error: %d\r\n", bmp280_init_code);
    }
    else
    {
        puts("bmp280 initialized successfully\r\n");
        msg_t message_receive, message_send;
        // xtimer_ticks32_t timer = xtimer_now();
        // const uint32_t wakeup_period = 1000000;
        while (1)
        {
            msg_receive(&message_receive);
            int16_t temp = bmx280_read_temperature(&dev);
            uint32_t preassure = bmx280_read_pressure(&dev);

            memcpy(message, &temp, 2);
            memcpy(message + 2, &preassure, 4);

            printf("MSG Updated by thread_update_msg\r\n");
            print_message(message, MSG_LENGTH);
            msg_send(&message_send, sender_pid);
        };
    }
    return NULL;
}

static void _alarm_cb(void *arg)
{
    (void)arg;
    msg_t msg;
    msg_send(&msg, sender_pid);
}

static void _prepare_next_alarm(void)
{
#if IS_USED(MODULE_PERIPH_RTC)
    struct tm time;
    rtc_get_time(&time);
    /* set initial alarm */
    time.tm_sec += SEND_PERIOD_S;
    mktime(&time);
    rtc_set_alarm(&time, _alarm_cb, NULL);
#else
    timer.callback = _alarm_cb;
    ztimer_set(ZTIMER_MSEC, &timer, SEND_PERIOD_S * MS_PER_SEC);
#endif
}

static void _send_message(void)
{
    printf("Sending...\n");
    print_message(message, MSG_LENGTH);
    /* Try to send the message */
    uint8_t ret = semtech_loramac_send(&loramac,
                                       message, sizeof(message));
    if (ret != SEMTECH_LORAMAC_TX_DONE)
    {
        printf("Cannot send message - ret code: %d\n", ret);
        return;
    }
}

static void *sender(void *arg)
{
    (void)arg;

    msg_t msg_receive_wakeup, msg_receive_update_confirm, message_send;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    while (1)
    {
        msg_receive(&msg_receive_wakeup);
        msg_send(&message_send, update_msg_pid);
        msg_receive(&msg_receive_update_confirm);

        /* Trigger the message send */
        _send_message();

        /* Schedule the next wake-up alarm */
        _prepare_next_alarm();
    }

    /* this should never be reached */
    return NULL;
}

int main(void)
{
    puts("LoRaWAN Class A low-power application");
    puts("=====================================");

    /*
     * Enable deep sleep power mode (e.g. STOP mode on STM32) which
     * in general provides RAM retention after wake-up.
     */
#if IS_USED(MODULE_PM_LAYERED)
    for (unsigned i = 1; i < PM_NUM_MODES - 1; ++i)
    {
        pm_unblock(i);
    }
#endif

#ifdef USE_OTAA /* OTAA activation mode */
    /* Convert identifiers and keys strings to byte arrays */
    fmt_hex_bytes(deveui, CONFIG_LORAMAC_DEV_EUI_DEFAULT);
    fmt_hex_bytes(appeui, CONFIG_LORAMAC_APP_EUI_DEFAULT);
    fmt_hex_bytes(appkey, CONFIG_LORAMAC_APP_KEY_DEFAULT);
    semtech_loramac_set_deveui(&loramac, deveui);
    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_appkey(&loramac, appkey);

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

    /* Join the network if not already joined */
    if (!semtech_loramac_is_mac_joined(&loramac))
    {
        /* Start the Over-The-Air Activation (OTAA) procedure to retrieve the
         * generated device address and to get the network and application session
         * keys.
         */
        puts("Starting join procedure");
        if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED)
        {
            puts("Join procedure failed");
            return 1;
        }

#ifdef MODULE_PERIPH_EEPROM
        /* Save current MAC state to EEPROM */
        semtech_loramac_save_config(&loramac);
#endif
    }
#endif

#ifdef USE_ABP /* ABP activation mode */
    /* Convert identifiers and keys strings to byte arrays */
    fmt_hex_bytes(devaddr, CONFIG_LORAMAC_DEV_ADDR_DEFAULT);
    fmt_hex_bytes(nwkskey, CONFIG_LORAMAC_NWK_SKEY_DEFAULT);
    fmt_hex_bytes(appskey, CONFIG_LORAMAC_APP_SKEY_DEFAULT);
    semtech_loramac_set_devaddr(&loramac, devaddr);
    semtech_loramac_set_nwkskey(&loramac, nwkskey);
    semtech_loramac_set_appskey(&loramac, appskey);

    /* Configure RX2 parameters */
    semtech_loramac_set_rx2_freq(&loramac, CONFIG_LORAMAC_DEFAULT_RX2_FREQ);
    semtech_loramac_set_rx2_dr(&loramac, CONFIG_LORAMAC_DEFAULT_RX2_DR);

#ifdef MODULE_PERIPH_EEPROM
    /* Store ABP parameters to EEPROM */
    semtech_loramac_save_config(&loramac);
#endif

    /* Use a fast datarate, e.g. BW125/SF7 in EU868 */
    semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

    /* ABP join procedure always succeeds */
    semtech_loramac_join(&loramac, LORAMAC_JOIN_ABP);
#endif
    puts("Join procedure succeeded");

    xtimer_msleep(2000);

    update_msg_pid = thread_create(
        stack_thread_update_msg,
        THREAD_STACKSIZE_MAIN,
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        thread_update_msg,
        NULL,
        "Meas & Update MSG");

    /* start the sender thread */
    sender_pid = thread_create(sender_stack, sizeof(sender_stack),
                               SENDER_PRIO, 0, sender, NULL, "sender");

    /* trigger the first send */
    msg_t msg;
    msg_send(&msg, sender_pid);
    return 0;
}
