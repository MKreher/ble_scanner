// Waveshare EPaper Display
// Implementation for Nordic nRF5x GFX Library

#include "sdk_common.h"

#include "nrf_lcd.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "app_scheduler.h"

#include "utils.h"

// EPD1IN54 commands
#define DRIVER_OUTPUT_CONTROL                       0x01
#define BOOSTER_SOFT_START_CONTROL                  0x0C
#define GATE_SCAN_START_POSITION                    0x0F
#define DEEP_SLEEP_MODE                             0x10
#define DATA_ENTRY_MODE_SETTING                     0x11
#define SW_RESET                                    0x12
#define TEMPERATURE_SENSOR_CONTROL                  0x1A
#define MASTER_ACTIVATION                           0x20
#define DISPLAY_UPDATE_CONTROL_1                    0x21
#define DISPLAY_UPDATE_CONTROL_2                    0x22
#define WRITE_RAM                                   0x24
#define WRITE_VCOM_REGISTER                         0x2C
#define WRITE_LUT_REGISTER                          0x32
#define SET_DUMMY_LINE_PERIOD                       0x3A
#define SET_GATE_TIME                               0x3B
#define BORDER_WAVEFORM_CONTROL                     0x3C
#define SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define TERMINATE_FRAME_READ_WRITE                  0xFF

#define WSEPD_HEIGHT      200
#define WSEPD_WIDTH       200

