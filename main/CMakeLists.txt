set(COMPONENT_SRCS main.c network.c oled.c button.c gattc_client.c gatts_server.c ble.c dht11.c mqtt.c gpio_task.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
