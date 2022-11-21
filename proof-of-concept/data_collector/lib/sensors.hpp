#pragma once

extern "C"
{
#include "bmx280.h"
#include "bmx280_params.h"
#include "bh1750fvi.h"
}
#include "common.hpp"
#include <tuple>

namespace sensors
{
    /* -------------------------------- Variables ------------------------------- */
    constexpr auto wakeup_period = 60 * 1000; // in milliseconds
    bmx280_t bmp280_dev;
    bh1750fvi_t bh1750fvi_dev;

    /* --------------------------------- Thread --------------------------------- */
    char stack[THREAD_STACKSIZE_SMALL];

    struct Data
    {
        uint16_t temperature;    // in centi celcius (XX.XX°C)
        uint16_t light_intesity; // in lux
        uint32_t air_pressure;   // in pascal
    };

    auto thread(void *arg) -> void *
    {
        auto target_pid = *static_cast<kernel_pid_t *>(arg);
        auto last_wakeup = xtimer_ticks32_t{};
        while (true)
        {
            // Make measurements
            static auto data = Data{};
            data.light_intesity = bh1750fvi_sample(&bh1750fvi_dev);
            data.temperature = bmx280_read_temperature(&bmp280_dev);
            data.air_pressure = bmx280_read_pressure(&bmp280_dev);

            // Send data
            auto msg = msg_t{};
            msg.content.ptr = &data;
            msg.content.value = sizeof(data);
            msg_send(&msg, target_pid);

            // Go to sleep
            xtimer_periodic_wakeup(&last_wakeup, wakeup_period);
        }
    }

    /* ----------------------------- Initialization ----------------------------- */
    auto run(i2c_t i2c_dev, kernel_pid_t consumer) -> std::tuple<kernel_pid_t, Error>
    {
        // Initialize bmp280
        if (bmx280_init(&bmp280_dev, bmx280_params) != BMX280_OK)
            return {0, "failed to initialize bmp280"};

        // Initialize bh1750fvi
        auto bh1750fvi_params = bh1750fvi_params_t{i2c_dev, BH1750FVI_ADDR_PIN_LOW};
        if (bh1750fvi_init(&bh1750fvi_dev, &bh1750fvi_params) != 0)
            return {0, "failed initialize bh1750fvi"};

        // Start the thread
        auto pid = thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1, 0, thread, (void *)&consumer, "sensors");
        if (pid < 0)
            return {0, "failed to start a thread"};

        return {pid, NULL};
    }
}