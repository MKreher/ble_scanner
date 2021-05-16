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
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_serial.h"
#include "nrf_gfx.h"
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "module_ble.h"
#include "waveshare_epd.h"
#include "ImageData.h"
#include "em3000h.h"
#include "NDEF_Wrapper.h"

// Waveshare ePaper
static uint8_t epaper_pending;
static const nrf_lcd_t * p_lcd = &nrf_lcd_wsepd154;
extern const nrf_gfx_font_desc_t orkney_24ptFontInfo;
static const nrf_gfx_font_desc_t * p_font = &orkney_24ptFontInfo;

#define LED_CONNECTION_STATUS 2  // Is on when device is advertising (Blue)
#define LED_FEEDBACK_OK       26 // LED indicates a successfull scanning (Green)
#define LED_FEEDBACK_ERROR    27 // LED indicates an error (Red)
#define BUZZER                42 // Buzzer
#define VIBRATION_MOTOR       43 // Vibration motor

static const uint8_t leds_list[7] = { LED_CONNECTION_STATUS, LED_FEEDBACK_OK, LED_FEEDBACK_ERROR, LED_1, LED_2,LED_3, LED_4 };

#define UART_PIN_DISCONNECTED 0xFFFFFFFF

// define pins to barcode module
#define BCM_TRIGGER 46                    // P1.14 on board, Pin #12 at barcode scanner module
#define BCM_WAKEUP  47                    // P1.15 on board, Pin #11 at barcode scanner module
#define BCM_LED     45                    // P1.13 on board, Pin #10 at barcode scanner module
#define BCM_BUZZER  44                    // P1.12 on board, Pin #09 at barcode scanner module
#define BCM_TX      TX_PIN_NUMBER         // RX-Pin #4 at barcode scanner module
#define BCM_RX      RX_PIN_NUMBER         // TX-Pin #5 at barcode scanner module
#define CTS_PIN     UART_PIN_DISCONNECTED // Not connected
#define RTS_PIN     UART_PIN_DISCONNECTED // Not connected

#define BUTTON_DETECTION_DELAY APP_TIMER_TICKS(50) // Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks)

#define DEAD_BEEF 0xDEADBEEF // Value used as error code on stack dump, can be used to identify stack location on stack unwind

#define SCHED_MAX_EVENT_DATA_SIZE 16 // Maximum size of scheduler events
#define SCHED_QUEUE_SIZE 196         // Maximum number of events in the scheduler queue

#define APP_ENABLE_LOGS 1 // Enable logs in the application

#if (APP_ENABLE_LOGS == 1)
#define APPL_LOG  NRF_LOG_INFO
#define APPL_DUMP NRF_LOG_RAW_HEXDUMP_INFO
#define APPL_ADDR IPV6_ADDRESS_LOG
#else // APP_ENABLE_LOGS
#define APPL_LOG(...)
#define APPL_DUMP(...)
#define APPL_ADDR(...)
#endif // APP_ENABLE_LOGS

#define TICKS_PER_MILLISECOND APP_TIMER_TICKS(1) // Factor for converting ticks to milliseconds

typedef enum
{
    IDLE,
    AWAITING_BARCODE,
    AWAITING_ACK_MESSAGE,
    AWAITING_PARAM_DATA
} scan_engine_communication_state_t;


static scan_engine_communication_state_t m_scan_engine_comm_state = IDLE;

static char m_scan_engine_inbound_barcode[16];
static char m_scan_engine_inbound_message_hex[16];

static bool m_is_receiving_data_from_scan_engine = false;
static bool m_is_new_message_from_scan_engine = false;
static bool m_is_new_barcode_from_scan_engine = false;
static uint16_t m_number_of_bytes_transfered = 0;

static bool m_scan_engine_cancel_operation = false;

#define SERIAL_MAX_WRITE_TIMEOUT NRF_SERIAL_MAX_TIMEOUT

NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uart0_drv_config, BCM_RX, BCM_TX, RTS_PIN, CTS_PIN,
                               NRF_UART_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED, NRF_UART_BAUDRATE_9600,
                               UART_DEFAULT_CONFIG_IRQ_PRIORITY);

#define SERIAL_FIFO_TX_SIZE 32
#define SERIAL_FIFO_RX_SIZE 32
NRF_SERIAL_QUEUES_DEF(m_serial_queues, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);

#define SERIAL_BUFF_TX_SIZE 1
#define SERIAL_BUFF_RX_SIZE 1
NRF_SERIAL_BUFFERS_DEF(m_serial_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

NRF_SERIAL_UART_DEF(m_serial_uart, 0);

APP_TIMER_DEF(m_serial_receive_timer_id);
APP_TIMER_DEF(m_feedback_timer_id);
APP_TIMER_DEF(m_display_timer_id);

// PN532
static const nrf_drv_spi_t spi_pn532 = NRF_DRV_SPI_INSTANCE(PN532_SPI_INSTANCE);
static NfcAdapter * m_nfc;
static bool m_read_nfc = false;

static uint32_t m_display_wait_for_ble_tx_starttime;
static const uint32_t m_ble_tx_transfer_wait_timeout_millis = 5000; // max wait for BLE transmission

static bool is_trigger_button_push_busy = false;
static bool is_trigger_button_release_busy = false;

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    #if NRF_LOG_ENABLED
        error_info_t * err_info = (error_info_t*)info;
        NRF_LOG_ERROR(" id = %d, pc = %d, file = %s, line number: %d, error code = %d = %s \r\n", \
        id, pc, nrf_log_push((char*)err_info->p_file_name), err_info->line_num, err_info->err_code, nrf_log_push((char*)nrf_strerror_find(err_info->err_code)));
    #endif
    
    //(void)m_ui_led_set_event(M_UI_ERROR);
    NRF_LOG_FINAL_FLUSH();
    nrf_delay_ms(5);
    
    // On assert, the system can only recover with a reset.
    #ifndef DEBUG
        NVIC_SystemReset();
    #endif

    app_error_save_and_stop(id, pc, info);
}


