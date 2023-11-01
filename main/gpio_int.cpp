// https://zhuanlan.zhihu.com/p/654350516
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "main";
#define GPIO_PIN            GPIO_NUM_2 // 选择要使用的 GPIO 引脚

// 防抖动延迟时间（以毫秒为单位）
#define DEBOUNCE_DELAY_MS   50

static volatile bool gpio_int_triggered = false;
QueueHandle_t xQueue;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
	gpio_int_triggered = true;
	portYIELD_FROM_ISR(); // 唤醒任务以处理中断
}

static void gpio_task(void *arg) {
	uint32_t ioNum;
	while (1) {
		if (gpio_int_triggered) {
			int value = gpio_get_level(GPIO_PIN);
			vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)); // 防抖动延迟
			if (gpio_get_level(GPIO_PIN) == value) {
				ioNum = (uint32_t)GPIO_PIN;
				gpio_int_triggered = false;
				int gpio_value = gpio_get_level(ioNum);
				ESP_LOGI(TAG, "GPIO[%lu] 触发中断，电平：%d", ioNum, gpio_value);
			}
		}
		vTaskDelay(10 / portTICK_PERIOD_MS); // 短暂延迟以降低 CPU 负载
	}
}

void app_main(void) {
	// 配置 GPIO 引脚
	gpio_set_direction(GPIO_PIN, GPIO_MODE_INPUT);
	// 任何边沿触发，电平变化时都触发中断
	gpio_set_intr_type(GPIO_PIN, GPIO_INTR_ANYEDGE);

	// 创建中断句柄
	gpio_install_isr_service(0);

	// 注册中断处理函数
	gpio_isr_handler_add(GPIO_PIN, gpio_isr_handler, (void*)GPIO_PIN);

	// 创建任务来处理 GPIO 事件
	xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);

	// 主任务继续执行其他操作
	while (1) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}