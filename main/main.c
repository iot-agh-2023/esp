#define BLE_CLIENT 2
#define PROGRAM_NORMAL 3
#define MQTT_CLIENT 4
#define PROGRAM_BUZZER 5
#define PROGRAM PROGRAM_NORMAL

#if PROGRAM == BLE_CLIENT
    #include "gattc_client.c"
#elif PROGRAM == MQTT_CLIENT
    #include "mqtt_main.c"
#elif PROGRAM == PROGRAM_BUZZER
    #include "buzzer_main.c"
#else
// custom logic
#include "oled.h"
#include "network.h"
#include "button.h"
#include "dht11.h"
#include "gatts_server.h"
#include "mqtt.h"
#include "gpio_task.h"
// esp libs
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "mdns.h"
// std libs
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char const *TAG = "main";

void init();
void wifi_sanity_check();

void app_main()
{
    init();
    display_text(1, "Hello");
    wifi_sanity_check();

    int prevWifi = true;
    http_rq_rs http_ping = {
        .url = "init",
        .state = IDLE
    };
    http_ping.url = "http://192.168.137.1:3000/ping";
    xTaskCreate((void (*)(void*))task_http_get, "Trying tasks", 4096, &http_ping, 10, NULL);

    while(1) {
        if (http_ping.state == FINISHED) {
            display_text(2, http_ping.rs_data);
            free(http_ping.rs_data);
            http_ping.state = IDLE;
        }

        if (button_just_pressed()) {
            ESP_LOGI(TAG, "Button pressed");
            xTaskCreate(gpio_task, "gpio_task", 4096, NULL, 5, NULL);
            if (is_wifi_connected() && http_ping.state == IDLE) {
                // http_ping.url = "http://iot-server.glitch.me/ping";
                http_ping.url = "http://192.168.137.1:3000/ping";
                xTaskCreate((void (*)(void*))task_http_get, "Trying tasks", 4096, &http_ping, 10, NULL);
            }
        }

        if (!is_wifi_connected() && prevWifi) {
            display_text(0, "DISCONNECTED");
        }
        else if (is_wifi_connected() && !prevWifi) {
            display_text(0, "WIFI: " CONFIG_ESP_WIFI_SSID);
        }
        prevWifi = is_wifi_connected();

        vTaskDelay(pdMS_TO_TICKS(25));
    }
}

void init()
{
    // Non Volatile Storage => storage that persists when power goes off
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

    DHT11_init(GPIO_NUM_5);

    init_ble();
    init_display();
    init_button();

    display_text(0, "Connecting...");
	if (wifi_init_sta() != ESP_OK) {
		ESP_LOGE(TAG, "Connection failed");
        display_text(0, "Connection failed");
		while(1) { vTaskDelay(1000); }
	}
    display_text(0, "WIFI: " CONFIG_ESP_WIFI_SSID);

    mqtt_app_start();
}

void wifi_sanity_check() {
    const char* SANITY_URL = "http://example.com";
    ESP_LOGI(TAG, "Performing WIFI sanity check with %s", SANITY_URL);
    char* response = http_get(SANITY_URL);
    ESP_LOGI(TAG, "Received response: %s", response);
    display_text(1, "Response:");
    display_text(2, response);
    free(response);
}

#endif