/**@brief Function for initializing low-frequency clock.
 */
static void lfclk_init(void)
{
    //NRF_LOG_INFO("lfclk_config()");

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
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    NRF_LOG_INFO("assert_nrf_callback()");

    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    NRF_LOG_INFO("nrf_qwr_error_handler()");

    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/**@brief Function for initializing power management.
 * See https://embeddedcentric.com/lesson-14-nrf5x-power-management-tutorial/
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    //NRF_LOG_INFO("timers_init()");

    // Initialize timer module, making it use the scheduler
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void) {
    //NRF_LOG_INFO("leds_init()");
    
    //bsp_board_init(BSP_INIT_LEDS);
    nrf_gpio_cfg_output(LED_CONNECTION_STATUS);
    nrf_gpio_pin_clear(LED_CONNECTION_STATUS);
    nrf_gpio_cfg_output(LED_FEEDBACK_OK);
    nrf_gpio_pin_clear(LED_FEEDBACK_OK);
    nrf_gpio_cfg_output(LED_FEEDBACK_ERROR);
    nrf_gpio_pin_clear(LED_FEEDBACK_ERROR);

    nrf_gpio_pin_set(LED_CONNECTION_STATUS);
    non_blocking_delay_ms(500);
    nrf_gpio_pin_clear(LED_CONNECTION_STATUS);
    nrf_gpio_pin_set(LED_FEEDBACK_OK);
    non_blocking_delay_ms(500);
    nrf_gpio_pin_clear(LED_FEEDBACK_OK);
    nrf_gpio_pin_set(LED_FEEDBACK_ERROR);
    non_blocking_delay_ms(500);
    nrf_gpio_pin_clear(LED_FEEDBACK_ERROR);
}


/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void buzzer_init(void) {
    NRF_LOG_INFO("buzzer_init()");
    
    nrf_gpio_cfg_output(BUZZER);
    nrf_gpio_pin_clear(BUZZER);
}


/**@brief Function for the vibration motor initialization.
 *
 * @details Initializes the vibration motor.
 */
static void vibrator_init(void) {
    NRF_LOG_INFO("vibrator_init()");
    
    nrf_gpio_cfg_output(VIBRATION_MOTOR);
    nrf_gpio_pin_set(VIBRATION_MOTOR);
}

static void log_execution_mode(const char * context_name)
{
    // Log execution mode.
    if (current_int_priority_get() == APP_IRQ_PRIORITY_THREAD)
    {
        NRF_LOG_INFO("%s: executing in thread/main mode", context_name);
    }
    else
    {
        NRF_LOG_INFO("%s: executing in interrupt handler mode", context_name);
    }
}


static void start_display_timer(void)
{
    NRF_LOG_INFO("start_display_timer()");    

    m_display_wait_for_ble_tx_starttime = app_timer_cnt_get();

    ret_code_t ret_code = app_timer_start(m_display_timer_id, APP_TIMER_TICKS(50), NULL);
    APP_ERROR_CHECK(ret_code);
}

static void stop_display_timer(void)
{
    NRF_LOG_INFO("stop_display_timer");
    ret_code_t err_code;

    err_code = app_timer_stop(m_display_timer_id);
    APP_ERROR_CHECK(err_code);
}

static void start_serial_receive_timer(void)
{
    NRF_LOG_INFO("start_serial_receive_timer()");
    ret_code_t ret_code;
    ret_code = app_timer_start(m_serial_receive_timer_id, APP_TIMER_TICKS(25), NULL);
    APP_ERROR_CHECK(ret_code);
}




static void epaper_demo_clear(void)
{
    NRF_LOG_INFO("Display clear.");
    APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
    nrf_gfx_screen_fill(p_lcd, 0xff);
    nrf_gfx_display(p_lcd);

    //nrf_gfx_rotation_set(p_lcd, NRF_LCD_ROTATE_90);
    //wsepd154_draw_monobmp((const uint8_t *)gImage_1in54);
    //nrf_gfx_display(p_lcd);

    //nrf_gfx_uninit(p_lcd);
}

static void epaper_demo_text(const char * text)
{
    NRF_LOG_INFO("Display text.");
    //APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
    nrf_gfx_rotation_set(p_lcd, NRF_LCD_ROTATE_180);
    nrf_gfx_point_t text_start = NRF_GFX_POINT(15, 70);
    nrf_gfx_screen_fill(p_lcd, 0xff);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &text_start, 0x00, text, p_font, true));
    nrf_gfx_display(p_lcd);
    //nrf_gfx_uninit(p_lcd);
}

/**@brief Draws geometric objects and text on epaper display
 *
 * @details Translation of Waveshare epaper demo code to run on nRF52
 */
