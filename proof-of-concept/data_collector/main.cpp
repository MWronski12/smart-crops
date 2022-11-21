#include "lib/lora.hpp"
#include "lib/sensors.hpp"
#include "lib/common.hpp"

auto main() -> int
{
    // Initialize lora
    auto [pid_lora, err] = lora::run();
    if (err != NULL)
    {
        printf("Error: failed to initialize LoRa: %s", err);
        return 1;
    }

    // Initialize I2C
    auto i2c_dev = i2c_t{0};
    i2c_init(i2c_dev);

    // Initialize sensors
    auto [pid_sensors, err2] = sensors::run(i2c_dev, pid_lora);
    if (err2 != NULL)
    {
        printf("Error: failed to initialize sensors: %s", err2);
        return 1;
    }

    printf("Pid of sensors: %d", pid_sensors);

    return 0;
}