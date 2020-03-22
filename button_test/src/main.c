#include <stdbool.h>
#include <stdint.h>

#include "app_button.h"
#include "app_timer.h"
#include "boards.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"

// define Timer
APP_TIMER_DEF(m_button_action);

// Intervall in millis checking the button state
#define BUTTON_STATE_POLL_INTERVAL_MS 100

// macro for calculating the timer counter which indicates a long press 
#define LONG_PRESS(MS) (uint32_t)(MS) / BUTTON_STATE_POLL_INTERVAL_MS

// Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks).
#define BUTTON_DETECTION_DELAY APP_TIMER_TICKS(50)


/**@brief Function for initializing low-frequency clock.
 */
static void lfclk_config(void) {
  ret_code_t err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);

  nrf_drv_clock_lfclk_request(NULL);
}

/**@brief Function for initializing the timer module.
 */
static void timers_init(void) {
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the nrf log module.
 */
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

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until the next event occurs.
 */
static void idle_state_handle(void) {
  UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
  nrf_pwr_mgmt_run();
}

void button_timeout_handler(void *p_context) {
  ret_code_t err_code;
  static uint32_t button_timeout_cnt = 0;

  if (app_button_is_pushed(0)) {
    button_timeout_cnt++;
    if (button_timeout_cnt >= LONG_PRESS(5000)) {
      button_timeout_cnt = 0; // reset counter variable
      NRF_LOG_INFO("Long Button press");
    } else {
      err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
      APP_ERROR_CHECK(err_code);
    }
  } else {
    err_code = app_timer_stop(m_button_action);
    APP_ERROR_CHECK(err_code);
    button_timeout_cnt = 0; // reset counter variable
    NRF_LOG_INFO("Short button press");
  }
}

void button_callback(uint8_t pin_no, uint8_t button_action) {
  ret_code_t err_code;

  switch (pin_no) {
  case BUTTON_2:
    if (button_action == APP_BUTTON_PUSH) {
      NRF_LOG_INFO("Button push");
      err_code = app_timer_start(m_button_action, APP_TIMER_TICKS(BUTTON_STATE_POLL_INTERVAL_MS), NULL);
      APP_ERROR_CHECK(err_code);
    } else if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button released");
    }
    break;

  default:
    APP_ERROR_HANDLER(pin_no);
    break;
  }
}

static void buttons_init() {
  NRF_LOG_INFO("buttons_init()");

  uint32_t err_code;

  //The array must be static because a pointer to it will be saved in the button handler module.
  static app_button_cfg_t buttons[] =
      {
          {BUTTON_2, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_callback}};

  err_code = app_button_init(buttons, ARRAY_SIZE(buttons), BUTTON_DETECTION_DELAY);
  APP_ERROR_CHECK(err_code);

  err_code = app_button_enable();
  APP_ERROR_CHECK(err_code);

  /*Enable an app timer instance to detect long button press*/
  err_code = app_timer_create(&m_button_action, APP_TIMER_MODE_SINGLE_SHOT, button_timeout_handler);
  APP_ERROR_CHECK(err_code);
}

int main(void) {
  lfclk_config();
  log_init();
  timers_init();
  buttons_init();
  power_management_init();

  for (;;) {
    __WFE(); // Activate only for Debugging
    //idle_state_handle();  // Active if not Debugging
  }
}

/*
int main(void)
{
   bool button_pressed = false;

   nrf_gpio_cfg_output(LED_1);
   nrf_gpio_pin_clear(LED_1);

   nrf_gpio_cfg_output(LED_2);
   nrf_gpio_pin_clear(LED_2);

   nrf_gpio_cfg_output(LED_3);
   nrf_gpio_pin_clear(LED_3);

   nrf_gpio_cfg_input(BUTTON_1,NRF_GPIO_PIN_PULLUP);
   nrf_gpio_cfg_input(BUTTON_2,NRF_GPIO_PIN_PULLUP);


    while (true)
    { 
        button_pressed = nrf_gpio_pin_read(BUTTON_2);

        if(button_pressed == false)
        {
          //nrf_delay_us(100000);
          nrf_gpio_pin_write(LED_1, 1);
          nrf_gpio_pin_write(LED_2, 1);
          nrf_gpio_pin_write(LED_3, 1);
          // Do nothing.
        } else {
          nrf_gpio_pin_write(LED_1, 0);
          nrf_gpio_pin_write(LED_2, 0);
          nrf_gpio_pin_write(LED_3, 0);
        }
    }
}
*/