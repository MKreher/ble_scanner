/**
 * @modified picospuch
 */
extern "C" {
  #include "nrf_delay.h"
  #include "nrf_log.h"
}

#include "PN532_I2C.h"

#define PN532_I2C_ADDRESS       (0x48 >> 1)

PN532_I2C::PN532_I2C(nrf_drv_twi_t &twi_master)
{
    *m_twi_master = twi_master;
    command = 0;
}

void PN532_I2C::begin()
{
    //TODO: MKR init TWI here
    //_wire->begin();
}

void PN532_I2C::wakeup()
{
    nrf_delay_ms(500); // wait for all ready to manipulate pn532
}

int8_t PN532_I2C::writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    command = header[0];
    //_wire->beginTransmission(PN532_I2C_ADDRESS); TODO: MKR ???
    
    write(PN532_PREAMBLE);
    write(PN532_STARTCODE1);
    write(PN532_STARTCODE2);
    
    uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
    write(length);
    write(~length + 1);                 // checksum of length
    
    write(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA
    
    NRF_LOG_INFO("write: ");
       
    for (uint8_t i = 0; i < hlen; i++) {
        if (write(header[i])) {
            sum += header[i];
            
            NRF_LOG_INFO("0x%X", header[i]);
        } else {
            NRF_LOG_INFO("\nToo many data to send, I2C doesn't support such a big packet\n");     // I2C max packet: 32 bytes
            return PN532_INVALID_FRAME;
        }
    }

    for (uint8_t i = 0; i < blen; i++) {
        if (write(body[i])) {
            sum += body[i];
            
            NRF_LOG_INFO("0x%X", body[i]);
        } else {
            NRF_LOG_INFO("\nToo many data to send, I2C doesn't support such a big packet\n");     // I2C max packet: 32 bytes
            return PN532_INVALID_FRAME;
        }
    }
  
    uint8_t checksum = ~sum + 1;            // checksum of TFI + DATA
    write(checksum);
    write(PN532_POSTAMBLE);
    
    //_wire->endTransmission(); TODO: MKR ???
    
    NRF_LOG_INFO("\n");

    return readAckFrame();
}

int16_t PN532_I2C::getResponseLength(uint8_t buf[], uint8_t len, uint16_t timeout) {
    const uint8_t PN532_NACK[] = {0, 0, 0xFF, 0xFF, 0, 0};
    uint16_t time = 0;

    do {
//        if (_wire->requestFrom(PN532_I2C_ADDRESS, 6)) { TODO: MKR ???
            if (read() & 1) {  // check first byte --- status
                break;         // PN532 is ready
            }
//        }

        nrf_delay_ms(1);
        time++;
        if ((0 != timeout) && (time > timeout)) {
            return -1;
        }
    } while (1); 
    
    if (0x00 != read()      ||       // PREAMBLE
            0x00 != read()  ||       // STARTCODE1
            0xFF != read()           // STARTCODE2
        ) {
        
        return PN532_INVALID_FRAME;
    }
    
    uint8_t length = read();

    // request for last respond msg again
    //_wire->beginTransmission(PN532_I2C_ADDRESS); TODO: MKR ???
    for (uint16_t i = 0; i < sizeof(PN532_NACK); ++i) {
      write(PN532_NACK[i]);
    }
    //_wire->endTransmission(); TODO: MKR ???

    return length;
}

int16_t PN532_I2C::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint16_t time = 0;
    uint8_t length;

    length = getResponseLength(buf, len, timeout);

    // [RDY] 00 00 FF LEN LCS (TFI PD0 ... PDn) DCS 00
    do {
        //if (_wire->requestFrom(PN532_I2C_ADDRESS, 6 + length + 2)) { TODO: MKR ???
            if (read() & 1) {  // check first byte --- status
                break;         // PN532 is ready
            }
        //}

        nrf_delay_ms(1);
        time++;
        if ((0 != timeout) && (time > timeout)) {
            return -1;
        }
    } while (1); 
    
    if (0x00 != read()      ||       // PREAMBLE
            0x00 != read()  ||       // STARTCODE1
            0xFF != read()           // STARTCODE2
        ) {
        
        return PN532_INVALID_FRAME;
    }
    
    length = read();

    if (0 != (uint8_t)(length + read())) {   // checksum of length
        return PN532_INVALID_FRAME;
    }
    
    uint8_t cmd = command + 1;               // response command
    if (PN532_PN532TOHOST != read() || (cmd) != read()) {
        return PN532_INVALID_FRAME;
    }
    
    length -= 2;
    if (length > len) {
        return PN532_NO_SPACE;  // not enough space
    }
    
    NRF_LOG_INFO("read:  ");
    NRF_LOG_INFO("0x%X", cmd);
    
    uint8_t sum = PN532_PN532TOHOST + cmd;
    for (uint8_t i = 0; i < length; i++) {
        buf[i] = read();
        sum += buf[i];
        
        NRF_LOG_INFO("0x%X", buf[i]);
    }
    NRF_LOG_INFO("\n");
    
    uint8_t checksum = read();
    if (0 != (uint8_t)(sum + checksum)) {
        NRF_LOG_INFO("checksum is not ok\n");
        return PN532_INVALID_FRAME;
    }
    read();         // POSTAMBLE
    
    return length;
}

int8_t PN532_I2C::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ackBuf[sizeof(PN532_ACK)];
    
    //TODO: MKR umstellen auf Timer

    NRF_LOG_INFO("wait for ack at");
    NRF_LOG_INFO("\n");
    
    uint16_t time = 0;
    do {
        //if (_wire->requestFrom(PN532_I2C_ADDRESS,  sizeof(PN532_ACK) + 1)) { TODO: MKR ???
            if (read() & 1) {  // check first byte --- status
                break;         // PN532 is ready
            }
        //}

        nrf_delay_ms(1);
        time++;
        if (time > PN532_ACK_WAIT_TIME) {
            NRF_LOG_INFO("Time out when waiting for ACK\n");
            return PN532_TIMEOUT;
        }
    } while (1); 
    
    NRF_LOG_INFO("ready after %d millis", time);
    NRF_LOG_INFO("\n");
    

    for (uint8_t i = 0; i < sizeof(PN532_ACK); i++) {
        ackBuf[i] = read();
    }
    
    if (memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK))) {
        NRF_LOG_INFO("Invalid ACK\n");
        return PN532_INVALID_ACK;
    }
    
    return 0;
}
