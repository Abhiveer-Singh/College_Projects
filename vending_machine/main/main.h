#pragma once

#include "driver/spi_master.h"

// ============================================================
// ESP32 DEVKIT V1 PIN MAP
// ============================================================

// ---------------- TFT ILI9341 ----------------
#define TFT_SCK     18
#define TFT_MOSI    23
#define TFT_MISO    19
#define TFT_CS      33
#define TFT_DC      25
#define TFT_RST     22

// ---------------- XPT2046 TOUCH ----------------
#define TOUCH_CS    21
#define TOUCH_IRQ   34

// ---------------- MFRC522 RFID ----------------
#define RFID_SS     15
#define RFID_RST    16

// ---------------- SERVOS ----------------
#define SERVO_RICE   13
#define SERVO_WHEAT  26
#define SERVO_SUGAR  14
#define SERVO_DAL    27

// ---------------- HX711 ----------------
#define HX711_1_DT   4
#define HX711_1_SCK  5
#define HX711_2_DT   17
#define HX711_2_SCK  32

// ---------------- DISPLAY ----------------
#define SCREEN_W 320
#define SCREEN_H 240

// ---------------- SERVO ----------------
#define SERVO_CLOSED_ANGLE 0
#define SERVO_OPEN_ANGLE   90

// ============================================================
// SPI DEVICES
// ============================================================

extern spi_device_handle_t g_tftSpi;
extern spi_device_handle_t g_touchSpi;
extern spi_device_handle_t g_rfidSpi;
