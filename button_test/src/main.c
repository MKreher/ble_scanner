#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nordic_common.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

/**
 * @brief Function for application main entry.
 */
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


    while (true)
    { 
        button_pressed = nrf_gpio_pin_read(BUTTON_1);

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
/** @} */
