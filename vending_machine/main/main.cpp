#include "main.h"

#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "nvs_flash.h"

#include "tft_ili9341.h"
#include "touch_xpt2046.h"
#include "mf_rc522.h"

#include "ration_data.h"
#include "rfid_task.h"
#include "dispenser_task.h"
#include "ui_task.h"

#include <stdio.h>
#include <string.h>

spi_device_handle_t g_tftSpi = nullptr;
spi_device_handle_t g_touchSpi = nullptr;
spi_device_handle_t g_rfidSpi = nullptr;

enum MachineState {
    STATE_WAIT_FOR_CARD = 0,
    STATE_SHOW_FAMILY,
    STATE_SHOW_RATION,
    STATE_DISPENSING,
    STATE_UNKNOWN_CARD
};

static MachineState machineState = STATE_WAIT_FOR_CARD;
static int activeCard = -1;
static int64_t unknownCardStart = 0;

static void spiInit()
{
    spi_bus_config_t busConfig = {};

    busConfig.mosi_io_num = TFT_MOSI;
    busConfig.miso_io_num = TFT_MISO;
    busConfig.sclk_io_num = TFT_SCK;
    busConfig.quadwp_io_num = -1;
    busConfig.quadhd_io_num = -1;
    busConfig.max_transfer_sz = SCREEN_W * SCREEN_H * 2;

    ESP_ERROR_CHECK(
        spi_bus_initialize(SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO)
    );

    // ========================================================
    // TFT ILI9341
    // ========================================================
    // Changed from 40 MHz to 20 MHz.
    // ESP-IDF was rejecting 40 MHz on this SPI configuration.
    spi_device_interface_config_t tftConfig = {};

    tftConfig.clock_speed_hz = 20 * 1000 * 1000;
    tftConfig.mode = 0;
    tftConfig.spics_io_num = TFT_CS;
    tftConfig.queue_size = 1;

    ESP_ERROR_CHECK(
        spi_bus_add_device(SPI2_HOST, &tftConfig, &g_tftSpi)
    );

    // ========================================================
    // XPT2046 TOUCH
    // ========================================================
    spi_device_interface_config_t touchConfig = {};

    touchConfig.clock_speed_hz = 2 * 1000 * 1000;
    touchConfig.mode = 0;
    touchConfig.spics_io_num = TOUCH_CS;
    touchConfig.queue_size = 1;

    ESP_ERROR_CHECK(
        spi_bus_add_device(SPI2_HOST, &touchConfig, &g_touchSpi)
    );

    // ========================================================
    // MFRC522 RFID
    // ========================================================
    spi_device_interface_config_t rfidConfig = {};

    rfidConfig.clock_speed_hz = 5 * 1000 * 1000;
    rfidConfig.mode = 0;
    rfidConfig.spics_io_num = RFID_SS;
    rfidConfig.queue_size = 1;

    ESP_ERROR_CHECK(
        spi_bus_add_device(SPI2_HOST, &rfidConfig, &g_rfidSpi)
    );

    printf("SPI initialized\n");
}

static void checkMonthlyReset()
{
    static int currentMonth = 9;

    for (int i = 0; i < NUM_CARDS; i++) {

        if (cards[i].lastResetMonth != currentMonth) {

            cards[i].wheatUsed = 0.0f;
            cards[i].riceUsed  = 0.0f;
            cards[i].dalUsed   = 0.0f;
            cards[i].sugarUsed = 0.0f;

            cards[i].lastResetMonth = currentMonth;

            printf(
                "Monthly ration reset: %s\n",
                cards[i].familyName
            );
        }
    }
}

extern "C" void app_main()
{
    printf("\n");
    printf("====================================\n");
    printf(" AUTONOMOUS RATION VENDING MACHINE\n");
    printf(" ESP32 + ILI9341 + XPT2046 + RFID\n");
    printf("====================================\n");

    // ========================================================
    // NVS INITIALIZATION
    // ========================================================

    esp_err_t nvsResult = nvs_flash_init();

    if (
        nvsResult == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvsResult == ESP_ERR_NVS_NEW_VERSION_FOUND
    ) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // ========================================================
    // RATION DATA
    // ========================================================

    for (int i = 0; i < NUM_CARDS; i++) {
        computeAllotment(cards[i]);
        seedDemoUsage(cards[i], i);
    }

    checkMonthlyReset();

    // ========================================================
    // HARDWARE INITIALIZATION
    // ========================================================

    spiInit();

    uiInit();

    rfidInit();

    dispenserInit();

    uiShowIdle();

    machineState = STATE_WAIT_FOR_CARD;

    printf("System ready.\n");

    // ========================================================
    // MAIN STATE MACHINE
    // ========================================================

    while (true) {

        // ----------------------------------------------------
        // WAIT FOR RFID CARD
        // ----------------------------------------------------

        if (machineState == STATE_WAIT_FOR_CARD) {

            int cardIndex = -1;
            char uid[32] = {};

            if (rfidPoll(&cardIndex, uid, sizeof(uid))) {

                if (cardIndex >= 0) {

                    activeCard = cardIndex;

                    uiShowFamily(activeCard);

                    machineState = STATE_SHOW_FAMILY;
                }

                else {

                    uiShowUnknown(uid);

                    unknownCardStart = esp_timer_get_time();

                    machineState = STATE_UNKNOWN_CARD;
                }
            }
        }

        // ----------------------------------------------------
        // UNKNOWN CARD SCREEN
        // ----------------------------------------------------

        else if (machineState == STATE_UNKNOWN_CARD) {

            int64_t now = esp_timer_get_time();

            if (now - unknownCardStart >= 2000000) {

                uiShowIdle();

                machineState = STATE_WAIT_FOR_CARD;
            }
        }

        // ----------------------------------------------------
        // FAMILY SCREEN
        // ----------------------------------------------------

        else if (machineState == STATE_SHOW_FAMILY) {

            UIEvent event = uiUpdateFamily(activeCard);

            if (event == UI_EVENT_VIEW_RATION) {

                uiShowRation(activeCard);

                machineState = STATE_SHOW_RATION;
            }
        }

        // ----------------------------------------------------
        // RATION SCREEN
        // ----------------------------------------------------

        else if (machineState == STATE_SHOW_RATION) {

            UIEvent event = uiUpdateRation(activeCard);

            if (event == UI_EVENT_BACK) {

                uiShowFamily(activeCard);

                machineState = STATE_SHOW_FAMILY;
            }

            else {

                int grainIndex = -1;

                switch (event) {

                    case UI_EVENT_DISPENSE_WHEAT:
                        grainIndex = 0;
                        break;

                    case UI_EVENT_DISPENSE_RICE:
                        grainIndex = 1;
                        break;

                    case UI_EVENT_DISPENSE_DAL:
                        grainIndex = 2;
                        break;

                    case UI_EVENT_DISPENSE_SUGAR:
                        grainIndex = 3;
                        break;

                    default:
                        break;
                }

                if (grainIndex >= 0) {

                    if (dispenserStart(activeCard, grainIndex)) {

                        machineState = STATE_DISPENSING;
                    }
                }
            }
        }

        // ----------------------------------------------------
        // DISPENSING
        // ----------------------------------------------------

        else if (machineState == STATE_DISPENSING) {

            dispenserUpdate();

            if (!dispenserBusy()) {

                uiShowRation(activeCard);

                machineState = STATE_SHOW_RATION;
            }
        }

        // ----------------------------------------------------
        // SMALL CPU DELAY
        // ----------------------------------------------------

        for (volatile int i = 0; i < 10000; i++) {
        }
    }
}