const unsigned char lut_full_update[] = {
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

const unsigned char lut_partial_update[] = {
    0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static lcd_cb_t wsepd154_cb = {
    .height = WSEPD_HEIGHT,
    .width = WSEPD_WIDTH,
};

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(WSEPD_SPI_INSTANCE);
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

uint8_t screen_buffer[((WSEPD_WIDTH % 8 == 0)? (WSEPD_WIDTH / 8 ): (WSEPD_WIDTH / 8 + 1)) * WSEPD_HEIGHT];


void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context)
{
    spi_xfer_done = true;
}

static inline void spi_write(uint8_t * data, size_t size)
{
    spi_xfer_done = false;

    if (size < 1) {
        return;
    }
    
    //NRF_LOG_INFO("spi_write: data=%x , size=%d", data, size);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, data, size, NULL, 0));

    while (!spi_xfer_done)
    {
        //NRF_LOG_INFO("SPI transfer in progress...");
        __WFE();
    }

}

static inline void write_command(uint8_t c)
{
    nrf_gpio_pin_clear(WSEPD_DC_PIN);
    nrf_gpio_pin_clear(SPI_SS_PIN);
    spi_write(&c, sizeof(c));
    nrf_gpio_pin_set(SPI_SS_PIN);
}

static inline void write_data(uint8_t c)
{
    nrf_gpio_pin_set(WSEPD_DC_PIN);
    nrf_gpio_pin_clear(SPI_SS_PIN);
    spi_write(&c, sizeof(c));
    nrf_gpio_pin_set(SPI_SS_PIN);
}

static void set_cursor(uint16_t x, uint16_t y)
{
    write_command(SET_RAM_X_ADDRESS_COUNTER);
    write_data((x >> 3) & 0xff);

    write_command(SET_RAM_Y_ADDRESS_COUNTER);
    write_data(y & 0xff);
    write_data((y >> 8) & 0xff);
}

static void set_addr_window(uint16_t x_0, uint16_t y_0, uint16_t x_1, uint16_t y_1)
{
    ASSERT(x_0 < x_1);
    ASSERT(y_0 < y_1);

    write_command(SET_RAM_X_ADDRESS_START_END_POSITION);
    write_data((x_0 >> 3) & 0xff);
    write_data((x_1 >> 3) & 0xff);
    write_command(SET_RAM_Y_ADDRESS_START_END_POSITION);
    write_data(y_0 & 0xff);
    write_data((y_0 >> 8) & 0xff);
    write_data(y_1 & 0xff);
    write_data((y_1 >> 8) & 0xff);
}

static void wait_until_idle(void)
{
    NRF_LOG_INFO("epd busy\r\n");
    while(nrf_gpio_pin_read(WSEPD_BUSY_PIN) == 1) {      //LOW: idle, HIGH: busy        
        //NRF_LOG_INFO("epd is busy...\r\n");
        //app_sched_execute();  // prevent overflow of the app scheduler queue (NRF_ERROR_NO_MEM)
        non_blocking_delay_ms(5);
    }
    NRF_LOG_INFO("epd busy release\r\n");
}

static void turn_on_display(void)
{
    write_command(DISPLAY_UPDATE_CONTROL_2);
    write_data(0xc4);
    write_command(MASTER_ACTIVATION);
    write_command(TERMINATE_FRAME_READ_WRITE);

    wait_until_idle();
}

static void wsepd_reset(void)
{
    //nrf_gpio_pin_set(WSEPD_RST_PIN);
    //nrf_delay_ms(200);
    nrf_gpio_pin_clear(WSEPD_RST_PIN);
    nrf_delay_ms(20);
    nrf_gpio_pin_set(WSEPD_RST_PIN);
    //nrf_delay_ms(200);
}

static void command_list(void)
{
    wsepd_reset();
    
    wait_until_idle();
    write_command(0x12);  //SWRESET
    wait_until_idle();

    write_command(0x01); //Driver output control
    write_data(0xC7);
    write_data(0x00);
    write_data(0x00);

    write_command(0x11); //data entry mode
    write_data(0x01);

    write_command(0x44); //set Ram-X address start/end position
    write_data(0x00);
    write_data(0x18);    //0x0C-->(18+1)*8=200

    write_command(0x45); //set Ram-Y address start/end position
    write_data(0xC7);   //0xC7-->(199+1)=200
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);

    write_command(0x3C); //BorderWavefrom
    write_data(0x01);
    
    write_command(0x18);
    write_data(0x80);

    write_command(0x22); // Load Temperature and waveform setting.
    write_data(0XB1);
    write_command(0x20);

    write_command(0x4E);   // set RAM x address count to 0;
    write_data(0x00);
    write_command(0x4F);   // set RAM y address count to 0X199;
    write_data(0xC7);
    write_data(0x00);

    wait_until_idle();

    /*
    wsepd_reset();
    
    wait_until_idle();
    write_command(0x12);  //SWRESET
    wait_until_idle();

    write_command(0x01); //Driver output control
    write_data(0xC7);
    write_data(0x00);
    write_data(0x01);

    write_command(0x11); //data entry mode
    write_data(0x01);

    write_command(0x44); //set Ram-X address start/end position
    write_data(0x00);
    write_data(0x18);    //0x0C-->(18+1)*8=200

    write_command(0x45); //set Ram-Y address start/end position
    write_data(0xC7);   //0xC7-->(199+1)=200
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);

    write_command(0x3C); //BorderWavefrom
    write_data(0x01);
    
    write_command(0x18);
    write_data(0x80);

    write_command(0x22); // //Load Temperature and waveform setting.
    write_data(0XB1);
    write_command(0x20);

    write_command(0x4E);   // set RAM x address count to 0;
    write_data(0x00);
    write_command(0x4F);   // set RAM y address count to 0X199;
    write_data(0xC7);
    write_data(0x00);

    wait_until_idle();
    */

    /*
    write_command(DRIVER_OUTPUT_CONTROL);
    write_data((WSEPD_HEIGHT - 1) & 0xFF);
    write_data(((WSEPD_HEIGHT - 1) >> 8) & 0xFF);
    write_data(0x00);
    write_command(BOOSTER_SOFT_START_CONTROL);
    write_data(0xD7);
    write_data(0xD6);
    write_data(0x9D);
    write_command(WRITE_VCOM_REGISTER);
    write_data(0xA8);
    write_command(SET_DUMMY_LINE_PERIOD);
    write_data(0x1A);
    write_command(SET_GATE_TIME);
    write_data(0x08);
    write_command(DATA_ENTRY_MODE_SETTING);
    write_data(0x03);

    write_command(WRITE_LUT_REGISTER);
    for(uint8_t i = 0; i < 30; i++)
    {
        // TODO: allow partial update mode
        write_data(lut_full_update[i]);
    }
    */
}

static ret_code_t hardware_init(void)
{
  nrf_gpio_cfg_output(WSEPD_RST_PIN);
  nrf_gpio_cfg_output(WSEPD_DC_PIN);
  nrf_gpio_cfg_input(WSEPD_BUSY_PIN, NRF_GPIO_PIN_PULLUP);

  nrf_drv_spi_config_t spi_config = 
    {
      spi_config.sck_pin = SPI_SCK_PIN,
      spi_config.mosi_pin = SPI_MOSI_PIN,
      spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED,
      spi_config.ss_pin = SPI_SS_PIN,
      spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
      spi_config.orc = 0xFF,
      spi_config.frequency = NRF_DRV_SPI_FREQ_8M,
      spi_config.mode = NRF_DRV_SPI_MODE_0,
      spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
    };

    return nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
}

