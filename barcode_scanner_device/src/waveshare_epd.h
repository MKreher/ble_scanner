#ifndef WAVESHARE_EPD_H__
#define WAVESHARE_EPD_H__

extern const nrf_lcd_t nrf_lcd_wsepd154;
void wsepd154_draw_monobmp(const uint8_t *image_buffer);

#endif