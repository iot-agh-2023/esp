#include "button.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <stdio.h>

void init_button(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<CONFIG_BUTTON_PIN), // Specify the pin bit mask using the GPIO number
        .mode = GPIO_MODE_INPUT,                   // Set as Input mode
        .pull_up_en = GPIO_PULLUP_ENABLE,          // Enable the internal pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE,     // Disable the internal pull-down resistor
        .intr_type = GPIO_INTR_DISABLE             // Disable GPIO interrupts
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE("gpio", "GPIO [%d] configuration failed with error %d", CONFIG_BUTTON_PIN, ret);
    } else {
        ESP_LOGI("gpio", "GPIO [%d] configured as input with pull-up", CONFIG_BUTTON_PIN);
    }
}

bool button_down(void)
{
    int level = !gpio_get_level(CONFIG_BUTTON_PIN); // negating because it returned 1 for unpressed button
    // ESP_LOGI("gpio", "GPIO [%d] level: %d", CONFIG_BUTTON_PIN, level);
    return level;
}

static bool prev_pressed = false;
bool button_just_pressed(void)
{
    bool current = button_down();
    bool just_pressed = current && !prev_pressed;
    prev_pressed = current;
    return just_pressed;
}
