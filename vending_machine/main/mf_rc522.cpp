#include "mf_rc522.h"

#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "MFRC522";

// ============================================================
// MFRC522 REGISTERS
// ============================================================

static constexpr uint8_t CommandReg    = 0x01;
static constexpr uint8_t ComIEnReg     = 0x02;
static constexpr uint8_t ComIrqReg     = 0x04;
static constexpr uint8_t ErrorReg      = 0x06;
static constexpr uint8_t FIFODataReg   = 0x09;
static constexpr uint8_t FIFOLevelReg  = 0x0A;
static constexpr uint8_t ControlReg    = 0x0C;
static constexpr uint8_t BitFramingReg = 0x0D;
static constexpr uint8_t ModeReg       = 0x11;
static constexpr uint8_t TxControlReg  = 0x14;
static constexpr uint8_t TxASKReg      = 0x15;
static constexpr uint8_t TModeReg      = 0x2A;
static constexpr uint8_t TPrescalerReg = 0x2B;
static constexpr uint8_t TReloadRegH   = 0x2C;
static constexpr uint8_t TReloadRegL   = 0x2D;
static constexpr uint8_t VersionReg    = 0x37;
static constexpr uint8_t Status2Reg    = 0x08;

// ============================================================
// COMMANDS
// ============================================================

static constexpr uint8_t PCD_Idle       = 0x00;
static constexpr uint8_t PCD_Transceive = 0x0C;
static constexpr uint8_t PCD_SoftReset  = 0x0F;

static constexpr uint8_t PICC_CMD_REQA    = 0x26;
static constexpr uint8_t PICC_CMD_SEL_CL1 = 0x93;
static constexpr uint8_t PICC_CMD_HLTA    = 0x50;


// ============================================================
// CONSTRUCTOR
// ============================================================

MFRC522Driver::MFRC522Driver(
    spi_device_handle_t spi
)
    : spi_(spi),
      rstPin_(-1)
{
}


// ============================================================
// WRITE REGISTER
// ============================================================

bool MFRC522Driver::writeReg(
    uint8_t reg,
    uint8_t value
)
{
    if (spi_ == nullptr)
    {
        ESP_LOGE(TAG, "SPI handle is NULL");
        return false;
    }

    uint8_t tx[2] =
    {
        (uint8_t)((reg << 1) & 0x7E),
        value
    };

    spi_transaction_t t = {};

    t.length = 16;
    t.tx_buffer = tx;

    esp_err_t result =
        spi_device_transmit(spi_, &t);

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "SPI WRITE failed: reg=0x%02X error=%s",
            reg,
            esp_err_to_name(result)
        );

        return false;
    }

    return true;
}


// ============================================================
// READ REGISTER
// ============================================================

bool MFRC522Driver::readReg(
    uint8_t reg,
    uint8_t *value
)
{
    if (spi_ == nullptr)
    {
        ESP_LOGE(TAG, "SPI handle is NULL");
        return false;
    }

    if (value == nullptr)
    {
        return false;
    }

    uint8_t tx[2] =
    {
        (uint8_t)(((reg << 1) & 0x7E) | 0x80),
        0
    };

    uint8_t rx[2] = {};

    spi_transaction_t t = {};

    t.length = 16;
    t.tx_buffer = tx;
    t.rx_buffer = rx;

    esp_err_t result =
        spi_device_transmit(spi_, &t);

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "SPI READ failed: reg=0x%02X error=%s",
            reg,
            esp_err_to_name(result)
        );

        return false;
    }

    *value = rx[1];

    return true;
}


// ============================================================
// READ REGISTER
// ============================================================

uint8_t MFRC522Driver::readReg(
    uint8_t reg
)
{
    uint8_t value = 0;

    if (!readReg(reg, &value))
    {
        return 0;
    }

    return value;
}


// ============================================================
// SET BIT MASK
// ============================================================

