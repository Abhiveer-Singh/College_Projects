#include "tft_ili9341.h"

#include <ctype.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "main.h"

// ============================================================
// ILI9341 COMMANDS
// ============================================================

static constexpr uint8_t SWRESET = 0x01;
static constexpr uint8_t SLPOUT  = 0x11;
static constexpr uint8_t DISPON  = 0x29;
static constexpr uint8_t MADCTL  = 0x36;
static constexpr uint8_t PIXFMT  = 0x3A;
static constexpr uint8_t CASET   = 0x2A;
static constexpr uint8_t PASET   = 0x2B;
static constexpr uint8_t RAMWR   = 0x2C;

// ============================================================
// COLORS
// ============================================================

static constexpr uint16_t BLACK = 0x0000;
static constexpr uint16_t WHITE = 0xFFFF;

// ============================================================
// 5x7 FONT
//
// Order:
// A-Z, 0-9, space, . : + - < > /
// ============================================================

static const uint8_t FONT[44][7] = {

    // A
    {14,17,17,31,17,17,17},

    // B
    {30,17,17,30,17,17,30},

    // C
    {14,17,16,16,16,16,14},

    // D
    {30,17,17,17,17,17,30},

    // E
    {31,16,16,30,16,16,31},

    // F
    {31,16,16,30,16,16,16},

    // G
    {14,17,16,23,17,17,14},

    // H
    {17,17,17,31,17,17,17},

    // I
    {31,4,4,4,4,4,31},

    // J
    {7,2,2,2,3,2,12},

    // K
    {17,18,20,24,20,18,17},

    // L
    {16,16,16,16,16,16,31},

    // M
    {17,27,21,21,17,17,17},

    // N
    {17,25,29,27,17,17,17},

    // O
    {14,17,17,17,17,17,14},

    // P
    {30,17,17,30,16,16,16},

    // Q
    {14,17,17,17,21,18,13},

    // R
    {30,17,17,30,20,18,17},

    // S
    {15,16,16,14,1,1,30},

    // T
    {31,4,4,4,4,4,4},

    // U
    {17,17,17,17,17,17,14},

    // V
    {17,17,17,17,17,8,8},

    // W
    {17,17,17,21,21,27,17},

    // X
    {17,17,10,4,10,20,9},

    // Y
    {17,17,10,4,4,4,4},

    // Z
    {31,1,2,4,8,16,31},

    // 0
    {14,17,19,21,25,17,14},

    // 1
    {4,12,4,4,4,4,14},

    // 2
    {14,17,1,2,4,8,31},

    // 3
    {31,1,1,6,1,17,14},

    // 4
    {2,6,10,18,31,2,2},

    // 5
    {31,16,16,30,1,1,30},

    // 6
    {14,16,16,30,17,17,14},

    // 7
    {31,1,2,4,8,8,8},

    // 8
    {14,17,17,14,17,17,14},

    // 9
    {14,17,17,15,1,1,14},

    // SPACE
    {0,0,0,0,0,0,0},

    // .
    {0,0,0,0,0,0,4},

    // :
    {0,2,0,0,0,8,0},

    // +
    {0,2,0,31,4,2,0},

    // -
    {0,0,0,31,0,0,0},

    // <
    {1,2,4,2,0,16,4},

    // >
    {16,4,2,2,4,8,16},

    // /
    {1,1,2,4,8,16,16}
};

// ============================================================
// FONT LOOKUP
// ============================================================

static int glyphIndex(char c)
{
    c = (char)toupper((unsigned char)c);

    if (c >= 'A' && c <= 'Z')
        return c - 'A';

    if (c >= '0' && c <= '9')
        return 26 + (c - '0');

    switch (c) {

        case ' ':
            return 36;

        case '.':
            return 37;

        case ':':
            return 38;

        case '+':
            return 39;

        case '-':
            return 40;

        case '<':
            return 41;

        case '>':
            return 42;

        case '/':
            return 43;

        default:
            return 36;
    }
}

// ============================================================
// CONSTRUCTOR
// ============================================================

