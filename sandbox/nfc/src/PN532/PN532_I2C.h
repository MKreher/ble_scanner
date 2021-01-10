/**
 * @modified picospuch
 */

#ifndef __PN532_I2C_H__
#define __PN532_I2C_H__

extern "C" {
  #include "nrf_drv_twi.h"
}

#include "PN532Interface.h"

class PN532_I2C : public PN532Interface {
public:
    PN532_I2C(nrf_drv_twi_t &twi_master);
    
    void begin();
    void wakeup();
    virtual int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);
    int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);
    
private:
    nrf_drv_twi_t* m_twi_master;
    uint8_t command;
    
    int8_t readAckFrame();
    int16_t getResponseLength(uint8_t buf[], uint8_t len, uint16_t timeout);
    
    inline uint8_t write(uint8_t data) {
        //TODO: MKR migrate
        return 0;
        //return m_twi_master->send(data);
    }
    
    inline uint8_t read() {
        //TODO: MKR migrate
        return 0;
        //m_twi_master->receive();
    }
};

#endif
