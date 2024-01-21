#include "network.h"

#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mdns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static char const *TAG = "net";

static int s_retry_num = 0;
static bool connected = false;

bool is_wifi_connected(void) {
    return connected;
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		connected = false;
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        connected = true;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

esp_err_t read_str_from_nvs(const char *key, char *value, size_t max_length)
{
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("iot", NVS_READONLY, &nvs);
    if (ret != ESP_OK)
    {
        printf("Failed to open NVS\n");
        return ret;
    }

    size_t required_size;
    ret = nvs_get_str(nvs, key, NULL, &required_size);
    if (ret == ESP_OK)
    {
        if (required_size <= max_length)
        {
            ret = nvs_get_str(nvs, key, value, &required_size);
        }
        else
        {
            ret = ESP_ERR_NVS_INVALID_LENGTH;
        }
    }
    else
    {
        printf("Failed to get NVS\n");
    }

    nvs_close(nvs);
    return ret;
}

esp_err_t wifi_init_sta()
{
	static int initialized = false;
    if (initialized) {
        ESP_LOGE(TAG, "wifi_init_sta was already called");
        while(1) {
            vTaskDelay(1000);
        };
    }
    initialized = true;

    esp_err_t ret_value = ESP_OK;
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // ESP_ERROR_CHECK(read_str_from_nvs("wifi_ssid", ssid, 100));
    // ESP_ERROR_CHECK(read_str_from_nvs("wifi_pass", pass, 100));
    // ESP_LOGI(GATTS_TABLE_TAG, "ssid: %s, pass: %s", ssid, pass);

    char ssid[100] = "";
    ESP_ERROR_CHECK(read_str_from_nvs("wifi_ssid", ssid, 100));

    char pass[100] = "";
    ESP_ERROR_CHECK(read_str_from_nvs("wifi_pass", pass, 100));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = "",
			.password = ""
		},
	};
    // printf("|%s|: |%s|\n", ssid, pass);
    // printf("|%s|: |%s|\n", wifi_config.sta.ssid, wifi_config.sta.password);
    memcpy(wifi_config.sta.ssid, ssid, 100);
    memcpy(wifi_config.sta.password, pass, 100);
    // printf("|%s|: |%s|\n", ssid, pass);
    // printf("|%s|: |%s|\n", wifi_config.sta.ssid, wifi_config.sta.password);

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,			// xClearOnExit
		pdFALSE,			// xWaitForAllBits
		portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
			 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGE(TAG, "Failed to connect to SSID:%s, password:%s",
			 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
		ret_value = ESP_FAIL;
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
		ret_value = ESP_ERR_INVALID_STATE;
	}

	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s", CONFIG_ESP_WIFI_SSID);
	// vEventGroupDelete(s_wifi_event_group);
	return ret_value;
}

typedef struct {
    char* buffer;
    int length;
} http_response_t;

static http_response_t http_response;

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER");
            printf("HEADER: %.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            // NOTE jesli cos nie dziala z http, moze trzeba bedzie doimplementowac obsluge chunkow
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Allocate or reallocate memory for the buffer depending on if it has already been used
                http_response.buffer = realloc(http_response.buffer, http_response.length + evt->data_len);
                if (http_response.buffer == NULL) {
                    ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                    return ESP_FAIL;
                }

                // Append the received data to the buffer
                memcpy(http_response.buffer + http_response.length, evt->data, evt->data_len);
                http_response.length += evt->data_len;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        default:
            ESP_LOGW(TAG, "another event. event_id: %i", evt->event_id);
            break;
    }
    return ESP_OK;
}

char* http_get(char const* url)
{
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    http_response.buffer = NULL;
    http_response.length = 0;

    ESP_LOGI(TAG, "Perform");
    // Performing the request
    esp_err_t err = esp_http_client_perform(client);
    ESP_LOGI(TAG, "Performed");

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);

    if (http_response.buffer) {
        http_response.buffer = realloc(http_response.buffer, http_response.length + 1);
        if (http_response.buffer) {
            http_response.buffer[http_response.length] = '\0'; // Null-terminate the buffer
        }
    }

    return http_response.buffer;
}

void task_http_get(http_rq_rs* arg)
{
    arg->state = STARTED;
    arg->rs_data = http_get(arg->url);
    arg->state = FINISHED;
    ESP_LOGI(TAG, "Received response: %s", arg->rs_data);
    vTaskDelete(NULL);
}