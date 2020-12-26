/**
 * Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @brief Blinky Sample Application main file.
 *
 * This file contains the source code for a sample server application using the LED Button service.
 */

#include "app_button.h"
#include "app_error.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "app_util_platform.h"

#include "boards.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_serial.h"
#include <stdint.h>
#include <string.h>

#include "em3000h.h"

#define ADVERTISING_LED LED_1 /**< Is on when device is advertising. */
#define CONNECTED_LED   LED_2   /**< Is on when device has connected. */
#define LEDBUTTON_LED   LED_3   /**< LED to be toggled with the help of the LED Button Service. */
#define FEEDBACK_LED    LED_4   /**< LED indicates a successfull scanning. */

#define UART_PIN_DISCONNECTED 0xFFFFFFFF

// define pins to barcode module
#define BCM_TRIGGER 47                 /** P1.14 on board, Pin #12 at barcode scanner module (Driving this pin low causes the scan engine to start a scan and decode session).*/
#define BCM_WAKEUP  46                 /** P1.15 on board, Pin #11 at barcode scanner module (When the scan engine is in low power mode, pulsing this pin low for 200 nsec awakens the scan engine). */
#define BCM_LED     45                 /** P1.13 on board, Pin #10 at barcode scanner module. */
#define BCM_BUZZER  44                 /** P1.12 on board, Pin #09 at barcode scanner module. */
#define BCM_TX TX_PIN_NUMBER           /** RX-Pin #4 at barcode scanner module. */
#define BCM_RX RX_PIN_NUMBER           /** TX-Pin #5 at barcode scanner module. */
#define CTS_PIN UART_PIN_DISCONNECTED  /** Not connected. */
#define RTS_PIN UART_PIN_DISCONNECTED  /** Not connected. */

#define BUTTON_DETECTION_DELAY APP_TIMER_TICKS(50) /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define DEAD_BEEF 0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define SCHED_MAX_EVENT_DATA_SIZE       16                                                    /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                196                                                   /**< Maximum number of events in the scheduler queue. */

#define APP_ENABLE_LOGS                 1                                                     /**< Enable logs in the application. */

#if (APP_ENABLE_LOGS == 1)
#define APPL_LOG  NRF_LOG_INFO
#define APPL_DUMP NRF_LOG_RAW_HEXDUMP_INFO
#define APPL_ADDR IPV6_ADDRESS_LOG
#else // APP_ENABLE_LOGS
#define APPL_LOG(...)
#define APPL_DUMP(...)
#define APPL_ADDR(...)
#endif // APP_ENABLE_LOGS

char scan_engine_inbound_message[30];
char scan_engine_inbound_message_temp[30];
bool is_receiving_data_from_scan_engine = false;

#define SERIAL_MAX_WRITE_TIMEOUT NRF_SERIAL_MAX_TIMEOUT


NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uart0_drv_config,
    BCM_RX, BCM_TX, RTS_PIN, CTS_PIN,
    NRF_UART_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED,
    NRF_UART_BAUDRATE_9600,
    UART_DEFAULT_CONFIG_IRQ_PRIORITY);

#define SERIAL_FIFO_TX_SIZE 32
#define SERIAL_FIFO_RX_SIZE 32
NRF_SERIAL_QUEUES_DEF(serial_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);

