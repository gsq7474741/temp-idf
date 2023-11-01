#include <esp_log.h>
#include <esp_pthread.h>
#include <stdio.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include "driver/gpio.h"
#include "usbh_modem_board.h"

//int rx_buffer_size;             /*!< USB RX Buffer Size */
//int tx_buffer_size;             /*!< USB TX Buffer Size */
//int line_buffer_size;           /*!< Line buffer size for command mode */
//int event_task_priority;        /*!< USB Event/Data Handler Task Priority*/
//int event_task_stack_size;      /*!< USB Event/Data Handler Task Stack Size*/
//esp_event_handler_t handler;    /*!< Modem event handler */
//void *handler_arg;              /*!< Modem event handler arg */
//int flags;                      /*!< Modem config flag bits */

modem_config_t modemConfig = {
		.rx_buffer_size        = 1024,
		.tx_buffer_size        = 512,
		.line_buffer_size      = 128,
		.event_task_priority   = 10,
		.event_task_stack_size = 2048,
		.handler               = NULL,
		.handler_arg           = NULL,
		.flags                 = 0,
};

extern "C" void app_main(void) {
	modem_board_init(&modemConfig);
	int ifReady = 0;
	modem_board_get_sim_card_state(&ifReady);
	printf("SIM Card ready: %d\n", ifReady);
}