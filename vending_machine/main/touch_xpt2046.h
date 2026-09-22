#pragma once

#include <stdint.h>
#include "driver/spi_master.h"

struct TouchPoint {
    int x;
    int y;
    bool touched;
};

class XPT2046Driver {
public:
    explicit XPT2046Driver(spi_device_handle_t spi);
    bool touched();
    TouchPoint readPoint();

private:
    spi_device_handle_t spi_;
    uint16_t read12(uint8_t command);
};