ILI9341Driver::ILI9341Driver(spi_device_handle_t spi)
    : spi_(spi)
{
}

// ============================================================
// SEND COMMAND
// ============================================================

void ILI9341Driver::command(uint8_t cmd)
{
    gpio_set_level((gpio_num_t)TFT_DC, 0);

    spi_transaction_t t = {};

    t.length = 8;
    t.tx_buffer = &cmd;

    spi_device_transmit(spi_, &t);
}

// ============================================================
// SEND DATA
// ============================================================

void ILI9341Driver::data(const uint8_t *bytes, size_t len)
{
    gpio_set_level((gpio_num_t)TFT_DC, 1);

    spi_transaction_t t = {};

    t.length = len * 8;
    t.tx_buffer = bytes;

    spi_device_transmit(spi_, &t);
}

// ============================================================
// INITIALIZE DISPLAY
// ============================================================

void ILI9341Driver::begin()
{
    // ---------------- DC ----------------

    gpio_config_t dc = {};

    dc.pin_bit_mask = 1ULL << TFT_DC;
    dc.mode = GPIO_MODE_OUTPUT;

    gpio_config(&dc);


    // ---------------- RESET ----------------

    gpio_config_t rst = {};

    rst.pin_bit_mask = 1ULL << TFT_RST;
    rst.mode = GPIO_MODE_OUTPUT;

    gpio_config(&rst);


    // Hardware reset

    gpio_set_level((gpio_num_t)TFT_RST, 0);

    esp_rom_delay_us(10000);

    gpio_set_level((gpio_num_t)TFT_RST, 1);

    esp_rom_delay_us(120000);


    // Software reset

    command(SWRESET);

    esp_rom_delay_us(120000);


    // Exit sleep

    command(SLPOUT);

    esp_rom_delay_us(120000);


    // ---------------- PIXEL FORMAT ----------------

    uint8_t pixel = 0x55;   // RGB565

    command(PIXFMT);

    data(&pixel, 1);


    // ========================================================
    // LANDSCAPE ORIENTATION
    //
    // MADCTL:
    //
    // MX = 0x40
    // MY = 0x80
    // MV = 0x20
    // RGB = 0x00
    //
    // 0x28 = MV + BGR
    //
    // This rotates the ILI9341 into landscape orientation.
    // ========================================================

    uint8_t rotation = 0x28;

    command(MADCTL);

    data(&rotation, 1);


    // Display ON

    command(DISPON);

    esp_rom_delay_us(20000);


    // Clear display

    fillScreen(BLACK);
}

// ============================================================
// SET DRAWING WINDOW
// ============================================================

void ILI9341Driver::setWindow(
    int x0,
    int y0,
    int x1,
    int y1)
{
    uint8_t buf[4];


    // Column address

    command(CASET);

    buf[0] = x0 >> 8;
    buf[1] = x0 & 0xFF;

    buf[2] = x1 >> 8;
    buf[3] = x1 & 0xFF;

    data(buf, 4);


    // Page address

    command(PASET);

    buf[0] = y0 >> 8;
    buf[1] = y0 & 0xFF;

    buf[2] = y1 >> 8;
    buf[3] = y1 & 0xFF;

    data(buf, 4);


    // Start memory write

    command(RAMWR);
}

// ============================================================
// PUSH ONE COLOR
// ============================================================

void ILI9341Driver::pushColor(uint16_t color)
{
    uint8_t b[2];

    b[0] = color >> 8;
    b[1] = color;

    data(b, 2);
}

// ============================================================
// FILL SCREEN
// ============================================================

void ILI9341Driver::fillScreen(uint16_t color)
{
    fillRect(
        0,
        0,
        SCREEN_W,
        SCREEN_H,
        color
    );
}

// ============================================================
// FILL RECTANGLE
// ============================================================

