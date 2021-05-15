#include <stdint.h>
#include <string.h>

#define REPORT_LEN 8

uint8_t KEY_NULL                [REPORT_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_BS                  [REPORT_LEN] = {0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_HT                  [REPORT_LEN] = {0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_CR                  [REPORT_LEN] = {0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_ESC                 [REPORT_LEN] = {0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_0                   [REPORT_LEN] = {0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_1                   [REPORT_LEN] = {0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_2                   [REPORT_LEN] = {0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_3                   [REPORT_LEN] = {0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_4                   [REPORT_LEN] = {0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_5                   [REPORT_LEN] = {0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_6                   [REPORT_LEN] = {0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_7                   [REPORT_LEN] = {0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_8                   [REPORT_LEN] = {0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_9                   [REPORT_LEN] = {0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_SPACE               [REPORT_LEN] = {0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_EXCLAMATION_MARK    [REPORT_LEN] = {0x02, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_QUOTATION_MARK      [REPORT_LEN] = {0x02, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_NUMBER_SIGN         [REPORT_LEN] = {0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_DOLLAR_SIGN         [REPORT_LEN] = {0x02, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_PERCENT_SIGN        [REPORT_LEN] = {0x02, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_AMPERSAND           [REPORT_LEN] = {0x02, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_APOSTROPHE          [REPORT_LEN] = {0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_ROUND_BRACKET_LEFT  [REPORT_LEN] = {0x02, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_ROUND_BRACKET_RIGHT [REPORT_LEN] = {0x02, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_ASTERIX             [REPORT_LEN] = {0x02, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_PLUS_SIGN           [REPORT_LEN] = {0x02, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_COMMA               [REPORT_LEN] = {0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_HYPHEN              [REPORT_LEN] = {0x00, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_DOT                 [REPORT_LEN] = {0x00, 0x00, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_SLASH               [REPORT_LEN] = {0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_COLON               [REPORT_LEN] = {0x02, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_SEMICOLON           [REPORT_LEN] = {0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LESS_THAN_SIGN      [REPORT_LEN] = {0x02, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_EQUALS_SIGN         [REPORT_LEN] = {0x00, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_GREATER_THAN_SIGN   [REPORT_LEN] = {0x02, 0x00, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_QUESTION_MARK       [REPORT_LEN] = {0x02, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_AT_SIGN             [REPORT_LEN] = {0x02, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_A         [REPORT_LEN] = {0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_B         [REPORT_LEN] = {0x02, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_C         [REPORT_LEN] = {0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_D         [REPORT_LEN] = {0x02, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_E         [REPORT_LEN] = {0x02, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_F         [REPORT_LEN] = {0x02, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_G         [REPORT_LEN] = {0x02, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_H         [REPORT_LEN] = {0x02, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_I         [REPORT_LEN] = {0x02, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_J         [REPORT_LEN] = {0x02, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_K         [REPORT_LEN] = {0x02, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_L         [REPORT_LEN] = {0x02, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_M         [REPORT_LEN] = {0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_N         [REPORT_LEN] = {0x02, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_O         [REPORT_LEN] = {0x02, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_P         [REPORT_LEN] = {0x02, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_Q         [REPORT_LEN] = {0x02, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_R         [REPORT_LEN] = {0x02, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_S         [REPORT_LEN] = {0x02, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_T         [REPORT_LEN] = {0x02, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_U         [REPORT_LEN] = {0x02, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_V         [REPORT_LEN] = {0x02, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_W         [REPORT_LEN] = {0x02, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_X         [REPORT_LEN] = {0x02, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_Y         [REPORT_LEN] = {0x02, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UPPERCASE_Z         [REPORT_LEN] = {0x02, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_SQUARE_BRACKET_LEFT [REPORT_LEN] = {0x00, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_BACKSLASH           [REPORT_LEN] = {0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_SQUARE_BRACKET_RIGHT[REPORT_LEN] = {0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_CARET               [REPORT_LEN] = {0x02, 0x00, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_UNDERSCORE          [REPORT_LEN] = {0x02, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_GRACE_ACCENT        [REPORT_LEN] = {0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_A         [REPORT_LEN] = {0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_B         [REPORT_LEN] = {0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_C         [REPORT_LEN] = {0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_D         [REPORT_LEN] = {0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_E         [REPORT_LEN] = {0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_F         [REPORT_LEN] = {0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_G         [REPORT_LEN] = {0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_H         [REPORT_LEN] = {0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_I         [REPORT_LEN] = {0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_J         [REPORT_LEN] = {0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_K         [REPORT_LEN] = {0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_L         [REPORT_LEN] = {0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_M         [REPORT_LEN] = {0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_N         [REPORT_LEN] = {0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_O         [REPORT_LEN] = {0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_P         [REPORT_LEN] = {0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_Q         [REPORT_LEN] = {0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_R         [REPORT_LEN] = {0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_S         [REPORT_LEN] = {0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_T         [REPORT_LEN] = {0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_U         [REPORT_LEN] = {0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_V         [REPORT_LEN] = {0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_W         [REPORT_LEN] = {0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_X         [REPORT_LEN] = {0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_Y         [REPORT_LEN] = {0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_LOWERCASE_Z         [REPORT_LEN] = {0x00, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_CURLY_BRACKET_LEFT  [REPORT_LEN] = {0x02, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_VERTICAL_BAR        [REPORT_LEN] = {0x02, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_CURLY_BRACKET_RIGHT [REPORT_LEN] = {0x02, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t KEY_TILDE               [REPORT_LEN] = {0x02, 0x00, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00};



uint8_t* get_hid_report_for_char(const uint8_t p_char)
{
    
    switch (p_char) 
    {
        case 0  : return KEY_NULL                ; break;  // NULL     (Null character)
        case 1  : return NULL                    ; break;  // SOH      (Start of Header)           
        case 2  : return NULL                    ; break;  // STX      (Start of Text)         
        case 3  : return NULL                    ; break;  // ETX      (End of Text)           
        case 4  : return NULL                    ; break;  // EOT      (End of Transmission)           
        case 5  : return NULL                    ; break;  // ENQ      (Enquiry)           
        case 6  : return NULL                    ; break;  // ACK      (Acknowledgement)           
        case 7  : return NULL                    ; break;  // BEL      (Bell)          
        case 8  : return KEY_BS                  ; break;  // BS       (Backspace)         
        case 9  : return KEY_HT                  ; break;  // HT       (Horizontal Tab)            
        case 10 : return NULL                    ; break;  // LF       (Line feed)         
        case 11 : return NULL                    ; break;  // VT       (Vertical Tab)          
        case 12 : return NULL                    ; break;  // FF       (Form feed)         
        case 13 : return KEY_CR                  ; break;  // CR       (Carriage return)           
        case 14 : return NULL                    ; break;  // SO       (Shift Out)         
        case 15 : return NULL                    ; break;  // SI       (Shift In)          
        case 16 : return NULL                    ; break;  // DLE      (Data link escape)          
        case 17 : return NULL                    ; break;  // DC1      (Device control 1)          
        case 18 : return NULL                    ; break;  // DC2      (Device control 2)          
        case 19 : return NULL                    ; break;  // DC3      (Device control 3)          
        case 20 : return NULL                    ; break;  // DC4      (Device control 4)          
        case 21 : return NULL                    ; break;  // NAK      (Negative acknowledgement)          
        case 22 : return NULL                    ; break;  // SYN      (Synchronous idle)          
        case 23 : return NULL                    ; break;  // ETB      (End of transmission block)         
        case 24 : return NULL                    ; break;  // CAN      (Cancel)            
        case 25 : return NULL                    ; break;  // EM       (End of medium)         
        case 26 : return NULL                    ; break;  // SUB      (Substitute)            
        case 27 : return KEY_ESC                 ; break;  // ESC      (Escape)            
        case 28 : return NULL                    ; break;  // FS       (File separator)            
        case 29 : return NULL                    ; break;  // GS       (Group separator)           
        case 30 : return NULL                    ; break;  // RS       (Record separator)          
        case 31 : return NULL                    ; break;  // US       (Unit separator)            
        case 32 : return KEY_SPACE               ; break;  //          (space)         
        case 33 : return KEY_EXCLAMATION_MARK    ; break;  // !        (exclamation mark)          
        case 34 : return KEY_QUOTATION_MARK      ; break;  // "        (Quotation mark)            
        case 35 : return KEY_NUMBER_SIGN         ; break;  // #        (Number sign)           
        case 36 : return KEY_DOLLAR_SIGN         ; break;  // $        (Dollar sign)           
        case 37 : return KEY_PERCENT_SIGN        ; break;  // %        (Percent sign)          
        case 38 : return KEY_AMPERSAND           ; break;  // &        (Ampersand)         
        case 39 : return KEY_APOSTROPHE          ; break;  // '        (Apostrophe)            
        case 40 : return KEY_ROUND_BRACKET_LEFT  ; break;  // (        (round brackets or parentheses)         
        case 41 : return KEY_ROUND_BRACKET_RIGHT ; break;  // )        (round brackets or parentheses)         
        case 42 : return KEY_ASTERIX             ; break;  // *        (Asterisk)          
        case 43 : return KEY_PLUS_SIGN           ; break;  // +        (Plus sign)         
        case 44 : return KEY_COMMA               ; break;  // ,        (Comma)         
        case 45 : return KEY_HYPHEN              ; break;  // -        (Hyphen)            
        case 46 : return KEY_DOT                 ; break;  // .        (Full stop , dot)           
        case 47 : return KEY_SLASH               ; break;  // /        (Slash)         
        case 48 : return KEY_0                   ; break;  // 0        (number zero)           
        case 49 : return KEY_1                   ; break;  // 1        (number one)            
        case 50 : return KEY_2                   ; break;  // 2        (number two)            
        case 51 : return KEY_3                   ; break;  // 3        (number three)          
        case 52 : return KEY_4                   ; break;  // 4        (number four)           
        case 53 : return KEY_5                   ; break;  // 5        (number five)           
        case 54 : return KEY_6                   ; break;  // 6        (number six)            
        case 55 : return KEY_7                   ; break;  // 7        (number seven)          
        case 56 : return KEY_8                   ; break;  // 8        (number eight)          
        case 57 : return KEY_9                   ; break;  // 9        (number nine)           
        case 58 : return KEY_COLON               ; break;  // :        (Colon)         
        case 59 : return KEY_SEMICOLON           ; break;  // ;        (Semicolon)         
        case 60 : return KEY_LESS_THAN_SIGN      ; break;  // <        (Less-than sign )           
        case 61 : return KEY_EQUALS_SIGN         ; break;  // =        (Equals sign)           
        case 62 : return KEY_GREATER_THAN_SIGN   ; break;  // >        (Greater-than sign ; Inequality)            
        case 63 : return KEY_QUESTION_MARK       ; break;  // ?        (Question mark)         
        case 64 : return KEY_AT_SIGN             ; break;  // @        (At sign)
        case 65 : return KEY_UPPERCASE_A         ; break;  // A        (Capital A )            
        case 66 : return KEY_UPPERCASE_B         ; break;  // B        (Capital B )            
        case 67 : return KEY_UPPERCASE_C         ; break;  // C        (Capital C )            
        case 68 : return KEY_UPPERCASE_D         ; break;  // D        (Capital D )            
        case 69 : return KEY_UPPERCASE_E         ; break;  // E        (Capital E )            
        case 70 : return KEY_UPPERCASE_F         ; break;  // F        (Capital F )            
        case 71 : return KEY_UPPERCASE_G         ; break;  // G        (Capital G )            
        case 72 : return KEY_UPPERCASE_H         ; break;  // H        (Capital H )            
        case 73 : return KEY_UPPERCASE_I         ; break;  // I        (Capital I )            
        case 74 : return KEY_UPPERCASE_J         ; break;  // J        (Capital J )            
        case 75 : return KEY_UPPERCASE_K         ; break;  // K        (Capital K )            
        case 76 : return KEY_UPPERCASE_L         ; break;  // L        (Capital L )            
        case 77 : return KEY_UPPERCASE_M         ; break;  // M        (Capital M )            
        case 78 : return KEY_UPPERCASE_N         ; break;  // N        (Capital N )            
        case 79 : return KEY_UPPERCASE_O         ; break;  // O        (Capital O )            
        case 80 : return KEY_UPPERCASE_P         ; break;  // P        (Capital P )            
        case 81 : return KEY_UPPERCASE_Q         ; break;  // Q        (Capital Q )            
        case 82 : return KEY_UPPERCASE_R         ; break;  // R        (Capital R )            
        case 83 : return KEY_UPPERCASE_S         ; break;  // S        (Capital S )            
        case 84 : return KEY_UPPERCASE_T         ; break;  // T        (Capital T )            
        case 85 : return KEY_UPPERCASE_U         ; break;  // U        (Capital U )            
        case 86 : return KEY_UPPERCASE_V         ; break;  // V        (Capital V )            
        case 87 : return KEY_UPPERCASE_W         ; break;  // W        (Capital W )            
        case 88 : return KEY_UPPERCASE_X         ; break;  // X        (Capital X )            
        case 89 : return KEY_UPPERCASE_Y         ; break;  // Y        (Capital Y )            
        case 90 : return KEY_UPPERCASE_Z         ; break;  // Z        (Capital Z )  
        case 91 : return KEY_SQUARE_BRACKET_LEFT ; break;  // [        (square brackets or box brackets)           
        case 92 : return KEY_BACKSLASH           ; break;  // \        (Backslash)         
        case 93 : return KEY_SQUARE_BRACKET_RIGHT; break;  // ]        (square brackets or box brackets)           
        case 94 : return KEY_CARET               ; break;  // ^        (Caret or circumflex accent)            
        case 95 : return KEY_UNDERSCORE          ; break;  // _        (underscore , understrike , underbar or low line)           
        case 96 : return KEY_GRACE_ACCENT        ; break;  // `        (Grave accent)          
        case 97 : return KEY_LOWERCASE_A         ; break;  // a        (Lowercase  a )         
        case 98 : return KEY_LOWERCASE_B         ; break;  // b        (Lowercase  b )         
        case 99 : return KEY_LOWERCASE_C         ; break;  // c        (Lowercase  c )         
        case 100: return KEY_LOWERCASE_D         ; break;  // d        (Lowercase  d )         
        case 101: return KEY_LOWERCASE_E         ; break;  // e        (Lowercase  e )         
        case 102: return KEY_LOWERCASE_F         ; break;  // f        (Lowercase  f )         
        case 103: return KEY_LOWERCASE_G         ; break;  // g        (Lowercase  g )         
        case 104: return KEY_LOWERCASE_H         ; break;  // h        (Lowercase  h )         
        case 105: return KEY_LOWERCASE_I         ; break;  // i        (Lowercase  i )         
        case 106: return KEY_LOWERCASE_J         ; break;  // j        (Lowercase  j )         
        case 107: return KEY_LOWERCASE_K         ; break;  // k        (Lowercase  k )         
        case 108: return KEY_LOWERCASE_L         ; break;  // l        (Lowercase  l )         
        case 109: return KEY_LOWERCASE_M         ; break;  // m        (Lowercase  m )         
        case 110: return KEY_LOWERCASE_N         ; break;  // n        (Lowercase  n )         
        case 111: return KEY_LOWERCASE_O         ; break;  // o        (Lowercase  o )         
        case 112: return KEY_LOWERCASE_P         ; break;  // p        (Lowercase  p )         
        case 113: return KEY_LOWERCASE_Q         ; break;  // q        (Lowercase  q )         
        case 114: return KEY_LOWERCASE_R         ; break;  // r        (Lowercase  r )         
        case 115: return KEY_LOWERCASE_S         ; break;  // s        (Lowercase  s )         
        case 116: return KEY_LOWERCASE_T         ; break;  // t        (Lowercase  t )         
        case 117: return KEY_LOWERCASE_U         ; break;  // u        (Lowercase  u )         
        case 118: return KEY_LOWERCASE_V         ; break;  // v        (Lowercase  v )         
        case 119: return KEY_LOWERCASE_W         ; break;  // w        (Lowercase  w )         
        case 120: return KEY_LOWERCASE_X         ; break;  // x        (Lowercase  x )         
        case 121: return KEY_LOWERCASE_Y         ; break;  // y        (Lowercase  y )         
        case 122: return KEY_LOWERCASE_Z         ; break;  // z        (Lowercase  z )         
        case 123: return KEY_CURLY_BRACKET_LEFT  ; break;  // {        (curly brackets or braces)          
        case 124: return KEY_VERTICAL_BAR        ; break;  // |        (vertical-bar, vbar, vertical line or vertical slash)           
        case 125: return KEY_CURLY_BRACKET_RIGHT ; break;  // }        (curly brackets or braces)          
        case 126: return KEY_TILDE               ; break;  // ~        (Tilde ; swung dash)            
        case 127: return NULL                    ; break;  // DEL      (Delete)            
        case 128: return NULL                    ; break;  // Ç        (Majuscule C-cedilla )          
        case 129: return NULL                    ; break;  // ü        (letter "u" with umlaut or diaeresis ; "u-umlaut")          
        case 130: return NULL                    ; break;  // é        (letter "e" with acute accent or "e-acute")         
        case 131: return NULL                    ; break;  // â        (letter "a" with circumflex accent or "a-circumflex")           
        case 132: return NULL                    ; break;  // ä        (letter "a" with umlaut or diaeresis ; "a-umlaut")          
        case 133: return NULL                    ; break;  // à        (letter "a" with grave accent)          
        case 134: return NULL                    ; break;  // å        (letter "a"  with a ring)           
        case 135: return NULL                    ; break;  // ç        (Minuscule c-cedilla)           
        case 136: return NULL                    ; break;  // ê        (letter "e" with circumflex accent or "e-circumflex")           
        case 137: return NULL                    ; break;  // ë        (letter "e" with umlaut or diaeresis ; "e-umlaut")          
        case 138: return NULL                    ; break;  // è        (letter "e" with grave accent)          
        case 139: return NULL                    ; break;  // ï        (letter "i" with umlaut or diaeresis ; "i-umlaut")          
        case 140: return NULL                    ; break;  // î        (letter "i" with circumflex accent or "i-circumflex")           
        case 141: return NULL                    ; break;  // ì        (letter "i" with grave accent)          
        case 142: return NULL                    ; break;  // Ä        (letter "A" with umlaut or diaeresis ; "A-umlaut")          
        case 143: return NULL                    ; break;  // Å        (letter "A"  with a ring)           
        case 144: return NULL                    ; break;  // É        (Capital letter "E" with acute accent or "E-acute")         
        case 145: return NULL                    ; break;  // æ        (Latin diphthong "ae")          
        case 146: return NULL                    ; break;  // Æ        (Latin diphthong "AE")          
        case 147: return NULL                    ; break;  // ô        (letter "o" with circumflex accent or "o-circumflex")           
        case 148: return NULL                    ; break;  // ö        (letter "o" with umlaut or diaeresis ; "o-umlaut")          
        case 149: return NULL                    ; break;  // ò        (letter "o" with grave accent)          
        case 150: return NULL                    ; break;  // û        (letter "u" with circumflex accent or "u-circumflex")           
        case 151: return NULL                    ; break;  // ù        (letter "u" with grave accent)          
        case 152: return NULL                    ; break;  // ÿ        (letter "y" with diaeresis)         
        case 153: return NULL                    ; break;  // Ö        (letter "O" with umlaut or diaeresis ; "O-umlaut")          
        case 154: return NULL                    ; break;  // Ü        (letter "U" with umlaut or diaeresis ; "U-umlaut")          
        case 155: return NULL                    ; break;  // ø        (slashed zero or empty set)         
        case 156: return NULL                    ; break;  // £        (Pound sign ; symbol for the pound sterling)            
        case 157: return NULL                    ; break;  // Ø        (slashed zero or empty set)         
        case 158: return NULL                    ; break;  // ×        (multiplication sign)           
        case 159: return NULL                    ; break;  // ƒ        (function sign ; f with hook sign ; florin sign )           
        case 160: return NULL                    ; break;  // á        (letter "a" with acute accent or "a-acute")         
        case 161: return NULL                    ; break;  // í        (letter "i" with acute accent or "i-acute")         
        case 162: return NULL                    ; break;  // ó        (letter "o" with acute accent or "o-acute")         
        case 163: return NULL                    ; break;  // ú        (letter "u" with acute accent or "u-acute")         
        case 164: return NULL                    ; break;  // ñ        (letter "n" with tilde ; enye)          
        case 165: return NULL                    ; break;  // Ñ        (letter "N" with tilde ; enye)          
        case 166: return NULL                    ; break;  // ª        (feminine ordinal indicator )           
        case 167: return NULL                    ; break;  // º        (masculine ordinal indicator)           
        case 168: return NULL                    ; break;  // ¿        (Inverted question marks)           
        case 169: return NULL                    ; break;  // ®        (Registered trademark symbol)           
        case 170: return NULL                    ; break;  // ¬        (Logical negation symbol)           
        case 171: return NULL                    ; break;  // ½        (One half)          
        case 172: return NULL                    ; break;  // ¼        (Quarter or  one fourth)            
        case 173: return NULL                    ; break;  // ¡        (Inverted exclamation marks)            
        case 174: return NULL                    ; break;  // «        (Guillemets or  angle quotes)           
        case 175: return NULL                    ; break;  // »        (Guillemets or  angle quotes)           
        case 176: return NULL                    ; break;  // ░                    
        case 177: return NULL                    ; break;  // ▒                    
        case 178: return NULL                    ; break;  // ▓                    
        case 179: return NULL                    ; break;  // │        (Box drawing character)         
        case 180: return NULL                    ; break;  // ┤        (Box drawing character)         
        case 181: return NULL                    ; break;  // Á        (Capital letter "A" with acute accent or "A-acute")         
        case 182: return NULL                    ; break;  // Â        (letter "A" with circumflex accent or "A-circumflex")           
        case 183: return NULL                    ; break;  // À        (letter "A" with grave accent)          
        case 184: return NULL                    ; break;  // ©        (Copyright symbol)          
        case 185: return NULL                    ; break;  // ╣        (Box drawing character)         
        case 186: return NULL                    ; break;  // ║        (Box drawing character)         
        case 187: return NULL                    ; break;  // ╗        (Box drawing character)         
        case 188: return NULL                    ; break;  // ╝        (Box drawing character)         
        case 189: return NULL                    ; break;  // ¢        (Cent symbol)           
        case 190: return NULL                    ; break;  // ¥        (YEN and YUAN sign)         
        case 191: return NULL                    ; break;  // ┐        (Box drawing character)         
        case 192: return NULL                    ; break;  // └        (Box drawing character)         
        case 193: return NULL                    ; break;  // ┴        (Box drawing character)         
        case 194: return NULL                    ; break;  // ┬        (Box drawing character)         
        case 195: return NULL                    ; break;  // ├        (Box drawing character)         
        case 196: return NULL                    ; break;  // ─        (Box drawing character)         
        case 197: return NULL                    ; break;  // ┼        (Box drawing character)         
        case 198: return NULL                    ; break;  // ã        (letter "a" with tilde or "a-tilde")            
        case 199: return NULL                    ; break;  // Ã        (letter "A" with tilde or "A-tilde")            
        case 200: return NULL                    ; break;  // ╚        (Box drawing character)         
        case 201: return NULL                    ; break;  // ╔        (Box drawing character)         
        case 202: return NULL                    ; break;  // ╩        (Box drawing character)         
        case 203: return NULL                    ; break;  // ╦        (Box drawing character)         
        case 204: return NULL                    ; break;  // ╠        (Box drawing character)         
        case 205: return NULL                    ; break;  // ═        (Box drawing character)         
        case 206: return NULL                    ; break;  // ╬        (Box drawing character)         
        case 207: return NULL                    ; break;  // ¤        (generic currency sign )            
        case 208: return NULL                    ; break;  // ð        (lowercase "eth")           
        case 209: return NULL                    ; break;  // Ð        (Capital letter "Eth")          
        case 210: return NULL                    ; break;  // Ê        (letter "E" with circumflex accent or "E-circumflex")           
        case 211: return NULL                    ; break;  // Ë        (letter "E" with umlaut or diaeresis ; "E-umlaut")          
        case 212: return NULL                    ; break;  // È        (letter "E" with grave accent)          
        case 213: return NULL                    ; break;  // ı        (lowercase dot less i)          
        case 214: return NULL                    ; break;  // Í        (Capital letter "I" with acute accent or "I-acute")         
        case 215: return NULL                    ; break;  // Î        (letter "I" with circumflex accent or "I-circumflex")           
        case 216: return NULL                    ; break;  // Ï        (letter "I" with umlaut or diaeresis ; "I-umlaut")          
        case 217: return NULL                    ; break;  // ┘        (Box drawing character)         
        case 218: return NULL                    ; break;  // ┌        (Box drawing character)         
        case 219: return NULL                    ; break;  // █        (Block)         
        case 220: return NULL                    ; break;  // ▄                    
        case 221: return NULL                    ; break;  // ¦        (vertical broken bar )          
        case 222: return NULL                    ; break;  // Ì        (letter "I" with grave accent)          
        case 223: return NULL                    ; break;  // ▀                    
        case 224: return NULL                    ; break;  // Ó        (Capital letter "O" with acute accent or "O-acute")         
        case 225: return NULL                    ; break;  // ß        (letter "Eszett" ; "scharfes S" or "sharp S")           
        case 226: return NULL                    ; break;  // Ô        (letter "O" with circumflex accent or "O-circumflex")           
        case 227: return NULL                    ; break;  // Ò        (letter "O" with grave accent)          
        case 228: return NULL                    ; break;  // õ        (letter "o" with tilde or "o-tilde")            
        case 229: return NULL                    ; break;  // Õ        (letter "O" with tilde or "O-tilde")            
        case 230: return NULL                    ; break;  // µ        (Lowercase letter "Mu" ; micro sign or micron)          
        case 231: return NULL                    ; break;  // þ        (capital letter "Thorn")            
        case 232: return NULL                    ; break;  // Þ        (lowercase letter "thorn")          
        case 233: return NULL                    ; break;  // Ú        (Capital letter "U" with acute accent or "U-acute")         
        case 234: return NULL                    ; break;  // Û        (letter "U" with circumflex accent or "U-circumflex")           
        case 235: return NULL                    ; break;  // Ù        (letter "U" with grave accent)          
        case 236: return NULL                    ; break;  // ý        (letter "y" with acute accent)          
        case 237: return NULL                    ; break;  // Ý        (Capital letter "Y" with acute accent)          
        case 238: return NULL                    ; break;  // ¯        (macron symbol)         
        case 239: return NULL                    ; break;  // ´        (Acute accent)          
        case 240: return NULL                    ; break;  // ¬        (Hyphen)            
        case 241: return NULL                    ; break;  // ±        (Plus-minus sign)           
        case 242: return NULL                    ; break;  // ‗        (underline or underscore)           
        case 243: return NULL                    ; break;  // ¾        (three quarters)            
        case 244: return NULL                    ; break;  // ¶        (paragraph sign or pilcrow)         
        case 245: return NULL                    ; break;  // §        (Section sign)          
        case 246: return NULL                    ; break;  // ÷        (The division sign ; Obelus)            
        case 247: return NULL                    ; break;  // ¸        (cedilla)           
        case 248: return NULL                    ; break;  // °        (degree symbol )            
        case 249: return NULL                    ; break;  // ¨        (Diaeresis)         
        case 250: return NULL                    ; break;  // •        (Interpunct or space dot)           
        case 251: return NULL                    ; break;  // ¹        (superscript one)           
        case 252: return NULL                    ; break;  // ³        (cube or superscript three)         
        case 253: return NULL                    ; break;  // ²        (Square or superscript two)         
        case 254: return NULL                    ; break;  // ■        (black square)          
        case 255: return NULL                    ; break;  // nbsp    (non-breaking space or no-break space)          
        default : return NULL                    ; break;
    }
}