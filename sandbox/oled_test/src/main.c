#include <stdbool.h>
#include <stdint.h>

#include "app_button.h"
#include "app_timer.h"
#include "app_error.h"
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
#include "app_util_platform.h"

#include "myssd1351.h"

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

static void led_on(uint8_t led_pin) {
  NRF_LOG_INFO("LED ON");
  bsp_board_led_on(bsp_board_pin_to_led_idx(led_pin));
}

static void led_off(uint8_t led_pin) {
  NRF_LOG_INFO("LED OFF");
  bsp_board_led_off(bsp_board_pin_to_led_idx(led_pin));
}

static void button_callback(uint8_t pin_no, uint8_t button_action) {
  ret_code_t err_code;

  switch (pin_no) {
  case BUTTON_1:
    if (button_action == APP_BUTTON_PUSH) {
      NRF_LOG_INFO("Button #1 push");
      led_on(LED_1);
    } else if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button #1 released");
      led_off(LED_1);
    }
    break;

  case BUTTON_2:
    if (button_action == APP_BUTTON_PUSH) {
      NRF_LOG_INFO("Button #2 push");
      led_on(LED_2);
    } else if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button #2 released");
      led_off(LED_2);
    }
    break;

  case BUTTON_3:
    if (button_action == APP_BUTTON_PUSH) {
      NRF_LOG_INFO("Button #3 push");
      led_on(LED_3);
    } else if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button #3 released");
      led_off(LED_3);
    }
    break;

  case BUTTON_4:
    if (button_action == APP_BUTTON_PUSH) {
      NRF_LOG_INFO("Button #4 push");
      led_on(LED_4);
    } else if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button #4 released");
      led_off(LED_4);
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
          {BUTTON_1, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_callback}
         ,{BUTTON_2, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_callback}
         ,{BUTTON_3, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_callback}
         ,{BUTTON_4, APP_BUTTON_ACTIVE_LOW, false, NRF_GPIO_PIN_PULLUP, button_callback}
      };

  err_code = app_button_init(buttons, ARRAY_SIZE(buttons), BUTTON_DETECTION_DELAY);
  APP_ERROR_CHECK(err_code);

  err_code = app_button_enable();
  APP_ERROR_CHECK(err_code);
}


/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init()
{
  NRF_LOG_INFO("leds_init()");
  bsp_board_init(BSP_INIT_LEDS);
}


static void display_init()
{
  NRF_LOG_INFO("display_init()");
  SSD1351_init();
  SSD1351_fill(COLOR_BLACK);
  SSD1351_update();
  nrf_delay_ms(3000);
  SSD1351_set_cursor(0, 0);
  SSD1351_printf(SSD1351_get_rgb(245, 255, 20), med_font, "Hello worldI spent \n%i %s\n", 15, "Euros");
  SSD1351_update();
  nrf_delay_ms(3000);
  SSD1351_printf(COLOR_RED, small_font, "\nIn this screen");
  SSD1351_update();
  nrf_delay_ms(3000);
  SSD1351_printf(SSD1351_get_rgb(245, 255, 20), big_font, "\nnRF52840");
  SSD1351_update();
  nrf_delay_ms(3000);
  SSD1351_draw_filled_rect( 64 - 1/2, 64 - 1/2, 1, 1, 0x1111 + rand());
  SSD1351_update();
  //nrf_delay_ms(3000);
  //SSD1351_stop();
}

/**
 * Main function
 */
int main(void) {
  log_init();
  lfclk_config();
  timers_init();
  buttons_init();
  leds_init();
  display_init();
  power_management_init();
  
  NRF_LOG_INFO("Initialization finished.");

  for (;;) {
    //__WFE(); // Activate only for Debugging
    idle_state_handle();  // Active if not Debugging
  }
}
