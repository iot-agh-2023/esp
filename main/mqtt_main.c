// #include "mqtt.h"

// static char const *TAG = "main";

// void init();
// void wifi_sanity_check();

// void app_main()
// {
//     init();
//     wifi_sanity_check();

//     int prevWifi = true;
//     http_rq_rs http_ping = {
//         .url = "init",
//         .state = IDLE};


//     while (1)
//     {
//         if (http_ping.state == FINISHED)
//         {
//             display_text(2, http_ping.rs_data);
//             free(http_ping.rs_data);
//             http_ping.state = IDLE;
//         }

//         if (button_just_pressed())
//         {
//             ESP_LOGI(TAG, "Button pressed");
//             if (is_wifi_connected() && http_ping.state == IDLE)
//             {
//                 http_ping.url = "http://iot-server.glitch.me/ping";
//                 xTaskCreate((void (*)(void *))task_http_get, "Trying tasks", 4096, &http_ping, 10, NULL);
//             }
//         }

//         if (!is_wifi_connected() && prevWifi)
//         {
//             is_mqtt_stopped = true;
//             ESP_LOGI(TAG, "Stopping MQTT client");
//             esp_mqtt_client_disconnect(client);
//             esp_mqtt_client_stop(client);

//             display_text(0, "DISCONNECTED");
//         }
//         else if (is_wifi_connected() && !prevWifi)
//         {
//             display_text(0, "WIFI: " CONFIG_ESP_WIFI_SSID);
//             vTaskDelay(5000 / portTICK_PERIOD_MS); //wstrzymanie żeby na pewno flagi się dobrze ustawiły - bez tego jest lipa
//             if(is_mqtt_stopped) {
//                 mqtt_app_start();
//             }
//         }
//         prevWifi = is_wifi_connected();

//         vTaskDelay(pdMS_TO_TICKS(25));
//     }
// }

// void init()
// {
//     // Non Volatile Storage => storage that persists when power goes off
// 	esp_err_t ret = nvs_flash_init();
// 	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
// 		ESP_ERROR_CHECK(nvs_flash_erase());
// 		ret = nvs_flash_init();
// 	}
// 	ESP_ERROR_CHECK(ret);

//     init_display();
//     init_button();

//     display_text(0, "Connecting...");
// 	if (wifi_init_sta() != ESP_OK) {
// 		ESP_LOGE(TAG, "Connection failed");
//         display_text(0, "Connection failed");
// 		while(1) { vTaskDelay(1000); }
// 	}
//     display_text(0, "WIFI: " CONFIG_ESP_WIFI_SSID);
// }

// void wifi_sanity_check() {
//     const char* SANITY_URL = "http://example.com";
//     ESP_LOGI(TAG, "Performing WIFI sanity check with %s", SANITY_URL);
//     char* response = http_get(SANITY_URL);
//     // ESP_LOGI(TAG, "Received response: %s", response);
//     // display_text(1, "Response:");
//     // display_text(2, response);
//     free(response);
// }