void ILI9341Driver::fillRect(
    int x,
    int y,
    int w,
    int h,
    uint16_t color)
{
    if (w <= 0 || h <= 0)
        return;


    // Clip left

    if (x < 0) {

        w += x;
        x = 0;
    }


    // Clip top

    if (y < 0) {

        h += y;
        y = 0;
    }


    // Clip right

    if (x + w > SCREEN_W)
        w = SCREEN_W - x;


    // Clip bottom

    if (y + h > SCREEN_H)
        h = SCREEN_H - y;


    if (w <= 0 || h <= 0)
        return;


    setWindow(
        x,
        y,
        x + w - 1,
        y + h - 1
    );


    // Send pixels in chunks

    uint8_t line[2 * 40];

    for (int i = 0; i < 40; ++i) {

        line[2 * i]     = color >> 8;
        line[2 * i + 1] = color;
    }


    int pixels = w * h;


    while (pixels > 0) {

        int chunk =
            pixels > 40 ? 40 : pixels;

        data(
            line,
            chunk * 2
        );

        pixels -= chunk;
    }
}

// ============================================================
// DRAW RECTANGLE
// ============================================================

void ILI9341Driver::drawRect(
    int x,
    int y,
    int w,
    int h,
    uint16_t color)
{
    drawFastHLine(
        x,
        y,
        w,
        color
    );

    drawFastHLine(
        x,
        y + h - 1,
        w,
        color
    );

    drawFastVLine(
        x,
        y,
        h,
        color
    );

    drawFastVLine(
        x + w - 1,
        y,
        h,
        color
    );
}

// ============================================================
// HORIZONTAL LINE
// ============================================================

void ILI9341Driver::drawFastHLine(
    int x,
    int y,
    int w,
    uint16_t color)
{
    fillRect(
        x,
        y,
        w,
        1,
        color
    );
}

// ============================================================
// VERTICAL LINE
// ============================================================

void ILI9341Driver::drawFastVLine(
    int x,
    int y,
    int h,
    uint16_t color)
{
    fillRect(
        x,
        y,
        1,
        h,
        color
    );
}

// ============================================================
// DRAW CHARACTER
// ============================================================

void ILI9341Driver::drawChar(
    int x,
    int y,
    char c,
    uint16_t color,
    uint8_t scale)
{
    int idx = glyphIndex(c);

    if (scale == 0)
        scale = 1;


    int w = 5 * scale;
    int h = 7 * scale;


    uint8_t pixels[
        5 * 3 *
        7 * 3 *
        2
    ] = {};


    for (int py = 0; py < h; ++py) {

        int row = py / scale;


        for (int px = 0; px < w; ++px) {

            int col = px / scale;


            bool on =
                (FONT[idx][row] &
                 (1 << (4 - col))) != 0;


            uint16_t c16 =
                on ? color : BLACK;


            size_t p =
                ((size_t)py * w + px) * 2;


            pixels[p] =
                c16 >> 8;

            pixels[p + 1] =
                c16;
        }
    }


    if (
        x < 0 ||
        y < 0 ||
        x + w > SCREEN_W ||
        y + h > SCREEN_H
    )
        return;


    setWindow(
        x,
        y,
        x + w - 1,
        y + h - 1
    );


    data(
        pixels,
        (size_t)w * h * 2
    );
}

// ============================================================
// DRAW TEXT
// ============================================================

void ILI9341Driver::drawText(
    int x,
    int y,
    const char *text,
    uint16_t color,
    uint8_t scale)
{
    if (!text)
        return;


    int cursor = x;


    while (*text) {

        if (*text == '\n') {

            y += 8 * scale;

            cursor = x;

        } else {

            drawChar(
                cursor,
                y,
                *text,
                color,
                scale
            );

            cursor += 6 * scale;
        }


        ++text;
    }
}

// ============================================================
// CENTER TEXT
// ============================================================

void ILI9341Driver::drawTextCentered(
    int y,
    const char *text,
    uint16_t color,
    uint8_t scale)
{
    if (!text)
        return;


    int len = strlen(text);


    int width =
        len * 6 * scale;


    int x =
        (SCREEN_W - width) / 2;


    drawText(
        x,
        y,
        text,
        color,
        scale
    );
}
