/* pthread/std::thread example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <esp_pthread.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

using namespace std::chrono;

static const char *TAG = "main";
#define ONEWIRE_PIN_0 (9)
#define ONEWIRE_PIN_1 (10)
#define ONEWIRE_PIN_2 (11)
#define ONEWIRE_PIN_3 (12)

volatile int      received_temps[4] = {0};
volatile int      current_level[4];
volatile uint8_t  pattern[4];
volatile uint16_t parsed_temp[4];
volatile int      received_bits[4];

const auto sleep_time = seconds{5};

#define INT_HANDLER(i)                                                         \
	void int_when_FALLING_##i() {                                              \
		usleep(5);                                                             \
		current_level[i] = gpio_get_level((gpio_num_t) ONEWIRE_PIN_##i);       \
		if (pattern[i] != 0xbe) {                                              \
			pattern[i] = (current_level[i] << 7) | (pattern[i] >> 1);          \
		} else {                                                               \
			if (received_bits[i] == 0) {                                       \
				parsed_temp[i] = 0;                                            \
			}                                                                  \
                                                                               \
			parsed_temp[i] = (current_level[i] << 15) | (parsed_temp[i] >> 1); \
			received_bits[i]++;                                                \
			if (received_bits[i] >= 16) {                                      \
				pattern[i]       = 0;                                          \
				received_bits[i] = 0;                                          \
				++received_temps[i];                                           \
			}                                                                  \
		}                                                                      \
	}

INT_HANDLER(0)
INT_HANDLER(1)
INT_HANDLER(2)
INT_HANDLER(3)

void print_thread_info(const char *extra = nullptr) {
	std::stringstream ss;
	if (extra) {
		ss << extra;
	}
	ss << "Core id: " << xPortGetCoreID()
	   << ", prio: " << uxTaskPriorityGet(nullptr)
	   << ", minimum free stack: " << uxTaskGetStackHighWaterMark(nullptr) << " bytes.";
	ESP_LOGI(pcTaskGetName(nullptr), "%s", ss.str().c_str());
}

void thread_func_inherited() {
	while (true) {
		print_thread_info("This is the INHERITING thread with the same parameters as our parent, including name. ");
		std::this_thread::sleep_for(sleep_time);
	}
}

void spawn_another_thread() {
	// Create a new thread, it will inherit our configuration
	std::thread inherits(thread_func_inherited);

	while (true) {
		print_thread_info();
		std::this_thread::sleep_for(sleep_time);
	}
}

void thread_func_any_core() {
	while (true) {
		print_thread_info("This thread (with the default name) may run on any core.");
		std::this_thread::sleep_for(sleep_time);
	}
}

void thread_func() {
	while (true) {
		print_thread_info();
		std::this_thread::sleep_for(sleep_time);
	}
}

esp_pthread_cfg_t create_config(const char *name, int core_id, int stack, int prio) {
	auto cfg        = esp_pthread_get_default_config();
	cfg.thread_name = name;
	cfg.pin_to_core = core_id;
	cfg.stack_size  = stack;
	cfg.prio        = prio;
	return cfg;
}

extern "C" void app_main(void) {
	// Create a thread using default values that can run on any core
	auto cfg = esp_pthread_get_default_config();
	esp_pthread_set_cfg(&cfg);
	std::thread any_core(thread_func_any_core);

	// Create a thread on core 0 that spawns another thread, they will both have the same name etc.
	cfg             = create_config("Thread 1", 0, 3 * 1024, 5);
	cfg.inherit_cfg = true;
	esp_pthread_set_cfg(&cfg);
	std::thread thread_1(spawn_another_thread);

	// Create a thread on core 1.
	cfg = create_config("Thread 2", 1, 3 * 1024, 5);
	esp_pthread_set_cfg(&cfg);
	std::thread thread_2(thread_func);

	// Let the main task do something too
	while (true) {
		std::stringstream ss;
		ss << "core id: " << xPortGetCoreID()
		   << ", prio: " << uxTaskPriorityGet(nullptr)
		   << ", minimum free stack: " << uxTaskGetStackHighWaterMark(nullptr) << " bytes.";
		ESP_LOGI(pcTaskGetName(nullptr), "%s\n", ss.str().c_str());
		std::this_thread::sleep_for(sleep_time);
	}
}