static void epaper_demo_draw(void)
{
    NRF_LOG_INFO("Display draw.");
    //APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
    nrf_gfx_rotation_set(p_lcd, NRF_LCD_ROTATE_180);
    nrf_gfx_point_t text_start = NRF_GFX_POINT(15, 70);
    nrf_gfx_screen_fill(p_lcd, 0xff);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &text_start, 0x00, "MY_APP", p_font, true));
    nrf_gfx_circle_t circ_1 = NRF_GFX_CIRCLE(25, 25, 10);
    nrf_gfx_circle_draw(p_lcd, &circ_1, 0x00, false);
    circ_1.x = 125;
    nrf_gfx_circle_draw(p_lcd, &circ_1, 0x00, true);
    nrf_gfx_display(p_lcd);
    //nrf_gfx_uninit(p_lcd);
}

static void epaper_demo_imarray(void)
{
    NRF_LOG_INFO("Display image.");
    //APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
    wsepd154_draw_monobmp((const uint8_t *)gImage_1in54);
    nrf_gfx_display(p_lcd);
    //nrf_gfx_uninit(p_lcd);
}


static void print_state(void)
{
    NRF_LOG_INFO("m_scan_engine_comm_state: %d", m_scan_engine_comm_state);

    NRF_LOG_INFO("m_is_receiving_data_from_scan_engine: %d", m_is_receiving_data_from_scan_engine);
    NRF_LOG_INFO("m_is_new_message_from_scan_engine: %d", m_is_new_message_from_scan_engine);
    NRF_LOG_INFO("m_is_new_barcode_from_scan_engine: %d", m_is_new_barcode_from_scan_engine);
    //NRF_LOG_INFO("m_is_non_blocking_delay_wait: %d", m_is_non_blocking_delay_wait);
    NRF_LOG_INFO("m_scan_engine_cancel_operation: %d", m_scan_engine_cancel_operation);

    NRF_LOG_INFO("m_scan_engine_inbound_message_hex: %X", m_scan_engine_inbound_message_hex);
    NRF_LOG_INFO("m_scan_engine_inbound_barcode: %s", m_scan_engine_inbound_barcode);
}


bool em2000h_send_command(const char * command, size_t size, bool wait_for_ack)
{
    print_state();

    // clear last scan engine inbound messag
    //memset(m_scan_engine_inbound_message, 0, sizeof(m_scan_engine_inbound_message));
    m_is_new_message_from_scan_engine = false;
    //m_scan_engine_cancel_operation = false;

    if (command == CMD_START_DECODE)
    {
        // Clear barcode buffer
        memset(m_scan_engine_inbound_barcode, 0, sizeof(m_scan_engine_inbound_barcode));
        m_is_new_barcode_from_scan_engine = false;
    }

    ret_code_t ret = nrf_serial_write(&m_serial_uart, command, size, NULL, SERIAL_MAX_WRITE_TIMEOUT);
    APP_ERROR_CHECK(ret);

    // TODO: Implement wait-timeout to prevent infinite loop.
    if (wait_for_ack) {
        m_scan_engine_comm_state = AWAITING_ACK_MESSAGE;
        // Wait for scan engine inbound message
        while(!m_is_new_message_from_scan_engine)
        {
            NRF_LOG_INFO("Waiting for scan engine ACK message...");

            //if (m_scan_engine_cancel_operation) {              
            //    return false;
            //    m_scan_engine_comm_state = IDLE;
            //}
            if (NRF_LOG_PROCESS() == false)
            {
                nrf_pwr_mgmt_run();
            }
        }

        /*
        for (int i=0; i<sizeof(m_scan_engine_inbound_message_hex)/sizeof(char); i++)
        {
            NRF_LOG_INFO("HEX-Chars of ACK-message: 0x%X", m_scan_engine_inbound_message_hex[i]);
        }
        */

         
        // TODO: The check for the ACK-/NACK-message should be enhanced.
        //       Every byte of the message should be checked for categorization the message as ACK/NACK.
        if (m_scan_engine_inbound_message_hex[1] == 0xD0)
        {
            // ACK-Message
            NRF_LOG_INFO("ACK-Message received.");

            if (command == CMD_START_DECODE)
            {
                // Command for start a barcode decode session
                m_scan_engine_comm_state = AWAITING_BARCODE;
            }
            else
            {
                m_scan_engine_comm_state = IDLE;
            }

            //em2000h_send_command(CMD_ACK, 6, false); // ACK of ACK message not neccessary
            
            return true;
        }
        else if (m_scan_engine_inbound_message_hex[1] == 0xD1)
        {
            NRF_LOG_INFO("NACK-Message received.");

            // TODO: Implement Resend of the message.
            // NACK-Message
            //if (resend) {
            //   em2000h_send_command(CMD_ACK, 6, false, resend_counter=1);
            //}

            m_scan_engine_comm_state = IDLE;
            
            return false;
        }
        else
        {
            NRF_LOG_INFO("Unexpected-Message received.");
            // Unexpected Message

            m_scan_engine_comm_state = IDLE;

            return false;
        }
    }
    else
    {
        NRF_LOG_INFO("No ACK-Message expected.");

        if (command[1] == 0xC7)
        {
            // Command for inquiry an scan engine parameter 
            m_scan_engine_comm_state = AWAITING_PARAM_DATA;
        } else {
            m_scan_engine_comm_state = IDLE;
        }
    
        // When no ACK-message is expected, wait 50ms after sending the command.
        // So subsequent commands are separated by a minimum of 50ms.
        non_blocking_delay_ms(50);

        return true;
    }

    return false;
}