void MFRC522Driver::setBitMask(
    uint8_t reg,
    uint8_t mask
)
{
    uint8_t value = 0;

    if (!readReg(reg, &value))
    {
        return;
    }

    value |= mask;

    writeReg(
        reg,
        value
    );
}


// ============================================================
// CLEAR BIT MASK
// ============================================================

void MFRC522Driver::clearBitMask(
    uint8_t reg,
    uint8_t mask
)
{
    uint8_t value = 0;

    if (!readReg(reg, &value))
    {
        return;
    }

    value &= (uint8_t)~mask;

    writeReg(
        reg,
        value
    );
}


// ============================================================
// INITIALIZE RC522
// ============================================================

void MFRC522Driver::begin(
    int rstPin
)
{
    rstPin_ = rstPin;

    gpio_config_t io = {};

    io.pin_bit_mask =
        1ULL << rstPin_;

    io.mode = GPIO_MODE_OUTPUT;
    io.pull_up_en = GPIO_PULLUP_DISABLE;
    io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io);

    // Hardware reset
    gpio_set_level(
        (gpio_num_t)rstPin_,
        0
    );

    esp_rom_delay_us(2000);

    gpio_set_level(
        (gpio_num_t)rstPin_,
        1
    );

    esp_rom_delay_us(5000);

    // Software reset
    writeReg(
        CommandReg,
        PCD_SoftReset
    );

    esp_rom_delay_us(5000);

    // Timer configuration
    writeReg(
        TModeReg,
        0x8D
    );

    writeReg(
        TPrescalerReg,
        0x3E
    );

    writeReg(
        TReloadRegL,
        30
    );

    writeReg(
        TReloadRegH,
        0
    );

    // ASK modulation
    writeReg(
        TxASKReg,
        0x40
    );

    // CRC preset / mode
    writeReg(
        ModeReg,
        0x3D
    );

    antennaOn();

    uint8_t version =
        readReg(VersionReg);

    ESP_LOGI(
        TAG,
        "MFRC522 version: 0x%02X",
        version
    );

    if (version == 0x00 ||
        version == 0xFF)
    {
        ESP_LOGE(
            TAG,
            "RC522 is NOT responding correctly!"
        );
    }
    else
    {
        ESP_LOGI(
            TAG,
            "RC522 SPI communication OK"
        );
    }
}


// ============================================================
// ANTENNA ON
// ============================================================

void MFRC522Driver::antennaOn()
{
    uint8_t value =
        readReg(TxControlReg);

    if ((value & 0x03) != 0x03)
    {
        setBitMask(
            TxControlReg,
            0x03
        );
    }
}


// ============================================================
// TRANSCEIVE
// ============================================================

bool MFRC522Driver::transceive(
    const uint8_t *send,
    uint8_t sendLen,
    uint8_t *recv,
    uint8_t *recvLen,
    uint8_t *validBits
)
{
    if (send == nullptr ||
        recv == nullptr ||
        recvLen == nullptr ||
        validBits == nullptr)
    {
        return false;
    }

    // Stop previous command
    if (!writeReg(
            CommandReg,
            PCD_Idle))
    {
        return false;
    }

    // Clear interrupt flags
    if (!writeReg(
            ComIrqReg,
            0x7F))
    {
        return false;
    }

    // Flush FIFO
    if (!writeReg(
            FIFOLevelReg,
            0x80))
    {
        return false;
    }

    // Clear errors
    clearBitMask(
        ErrorReg,
        0x1B
    );

    // Put command into FIFO
    for (uint8_t i = 0;
         i < sendLen;
         ++i)
    {
        if (!writeReg(
                FIFODataReg,
                send[i]))
        {
            return false;
        }
    }

    // Configure valid bits
    if (!writeReg(
            BitFramingReg,
            *validBits))
    {
        return false;
    }

    // Start transceive
    if (!writeReg(
            CommandReg,
            PCD_Transceive))
    {
        return false;
    }

    // Start transmission
    setBitMask(
        BitFramingReg,
        0x80
    );

    // Poll for completion
    for (int i = 0;
         i < 100;
         ++i)
    {
        uint8_t irq = 0;

        if (!readReg(
                ComIrqReg,
                &irq))
        {
            clearBitMask(
                BitFramingReg,
                0x80
            );

            return false;
        }

        // Timer expired
        if (irq & 0x01)
        {
            clearBitMask(
                BitFramingReg,
                0x80
            );

            return false;
        }

        // Receive complete / idle
        if (irq & 0x30)
        {
            clearBitMask(
                BitFramingReg,
                0x80
            );

            uint8_t error =
                readReg(ErrorReg);

            if (error & 0x1B)
            {
                return false;
            }

            uint8_t n =
                readReg(FIFOLevelReg);

            if (n > *recvLen)
            {
                n = *recvLen;
            }

            for (uint8_t j = 0;
                 j < n;
                 ++j)
            {
                uint8_t value = 0;

                if (!readReg(
                        FIFODataReg,
                        &value))
                {
                    return false;
                }

                recv[j] = value;
            }

            *recvLen = n;

            uint8_t control =
                readReg(ControlReg);

            *validBits =
                control & 0x07;

            return true;
        }

        // Small delay between polls
        esp_rom_delay_us(500);
    }

    // Timeout
    clearBitMask(
        BitFramingReg,
        0x80
    );

    return false;
}


