#include "bh1750fvi.h"

int main(void)
{
    // Initialize I2C
    i2c_t i2c_dev = 0;
    i2c_init(i2c_dev);

    // Initialize bh1750fvi
    bh1750fvi_t bh1750fvi_dev;
    bh1750fvi_params_t bh1750fvi_params = {i2c_dev, BH1750FVI_ADDR_PIN_LOW};
    int err = bh1750fvi_init(&bh1750fvi_dev, &bh1750fvi_params);
    if (err != 0)
    {
        printf("failed to initialize bh1750fvi\n");
        return 1;
    }

    // Measure light intesity
    uint16_t intesity = bh1750fvi_sample(&bh1750fvi_dev);
    printf("Light intesity: %dlx\n", intesity);

    return 0;
}