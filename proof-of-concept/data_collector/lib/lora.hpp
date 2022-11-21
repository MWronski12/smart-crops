#pragma once

extern "C"
{
#include "stdio.h"
#include "string.h"
#include "net/loramac.h"     /* core loramac definitions */
#include "semtech_loramac.h" /* package API */
#include "xtimer.h"
#include "thread.h"
#include "msg.h"
}
#include "common.hpp"
#include <tuple>

// This object is initialized by the library
extern semtech_loramac_t loramac;

namespace lora
{
    /* -------------------------------- Constants ------------------------------- */
    static const uint8_t deveui[LORAMAC_DEVEUI_LEN] = {0x60, 0x81, 0xF9, 0xC1, 0xF0, 0xB0, 0x7B, 0xE6};
    static const uint8_t appeui[LORAMAC_APPEUI_LEN] = {0x60, 0x81, 0xF9, 0x4B, 0x7B, 0x86, 0x37, 0xA4};
    static const uint8_t appkey[LORAMAC_APPKEY_LEN] = {0x18, 0xE3, 0xE6, 0x56, 0x34, 0xD7, 0x01, 0xF3, 0x4B, 0xD2, 0xE6, 0xBE, 0x85, 0x2F, 0x0B, 0x50};

    /* --------------------------------- Threads -------------------------------- */
    static char stack[THREAD_STACKSIZE_SMALL];

    auto thread(void *arg) -> void *
    {
        (void)arg; // Ignore unused argument
        while (true)
        {
            auto msg = msg_t{};
            msg_receive(&msg);                                   // Wait for a message
            auto buf = (uint8_t *)msg.content.ptr;               // Data buffer
            auto len = msg.content.value;                        // Length of the data buffer
            auto ret = semtech_loramac_send(&loramac, buf, len); // Send data over LoRa
            if (ret != SEMTECH_LORAMAC_TX_DONE)                  // Verify result
                puts("LoRa: Failed to send message");
        }
    };

    /* ------------------------- Initialization function ------------------------ */
    auto run() -> std::tuple<kernel_pid_t, Error>
    {
        // Set all required variables
        semtech_loramac_set_deveui(&loramac, deveui);
        semtech_loramac_set_appeui(&loramac, appeui);
        semtech_loramac_set_appkey(&loramac, appkey);
        semtech_loramac_set_dr(&loramac, LORAMAC_DR_5);

        // Join the network
        auto ret = semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA);
        if (ret != SEMTECH_LORAMAC_JOIN_SUCCEEDED)
            return {0, "join procedure failed"};

        // Wait before starting the thread to eliminate duty cycle issues
        xtimer_msleep(1000);

        // Start the sender thread
        auto pid = thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1, 0, thread, NULL, "lora");
        if (pid < 0)
            return {0, "failed to start the lora thread"};

        return {pid, NULL};
    }
}