static void barcode_module_init()
{
    NRF_LOG_INFO("barcode_module_init()");

    nrf_gpio_cfg_output(BCM_TRIGGER);
    nrf_gpio_pin_write(BCM_TRIGGER, 1);

    nrf_gpio_cfg_output(BCM_WAKEUP);
    nrf_gpio_pin_write(BCM_WAKEUP, 1);

    nrf_gpio_cfg_input(BCM_LED, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(BCM_BUZZER, NRF_GPIO_PIN_NOPULL);


    bool send_ret = false;

    /*
    NRF_LOG_INFO("Send command CMD_PARAM_SET_SOFTWARE_HANDSHAKING_DISABLE");
    send_ret = em2000h_send_command(CMD_PARAM_SET_SOFTWARE_HANDSHAKING_DISABLE, 9, false);
    if (!send_ret) {
      NRF_LOG_ERROR("Sending command CMD_PARAM_SET_SOFTWARE_HANDSHAKING_DISABLE failed.");
    }
    */
    /*
    NRF_LOG_INFO("Send command CMD_PARAM_SET_SOFTWARE_HANDSHAKING_ENABLE");
    send_ret = em2000h_send_command(CMD_PARAM_SET_SOFTWARE_HANDSHAKING_ENABLE, 9, false);
    if (!send_ret) {
      NRF_LOG_ERROR("Sending command CMD_PARAM_SET_SOFTWARE_HANDSHAKING_ENABLE failed.");
    }
    */
    /*
    NRF_LOG_INFO("Send command CMD_PARAM_SET_POWER_MODE_LOW");
    send_ret = em2000h_send_command(CMD_PARAM_SET_POWER_MODE_LOW, 9, false);
    if (!send_ret) {
      NRF_LOG_ERROR("Sending command CMD_PARAM_SET_POWER_MODE_LOW failed.");
    }
    NRF_LOG_INFO("Send command CMD_PARAM_SET_TRIGGER_MODE_HOST");
    send_ret = em2000h_send_command(CMD_PARAM_SET_TRIGGER_MODE_HOST, 9, false);
    if (!send_ret) {
      NRF_LOG_ERROR("Sending command CMD_PARAM_SET_TRIGGER_MODE_HOST failed.");
    }
    */

    NRF_LOG_INFO("Send command CMD_WAKEUP");
    send_ret = em2000h_send_command(CMD_WAKEUP, 1, false);
    if (!send_ret)
    {
        NRF_LOG_ERROR("Sending command CMD_WAKEUP failed.");
    }

    NRF_LOG_INFO("Send command CMD_SCAN_DISABLE");
    send_ret = em2000h_send_command(CMD_SCAN_DISABLE, 6, true);
    if (!send_ret)
    {
        NRF_LOG_ERROR("Sending command CMD_SCAN_DISABLE failed.");
    }

    NRF_LOG_INFO("Send command CMD_SCAN_ENABLE");
    send_ret = em2000h_send_command(CMD_SCAN_ENABLE, 6, true);
    if (!send_ret)
    {
        NRF_LOG_ERROR("Sending command CMD_SCAN_ENABLE failed.");
    }

    NRF_LOG_INFO("Send command CMD_PARAM_GET_SOFTWARE_HANDSHAKING");
    send_ret = em2000h_send_command(CMD_PARAM_GET_SOFTWARE_HANDSHAKING, 7, false);
    if (!send_ret)
    {
        NRF_LOG_ERROR("Sending command CMD_PARAM_GET_SOFTWARE_HANDSHAKING failed.");
    }

    NRF_LOG_INFO("Send command CMD_PARAM_SET_TERMINATOR_DISABLE");
    send_ret = em2000h_send_command(CMD_PARAM_SET_TERMINATOR_DISABLE, 10, false);
    if (!send_ret) {
      NRF_LOG_ERROR("Sending command CMD_PARAM_SET_TERMINATOR_DISABLE failed.");
    }

    NRF_LOG_INFO("Scan Engine Init Finished.");
}


static void signal_feedback_positive()
{
    NRF_LOG_INFO("signal_feedback_positive()");
    APP_ERROR_CHECK(app_timer_stop(m_feedback_timer_id));
    //bsp_board_led_on(bsp_board_pin_to_led_idx(LED_FEEDBACK_OK));
    nrf_gpio_pin_set(LED_FEEDBACK_OK);
    nrf_gpio_pin_set(BUZZER);
    nrf_gpio_pin_set(VIBRATION_MOTOR);
    APP_ERROR_CHECK(app_timer_start(m_feedback_timer_id, APP_TIMER_TICKS(500), NULL));
}

static void signal_feedback_positive_scheduled_handler(void * p_event_data, uint16_t event_size)
{
    signal_feedback_positive();
}

static void display_barcode(const char * barcode)
{
    NRF_LOG_INFO("display_barcode(): x %s x", barcode);
    epaper_demo_text(barcode);
}

static void display_barcode_scheduled_handler(void * p_event_data, uint16_t event_size)
{
    log_execution_mode("display_barcode_scheduled_handler()"); 
    const char * barcode = (const char *) p_event_data;
    display_barcode(barcode);
}

static void send_barcode_to_server_scheduled_handler(void * p_event_data, uint16_t event_size)
{   
    log_execution_mode("send_barcode_to_server_scheduled_handler()"); 
    char * barcode = (char *) p_event_data;
}

static void send_barcode_to_hid_scheduled_handler(void * p_event_data, uint16_t event_size)
{   
    log_execution_mode("send_barcode_to_hid_scheduled_handler()"); 
    char * barcode = (char *) p_event_data;
    hid_send_barcode(barcode, event_size);
}

static void process_barcode()
{
    NRF_LOG_INFO("process_barcode(): BARCODE=%s", m_scan_engine_inbound_barcode);
    
    // Log execution mode.
    log_execution_mode("process_barcode()");
    
    // Signal positiv feedback
    //app_sched_event_put(NULL, NULL, signal_feedback_positive_scheduled_handler);
    signal_feedback_positive();

    // Send barcode as keys to HID
    //app_sched_event_put((void*) p_scan_engine_inbound_barcode, strlen(p_scan_engine_inbound_barcode), send_barcode_to_hid_scheduled_handler);
    
    if (hid_is_connected())
    {
        NRF_LOG_INFO("*** HID is connected.");
        g_ble_tx_busy = true;
        hid_send_barcode((uint8_t *)m_scan_engine_inbound_barcode, strlen(m_scan_engine_inbound_barcode));    
    }   
    else
    {
        NRF_LOG_INFO("*** HID is NOT connected.");
        g_ble_tx_busy = false;
    }

    start_display_timer();
}    

bool read_mifare_tag(char * p_barcode)
{
    NRF_LOG_INFO("read_mifare_tag()" );
    nfc_wakeup(m_nfc);
    if (nfc_tag_present(m_nfc, 0))
    {
        NRF_LOG_DEBUG("Tag detected.");
        NfcTag * tag = nfc_read(m_nfc);

        uint8_t nfc_error_code = nfc_tag_get_error_code(tag);

        if (nfc_error_code != 0)
        {
            switch (nfc_error_code)
            {
                case 1: // NFCTAG_NOT_NDEF_FORMATTED
                    NRF_LOG_ERROR("NFC tag not NDEF formatted.");
                    break;
                case 2: // NFCTAG_AUTHENTICATION_FAILED
                    NRF_LOG_ERROR("NFC tag authentication error.");
                    break;
                case 3: // NFCTAG_READ_FAILED
                    NRF_LOG_ERROR("NFC tag read failed.");
                    break;
                case 9: // NFCTAG_OTHER_ERROR
                    NRF_LOG_ERROR("NFC tag other error.");
                    break;
            }
        }
        else
        {
            if (nfc_tag_has_ndef_message(tag))
            {
                NRF_LOG_DEBUG("Tag %d has NDEF message.", nfc_tag_get_uid(tag));

                NdefMessage* ndef_message = nfc_tag_get_ndef_message(tag);
                ndef_message_print(ndef_message);
                
                // take the first record
                NdefRecord* ndef_record = ndef_message_get_record(ndef_message, 0);
                
                //uint8_t record_tnf = ndef_record_get_tnf(ndef_record);                                    
                //uint8_t record_type_length = ndef_record_get_type_length(ndef_record);
                //uint8_t record_type[record_type_length];
                //ndef_record_get_type(ndef_record, record_type);

                uint8_t payload_length = ndef_record_get_payload_length(ndef_record);
                uint8_t payload[payload_length];
                ndef_record_get_payload(ndef_record, payload);
                strcat(payload, "\0");
                strcpy(p_barcode, payload);
            
                //struct payload_record new_payload_record;
                //memcpy(new_payload_record.type, record_type, record_type_length);
                //new_payload_record.type_length = record_type_length;
                //memcpy(new_payload_record.payload, payload, payload_length);
                //new_payload_record.payload_length = payload_length;
                //m_payload_records[i] = new_payload_record;

                destroy_nfc_tag(tag);
                return true;
            }
            else
            {
                NRF_LOG_ERROR("Tag %d has no NDEF message.", nfc_tag_get_uid(tag));
            }
        }

        destroy_nfc_tag(tag);        
    }

    return false;
}


/**@brief Button 1 push event handler function to be called by the scheduler.
 */
void button1_scheduled_push_event_handler(void * p_event_data, uint16_t event_size)
{
    NRF_LOG_INFO("button1_scheduled_push_event_handler()");

    // Log execution mode.
    log_execution_mode("button1_scheduled_push_event_handler()");
    
    bool se_command_resp = false;
    NRF_LOG_INFO("Button_1 pushed");
    // Start barcode decode session
    NRF_LOG_INFO("Send comannd CMD_WAKEUP.");
    se_command_resp = em2000h_send_command(CMD_WAKEUP, 1, false);
    NRF_LOG_INFO("CMD_WAKEUP returned %d", se_command_resp);
    NRF_LOG_INFO("Send comannd CMD_START_DECODE.");
    se_command_resp = em2000h_send_command(CMD_START_DECODE, 6, true);
    NRF_LOG_INFO("CMD_START_DECODE returned %d", se_command_resp);
    
    // Waiting for barcode
    NRF_LOG_INFO("Waiting for barcode - BEFORE");
    while (m_scan_engine_comm_state == AWAITING_BARCODE)
    {
        NRF_LOG_INFO("Waiting for barcode...");
        
        if (m_scan_engine_cancel_operation) {
            NRF_LOG_INFO("Waiting for barcode... CANCEL");
            m_is_new_barcode_from_scan_engine = false;
            m_scan_engine_comm_state = IDLE;                
            break;
        }

        if (m_is_new_barcode_from_scan_engine) {
            // Here we have received successfully a barcode from the scan engine
            NRF_LOG_INFO("******************************");
            NRF_LOG_INFO("*** Processing BARCODE: %s ***", m_scan_engine_inbound_barcode);
            NRF_LOG_INFO("******************************");                
            m_is_new_barcode_from_scan_engine = false;
            m_scan_engine_comm_state = IDLE;
            
            process_barcode();
        }
        if (NRF_LOG_PROCESS() == false)
        {
            nrf_pwr_mgmt_run();
        }
    }

    NRF_LOG_INFO("button1_scheduled_push_event_handler() - end");
}

/**@brief Button 1 release event handler function to be called by the scheduler.
 */
void button1_scheduled_release_event_handler(void * p_event_data, uint16_t event_size)
{
    NRF_LOG_INFO("button1_scheduled_release_event_handler()");

    // Log execution mode.
    log_execution_mode("button1_scheduled_release_event_handler()");

    NRF_LOG_INFO("Button_1 released");

    print_state();

    // Stop barcode decode session
    if (m_scan_engine_cancel_operation)
    {
        bool se_command_resp = false;
        NRF_LOG_INFO("Send comannd CMD_WAKEUP.");
        se_command_resp = em2000h_send_command(CMD_WAKEUP, 1, false);
        NRF_LOG_INFO("CMD_WAKEUP returned %d", se_command_resp);
        NRF_LOG_INFO("Send comannd CMD_STOP_DECODE.");
        se_command_resp = em2000h_send_command(CMD_STOP_DECODE, 6, true);
        NRF_LOG_INFO("CMD_STOP_DECODE returned %d", se_command_resp);
    }

    is_trigger_button_push_busy = false;
    is_trigger_button_release_busy = false;

    NRF_LOG_INFO("button1_scheduled_release_event_handler() - end");
}


/**@brief Button 2 handler function to be called by the scheduler.
 */
void button2_scheduled_event_handler(void * p_event_data, uint16_t event_size)
{
    NRF_LOG_INFO("Button_2 pushed");
    m_read_nfc = true;
    while (m_read_nfc == true)
    {
        char barcode[16];
        bool read_success = read_mifare_tag(barcode);
        if (read_success)
        {
          m_read_nfc = false;
          //TODO: handle read success
          NRF_LOG_INFO("Stop NFC polling.");
          NRF_LOG_INFO("NFC read successfully:");
          NRF_LOG_INFO("Read barcode %s:", barcode);
          memcpy(m_scan_engine_inbound_barcode, barcode, 16);
          process_barcode();
          return;
        }
        //app_sched_execute();
        nrf_pwr_mgmt_run();
    }
}


/**@brief Button 3 handler function to be called by the scheduler.
 */
void button3_scheduled_event_handler(void * p_event_data, uint16_t event_size)
{
    uint8_t button_action = *((uint8_t*) p_event_data);

    if (button_action == APP_BUTTON_PUSH)
    {
        NRF_LOG_INFO("Button_3 pushed");
    }
    else if (button_action == APP_BUTTON_RELEASE)
    {
        NRF_LOG_INFO("Button_3 released");
        epaper_demo_draw();
    }
}


/**@brief Button 4 handler function to be called by the scheduler.
 */
void button4_scheduled_event_handler(void * p_event_data, uint16_t event_size)
{
    
    log_execution_mode("button4_scheduled_event_handler()");

    uint8_t button_action = *((uint8_t*) p_event_data);

    if (button_action == APP_BUTTON_PUSH)
    {
        NRF_LOG_INFO("Button_4 pushed");
        //non_blocking_delay_ms(10000);
    }    
    else if (button_action == APP_BUTTON_RELEASE)
    {
        NRF_LOG_INFO("Button_4 released");
        epaper_demo_imarray();
        // Reset scan engine
        nrf_gpio_pin_write(BCM_WAKEUP, 0);
        NRF_LOG_INFO("Wait...");
        non_blocking_delay_ms(2000);
        NRF_LOG_INFO("Wait... end.");
        nrf_gpio_pin_write(BCM_WAKEUP, 1);        
    }
    else 
    {
      NRF_LOG_INFO("Button_4 unknown button action: %d", button_action);
    }
}


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    // NRF_LOG_INFO("button_event_handler()");
    ret_code_t err_code;

    switch (pin_no)
    {
        case BUTTON_1:
            /* TODO: Prevent subsequent invocations of the handler functions: dont call the handle functions if the function still working
            if (nrf_atomic_flag_set_fetch(g_functiony_xy_is_busy))
            {
                return NRF_ERROR_BUSY;
            }
            // in the function call...
                UNUSED_RETURN_VALUE(nrf_atomic_flag_clear(g_functiony_xy_is_busy));
              when finished.
            */
            NRF_LOG_INFO("Button_1 handle");

            if (button_action == APP_BUTTON_PUSH)
            {
                if (is_trigger_button_push_busy)
                {
                    NRF_LOG_INFO("***Button 1 PUSH is busy.");
                    return;
                }
                is_trigger_button_push_busy = true;

                m_scan_engine_comm_state = IDLE;

                memset(m_scan_engine_inbound_message_hex, 0, sizeof(m_scan_engine_inbound_message_hex));
                memset(m_scan_engine_inbound_barcode, 0, sizeof(m_scan_engine_inbound_barcode));

                m_is_receiving_data_from_scan_engine = false;
                m_is_new_message_from_scan_engine = false;
                m_is_new_barcode_from_scan_engine = false;
                m_number_of_bytes_transfered = 0;

                //stop_non_blocking_delay();
                m_scan_engine_cancel_operation = false;
                
                app_sched_event_put(&button_action, sizeof(button_action), button1_scheduled_push_event_handler);
            }
            else if (button_action == APP_BUTTON_RELEASE) 
            {
                if (is_trigger_button_release_busy)
                {
                    NRF_LOG_INFO("***Button 1 RELEASE is busy.");
                    return;
                }
                is_trigger_button_release_busy = true;

                if (m_scan_engine_comm_state == AWAITING_BARCODE)
                {
                    m_scan_engine_cancel_operation = true;
                }

                app_sched_event_put(&button_action, sizeof(button_action), button1_scheduled_release_event_handler);                
            }            
            break;
        case BUTTON_2:
            NRF_LOG_INFO("Button_2 handle");
            if (button_action == APP_BUTTON_PUSH)
            {
                // start NFC polling
                NRF_LOG_INFO("Start NFC polling...");
                app_sched_event_put(&button_action, sizeof(button_action), button2_scheduled_event_handler);            
            }
            else if (button_action == APP_BUTTON_RELEASE) 
            {
                // stop NFC reading
                NRF_LOG_INFO("Stop NFC polling.");
                m_read_nfc = false;
            }
            break;
        case BUTTON_3:
            NRF_LOG_INFO("Button_3 handle");
            app_sched_event_put(&button_action, sizeof(button_action), button3_scheduled_event_handler);
            break;
        case BUTTON_4:
            NRF_LOG_INFO("Button_4 handle");
            app_sched_event_put(&button_action, sizeof(button_action), button4_scheduled_event_handler);
            break;
        default:
            // APP_ERROR_HANDLER(pin_no);
            NRF_LOG_WARNING("Unsupported button: pin=%d", pin_no);
            break;
    }
}

