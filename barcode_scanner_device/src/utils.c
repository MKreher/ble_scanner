#include "app_error.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "app_util_platform.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include <stdint.h>
#include <string.h>

#include "utils.h"

#define TICKS_PER_MILLISECOND APP_TIMER_TICKS(1) // Factor for converting ticks to milliseconds

APP_TIMER_DEF(m_non_blocking_delay_timer_id);


static bool m_is_non_blocking_delay_wait = false;


void stop_non_blocking_delay()
{
    //NRF_LOG_INFO("stop_non_blocking_delay_timer()");
    
    APP_ERROR_CHECK(app_timer_stop(m_non_blocking_delay_timer_id));

    m_is_non_blocking_delay_wait = false;
}


void non_blocking_delay_timeout_handler()
{
    //NRF_LOG_INFO("non_blocking_deleay_timeout_handler()");

    m_is_non_blocking_delay_wait = false;
}


void non_blocking_delay_ms(uint64_t delay)
{
    //NRF_LOG_INFO("non_blocking_delay_ms(): START - %d ms", delay);

    m_is_non_blocking_delay_wait = true;

    // timer for non blocking delays
    ret_code_t ret_code = app_timer_create(&m_non_blocking_delay_timer_id, APP_TIMER_MODE_SINGLE_SHOT, non_blocking_delay_timeout_handler);
    APP_ERROR_CHECK(ret_code);



    APP_ERROR_CHECK(app_timer_start(m_non_blocking_delay_timer_id, APP_TIMER_TICKS(delay), NULL));

    uint32_t timeStart = app_timer_cnt_get();

    while (m_is_non_blocking_delay_wait)
    {
        //NRF_LOG_INFO("Wait for is_non_blocking_delay is getting FALSE...");
        if (NRF_LOG_PROCESS() == false)
        {
            nrf_pwr_mgmt_run();
        }
    }    

    uint32_t timeEnd = app_timer_cnt_get();
    uint32_t timeDiff = app_timer_cnt_diff_compute(timeEnd, timeStart);	

    //NRF_LOG_INFO("non_blocking_delay_ms(): END - %d ms (real wait time %d)", delay, timeDiff / TICKS_PER_MILLISECOND);
}


void utils_init()
{
    // timer for non blocking delays
    ret_code_t ret_code = app_timer_create(&m_non_blocking_delay_timer_id, APP_TIMER_MODE_SINGLE_SHOT, non_blocking_delay_timeout_handler);
    APP_ERROR_CHECK(ret_code);
}
