#define CMD_ACK_OPCODE  0x<D0;
#define CMD_NACK_OPCODE 0xD1;

// comannds from host to the scan engine
char CMD_ACK[] = { 0x04, 0xD0, 0x04, 0x00, 0xFF, 0x28 };
char CMD_NACK_RESEND[] = { 0x05, 0xD1, 0x04, 0x00, 0x01, 0xFF, 0x25 };
char CMD_NACK_BAD_CONTEXT[] = { 0x05, 0xD1, 0x04, 0x00, 0x02, 0xFF, 0x24 };
char CMD_NACK_DENIED[] = { 0x05, 0xD1, 0x04, 0x00, 0x06, 0xFF, 0x20 };

char CMD_DECODE_DATE[] = { 0x00 };

char CMD_LED_OFF[] =  { 0x05, 0xE8, 0x04, 0x00, 0x01, 0xFF, 0x0E };
char CMD_LED_ON[] =  { 0x05, 0xE7, 0x04, 0x00, 0x01, 0xFF, 0x0F };

char CMD_REQUEST_REVISION[] =  { 0x04, 0xA3, 0x04, 0x00, 0xFF, 0x55 };
char CMD_REPLY_REVISION[] = { 0x00 };

char CMD_SCAN_DISABLE[] =  { 0x04, 0xEA, 0x04, 0x00, 0xFF, 0x0E };
char CMD_SCAN_ENABLE[] =  { 0x04, 0xE9, 0x04, 0x00, 0xFF, 0x0F };

char CMD_START_DECODE[] =  { 0x04, 0xE4, 0x04, 0x00, 0xFF, 0x14 };
char CMD_STOP_DECODE[] =  { 0x04, 0xE5, 0x04, 0x00, 0xFF, 0x13 };

char CMD_WAKEUP[] = { 0x00 };
char CMD_SLEEP[] =  { 0x04, 0xEB, 0x04, 0x00, 0xFF, 0x0D };
char CMD_RESET[] =  { 0x04, 0xFA, 0x04, 0x00, 0xFE, 0xFE };

char CMD_PARAM_SET_DEFAULTS[] = { 0x04, 0xC8, 0x04, 0x00, 0xFF, 0x30 };

char CMD_PARAM_GET_POWER_MODE[] = { 0x05, 0xC7, 0x04, 0x00, 0x80, 0xFE, 0xB0 };
char CMD_PARAM_SET_POWER_MODE_CONTINUOUS[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x80, 0x00, 0xFE, 0xA7 };
char CMD_PARAM_SET_POWER_MODE_LOW[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x80, 0x01, 0xFE, 0xA6 };

char CMD_PARAM_GET_TRIGGER_MODE[] = { 0x05, 0xC7, 0x04, 0x00, 0x8A, 0xFE, 0xA6 };
char CMD_PARAM_SET_TRIGGER_MODE_LEVEL[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x00, 0xFE, 0x9D };
char CMD_PARAM_SET_TRIGGER_MODE_PULSE[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x02, 0xFE, 0x9B };
char CMD_PARAM_SET_TRIGGER_MODE_CONTINUOUS[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x04, 0xFE, 0x99 };
char CMD_PARAM_SET_TRIGGER_MODE_HOST[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x08, 0xFE, 0x95 };
char CMD_PARAM_SET_TRIGGER_MODE_AUTOMATIC_INDUCTION[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x09, 0xFE, 0x94 };
char CMD_PARAM_SET_TRIGGER_MODE_BUTTON_CONTINUOUS[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x8A, 0x0A, 0xFE, 0x93 };

char CMD_PARAM_GET_BAUD_RATE[] = { 0x05, 0xC7, 0x04, 0x00, 0x9C, 0xFE, 0x94 };

char CMD_PARAM_GET_COMMUNICATION_MODE[] = { 0x06, 0xC7, 0x04, 0x00, 0xF2, 0x01, 0xFE, 0x3C };

char CMD_PARAM_GET_SOFTWARE_HANDSHAKING[] = { 0x05, 0xC7, 0x04, 0x00, 0x9F, 0xFE, 0x91 };
char CMD_PARAM_SET_SOFTWARE_HANDSHAKING_ENABLE[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x9F, 0x01, 0xFE, 0x87 };
char CMD_PARAM_SET_SOFTWARE_HANDSHAKING_DISABLE[] = { 0x07, 0xC6, 0x04, 0x08, 0x00, 0x9F, 0x00, 0xFE, 0x88 };

// response messages from scan engine