/**@brief Function for initializing the button handler module.
 */
static void buttons_init()
{
    NRF_LOG_INFO("buttons_init()");

    uint32_t err_code;

    // The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] = {
        {BUTTON_1, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_event_handler},
        {BUTTON_2, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_event_handler},
        {BUTTON_3, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_event_handler},
        {BUTTON_4, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_event_handler}};

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons), BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}

/**
 * Pin change handler
 */
static void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin = BCM_LED)
    {
        NRF_LOG_INFO("Feedback-LED: %d", action);
        //bsp_board_led_on(bsp_board_pin_to_led_idx(FEEDBACK_LED));
    } else if (pin == BCM_BUZZER)
    {
        NRF_LOG_INFO("Feedback-Buzzer: %d", action);
    }

    // nrf_drv_gpiote_out_toggle(PIN_OUT);
}

/**
 * @brief Function for configuring: Configures GPIOTE to give an interrupt on pin change.
 */
static void gpio_init(void)
{
    ret_code_t err_code;

    // Initialization of GPIOTE is allready done by app_button library
    /*
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    // Button GPIO config
    nrf_drv_gpiote_in_config_t in_config_buttons = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config_buttons.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(BUTTON_1, &in_config_buttons, button_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_2, &in_config_buttons, button_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_3, &in_config_buttons, button_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_4, &in_config_buttons, button_event_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(BUTTON_1, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_2, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_3, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_4, true);
    */

    // BCM LED GPIO config
    nrf_drv_gpiote_in_config_t in_config_bcm_led = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config_bcm_led.pull = NRF_GPIO_PIN_PULLUP;
    
    /*
    err_code = nrf_drv_gpiote_in_init(BCM_LED, &in_config_bcm_led, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(BCM_LED, true);
    */
}