static ret_code_t wsepd154_init(void)
{
    ret_code_t err_code;

    err_code = hardware_init();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    command_list();
    // Clear screen buffer to white
    memset(screen_buffer, 0xff, sizeof(screen_buffer));
    return 0;
}

static void wsepd154_uninit(void)
{
    write_command(DEEP_SLEEP_MODE);
    write_data(0x01);
    nrf_drv_spi_uninit(&spi);
}


static void wsepd154_pixel_draw(uint16_t x, uint16_t y, uint32_t color)
{
    uint16_t x_, y_;
 
    if(x > WSEPD_WIDTH || y > WSEPD_HEIGHT){
        NRF_LOG_INFO("Exceeding display boundaries\r\n");
        return;
    }
    switch(wsepd154_cb.rotation) 
    {
        case NRF_LCD_ROTATE_90:
            x_ = WSEPD_WIDTH - y - 1;
            y_ = x;
            break;
        case NRF_LCD_ROTATE_180:
            x_ = WSEPD_WIDTH - x - 1;
            y_ = WSEPD_HEIGHT - y - 1;
            break;
        case NRF_LCD_ROTATE_270:
            x_ = y;
            y_ = WSEPD_HEIGHT - x - 1;
            break;
        default:
            x_ = x;
            y_ = y;
            break;
    }
    if(x_ > WSEPD_WIDTH || y_ > WSEPD_HEIGHT){
        NRF_LOG_INFO("Exceeding display boundaries\r\n");
        return;
    }
    uint16_t addr = x_ / 8 + y_ * ((WSEPD_WIDTH % 8) == 0 ? WSEPD_WIDTH / 8 : WSEPD_WIDTH / 8 + 1);
    uint8_t rdata = screen_buffer[addr];
    if(color == 0)
        screen_buffer[addr] = rdata & ~(0x80 >> (x_ % 8));
    else
        screen_buffer[addr] = rdata | (0x80 >> (x_ % 8));
}

static void wsepd154_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
    uint16_t x_, y_;
 
    // TODO: This could be done about 8x faster with cheeky bitfield manipulations
    //     but it's really difficult to get it all perfect given the 6 conditions
    for(y_ = y; y_ < (y + height); y_ ++)
    {
        for(x_ = x; x_ < (x + width); x_ ++)
        {
            wsepd154_pixel_draw(x_, y_, color);
        }
    }
}

static void wsepd154_display(void)
{
    uint16_t width;
    uint32_t addr = 0;
    uint16_t i, j;

    width = (WSEPD_WIDTH % 8 == 0) ? (WSEPD_WIDTH / 8) : (WSEPD_WIDTH / 8 + 1);
    set_addr_window(0, 0, WSEPD_WIDTH, WSEPD_HEIGHT);
    for(i = 0; i < WSEPD_HEIGHT; i++)
    {
        set_cursor(0, i);
        write_command(WRITE_RAM);
        for(j = 0; j < width; j++)
        {
            addr = j + i * width;
            write_data(screen_buffer[addr]);
        }
    }
    turn_on_display();
}

static void wsepd154_rotation_set(nrf_lcd_rotation_t rotation)
{
    // dummy function - actions are taken in the drawing primitives
}

static void wsepd154_display_invert(bool invert)
{
    // dummy function - doesn't have feature built in
}

// Can only draw an image the size of screen or larger
void wsepd154_draw_monobmp(const uint8_t *image_buffer)
{
    uint16_t x, y;
    uint32_t addr = 0;
    uint8_t byte_width = (WSEPD_WIDTH % 8 == 0) ? (WSEPD_WIDTH / 8) : (WSEPD_WIDTH / 8 + 1);

    for (y = 0; y < WSEPD_HEIGHT; y++) {
        for (x = 0; x < byte_width; x++) 
        {//8 pixel =  1 byte
            addr = x + y * byte_width;
            screen_buffer[addr] = (uint8_t)image_buffer[addr];
        }
    }
}

const nrf_lcd_t nrf_lcd_wsepd154 = {
    .lcd_init = wsepd154_init, // Done
    .lcd_uninit = wsepd154_uninit, // Done
    .lcd_pixel_draw = wsepd154_pixel_draw, // Done
    .lcd_rect_draw = wsepd154_rect_draw, // TODO: reqd
    .lcd_display = wsepd154_display,
    .lcd_rotation_set = wsepd154_rotation_set,
    .lcd_display_invert = wsepd154_display_invert, 
    .p_lcd_cb = &wsepd154_cb
};