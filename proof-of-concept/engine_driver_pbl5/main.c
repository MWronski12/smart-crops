#include <stdio.h>
#include "stm32l072xx.h"
#include "xtimer.h"
#include "timex.h"
#include "periph/gpio.h"
#include "periph/adc.h"
#include "inttypes.h"
#include "msg.h"
#include <math.h>

/* set interval to 1 second */
#define INTERVAL (1U * US_PER_SEC)

/* --------------------------------- STACKS --------------------------------- */
char stack_thread_measure_current[THREAD_STACKSIZE_DEFAULT];
char stack_thread_measure_fan_speed[THREAD_STACKSIZE_DEFAULT];
char stack_thread_calculate_average_current[THREAD_STACKSIZE_DEFAULT];
char stack_thread_signal_fault[THREAD_STACKSIZE_DEFAULT];

/* ---------------------------------- MUTEX --------------------------------- */
mutex_t mutex_adc;

/* -------------------------------- MSG_TYPES ------------------------------- */
const uint16_t MSG_CURRENT = 1;
const uint16_t MSG_FAN_SPEED = 2;


/* --------------------------------- THREADS -------------------------------- */
void *thread_measure_current(void *arg __attribute__((unused)))
{
    const adc_t adc_line = ADC_LINE(0);
    const uint32_t wakeup_period = 1000000 / 1000;

    //adc_init(adc_line);
    xtimer_ticks32_t timer = xtimer_now();
    msg_t msg;

    kernel_pid_t thread_calculate_average_current_pid = *(kernel_pid_t *)arg;

    while (1)
    {
        mutex_lock(&mutex_adc);
        const int32_t result = adc_sample(adc_line, ADC_RES_10BIT);
        mutex_unlock(&mutex_adc);

        const float voltage = (result / 4096.0) * 3.3;
        const uint32_t current_mA = voltage * 5 * 1000;

        msg.content.value = current_mA;
        msg_send(&msg, thread_calculate_average_current_pid);

        xtimer_periodic_wakeup(&timer, wakeup_period);
    }

    return NULL;
}

void *thread_measure_fan_speed(void *arg __attribute__((unused)))
{
    const adc_t adc_line = ADC_LINE(2);
    const uint32_t wakeup_period = 1000000;

    adc_init(adc_line);

    xtimer_ticks32_t timer = xtimer_now();
    msg_t msg;

    kernel_pid_t thread_signal_fault_pid = *(kernel_pid_t *)arg;

    while (1)
    {
        mutex_lock(&mutex_adc);
        const int32_t result = adc_sample(adc_line, ADC_RES_10BIT);
        printf("fan_speed: %ld\r\n",result);
        mutex_unlock(&mutex_adc);

        const float voltage = (result / 4096.0) * 3.3;
        const uint32_t fan_speed_spins_per_min = voltage * 500;

        msg.content.value = fan_speed_spins_per_min;
        msg.type = MSG_FAN_SPEED;
        msg_send(&msg, thread_signal_fault_pid);

        xtimer_periodic_wakeup(&timer, wakeup_period);
    }

    return NULL;
}

void *thread_calculate_average_current(void *arg __attribute__((unused)))
{
    float current_squares_sum = 0;
    uint32_t sample_count = 0;
    msg_t msg;
    kernel_pid_t thread_signal_fault_pid = *(kernel_pid_t *)arg;

    while (1)
    {

        if (msg_receive(&msg) == 1)
        {
            const float current_A = msg.content.value / 1000.0;
            current_squares_sum += current_A * current_A;
            sample_count++;

            if (sample_count == 1000)
            {
                const float current_rms_A = sqrt(current_squares_sum / 1000);
                const uint32_t current_rms_mA = current_rms_A * 1000;

                sample_count = 0;
                current_squares_sum = 0;

                msg.content.value = current_rms_mA;
                msg.type = MSG_CURRENT;
                msg_send(&msg, thread_signal_fault_pid);
            }
        }
    }

    return NULL;
}

void *thread_signal_fault(void *arg __attribute__((unused)))
{
    msg_t msg;
    const uint32_t fan_speed_limit = 1500; // fan speed limit in rotations per second
    const uint32_t current_limit = 15000; //current limit in mA
    while (1)
    {
        if (msg_receive(&msg) == 1)
        {
            if (msg.type == MSG_CURRENT && msg.content.value >= current_limit)
            {
                printf("ALERT, Current RMS: %lu, limit: %lu\r\n", msg.content.value,current_limit);
            } else if (msg.type == MSG_FAN_SPEED && msg.content.value >=fan_speed_limit){
                printf("ALERT, fan speed: %lu, limit: %lu\r\n",msg.content.value,fan_speed_limit);
            }
        }
    }
    return NULL;
}

int main(void)
{

    kernel_pid_t thread_signal_fault_pid = thread_create(
        stack_thread_signal_fault,
        THREAD_STACKSIZE_DEFAULT,
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        thread_signal_fault,
        NULL,
        "thread_signal_fault");

    kernel_pid_t thread_calculate_average_current_pid = thread_create(
        stack_thread_calculate_average_current,
        THREAD_STACKSIZE_DEFAULT,
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        thread_calculate_average_current,
        (void *)&thread_signal_fault_pid,
        "thread_calculate_average_current");

    thread_create(
        stack_thread_measure_current,
        THREAD_STACKSIZE_DEFAULT,
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        thread_measure_current,
        (void *)&thread_calculate_average_current_pid,
        "thread_measure_current");

    thread_create(
        stack_thread_measure_fan_speed,
        THREAD_STACKSIZE_DEFAULT,
        THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST,
        thread_measure_fan_speed,
        (void *)&thread_signal_fault_pid,
        "thread_measurestack_thread_measure_fan_speed");

    return 0;
}