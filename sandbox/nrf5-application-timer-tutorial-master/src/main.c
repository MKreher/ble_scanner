/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
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

#include "app_scheduler.h"
#include "app_timer.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include <stdbool.h>

#define SCHED_MAX_EVENT_DATA_SIZE 16 /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE 196         /**< Maximum number of events in the scheduler queue. */

APP_TIMER_DEF(m_non_blocking_delay_timer_id);

static volatile uint8_t is_non_blocking_delay_wait = 0;

/**@brief Timeout handler for the single shot timer.
 */
static void non_blocking_delay_timer_handler(void * p_context)
{
    NRF_LOG_INFO("non_blocking_delay_timer_handler()");

    is_non_blocking_delay_wait = 0;
}

static void non_blocking_delay_ms(uint64_t delay)
{
    NRF_LOG_INFO("non_blocking_delay_ms(): START - %d ms", delay);
    is_non_blocking_delay_wait = 1;

    APP_ERROR_CHECK(app_timer_start(m_non_blocking_delay_timer_id, APP_TIMER_TICKS(delay), NULL));

    while (is_non_blocking_delay_wait == 1)
    {
        nrf_pwr_mgmt_run();
    }

    NRF_LOG_INFO("non_blocking_delay_ms(): END - %d ms", delay);
}


/**@brief Function starting the internal LFCLK oscillator.
 *
 * @details This is needed by RTC1 which is used by the Application Timer
 *          (When SoftDevice is enabled the LFCLK is always running and this is not needed).
 */
static void lfclk_request(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}


/**@brief Button event handler function.
 *
 * @details Responsible for controlling LEDs based on button presses.
 */
void button_handler(nrf_drv_gpiote_pin_t pin)
{
    // Log execution mode.
    if (current_int_priority_get() == APP_IRQ_PRIORITY_THREAD)
    {
        NRF_LOG_INFO("button_event_handler() is executing in thread/main mode.");
    }
    else
    {
        NRF_LOG_INFO("button_event_handler() is executing in interrupt handler mode.");
    }

    ret_code_t err_code;
    static uint32_t timeout = 0;

    switch (pin)
    {
        case BUTTON_1:
            NRF_LOG_INFO("BUTTON_1 pressed.");
            NRF_LOG_INFO("Warte...");
            non_blocking_delay_ms(10000);
            NRF_LOG_INFO("Warten beendet.");
            break;
        case BUTTON_2:
            NRF_LOG_INFO("BUTTON_2 pressed.");
            break;
        case BUTTON_3:
            NRF_LOG_INFO("BUTTON_3 pressed.");
            break;
        case BUTTON_4:
            NRF_LOG_INFO("BUTTON_4 pressed.");
            break;
        default:
            break;
    }
}

/**@brief Button handler function to be called by the scheduler.
 */
void button_scheduler_event_handler(void * p_event_data, uint16_t event_size)
{
    // In this case, p_event_data is a pointer to a nrf_drv_gpiote_pin_t that represents
    // the pin number of the button pressed. The size is constant, so it is ignored.
    button_handler(*((nrf_drv_gpiote_pin_t *)p_event_data));
}


/**@brief Button event handler.
 */
static void button_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    // The button_handler function could be implemented here directly, but is extracted to a
    // separate function as it makes it easier to demonstrate the scheduler with less modifications
    // to the code later in the tutorial.

    app_sched_event_put(&pin, sizeof(pin), button_scheduler_event_handler);
}

/**@brief Function for initializing GPIO pins.
 */
static void gpio_config()
{
    ret_code_t err_code;

    // Initialize GPIOTE driver.
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    // Configure output pins for LEDs.
    nrf_gpio_range_cfg_output(LED_1, LED_2);

    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(true);

    // Configure output pins for LEDs.
    nrf_drv_gpiote_out_init(LED_1, &out_config);
    nrf_drv_gpiote_out_init(LED_2, &out_config);

    // Set output pins (this will turn off the LEDs).
    nrf_drv_gpiote_out_set(LED_1);
    nrf_drv_gpiote_out_set(LED_2);

    // Make a configuration for input pints. This is suitable for both pins in this example.
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    // Configure input pins for 4 buttons, all using the same event handler.
    err_code = nrf_drv_gpiote_in_init(BUTTON_1, &in_config, button_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_2, &in_config, button_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_3, &in_config, button_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_4, &in_config, button_event_handler);
    APP_ERROR_CHECK(err_code);

    // Enable input pins for buttons.
    nrf_drv_gpiote_in_event_enable(BUTTON_1, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_2, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_3, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_4, true);
}


/**@brief Function for initializing logging.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Timeout handler for the repeated timer.
 */
static void repeated_timer_handler(void * p_context)
{
    nrf_drv_gpiote_out_toggle(LED_1);
}


/**@brief Create timers.
 */
static void create_timers()
{
    ret_code_t err_code;

    err_code = app_timer_create(&m_non_blocking_delay_timer_id, APP_TIMER_MODE_SINGLE_SHOT,
        non_blocking_delay_timer_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


/**@brief Function for the power management library initialization.
 * See: https://embeddedcentric.com/lesson-14-nrf5x-power-management-tutorial/
 */
static void power_management_init(void)
{
    nrf_pwr_mgmt_init();
}

/**@brief Main function.
 */
int main(void)
{
    log_init();
    lfclk_request();
    power_management_init();
    scheduler_init();
    gpio_config();
    app_timer_init();
    create_timers();

    NRF_LOG_INFO("Application timer tutorial example started.");

    NRF_LOG_INFO("Warte...");
    non_blocking_delay_ms(1000);
    NRF_LOG_INFO("Warten beendet.");

    // Enter main loop.
    while (true)
    {
        if (NRF_LOG_PROCESS() == false)
        {
            nrf_pwr_mgmt_run();
            app_sched_execute();
        }
    }
}