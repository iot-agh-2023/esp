#include "oled.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// #include "ssd1306.h"
#include "font8x8_basic.h"
#include "driver/i2c.h"

#define WIDTH 128
#define HEIGHT 64
#define I2C_NUM 0
#define OLED_CONTROL_BYTE_CMD_STREAM  0x00
#define OLED_CONTROL_BYTE_DATA_STREAM 0x40

static char const *tag = "oled";

static SSD1306_t dev;

void print(SSD1306_t *dev, int page, char const* text, int text_len);
void i2c_display_image(SSD1306_t * dev, int page, int seg, uint8_t * images, int width);

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

void print(SSD1306_t *dev, int page, char const* text, int text_len) {
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

void i2c_display_image(SSD1306_t * dev, int page, int seg, uint8_t * images, int width) {
	i2c_cmd_handle_t cmd;

	if (page >= dev->_pages) return;
	if (seg >= dev->_width) return;

	int _seg = seg + CONFIG_OFFSETX;
	uint8_t columLow = _seg & 0x0F;
	uint8_t columHigh = (_seg >> 4) & 0x0F;

	int _page = page;
	if (dev->_flip) {
		_page = (dev->_pages - page) - 1;
	}

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	// Lower Column Start Address
	i2c_master_write_byte(cmd, (0x00 + columLow), true);
	// Set Higher Column Start Address
	i2c_master_write_byte(cmd, (0x10 + columHigh), true);
	// Set Page Start Address
	i2c_master_write_byte(cmd, 0xB0 | _page, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, images, width, true);

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}

void clear_line(SSD1306_t * dev, int page)
{
	char space[16];
	memset(space, 0x00, sizeof(space));
	print(dev, page, space, sizeof(space));
}

void display_text(int line, char const* text) {
    clear_line(&dev, line);
    print(&dev, line, text, strlen(text));
}
