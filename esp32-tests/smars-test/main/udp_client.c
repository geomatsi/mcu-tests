#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "common.h"

#define HOST_IP_ADDR   "192.168.88.18"
#define HOST_PORT      5555

static const char *TAG = "mod:udp";

SemaphoreHandle_t sem_conn = NULL;

static void connect_handler(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data)
{
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

	ESP_LOGI(TAG, "STA got ipaddr:" IPSTR, IP2STR(&event->ip_info.ip));
	xSemaphoreGive(sem_conn);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
			       int32_t event_id, void *event_data)
{
	if (event_base != WIFI_EVENT) {
		ESP_LOGE(TAG, "%s: unexpected event_base: %s\n", __func__, event_base);
		return;
	}

	switch (event_id) {
	case WIFI_EVENT_STA_DISCONNECTED:
		ESP_LOGW(TAG, "STA disconnected\n");
		xSemaphoreTake(sem_conn, portMAX_DELAY);
		break;
	default:
		ESP_LOGW(TAG, "Unhandled event: %s:%ld\n", event_base, event_id);
		break;
	}
}

void udp_task(void *args)
{
	static const char *payload = "HELLO";
	struct sockaddr_in dest_addr;
	int addr_family;
	int ip_protocol = 0;
	int ret;

	sem_conn = xSemaphoreCreateBinary();

	if (sem_conn == NULL) {
		ESP_LOGE(TAG, "%s: failed to create semaphore\n", __func__);
		goto err;
	}

	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
			IP_EVENT_STA_GOT_IP,
			&connect_handler,
			NULL,
			NULL));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
			WIFI_EVENT_STA_DISCONNECTED,
			&disconnect_handler,
			NULL,
			NULL));

	while (1) {
		dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(HOST_PORT);
		addr_family = AF_INET;
		ip_protocol = IPPROTO_IP;

		int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (sock < 0) {
			ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
			break;
		}

		ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, HOST_PORT);

		while (1) {
			xSemaphoreTake(sem_conn, portMAX_DELAY);

			ret = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
			if (ret < 0) {
				ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
				xSemaphoreGive(sem_conn);
				break;
			}

			ESP_LOGI(TAG, "Message sent");

			xSemaphoreGive(sem_conn);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
		}

		if (sock != -1) {
			ESP_LOGE(TAG, "Shutting down socket and restarting...");
			shutdown(sock, 0);
			close(sock);
		}
	}

err:
	ESP_LOGE(TAG, "Shutdown udp task...");
	vTaskDelete(NULL);
}
