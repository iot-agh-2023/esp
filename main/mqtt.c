#include "mqtt.h"

#include "dht11.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "mdns.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char const *TAG = "mqtt";
static bool is_mqtt_stopped = true;
static esp_mqtt_client_handle_t mqtt_client = NULL;

bool get_is_mqtt_stopped()
{
    return is_mqtt_stopped;
}

esp_mqtt_client_handle_t get_mqtt_client()
{
    return mqtt_client;
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGI(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        is_mqtt_stopped = false;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        while(true) {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            //2 to zahardcodowane id czujnika - Pałka chciał żeby tu było ESP32_%CHIPID%
            //Sending info -> np. "26.7 19.01.2024 16:25" ale inny format daty też może być albo bez daty
            if(!is_mqtt_stopped) {
                struct dht11_reading reading = DHT11_read();
                if (reading.status < 0) {
                    // -1 = timeout error
                    // -2 = CRC error
                    ESP_LOGI(TAG, "Faulty reading. Code=%d", reading.status);
                }
                else {
                    printf("Temperature is %d \n", reading.temperature);
                    printf("Humidity is %d\n", reading.humidity);
                    char temperature[4];
                    char humidity[4];
                    sprintf(temperature, "%d", reading.temperature);
                    sprintf(humidity, "%d", reading.humidity);

                    msg_id = esp_mqtt_client_publish(client, "sensor/temperature/2", temperature, 0, 0, 0);
                    ESP_LOGI(TAG, "sent publish after subscribing successful, msg_id=%d", msg_id);
                    msg_id = esp_mqtt_client_publish(client, "sensor/humidity/2", humidity, 0, 0, 0);
                    ESP_LOGI(TAG, "sent publish after subscribing successful, msg_id=%d", msg_id);
                }
            } else {
                break;
            }
        }

        break;

    case MQTT_EVENT_DISCONNECTED:
        is_mqtt_stopped = true;
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);

        msg_id = esp_mqtt_client_publish(client, "sensor/2", "Message after subscribing", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish after subscribing successful, msg_id=%d", msg_id);

        // Delay for 10sec (adjust the duration as needed)
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        msg_id = esp_mqtt_client_unsubscribe(client, "sensor/2");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    static const char *mqtt_username = "sensor1";
    static const char *mqtt_password = "Haslo12#";

    ESP_LOGI(TAG, "STARTING MQTT");
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.137.1",
        .credentials.username = mqtt_username,
        .credentials.authentication.password = mqtt_password
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt_client);
    esp_mqtt_client_start(mqtt_client);
}