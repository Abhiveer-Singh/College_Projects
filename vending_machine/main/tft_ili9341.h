#pragma once

#include <stdint.h>
#include <stddef.h>
#include "driver/spi_master.h"

class ILI9341Driver {
public:
    explicit ILI9341Driver(spi_device_handle_t spi);

    void begin();
    void fillScreen(uint16_t color);
    void fillRect(int x, int y, int w, int h, uint16_t color);
    void drawRect(int x, int y, int w, int h, uint16_t color);
    void drawFastHLine(int x, int y, int w, uint16_t color);
    void drawFastVLine(int x, int y, int h, uint16_t color);
    void drawText(int x, int y, const char *text, uint16_t color,
                  uint8_t scale = 1);
    void drawTextCentered(int y, const char *text, uint16_t color,
                          uint8_t scale = 1);

private:
    spi_device_handle_t spi_;

    void command(uint8_t cmd);
    void data(const uint8_t *data, size_t len);
    void setWindow(int x0, int y0, int x1, int y1);
    void pushColor(uint16_t color);
    void drawChar(int x, int y, char c, uint16_t color, uint8_t scale);
};
