#include "dispenser_task.h"

#include "driver/ledc.h"
#include "esp_timer.h"

#include "main.h"
#include "ration_data.h"

#include <stdio.h>
#include <algorithm>

static constexpr ledc_mode_t SERVO_MODE = LEDC_LOW_SPEED_MODE;
static constexpr ledc_timer_t SERVO_TIMER = LEDC_TIMER_0;

static constexpr int SERVO_FREQUENCY = 50;
static constexpr int SERVO_RESOLUTION = 14;

static constexpr ledc_channel_t CH_WHEAT = LEDC_CHANNEL_0;
static constexpr ledc_channel_t CH_RICE  = LEDC_CHANNEL_1;
static constexpr ledc_channel_t CH_DAL   = LEDC_CHANNEL_2;
static constexpr ledc_channel_t CH_SUGAR = LEDC_CHANNEL_3;

static bool g_busy = false;

static int g_activeCard = -1;
static int g_activeGrain = -1;

static int64_t g_dispenseStartTime = 0;
static int64_t g_dispenseDuration = 0;

// ============================================================
// SERVO
// ============================================================

static uint32_t angleToDuty(int angle)
{
    // 0 degrees  = 500 us
    // 90 degrees = 1500 us
    // 180 degrees = 2500 us

    int pulseUs = 500 + ((angle * 2000) / 180);

    const int periodUs = 1000000 / SERVO_FREQUENCY;

    uint32_t maxDuty =
        (1 << SERVO_RESOLUTION) - 1;

    return
        ((uint32_t)pulseUs * maxDuty) /
        periodUs;
}

static void setServo(
    ledc_channel_t channel,
    int angle
)
{
    uint32_t duty = angleToDuty(angle);

    ledc_set_duty(
        SERVO_MODE,
        channel,
        duty
    );

    ledc_update_duty(
        SERVO_MODE,
        channel
    );
}

// ============================================================
// SERVO INITIALIZATION
// ============================================================

void dispenserInit()
{
    ledc_timer_config_t timerConfig = {};

    timerConfig.speed_mode = SERVO_MODE;
    timerConfig.timer_num = SERVO_TIMER;
    timerConfig.duty_resolution =
        (ledc_timer_bit_t)SERVO_RESOLUTION;
    timerConfig.freq_hz = SERVO_FREQUENCY;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;

    ledc_timer_config(&timerConfig);

    ledc_channel_config_t channels[4] = {};

    int pins[4] = {
        SERVO_WHEAT,
        SERVO_RICE,
        SERVO_DAL,
        SERVO_SUGAR
    };

    ledc_channel_t channelIds[4] = {
        CH_WHEAT,
        CH_RICE,
        CH_DAL,
        CH_SUGAR
    };

    for (int i = 0; i < 4; i++) {

        channels[i].gpio_num = pins[i];
        channels[i].speed_mode = SERVO_MODE;
        channels[i].channel = channelIds[i];
        channels[i].intr_type = LEDC_INTR_DISABLE;
        channels[i].timer_sel = SERVO_TIMER;
        channels[i].duty = angleToDuty(
            SERVO_CLOSED_ANGLE
        );
        channels[i].hpoint = 0;

        ledc_channel_config(&channels[i]);
    }

    g_busy = false;

    printf("Dispenser initialized\n");
}

// ============================================================
// START DISPENSING
// ============================================================

bool dispenserStart(
    int cardIndex,
    int grainIndex
)
{
    if (g_busy) {
        return false;
    }

    if (cardIndex < 0 || cardIndex >= NUM_CARDS) {
        return false;
    }

    if (grainIndex < 0 || grainIndex > 3) {
        return false;
    }

    RationCard &card = cards[cardIndex];

    float *allotted = nullptr;
    float *used = nullptr;

    switch (grainIndex) {

        case 0:
            allotted = &card.wheatAllotted;
            used = &card.wheatUsed;
            break;

        case 1:
            allotted = &card.riceAllotted;
            used = &card.riceUsed;
            break;

        case 2:
            allotted = &card.dalAllotted;
            used = &card.dalUsed;
            break;

        case 3:
            allotted = &card.sugarAllotted;
            used = &card.sugarUsed;
            break;

        default:
            return false;
    }

    float remaining = *allotted - *used;

    if (remaining < 1.0f) {
        printf(
            "Not enough ration remaining for 1 kg\n"
        );

        return false;
    }

    // Reserve 1 kg immediately.
    *used += 1.0f;

    g_activeCard = cardIndex;
    g_activeGrain = grainIndex;

    g_busy = true;

    g_dispenseStartTime =
        esp_timer_get_time();

    // 800 ms per kg.
    g_dispenseDuration = 800000;

    ledc_channel_t channel;

    switch (grainIndex) {

        case 0:
            channel = CH_WHEAT;
            break;

        case 1:
            channel = CH_RICE;
            break;

        case 2:
            channel = CH_DAL;
            break;

        default:
            channel = CH_SUGAR;
            break;
    }

    setServo(
        channel,
        SERVO_OPEN_ANGLE
    );

    printf(
        "Dispensing 1 kg: card=%d grain=%d\n",
        cardIndex,
        grainIndex
    );

    return true;
}

// ============================================================
// UPDATE DISPENSER
// ============================================================

void dispenserUpdate()
{
    if (!g_busy) {
        return;
    }

    int64_t now = esp_timer_get_time();

    if ((now - g_dispenseStartTime)
        < g_dispenseDuration) {

        return;
    }

    ledc_channel_t channel;

    switch (g_activeGrain) {

        case 0:
            channel = CH_WHEAT;
            break;

        case 1:
            channel = CH_RICE;
            break;

        case 2:
            channel = CH_DAL;
            break;

        default:
            channel = CH_SUGAR;
            break;
    }

    setServo(
        channel,
        SERVO_CLOSED_ANGLE
    );

    printf(
        "Dispensing finished\n"
    );

    g_busy = false;
    g_activeCard = -1;
    g_activeGrain = -1;
}

bool dispenserBusy()
{
    return g_busy;
}
