#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ssd1306.h"
#include "font8x8_basic.h"

static char const *tag = "oled";

static SSD1306_t dev;

void init_display(void) {
	ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

    #if CONFIG_FLIP
	dev._flip = true;
	ESP_LOGI(tag, "Display flipped upside down");
    #endif

	ssd1306_init(&dev, 128, 64); // Resolution
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff); // Sets brightness
}

void display_text(int line, char const* text) {
    ssd1306_clear_line(&dev, line, false);
    ssd1306_display_text(&dev, line, text, strlen(text), false);
    // ssd1306_display_text_x3 robi wieksza czcionke (na kilka linii)
}

void dotstuff(void)
{
    init_display();
    // ssd1306_display_text_x3(&dev, 0, "Hello", 5, false);
    // vTaskDelay(3000 / portTICK_PERIOD_MS);
	char lineChar[20];
	int center, top, bottom;

	top = 2;
	center = 3;
	bottom = 8;
	// ssd1306_display_text(&dev, 0, "SSD1306 128x64", 14, false);
	// ssd1306_display_text(&dev, 1, "ABCDEFGHIJKLMNOP", 16, false);
	// ssd1306_display_text(&dev, 2, "abcdefghijklmnop",16, false);
	// ssd1306_display_text(&dev, 3, "Hello World!!", 13, false);
	// //ssd1306_clear_line(&dev, 4, true);
	// //ssd1306_clear_line(&dev, 5, true);
	// //ssd1306_clear_line(&dev, 6, true);
	// //ssd1306_clear_line(&dev, 7, true);
	// ssd1306_display_text(&dev, 4, "SSD1306 128x64", 14, true);
	// ssd1306_display_text(&dev, 5, "ABCDEFGHIJKLMNOP", 16, true);
	// ssd1306_display_text(&dev, 6, "abcdefghijklmnop",16, true);
	// ssd1306_display_text(&dev, 7, "Hello World!!", 13, true);

	// vTaskDelay(3000 / portTICK_PERIOD_MS);

	// Display Count Down
	uint8_t image[24];
	memset(image, 0, sizeof(image));
	ssd1306_display_image(&dev, top, (6*8-1), image, sizeof(image));
	ssd1306_display_image(&dev, top+1, (6*8-1), image, sizeof(image));
	ssd1306_display_image(&dev, top+2, (6*8-1), image, sizeof(image));

	for (int font = '9'; font > '0'; font--) {
		// memset(image, 0, sizeof(image));
		// ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
		memcpy(image, font8x8_basic_tr[font], 8);
		if (dev._flip) ssd1306_flip(image, 8);
		ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	// // Scroll Up
	// ssd1306_clear_screen(&dev, false);
	// ssd1306_contrast(&dev, 0xff);
	// ssd1306_display_text(&dev, 0, "---Scroll  UP---", 16, true);
	// ssd1306_software_scroll(&dev, (dev._pages - 1), 1);
	// for (int line=0;line<bottom+10;line++) {
	// 	lineChar[0] = 0x01;
	// 	sprintf(&lineChar[1], " Line %02d", line);
	// 	ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
	// 	vTaskDelay(500 / portTICK_PERIOD_MS);
	// }
	// vTaskDelay(3000 / portTICK_PERIOD_MS);

	// // Scroll Down
	// ssd1306_clear_screen(&dev, false);
	// ssd1306_contrast(&dev, 0xff);
	// ssd1306_display_text(&dev, 0, "--Scroll  DOWN--", 16, true);
	// ssd1306_software_scroll(&dev, 1, (dev._pages - 1) );
	// for (int line=0;line<bottom+10;line++) {
	// 	lineChar[0] = 0x02;
	// 	sprintf(&lineChar[1], " Line %02d", line);
	// 	ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
	// 	vTaskDelay(500 / portTICK_PERIOD_MS);
	// }
	// vTaskDelay(3000 / portTICK_PERIOD_MS);

	// // Page Down
	// ssd1306_clear_screen(&dev, false);
	// ssd1306_contrast(&dev, 0xff);
	// ssd1306_display_text(&dev, 0, "---Page	DOWN---", 16, true);
	// ssd1306_software_scroll(&dev, 1, (dev._pages-1) );
	// for (int line=0;line<bottom+10;line++) {
	// 	if ( (line % (dev._pages-1)) == 0) ssd1306_scroll_clear(&dev);
	// 	lineChar[0] = 0x02;
	// 	sprintf(&lineChar[1], " Line %02d", line);
	// 	ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
	// 	vTaskDelay(500 / portTICK_PERIOD_MS);
	// }
	// vTaskDelay(3000 / portTICK_PERIOD_MS);

	// // Horizontal Scroll
	// ssd1306_clear_screen(&dev, false);
	// ssd1306_contrast(&dev, 0xff);
	// ssd1306_display_text(&dev, center, "Horizontal", 10, false);
	// ssd1306_hardware_scroll(&dev, SCROLL_RIGHT);
	// vTaskDelay(5000 / portTICK_PERIOD_MS);
	// ssd1306_hardware_scroll(&dev, SCROLL_LEFT);
	// vTaskDelay(5000 / portTICK_PERIOD_MS);
	// ssd1306_hardware_scroll(&dev, SCROLL_STOP);

	// // Vertical Scroll
	// ssd1306_clear_screen(&dev, false);
	// ssd1306_contrast(&dev, 0xff);
	// ssd1306_display_text(&dev, center, "Vertical", 8, false);
	// ssd1306_hardware_scroll(&dev, SCROLL_DOWN);
	// vTaskDelay(5000 / portTICK_PERIOD_MS);
	// ssd1306_hardware_scroll(&dev, SCROLL_UP);
	// vTaskDelay(5000 / portTICK_PERIOD_MS);
	// ssd1306_hardware_scroll(&dev, SCROLL_STOP);

	// // Invert
	// ssd1306_clear_screen(&dev, true);
	// ssd1306_contrast(&dev, 0xff);
	// ssd1306_display_text(&dev, center, "  Good Bye!!", 12, true);
	// vTaskDelay(5000 / portTICK_PERIOD_MS);

	// // Fade Out
	// ssd1306_fadeout(&dev);

	// Restart module
	esp_restart();
}
