#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "sdkconfig.h"
#include "led_strip_spi.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

#define STACK_SIZE 3584

#define N_PIXEL 1
#define LED_CI	39
#define LED_DI	40

#define TAG "LED"

void set_color(rgb_t *color, uint8_t red, uint8_t green, uint8_t blue)
{
	color->r = red;
	color->g = green;
	color->b = blue;
}

void vTaskLed(void *pvParameters)
{
	const uint8_t bright[] = {
		0, 10, 22, 43, 70, 100, 120, 140, 170, 200, 250,
		200, 170, 140, 120, 100, 70, 43, 22, 10
	};

	static spi_device_handle_t device_handle;
	led_strip_spi_t strip = LED_STRIP_SPI_DEFAULT();
	rgb_t color;
	int i;

	strip.mosi_io_num = LED_DI;
	strip.sclk_io_num = LED_CI;
	strip.length = N_PIXEL;
	strip.device_handle = device_handle;
	strip.max_transfer_sz = LED_STRIP_SPI_BUFFER_SIZE(N_PIXEL);
	strip.clock_speed_hz = 1000000 * 10; // 10Mhz

	/* init module and turn off all LEDs */
	ESP_ERROR_CHECK(led_strip_spi_install());
	ESP_ERROR_CHECK(led_strip_spi_init(&strip));
	ESP_ERROR_CHECK(led_strip_spi_flush(&strip));

	while (1) {
		ESP_LOGI(TAG, "red...");
		for (i = 0; i < ARRAY_SIZE(bright); i++) {
			set_color(&color, 200, 0, 0);
			ESP_ERROR_CHECK(led_strip_spi_set_pixel_brightness(&strip, 0, color, bright[i]));
			ESP_ERROR_CHECK(led_strip_spi_flush(&strip));
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		ESP_LOGI(TAG, "green...");
		for (i = 0; i < ARRAY_SIZE(bright); i++) {
			set_color(&color, 0, 200, 0);
			ESP_ERROR_CHECK(led_strip_spi_set_pixel_brightness(&strip, 0, color, bright[i]));
			ESP_ERROR_CHECK(led_strip_spi_flush(&strip));
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		ESP_LOGI(TAG, "blue...");
		for (i = 0; i < ARRAY_SIZE(bright); i++) {
			set_color(&color, 0, 0, 200);
			ESP_ERROR_CHECK(led_strip_spi_set_pixel_brightness(&strip, 0, color, bright[i]));
			ESP_ERROR_CHECK(led_strip_spi_flush(&strip));
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
}

void app_main(void)
{
	TaskHandle_t xLedHandle = NULL;
	esp_chip_info_t chip_info;
	unsigned int minor_rev;
	unsigned int major_rev;
	uint32_t flash_size;

	/* Print ESP32 chip information */
	esp_chip_info(&chip_info);
	ESP_LOGI(TAG, "This is %s chip with %d CPU core(s), WiFi%s%s%s, ",
			CONFIG_IDF_TARGET,
			chip_info.cores,
			(chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
			(chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
			(chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

	major_rev = chip_info.revision / 100;
	minor_rev = chip_info.revision % 100;
	ESP_LOGI(TAG, "silicon revision v%d.%d, ", major_rev, minor_rev);
	if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
		ESP_LOGW(TAG, "Get flash size failed");
		return;
	}

	ESP_LOGI(TAG, "%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
			(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
	ESP_LOGI(TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

	/* Create LED task */

	xTaskCreate(vTaskLed, "LED", STACK_SIZE, NULL, tskIDLE_PRIORITY, &xLedHandle);
	if (!xLedHandle) {
		ESP_LOGE(TAG, "Failed to create LED task");
	}

	/* Main blink cycle */
	while (1) {
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		ESP_LOGI(TAG, "....");
	}
}
