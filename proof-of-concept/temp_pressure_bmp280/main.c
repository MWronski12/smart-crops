#include "bmx280.h"
#include "bmx280_params.h"
#include "xtimer.h"
#include "debug.h"

bmx280_t dev;


int main(void){

    int bmp280_init_code = bmx280_init(&dev,bmx280_params);
    if(bmp280_init_code!=BMX280_OK){
        printf("bmp280 init error: %d\r\n",bmp280_init_code);
    }else {
        puts("bmp280 initialized successfully\r\n");
        xtimer_ticks32_t timer = xtimer_now();
        const uint32_t wakeup_period = 1000000;
        while(1){
            int16_t temp = bmx280_read_temperature(&dev);
            uint32_t pressure = bmx280_read_pressure(&dev);
            printf("Temp=%d C Pressure=%lu PA\r\n",temp,pressure);
            xtimer_periodic_wakeup(&timer, wakeup_period);
        };
    }
    return 0;
}