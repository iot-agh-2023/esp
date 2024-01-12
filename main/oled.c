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

void print(SSD1306_t *dev, int page, char* text, int text_len);

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
		print(&dev, page, blank, sizeof(blank));
	}

    i2c_contrast(&dev, 0xff);
}

void print(SSD1306_t *dev, int page, char* text, int text_len) {
    if (page >= dev->_pages) return;
	int _text_len = text_len;
	if (_text_len > 16) _text_len = 16;

	uint8_t seg = 0;
	uint8_t image[8];
	for (uint8_t i = 0; i < _text_len; i++) {
		memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);

		i2c_display_image(dev, page, seg, image, 8);
        memcpy(&dev->_page[page]._segs[seg], image, 8);

		seg = seg + 8;
	}
}

void display_text(int line, char const* text) {
    // char space[16];
	// memset(space, 0x00, sizeof(space));
	// print(dev, line, space, sizeof(space), false);
    ssd1306_clear_line(&dev, line, false);
    ssd1306_display_text(&dev, line, text, strlen(text), false);
}
