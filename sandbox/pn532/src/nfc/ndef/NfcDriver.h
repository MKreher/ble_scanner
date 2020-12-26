// eventually the NFC drivers should extend this class
class NfcDriver
{
    public:
        virtual NfcTag read(uint8_t * uid, uint16_t uidLength) = 0;
        virtual boolean write(NdefMessage& message, uint8_t * uid, uint16_t uidLength) = 0;
        // erase()
        // format()
}