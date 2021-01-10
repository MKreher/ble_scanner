#include "test_c_w_cpp.h"

extern "C" {
  #include "nrf_log.h"
  #include "nrf_delay.h"
}

uint8_t TestCWithCpp::doSomething(uint16_t timeout)
{
    NRF_LOG_INFO("TestCWithCpp::doSomething() - START");
    
    nrf_delay_ms(timeout);

    NRF_LOG_INFO("TestCWithCpp::doSomething() - END");

    return 0;
}
