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

#include "epd1in54.h"
#include "epdpaint.h"
#include "imagedata.h"

#define COLORED     0
#define UNCOLORED   1

// Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks).
#define BUTTON_DETECTION_DELAY APP_TIMER_TICKS(50)

/**
  * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
  * In this case, a smaller image buffer is allocated and you have to 
  * update a partial display several times.
  * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
  */
unsigned char image[1024];
Paint paint(image, 0, 0);    // width should be the multiple of 8 
Epd epd;

/**@brief Function for init display.
 */
static void display_init(void) {
  uint32_t err_code;
  //err_code = epd.Init(lut_full_update);
  err_code = epd.HDirInit(lut_full_update);
  APP_ERROR_CHECK(err_code);
  
  NRF_LOG_INFO("e-Paper init successfully");
}

/**@brief Function for display something.
 */
static void display(void) {

  /** 
   *  there are 2 memory areas embedded in the e-paper display
   *  and once the display is refreshed, the memory area will be auto-toggled,
   *  i.e. the next action of SetFrameMemory will set the other memory area
   *  therefore you have to clear the frame memory twice.
   */
  epd.Clear();

  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();

  paint.SetRotate(ROTATE_0);
  paint.SetWidth(200);
  paint.SetHeight(24);

  /* For simplicity, the arguments are explicit numerical coordinates */
  NRF_LOG_INFO("Display cleared");
  paint.DrawStringAt(30, 4, "Hello world!", &Font16, UNCOLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 10, paint.GetWidth(), paint.GetHeight());
  
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(30, 4, "e-Paper Demo", &Font16, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 30, paint.GetWidth(), paint.GetHeight());

  paint.SetWidth(64);
  paint.SetHeight(64);
  
  paint.Clear(UNCOLORED);
  paint.DrawRectangle(0, 0, 40, 50, COLORED);
  paint.DrawLine(0, 0, 40, 50, COLORED);
  paint.DrawLine(40, 0, 0, 50, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 16, 60, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawCircle(32, 32, 30, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 120, 60, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledRectangle(0, 0, 40, 50, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 16, 130, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledCircle(32, 32, 30, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 120, 130, paint.GetWidth(), paint.GetHeight());
  epd.DisplayFrame();
  epd.Sleep();
}

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
      display();
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

  for (;;) {
    //__WFE(); // Activate only for Debugging
    idle_state_handle();  // Active if not Debugging
  }
}
