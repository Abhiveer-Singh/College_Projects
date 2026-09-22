#include "touch_xpt2046.h"
#include "main.h"
#include "driver/gpio.h"

XPT2046Driver::XPT2046Driver(spi_device_handle_t spi)
    : spi_(spi)
{
    gpio_config_t io = {};
    io.pin_bit_mask = 1ULL << TOUCH_IRQ;
    io.mode = GPIO_MODE_INPUT;
    io.pull_up_en = GPIO_PULLUP_DISABLE;
    io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io);
}

uint16_t XPT2046Driver::read12(uint8_t command)
{
    uint8_t tx[2] = {command, 0};
    uint8_t rx[2] = {};

    spi_transaction_t t = {};
    t.length = 16;
    t.tx_buffer = tx;
    t.rx_buffer = rx;

    esp_err_t result = spi_device_transmit(spi_, &t);

    if (result != ESP_OK)
        return 0;

    return ((uint16_t)rx[0] << 8 | rx[1]) >> 3;
}

bool XPT2046Driver::touched()
{
    return gpio_get_level((gpio_num_t)TOUCH_IRQ) == 0;
}

TouchPoint XPT2046Driver::readPoint()
{
    TouchPoint p = {};

    if (!touched())
    {
        p.touched = false;
        return p;
    }

    // Read raw X/Y values
    uint16_t rawX = read12(0xD1);
    uint16_t rawY = read12(0x91);

    printf("RAW TOUCH: X=%u Y=%u\n", rawX, rawY);

    // --------------------------------------------------------
    // RAW TOUCH CALIBRATION
    // --------------------------------------------------------
    //
    // These values are typical starting values for XPT2046.
    // We can fine-tune them after seeing the serial output.
    //
    const int TS_MINX = 200;
    const int TS_MAXX = 3800;
    const int TS_MINY = 200;
    const int TS_MAXY = 3800;

    int x = (int)rawX;
    int y = (int)rawY;

    // Convert raw coordinates to screen coordinates.
    x = (x - TS_MINX) * SCREEN_W / (TS_MAXX - TS_MINX);
    y = (y - TS_MINY) * SCREEN_H / (TS_MAXY - TS_MINY);

    // Keep coordinates inside screen.
    if (x < 0)
        x = 0;

    if (x >= SCREEN_W)
        x = SCREEN_W - 1;

    if (y < 0)
        y = 0;

    if (y >= SCREEN_H)
        y = SCREEN_H - 1;

    p.x = x;
    p.y = y;
    p.touched = true;

    printf("SCREEN TOUCH: X=%d Y=%d\n", p.x, p.y);

    return p;
}
