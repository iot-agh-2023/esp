/* Copyright (c) 2017 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "esp_system.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "sdkconfig.h"

#include "gpio_task.h"

#define GPIO_INPUT     0
#define GPIO_OUTPUT    18
#define GPIO_OUTPUT_SPEED LEDC_HIGH_SPEED_MODE

#define TAG "BUZZER"

EventGroupHandle_t alarm_eventgroup;

const int GPIO_SENSE_BIT = BIT0;

void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    BaseType_t xHigherPriorityTaskWoken;
    if (gpio_num==GPIO_INPUT) {
    	xEventGroupSetBitsFromISR(alarm_eventgroup, GPIO_SENSE_BIT, &xHigherPriorityTaskWoken);
    }
}

void sound(int gpio_num,uint32_t freq,uint32_t duration) {
    // printf("bruh");
	ledc_timer_config_t timer_conf;
	timer_conf.speed_mode = GPIO_OUTPUT_SPEED;
	timer_conf.timer_num  = LEDC_TIMER_0;
	timer_conf.freq_hz    = freq;
    timer_conf.duty_resolution = 10; // 10 - gra, 20 - nie gra
	ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf;
	ledc_conf.gpio_num   = gpio_num;
	ledc_conf.speed_mode = GPIO_OUTPUT_SPEED;
	ledc_conf.channel    = LEDC_CHANNEL_0;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.timer_sel  = LEDC_TIMER_0;
	ledc_conf.duty       = 0x3FFF; // 50%=0x3FFF, 100%=0x7FFF for 15 Bit
	                            // 50%=0x01FF, 100%=0x03FF for 10 Bit
	ledc_channel_config(&ledc_conf);

	// start
    ledc_set_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0, 0xF);
    ledc_update_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0);
	vTaskDelay(duration/portTICK_PERIOD_MS);
	// stop
    ledc_set_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0, 0);
    ledc_update_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0);
}

void gpio_task(void *pvParameters) {
	EventBits_t bits;

	uint32_t loop=0;

	alarm_eventgroup = xEventGroupCreate();

	ESP_LOGI(TAG, "Starting");
    /*
    10, 100, 300, 2000 - buczy tak samo
    */
    sound(GPIO_OUTPUT, 10, 200);
    vTaskDelay(200/portTICK_PERIOD_MS);
    sound(GPIO_OUTPUT, 10, 200);

	while (1) {
        vTaskDelay(100000/portTICK_PERIOD_MS);
		loop++;
	}

	ESP_LOGI(TAG, "All done!");

	vTaskDelete(NULL);
}
