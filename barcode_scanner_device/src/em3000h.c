#include "app_error.h"
#include "app_timer.h"

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
#include "nrf_serial.h"
#include <stdint.h>
#include <string.h>

/**@brief Function for calculating the 2's complement checksum for the given content.
 */
static uint16_t calc_checksum(const char * str, const uint8_t len)
{
    uint16_t checksum, i, sum = 0;

    for (i = 0; i < len; i++)
    {
        sum += str[i];
    }

    NRF_LOG_INFO("calc_checksum: sum is %x", sum);

    checksum = ~sum + 1;

    NRF_LOG_INFO("calc_checksum: checksum is %x", checksum);

    return checksum;
}
