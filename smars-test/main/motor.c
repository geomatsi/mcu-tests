#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "driver/gpio.h"

#include "common.h"

#define M1_PIN1	12
#define M1_PIN2 13
#define M2_PIN1 14
#define M2_PIN2 15

static const char *TAG = "mod:motor";

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
