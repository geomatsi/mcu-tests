#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "common.h"

static const char *TAG = "mod:motor";

#if 1 /* TEST_PWM */

#define M_PWM_TIMER           LEDC_TIMER_0
#define M_PWM_MODE            LEDC_LOW_SPEED_MODE
#define M_PWM_DUTY_RES        LEDC_TIMER_13_BIT   // Set duty resolution to 13 bits
#define M_PWM_FREQUENCY       (5000)              // Frequency in Hertz. Set frequency at 5 kHz
#define M_PWM_CLK             LEDC_AUTO_CLK       // Select source clock

#define M_PWM_DUTY_QUTR       (2047)              // Set duty to 25%  : ((2 ** 13) - 1) * 25%  - 1 = 2047
#define M_PWM_DUTY_HALF       (4095)              // Set duty to 50%  : ((2 ** 13) - 1) * 50%  - 1 = 4095
#define M_PWM_DUTY_FULL       (8191)              // Set duty to 100% : ((2 ** 13) - 1) * 100% - 1 = 8191

#define M1_PIN1               (12)
#define M1_PIN1_CH            LEDC_CHANNEL_0

#define M1_PIN2               (13)
#define M1_PIN2_CH            LEDC_CHANNEL_1

#define M2_PIN1               (14)
#define M2_PIN1_CH            LEDC_CHANNEL_2

#define M2_PIN2               (15)
#define M2_PIN2_CH            LEDC_CHANNEL_3

esp_err_t motor_init(void)
{
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = M_PWM_MODE,
		.timer_num        = M_PWM_TIMER,
		.duty_resolution  = M_PWM_DUTY_RES,
		.freq_hz          = M_PWM_FREQUENCY,
		.clk_cfg          = M_PWM_CLK
	};

	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel_config_t pwm_channel1 = {
		.timer_sel      = M_PWM_TIMER,
		.speed_mode     = M_PWM_MODE,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = M1_PIN1,
		.channel        = M1_PIN1_CH,
		.duty           = 0,
		.hpoint         = 0
	};

	ledc_channel_config_t pwm_channel2 = {
		.timer_sel      = M_PWM_TIMER,
		.speed_mode     = M_PWM_MODE,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = M1_PIN2,
		.channel        = M1_PIN2_CH,
		.duty           = 0,
		.hpoint         = 0
	};

	ledc_channel_config_t pwm_channel3 = {
		.timer_sel      = M_PWM_TIMER,
		.speed_mode     = M_PWM_MODE,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = M2_PIN1,
		.channel        = M2_PIN1_CH,
		.duty           = 0,
		.hpoint         = 0
	};

	ledc_channel_config_t pwm_channel4 = {
		.timer_sel      = M_PWM_TIMER,
		.speed_mode     = M_PWM_MODE,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = M2_PIN2,
		.channel        = M2_PIN2_CH,
		.duty           = 0,
		.hpoint         = 0
	};

	ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel1));
	ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel2));
	ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel3));
	ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel4));

	return ESP_OK;
}

void stop(void)
{
	ESP_LOGI(TAG, "STOP");

	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M1_PIN1_CH, 0));
	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M1_PIN2_CH, 0));
	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M2_PIN1_CH, 0));
	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M2_PIN2_CH, 0));

	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M1_PIN1_CH));
	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M1_PIN2_CH));
	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M2_PIN1_CH));
	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M2_PIN2_CH));
}

void fwd(uint32_t duty)
{
	ESP_LOGI(TAG, "FORWARD with duty %lu", duty);

	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M1_PIN1_CH, duty));
	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M1_PIN2_CH, 0));
	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M2_PIN1_CH, duty));
	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M2_PIN2_CH, 0));

	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M1_PIN1_CH));
	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M1_PIN2_CH));
	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M2_PIN1_CH));
	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M2_PIN2_CH));
}

void rot(uint32_t duty)
{
	ESP_LOGI(TAG, "ROTATE with duty %lu", duty);

	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M1_PIN1_CH, 0));
	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M1_PIN2_CH, duty));
	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M2_PIN1_CH, duty));
	ESP_ERROR_CHECK(ledc_set_duty(M_PWM_MODE, M2_PIN2_CH, 0));

	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M1_PIN1_CH));
	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M1_PIN2_CH));
	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M2_PIN1_CH));
	ESP_ERROR_CHECK(ledc_update_duty(M_PWM_MODE, M2_PIN2_CH));
}


void motor_task(void *args)
{

	while (1) {
		stop();
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		fwd(M_PWM_DUTY_QUTR);
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		rot(M_PWM_DUTY_QUTR);
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		fwd(M_PWM_DUTY_HALF);
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		rot(M_PWM_DUTY_HALF);
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		fwd(M_PWM_DUTY_FULL);
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		rot(M_PWM_DUTY_FULL);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

#else

#define M1_PIN1	12
#define M1_PIN2 13
#define M2_PIN1 14
#define M2_PIN2 15

esp_err_t motor_init(void)
{
	gpio_reset_pin(M1_PIN1);
	gpio_set_direction(M1_PIN1, GPIO_MODE_OUTPUT);
	gpio_set_level(M1_PIN1, 0);

	gpio_reset_pin(M1_PIN2);
	gpio_set_direction(M1_PIN2, GPIO_MODE_OUTPUT);
	gpio_set_level(M1_PIN2, 0);

	gpio_reset_pin(M2_PIN1);
	gpio_set_direction(M2_PIN1, GPIO_MODE_OUTPUT);
	gpio_set_level(M2_PIN1, 0);

	gpio_reset_pin(M2_PIN2);
	gpio_set_direction(M2_PIN2, GPIO_MODE_OUTPUT);
	gpio_set_level(M2_PIN2, 0);

	return ESP_OK;
}

void stop(void)
{
	ESP_LOGI(TAG, "STOP");

	gpio_set_level(M1_PIN1, 0);
	gpio_set_level(M1_PIN2, 0);

	gpio_set_level(M2_PIN1, 0);
	gpio_set_level(M2_PIN2, 0);
}

void fwd(void)
{
	ESP_LOGI(TAG, "FORWARD");

	gpio_set_level(M1_PIN1, 1);
	gpio_set_level(M1_PIN2, 0);

	gpio_set_level(M2_PIN1, 1);
	gpio_set_level(M2_PIN2, 0);
}

void rot(void)
{
	ESP_LOGI(TAG, "ROTATE");

	gpio_set_level(M1_PIN1, 1);
	gpio_set_level(M1_PIN2, 0);

	gpio_set_level(M2_PIN1, 0);
	gpio_set_level(M2_PIN2, 1);
}


void motor_task(void *args)
{

	while (1) {
		stop();
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		fwd();
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		rot();
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

#endif /* TEST_PWM */
