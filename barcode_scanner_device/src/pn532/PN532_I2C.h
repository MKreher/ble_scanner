/**
 * @modified picospuch
 */

#ifndef __PN532_I2C_H__
#define __PN532_I2C_H__

extern "C" {
  #include "nrf_drv_twi.h"
  #include "nrf_log.h"
}

#include "PN532Interface.h"

#define PN532_I2C_ADDRESS       (0x48 >> 1)

class PN532_I2C : public PN532Interface {
public:
    PN532_I2C(nrf_drv_twi_t twi_master);
    
    void begin();
    void wakeup();
    virtual int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);
    int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);
    
private:
    nrf_drv_twi_t m_twi_master;
    uint8_t command;
    
    int8_t readAckFrame();
    int16_t getResponseLength(uint8_t buf[], uint8_t len, uint16_t timeout);
    
    inline uint8_t write(uint8_t data) {
        NRF_LOG_INFO("Sending command");
        NRF_LOG_HEXDUMP_INFO(&data, sizeof(data));

        ret_code_t err_code = nrf_drv_twi_tx(&m_twi_master,
                                             PN532_I2C_ADDRESS,
                                             &data,
                                             sizeof(data),
                                             false);

        if (err_code != NRF_SUCCESS)
        {
            NRF_LOG_INFO("Failed while calling TWI tx, err_code = %d", err_code);
        }

        return err_code;
    }
    
    inline uint8_t read() {       
        const uint8_t bytes_to_read = 1;
        
        // for I2C, read the additional status byte.
        NRF_LOG_INFO("Reading (%d bytes): ", bytes_to_read + 1);
        
        uint8_t *rx_buffer;

        ret_code_t err_code;
        err_code = nrf_drv_twi_rx(&m_twi_master, PN532_I2C_ADDRESS, rx_buffer, bytes_to_read + 1);
       
        if (err_code != NRF_SUCCESS)
        {
            NRF_LOG_INFO("Failed while calling TWI rx, err_code = %d", err_code);
            APP_ERROR_HANDLER(err_code);
        }

        uint8_t byte_read;

        memcpy(&byte_read, rx_buffer + 1, bytes_to_read);

        NRF_LOG_HEXDUMP_INFO(&byte_read, bytes_to_read);

        return byte_read;
    }
};

#endif
