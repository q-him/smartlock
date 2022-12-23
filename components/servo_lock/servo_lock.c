#include "include/servo_lock.h"
#include "driver/mcpwm.h"

QueueHandle_t sl_queue;

void servo_lock_task(void *params);

void start_servo_lock_task() {
    sl_queue = xQueueCreate(8, sizeof(bool));
    xTaskCreate(servo_lock_task, SL_TAG, 8192, NULL, 1, NULL);
}

#define SERVO_MIN_PULSEWIDTH_US (1000) // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US (2000) // Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE        (90)   // Maximum angle in degree upto which servo can rotate

#define SERVO_PULSE_GPIO        (23)   // GPIO connects to the PWM signal line

static inline uint32_t angle_to_duty(int angle)
{
    return (angle + 90) * (CONFIG_SERVO_MAX_PULSEWIDTH_US - CONFIG_SERVO_MIN_PULSEWIDTH_US) / (2 * 90) + CONFIG_SERVO_MIN_PULSEWIDTH_US;
}

void servo_lock_task(void* params)
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_PULSE_GPIO); // To drive a RC servo, one MCPWM generator is enough

    mcpwm_config_t pwm_config = {
        .frequency = 50, // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        .cmpr_a = 0,     // duty cycle of PWMxA = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    bool state;
    while (1) {
        xQueueReceive(sl_queue, (void*)(&state), portMAX_DELAY);

        if (!state) {
            for (int angle = -10; angle < 10; angle += 1) {
                ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle_to_duty(angle)));
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            ESP_LOGI(SL_TAG, "servo lock locked");
        } else {
            for (int angle = 10; angle > -10; angle -= 1) {
                ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle_to_duty(angle)));
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            ESP_LOGI(SL_TAG, "servo lock unlocked");
        }
    }
}