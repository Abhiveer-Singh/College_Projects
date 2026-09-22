#pragma once

#include <stdint.h>
#include "driver/spi_master.h"

class MFRC522Driver
{
public:
    explicit MFRC522Driver(spi_device_handle_t spi);

    void begin(int rstPin);

    bool isNewCardPresent();

    bool readCardSerial(
        uint8_t *uid,
        uint8_t *uidLen
    );

    void haltA();

    void stopCrypto1();

private:
    spi_device_handle_t spi_;
    int rstPin_;

    bool writeReg(
        uint8_t reg,
        uint8_t value
    );

    bool readReg(
        uint8_t reg,
        uint8_t *value
    );

    uint8_t readReg(
        uint8_t reg
    );

    void setBitMask(
        uint8_t reg,
        uint8_t mask
    );

    void clearBitMask(
        uint8_t reg,
        uint8_t mask
    );

    void antennaOn();

    bool transceive(
        const uint8_t *send,
        uint8_t sendLen,
        uint8_t *recv,
        uint8_t *recvLen,
        uint8_t *validBits
    );

    bool requestA(
        uint8_t *atqa
    );

    bool anticoll(
        uint8_t *uid,
        uint8_t *uidLen
    );
};