#define SERIAL_BUFF_TX_SIZE 1
#define SERIAL_BUFF_RX_SIZE 1
NRF_SERIAL_BUFFERS_DEF(serial_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

NRF_SERIAL_UART_DEF(serial_uart, 0);

/**@brief Function for initializing low-frequency clock.
 */
static void lfclk_config(void) {
  ret_code_t err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);

  nrf_drv_clock_lfclk_request(NULL);
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name) {
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void) {
  if (NRF_LOG_PROCESS() == false) {
    //nrf_pwr_mgmt_run();
    app_sched_execute();

    // Wait for an event.
    __WFE();
    // Clear the event register.
    __SEV();
    __WFE();
  }
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void) {
  bsp_board_init(BSP_INIT_LEDS);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void) {
  // Initialize timer module, making it use the scheduler
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

bool em2000h_send_command(const char *command, size_t size, bool wait_for_ack) {

  // clear scan engine inbound messag
  memset(scan_engine_inbound_message, 0, sizeof(scan_engine_inbound_message));

  /*
  char c;
  ret_code_t ret_code = nrf_serial_read(&serial_uart, &c, sizeof(c), NULL, 0);
  APP_ERROR_CHECK(ret_code);
  NRF_LOG_INFO("Received: %d (0%x)", c);
  strcat(scan_engine_inbound_message, &c);


  while (is_receiving_data_from_scan_engine == true) {
    NRF_LOG_INFO("em2000h_send_command(): waiting for is_receiving_data_from_scan_engine is becoming false...");
    app_sched_execute();
  }
  */
  
  ret_code_t ret = nrf_serial_write(&serial_uart,
                                    command,
                                    size,
                                    NULL,
                                    SERIAL_MAX_WRITE_TIMEOUT);
  APP_ERROR_CHECK(ret);
  
  if (wait_for_ack) {
    while (scan_engine_inbound_message == 0) {
      NRF_LOG_INFO("Waiting for scan engine ACK message...");
      // wait for scan engine inbound message
      app_sched_execute();
      // Wait for an event.
      __WFE();
      // Clear the event register.
      __SEV();
      __WFE();
    }

    NRF_LOG_INFO("Received scan engine inbound message: %X", scan_engine_inbound_message);

    return false;

  } else {

    nrf_delay_ms(50);
    
    return true;

  }

  return false;

  /*
  if ack message erwartet:
  while (millis() < timeout)
  {
    if (_serial->available())
    {
      while (_serial->available())
      {
        byte incoming = _serial->read();
        if (incoming == DE2120_COMMAND_ACK)
          return (true);
        else if (incoming == DE2120_COMMAND_NACK)
          return (false);
      }
    }
    delay(1);
  }
  */

  //nrf_serial_flush(&serial_uart, 0);
}


static void barcode_module_init() {
  NRF_LOG_INFO("barcode_module_init()");

  nrf_gpio_cfg_output(BCM_TRIGGER);
  nrf_gpio_pin_write(BCM_TRIGGER, 1);

  nrf_gpio_cfg_output(BCM_WAKEUP);
  nrf_gpio_pin_write(BCM_WAKEUP, 1);

  nrf_gpio_cfg_input(BCM_LED, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(BCM_BUZZER, NRF_GPIO_PIN_NOPULL);
  

  bool send_ret = false;

  NRF_LOG_INFO("Send command CMD_PARAM_SET_SOFTWARE_HANDSHAKING_ENABLE");
  send_ret = em2000h_send_command(CMD_PARAM_SET_SOFTWARE_HANDSHAKING_ENABLE, 9, false);
  if (!send_ret) {
    NRF_LOG_ERROR("Sending command CMD_PARAM_SET_SOFTWARE_HANDSHAKING_ENABLE failed.");
  }
  //nrf_delay_ms(50);
  NRF_LOG_INFO("Send command CMD_PARAM_SET_POWER_MODE_LOW");
  send_ret = em2000h_send_command(CMD_PARAM_SET_POWER_MODE_LOW, 9, false);
  if (!send_ret) {
    NRF_LOG_ERROR("Sending command CMD_PARAM_SET_POWER_MODE_LOW failed.");
  }
  //nrf_delay_ms(50);
  NRF_LOG_INFO("Send command CMD_PARAM_SET_TRIGGER_MODE_HOST");
  send_ret = em2000h_send_command(CMD_PARAM_SET_TRIGGER_MODE_HOST, 9, false);
  if (!send_ret) {
    NRF_LOG_ERROR("Sending command CMD_PARAM_SET_TRIGGER_MODE_HOST failed.");
  }
  //nrf_delay_ms(50);
  NRF_LOG_INFO("Send command CMD_SCAN_ENABLE");
  send_ret = em2000h_send_command(CMD_SCAN_ENABLE, 6, false);
  if (!send_ret) {
    NRF_LOG_ERROR("Sending command CMD_SCAN_ENABLE failed.");
  }
}


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action) {
  //NRF_LOG_INFO("button_event_handler()");
  ret_code_t err_code;

  switch (pin_no) {
  case BUTTON_1:
    if (button_action == APP_BUTTON_PUSH) {
      //NRF_LOG_INFO("Button_1 push");
      em2000h_send_command(CMD_WAKEUP, 1, false);
      em2000h_send_command(CMD_START_DECODE, 6, false);
    } else if (button_action == APP_BUTTON_RELEASE) {
      //NRF_LOG_INFO("Button_1 released");
      em2000h_send_command(CMD_STOP_DECODE, 6, false);
      //em2000h_send_command(CMD_SLEEP, 6);
    }
    /*
      NRF_LOG_INFO("Send button state change.");
      err_code = ble_lbs_on_button_change(m_conn_handle, &m_lbs, button_action);
      if (err_code != NRF_SUCCESS &&
          err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
          err_code != NRF_ERROR_INVALID_STATE &&
          err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
      {
          APP_ERROR_CHECK(err_code);
      }
    */
    break;
  case BUTTON_2:
    if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button_2 released");
      barcode_module_init();
    }
    break;
  case BUTTON_3:
    if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button_3 released");
    }
    break;
  case BUTTON_4:
    if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button_4 released");
      // reset scan engine
      nrf_gpio_pin_write(BCM_WAKEUP, 1);
      nrf_delay_ms(1);
      nrf_gpio_pin_write(BCM_WAKEUP, 0);
      nrf_delay_ms(200);
      nrf_gpio_pin_write(BCM_WAKEUP, 1);
      //em2000h_send_command(CMD_RESET, 6);
    }
    break;
  default:
    //APP_ERROR_HANDLER(pin_no);
    NRF_LOG_INFO("Unsupported button: pin=%d", pin_no);
    break;
  }
}