static void stop_serial_receive_timer()
{
    NRF_LOG_INFO("stop_serial_receive_timer()");
    
    APP_ERROR_CHECK(app_timer_stop(m_serial_receive_timer_id));
        
    //memset(m_scan_engine_inbound_message_hex, 0, sizeof(m_scan_engine_inbound_message_hex));
    //memset(m_scan_engine_inbound_barcode, 0, sizeof(m_scan_engine_inbound_barcode));
    //m_is_receiving_data_from_scan_engine = false;
    //m_is_new_message_from_scan_engine = false;
    //m_is_new_barcode_from_scan_engine = false;
    //m_number_of_bytes_transfered = 0;
}


static void serial_receive_timeout_handler()
{
    NRF_LOG_INFO("serial_receive_timeout_handler(), state=%d", m_scan_engine_comm_state);
    
    if (m_scan_engine_comm_state == AWAITING_BARCODE) {
        m_is_new_barcode_from_scan_engine = true;
        NRF_LOG_INFO("***** serial_receive_timeout_handler: BARCODE=%s", m_scan_engine_inbound_barcode);
    }
    
    m_is_receiving_data_from_scan_engine = false;
    m_is_new_message_from_scan_engine = true;
    m_number_of_bytes_transfered = 0;
}

static void feedback_timeout_handler()
{
    //bsp_board_led_off(bsp_board_pin_to_led_idx(LED_FEEDBACK_OK));
    nrf_gpio_pin_clear(LED_FEEDBACK_OK);
    nrf_gpio_pin_clear(BUZZER);
    nrf_gpio_pin_clear(VIBRATION_MOTOR);    
}

