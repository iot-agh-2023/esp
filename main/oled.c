#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ssd1306.h"
#include "font8x8_basic.h"

#define WIDTH 128
#define HEIGHT 64

static char const *tag = "oled";

static SSD1306_t dev;

void init_display(void) {
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

	i2c_init(&dev, WIDTH, HEIGHT);
    for (int i = 0; i < dev._pages;i++) {
		memset(dev._page[i]._segs, 0, 128);
	}

    char blank[16];
	memset(blank, 0x00, sizeof(blank));
	for (int page = 0; page < dev._pages; page++) {
		ssd1306_display_text(&dev, page, blank, sizeof(blank), false);
	}

    i2c_contrast(&dev, 0xff);
}

void display_text(int line, char const* text) {
    ssd1306_clear_line(&dev, line, false);
    ssd1306_display_text(&dev, line, text, strlen(text), false);
}