/**@brief Function for initializing the button handler module.
 */
static void buttons_init() {
  NRF_LOG_INFO("buttons_init()");

  uint32_t err_code;

  //The array must be static because a pointer to it will be saved in the button handler module.
  static app_button_cfg_t buttons[] =
      {
          {BUTTON_1, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_event_handler}
         ,{BUTTON_2, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_event_handler}
         ,{BUTTON_3, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_event_handler}
         ,{BUTTON_4, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_event_handler}
      };

  err_code = app_button_init(buttons, ARRAY_SIZE(buttons), BUTTON_DETECTION_DELAY);
  APP_ERROR_CHECK(err_code);

  err_code = app_button_enable();
  APP_ERROR_CHECK(err_code);
}


static void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing power management.
 */
static void power_management_init(void) {
  ret_code_t err_code;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}

/**
 * Pin change handler
 */
static void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  //NRF_LOG_INFO("in_pin_handler(): pin=%d", pin);
  
  if (pin != BCM_LED) {
    return;
  }

  if (nrf_gpio_pin_read(BCM_LED) > 0) {
    //NRF_LOG_INFO("Feedback-LED ON");
    bsp_board_led_on(bsp_board_pin_to_led_idx(FEEDBACK_LED));
  } else {
    //NRF_LOG_INFO("Feedback-LED OFF");
    bsp_board_led_off(bsp_board_pin_to_led_idx(FEEDBACK_LED));
  }

  //nrf_drv_gpiote_out_toggle(PIN_OUT);
}

/**
 * @brief Function for configuring: Configures GPIOTE to give an interrupt on pin change.
 */
static void gpio_init(void) {
  ret_code_t err_code;

  // Initialization of GPIOTE is allready done by app_button library
  //err_code = nrf_drv_gpiote_init();
  //APP_ERROR_CHECK(err_code);

  /*
    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    err_code = nrf_drv_gpiote_out_init(PIN_OUT, &out_config);
    APP_ERROR_CHECK(err_code);
  */

  nrf_drv_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
  in_config.pull = NRF_GPIO_PIN_PULLUP;

  err_code = nrf_drv_gpiote_in_init(BCM_LED, &in_config, in_pin_handler);
  APP_ERROR_CHECK(err_code);

  nrf_drv_gpiote_in_event_enable(BCM_LED, true);
}



APP_TIMER_DEF(my_serial_receive_timer_id);

static void stop_serial_receive_timer(void) {
  ret_code_t ret_code;
  ret_code = app_timer_stop(my_serial_receive_timer_id);
  APP_ERROR_CHECK(ret_code);
  is_receiving_data_from_scan_engine = false;
}

static void serial_receive_timeout_handler() {
  //NRF_LOG_INFO("serial_receive_timeout_handler()");

  //NRF_LOG_INFO("Finally Received: %X", scan_engine_inbound_message_temp);

  strcpy(scan_engine_inbound_message, scan_engine_inbound_message_temp);

  memset(scan_engine_inbound_message_temp, 0, sizeof(scan_engine_inbound_message_temp));
  
  stop_serial_receive_timer();
}

static void create_serial_receive_timer(void) {
  ret_code_t ret_code;
  ret_code = app_timer_create(&my_serial_receive_timer_id, APP_TIMER_MODE_SINGLE_SHOT, serial_receive_timeout_handler);
  APP_ERROR_CHECK(ret_code);
}