// ============================================================
// REQUEST A
// ============================================================

bool MFRC522Driver::requestA(
    uint8_t *atqa
)
{
    if (atqa == nullptr)
    {
        return false;
    }

    uint8_t command =
        PICC_CMD_REQA;

    uint8_t recvLen = 2;

    uint8_t validBits = 7;

    bool result =
        transceive(
            &command,
            1,
            atqa,
            &recvLen,
            &validBits
        );

    return result &&
           recvLen == 2;
}


// ============================================================
// ANTICOLLISION
// ============================================================

bool MFRC522Driver::anticoll(
    uint8_t *uid,
    uint8_t *uidLen
)
{
    if (uid == nullptr ||
        uidLen == nullptr)
    {
        return false;
    }

    uint8_t command[2] =
    {
        PICC_CMD_SEL_CL1,
        0x20
    };

    uint8_t recv[10] = {};

    uint8_t recvLen =
        sizeof(recv);

    uint8_t validBits = 0;

    if (!transceive(
            command,
            2,
            recv,
            &recvLen,
            &validBits))
    {
        return false;
    }

    if (recvLen != 5)
    {
        return false;
    }

    // Check BCC
    uint8_t bcc =
        recv[0] ^
        recv[1] ^
        recv[2] ^
        recv[3];

    if (bcc != recv[4])
    {
        ESP_LOGW(
            TAG,
            "UID BCC error"
        );

        return false;
    }

    // Copy UID
    for (int i = 0;
         i < 4;
         ++i)
    {
        uid[i] = recv[i];
    }

    *uidLen = 4;

    return true;
}


// ============================================================
// CARD PRESENT
// ============================================================

bool MFRC522Driver::isNewCardPresent()
{
    uint8_t atqa[2] = {};

    return requestA(
        atqa
    );
}


// ============================================================
// READ CARD UID
// ============================================================

bool MFRC522Driver::readCardSerial(
    uint8_t *uid,
    uint8_t *uidLen
)
{
    return anticoll(
        uid,
        uidLen
    );
}


// ============================================================
// HALT CARD
// ============================================================

void MFRC522Driver::haltA()
{
    uint8_t command[2] =
    {
        PICC_CMD_HLTA,
        0x00
    };

    uint8_t recv[4] = {};

    uint8_t recvLen =
        sizeof(recv);

    uint8_t validBits = 0;

    transceive(
        command,
        2,
        recv,
        &recvLen,
        &validBits
    );
}


// ============================================================
// STOP CRYPTO1
// ============================================================

void MFRC522Driver::stopCrypto1()
{
    clearBitMask(
        Status2Reg,
        0x08
    );
}