static void display_timeout_handler()
{    
    NRF_LOG_INFO("display_timeout_handler()");

    uint32_t diff_time_millis = app_timer_cnt_diff_compute(app_timer_cnt_get(), m_display_wait_for_ble_tx_starttime) / TICKS_PER_MILLISECOND;

    //NRF_LOG_INFO("start time: %d, current time: %d, diff timer: %d", m_display_wait_for_ble_tx_starttime, current_time, diff_time);

    // Check if wait for a unreasonable time (timeout)
    if (diff_time_millis < m_ble_tx_transfer_wait_timeout_millis)
    {
        // no wait timeout, check if BLE transfer finished.
        if (g_ble_tx_busy)
        {
            // not finished wait further
            return;
        }
    }
    else
    {
        // wait timeout
        NRF_LOG_WARNING("Timeout while waiting with display for BLE transfer.");
    }
    
    stop_display_timer();

    // Display barcode on screen
    app_sched_event_put((void*) m_scan_engine_inbound_barcode, strlen(m_scan_engine_inbound_barcode), display_barcode_scheduled_handler);
    //display_barcode(m_scan_engine_inbound_barcode);
    //epaper_demo_text(p_scan_engine_inbound_barcode);
}

static void timers_create(void)
{
    ret_code_t ret_code;

    // timer for serial receiving
    ret_code = app_timer_create(&m_serial_receive_timer_id, APP_TIMER_MODE_SINGLE_SHOT, serial_receive_timeout_handler);
    APP_ERROR_CHECK(ret_code);
    
    // Timer for signaling feedback to the user via LED or Buzzer
    ret_code = app_timer_create(&m_feedback_timer_id, APP_TIMER_MODE_SINGLE_SHOT, feedback_timeout_handler);
    APP_ERROR_CHECK(ret_code);

    // Timer for show something to the user via Display
    ret_code = app_timer_create(&m_display_timer_id, APP_TIMER_MODE_REPEATED, display_timeout_handler);
    APP_ERROR_CHECK(ret_code);
}

