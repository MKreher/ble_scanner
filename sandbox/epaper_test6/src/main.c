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

// Waveshare ePaper
#include "nrf_gfx.h"
#include "waveshare_epd.h"
#include "ImageData.h"

static uint8_t disptxt_buffer[16] = "Hello World";
static uint8_t epaper_pending;

static const nrf_lcd_t * p_lcd = &nrf_lcd_wsepd154;
extern const nrf_gfx_font_desc_t orkney_24ptFontInfo;
static const nrf_gfx_font_desc_t * p_font = &orkney_24ptFontInfo;

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

static void epaper_demo_clear(void)
{
    NRF_LOG_INFO("Display clear.");
    APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
    nrf_gfx_screen_fill(p_lcd, 0xff);
    //nrf_gfx_display(p_lcd);

    nrf_gfx_rotation_set(p_lcd, NRF_LCD_ROTATE_180);
    wsepd154_draw_monobmp((const uint8_t *)gImage_1in54);
    nrf_gfx_display(p_lcd);

    nrf_gfx_uninit(p_lcd);
}

static void epaper_demo_text(void)
{
    NRF_LOG_INFO("Display text.");
    //APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
    nrf_gfx_rotation_set(p_lcd, NRF_LCD_ROTATE_180);
    nrf_gfx_point_t text_start = NRF_GFX_POINT(15, 70);
    nrf_gfx_screen_fill(p_lcd, 0xff);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &text_start, 0x00, (const char *)disptxt_buffer, p_font, true));
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

static void button_callback(uint8_t pin_no, uint8_t button_action) {
  ret_code_t err_code;

  switch (pin_no) {
  case BUTTON_1:
    if (button_action == APP_BUTTON_PUSH) {
      NRF_LOG_INFO("Button #1 push");
      led_on(LED_1);
      epaper_demo_text();
    } else if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button #1 released");
      led_off(LED_1);
    }
    break;

  case BUTTON_2:
    if (button_action == APP_BUTTON_PUSH) {
      NRF_LOG_INFO("Button #2 push");
      led_on(LED_2);
      epaper_demo_draw();
    } else if (button_action == APP_BUTTON_RELEASE) {
      NRF_LOG_INFO("Button #2 released");
      led_off(LED_2);
    }
    break;

  case BUTTON_3:
    if (button_action == APP_BUTTON_PUSH) {
      NRF_LOG_INFO("Button #3 push");
      led_on(LED_3);
      epaper_demo_imarray();
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
  epaper_demo_clear();
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
