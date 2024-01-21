#ifndef MQTT_H_
#define MQTT_H_

#include "mqtt_client.h"

#include <stdlib.h>
#include <stdbool.h>

bool get_is_mqtt_stopped();
esp_mqtt_client_handle_t get_mqtt_client();

void mqtt_app_start(void);

#endif