static void serial_sleep_handler(void)
{
    NRF_LOG_INFO("serial_sleep_handler()");

    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**
 * Serial Empfang via Event Handler ist schlecht dokumentiert. Hier gibt es ein paar interessante
 * Case: https://devzone.nordicsemi.com/f/nordic-q-a/34391/serial-port-library-receive-interrupt
 * https://devzone.nordicsemi.com/f/nordic-q-a/22320/serial-port-interrupt
 */
static void serial_event_handler(struct nrf_serial_s const * p_serial, nrf_serial_event_t event)
{
    switch (event)
    {
        case NRF_SERIAL_EVENT_TX_DONE:
            //NRF_LOG_INFO("serial_event_handler(): NRF_SERIAL_EVENT_TX_DONE");
            break;
        case NRF_SERIAL_EVENT_RX_DATA:
            //NRF_LOG_INFO("serial_event_handler(): NRF_SERIAL_EVENT_RX_DATA (state: %d)", m_scan_engine_comm_state);
            if (m_is_receiving_data_from_scan_engine == false)
            {
                // start of new transfer
                start_serial_receive_timer();
                m_is_receiving_data_from_scan_engine = true;
                m_number_of_bytes_transfered = 0;
                memset(m_scan_engine_inbound_message_hex, 0, sizeof(m_scan_engine_inbound_message_hex));
            }

            char c;
            ret_code_t ret_code = nrf_queue_read(p_serial->p_ctx->p_config->p_queues->p_rxq, &c, sizeof(c));
            APP_ERROR_CHECK(ret_code);
            
            NRF_LOG_INFO("Byte received: %d (0x%X) [state=%d]", c, c, m_scan_engine_comm_state);

            m_scan_engine_inbound_message_hex[m_number_of_bytes_transfered++] = c;
            
            // TODO: m_scan_engine_inbound_barcode im serial timeout handler aus m_scan_engine_inbound_message_hex zusammenbauen.
            if (m_scan_engine_comm_state == AWAITING_BARCODE)
            {
                strcat(m_scan_engine_inbound_barcode, &c);
            }

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
}


NRF_SERIAL_CONFIG_DEF(m_serial_config, NRF_SERIAL_MODE_DMA, &m_serial_queues, &m_serial_buffs, serial_event_handler, serial_sleep_handler);

/**
 * Function for initializing serial communication.
 */
static void serial_init(void)
{
    ret_code_t ret_code;
    ret_code = nrf_serial_init(&m_serial_uart, &m_uart0_drv_config, &m_serial_config);
    APP_ERROR_CHECK(ret_code);
}


static void display_init()
{
  NRF_LOG_INFO("display_init()");
  epaper_demo_clear();
}

void nfc_init()
{
    NRF_LOG_INFO("nfc_init()");
    m_nfc = create_nfc_adapter(spi_pn532);
    nfc_begin(m_nfc, true);

    // destroy_nfc_adapter(g_nfc);
}




/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    log_init();
    lfclk_init();
    power_management_init();
    scheduler_init();
    timers_init();
    timers_create();
    leds_init();
    vibrator_init();
    buzzer_init();
    buttons_init();
    gpio_init();
    serial_init();
    utils_init();
    //coap_ipv6_init();
    nfc_init();
    display_init();
    
    // Test if non-blocking delay works in main(). Remove it later.
    //non_blocking_delay_ms(1000);

    bool erase_bonds = false;
    start_ble_services(erase_bonds);

   // Start execution.
    NRF_LOG_INFO("TicTac Barcode Scanner started.");
    
    // Enter main loop.
    for (;;)
    {
        app_sched_execute();

        if (NRF_LOG_PROCESS() == false)
        {
            nrf_pwr_mgmt_run();
        }
    }
}

/**
 * @}
 */