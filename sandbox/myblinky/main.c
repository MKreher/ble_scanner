/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file main.c (changed by mkr)
 * @brief Blinky Example Application main file.
 *
 */

#define CUSTOM_BOARD 

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "boards.h"
#include "sdk_errors.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "nrf_drv_timer.h"
#include "app_timer.h"
#include "app_button.h"
#include "app_error.h"
#include "app_pwm.h"

#define LEDBUTTON_BUTTON 4
#define NUMBER_OF_BUTTONS 1

const uint32_t led_board_1  = 30;
const uint32_t led_board_2  = 31;
const uint32_t led_error    = 25;
const uint32_t led_success  = 24;

const nrf_drv_timer_t TIMER_LED = NRFX_DRV_TIMER_INSTANCE(0);

APP_PWM_INSTANCE(PWM1, 1); // Create the instance "PWM1" using TIMER1.

static volatile bool ready_flag; // A flag indicating PWM status.

void pwm_ready_callback(uint32_t pwm_id) // PWM callback function
{
    ready_flag = true;
}


/**
 * @brief Function for starting lfclk needed by APP_TIMER.
 * Function starting the internal LFCLK oscillator.
   This is needed by RTC1 which is used by the application timer
   (When SoftDevice is enabled the LFCLK is always running and this is not needed).
 */
static void lfclk_init(void)
{
    uint32_t err_code;
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}

/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{

    switch (pin_no)
    {
    case LEDBUTTON_BUTTON:
        nrf_gpio_pin_toggle(led_board_1);
        break;

    default:
        APP_ERROR_HANDLER(pin_no);
        break;
    }
}

/**@brief Function for initializing the button handler module.
 */

static void buttons_init(void)
{

    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static const app_button_cfg_t buttons[NUMBER_OF_BUTTONS] =
        {
            {LEDBUTTON_BUTTON, false, BUTTON_PULL, button_event_handler}
        };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons), APP_TIMER_TICKS(50));
    APP_ERROR_CHECK(err_code);
    if (err_code != NRFX_SUCCESS)
    {
        nrf_gpio_pin_set(led_error);
    }
    else
    {
        nrf_gpio_pin_set(led_success);
    }

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}

/*
static void init_pwm()
{
    
    ret_code_t err_code; 
    
    // 2-channel PWM, 200Hz, output on DK LED pins.
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(5000L, led_error, led_success);

    // Switch the polarity of the second channel.
    pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;
    pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;

    // Initialize and enable PWM.
    err_code = app_pwm_init(&PWM1, &pwm1_cfg, pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    app_pwm_enable(&PWM1);
}

static void pulse_leds_by_pwm()
{
    uint32_t value;
    for (uint8_t i = 0; i < 40; ++i)
    {
        value = (i < 20) ? (i * 5) : (100 - (i - 20) * 5);

        ready_flag = false;
        // Set the duty cycle - keep trying until PWM is ready...
        while (app_pwm_channel_duty_set(&PWM1, 0, value) == NRF_ERROR_BUSY)
            ;

        // ... or wait for callback.
        while (!ready_flag)
            ;
        APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 1, value));
        nrf_delay_ms(50);
    }
}
*/

/**
 * @brief Handler for timer events.
 */
void timer_led_event_handler(nrf_timer_event_t event_type, void *p_context)
{
    switch (event_type)
    {
    case NRF_TIMER_EVENT_COMPARE0:
        nrf_gpio_pin_toggle(led_board_2);
        break;
    default:
        //Do nothing.
        break;
    }
}

static void init_led_timer()
{
    uint32_t time_ms = 500; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;
    uint32_t err_code = NRF_SUCCESS;

    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    err_code = nrf_drv_timer_init(&TIMER_LED, &timer_cfg, timer_led_event_handler);
    APP_ERROR_CHECK(err_code);

    time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_LED, time_ms);

    nrf_drv_timer_extended_compare(&TIMER_LED, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

    nrf_drv_timer_enable(&TIMER_LED);
}

/**
 * @brief Function for application main entry.
 */
    int main(void)
    {
        ret_code_t err_code;

        // Configure LED-pin as outputs and clear.
        nrf_gpio_cfg_output(led_board_1);
        nrf_gpio_pin_clear(led_board_1);

        nrf_gpio_cfg_output(led_board_2);
        nrf_gpio_pin_clear(led_board_2);

        nrf_gpio_cfg_output(led_error);
        nrf_gpio_pin_clear(led_error);

        nrf_gpio_cfg_output(led_success);
        nrf_gpio_pin_clear(led_success);

        lfclk_init();

        buttons_init();

        // Start APP_TIMER to generate timeouts.
        err_code = app_timer_init();
        APP_ERROR_CHECK(err_code);        
        
        //init_pwm();

        if (false)
        init_led_timer();

        // Toggle LED.
        while (true)
        {
            //nrf_gpio_pin_toggle(led_board_1);
            //nrf_gpio_pin_toggle(led_board_2);
            //nrf_delay_ms(1000);

            //pulse_leds_by_pwm();

            __WFI();
        }
}

/** @} */