static void start_serial_receive_timer(void) {
  ret_code_t ret_code;
  if (is_receiving_data_from_scan_engine == false) {
    ret_code = app_timer_start(my_serial_receive_timer_id, APP_TIMER_TICKS(50), NULL);
    APP_ERROR_CHECK(ret_code);
    is_receiving_data_from_scan_engine = true;
  }
}

static void serial_sleep_handler(void) {
  //NRF_LOG_INFO("serial_sleep_handler()");
  __WFE();
  __SEV();
  __WFE();
}

/**
 * Serial Empfang via Event Handler ist schlecht dokumentiert. Hier gibt es ein paar interessante Case:
 * https://devzone.nordicsemi.com/f/nordic-q-a/34391/serial-port-library-receive-interrupt
 * https://devzone.nordicsemi.com/f/nordic-q-a/22320/serial-port-interrupt
 */
static void serial_event_handler(struct nrf_serial_s const *p_serial, nrf_serial_event_t event) {
  switch (event) {
  case NRF_SERIAL_EVENT_TX_DONE:
    //NRF_LOG_INFO("serial_event_handler(): NRF_SERIAL_EVENT_TX_DONE");
    break;
  case NRF_SERIAL_EVENT_RX_DATA:
    //NRF_LOG_INFO("serial_event_handler(): NRF_SERIAL_EVENT_RX_DATA");
    start_serial_receive_timer();
    char c;
    ret_code_t ret_code;
    ret_code = nrf_queue_read(p_serial->p_ctx->p_config->p_queues->p_rxq, &c, sizeof(c));
    //ret_code = nrf_serial_read(&serial_uart, &c, sizeof(c), NULL, 0);
    APP_ERROR_CHECK(ret_code);
    //NRF_LOG_INFO("Received: %d (0%x)", c);
    strcat(scan_engine_inbound_message_temp, &c);
    break;
  case NRF_SERIAL_EVENT_DRV_ERR:
    NRF_LOG_INFO("serial_event_handler(): NRF_SERIAL_EVENT_DRV_ERR");
    break;
  case NRF_SERIAL_EVENT_FIFO_ERR:
    NRF_LOG_INFO("serial_event_handler(): NRF_SERIAL_EVENT_FIFO_ERR");
    break;
  default:
    NRF_LOG_INFO("serial_event_handler(): default");
    break;
  }

  /*
  if (event == NRF_SERIAL_EVENT_RX_DATA) {
    char c;
    ret_code_t ret_code;
    ret_code = nrf_serial_read(&serial_uart, &c, sizeof(c), NULL, 500);
    APP_ERROR_CHECK(ret_code);
    NRF_LOG_INFO("serial_event_handler: %c", c);
  }
  */
}

NRF_SERIAL_CONFIG_DEF(serial_config, NRF_SERIAL_MODE_DMA, &serial_queues, &serial_buffs, serial_event_handler, serial_sleep_handler);

/**
 * Function for initializing serial communication.
 */
static void init_serial(void) {
  ret_code_t ret_code;
  ret_code = nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);
  APP_ERROR_CHECK(ret_code);
}

/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

/**@brief Function for calculating the 2's complement checksum for the given content.
 */
uint16_t calc_checksum(const char* str, const uint8_t len) {
  uint16_t checksum, i, sum =  0;

  for (i = 0; i < len; i++) {
    sum += str[i];
  }
  
  NRF_LOG_INFO("calc_checksum: sum is %x", sum);
  
  checksum =~ sum + 1;

  NRF_LOG_INFO("calc_checksum: checksum is %x", checksum);
  
  return checksum;
}

/**@brief Function for application main entry.
 */
int main(void) {
  uint32_t err_code;

  // Initialize.
  lfclk_config();
  log_init();
  leds_init();
  scheduler_init();
  timers_init();
  buttons_init();
  //barcode_module_init()
  gpio_init();
  init_serial();
  create_serial_receive_timer();
  power_management_init();
  
  //calc_checksum(test_cmd, 4);
  
  // Start execution.
  NRF_LOG_INFO("TicTac Barcode Scanner started.");

  // Enter main loop.
  for (;;) {
     idle_state_handle();
    /*
    app_sched_execute();

    // Wait for an event.
    __WFE();
    // Clear the event register.
    __SEV();
    __WFE();
    */
  }
}

/**
 * @}
 */