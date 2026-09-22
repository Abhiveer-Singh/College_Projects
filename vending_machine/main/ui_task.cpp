#include "ui_task.h"

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "ration_data.h"
#include "tft_ili9341.h"
#include "touch_xpt2046.h"

// ============================================================
// COLORS
// ============================================================

static constexpr uint16_t BLACK  = 0x0000;
static constexpr uint16_t WHITE  = 0xFFFF;
static constexpr uint16_t BLUE   = 0x001F;
static constexpr uint16_t GREEN  = 0x07E0;
static constexpr uint16_t RED    = 0xF800;
static constexpr uint16_t YELLOW = 0xFFE0;
static constexpr uint16_t CYAN   = 0x07FF;
static constexpr uint16_t GRAY   = 0x8410;

// ============================================================
// DISPLAY / TOUCH OBJECTS
// ============================================================

static ILI9341Driver *tft = nullptr;
static XPT2046Driver *touch = nullptr;

// ============================================================
// BUTTON DRAWING
// ============================================================

static void drawButton(
    int x,
    int y,
    int w,
    int h,
    const char *text,
    uint16_t borderColor,
    uint16_t textColor)
{
    tft->drawRect(
        x,
        y,
        w,
        h,
        borderColor
    );

    int textWidth = strlen(text) * 6;

    int textX = x + (w - textWidth) / 2;
    int textY = y + (h - 7) / 2;

    tft->drawText(
        textX,
        textY,
        text,
        textColor,
        1
    );
}

// ============================================================
// INITIALIZE UI
// ============================================================

void uiInit()
{
    tft = new ILI9341Driver(g_tftSpi);

    touch = new XPT2046Driver(g_touchSpi);

    tft->begin();

    uiShowIdle();
}

// ============================================================
// IDLE / START SCREEN
// ============================================================

void uiShowIdle()
{
    tft->fillScreen(BLUE);

    tft->drawTextCentered(
        25,
        "RATION VENDING",
        WHITE,
        2
    );

    tft->drawTextCentered(
        50,
        "MACHINE",
        WHITE,
        2
    );

    // Main scan-card box

    tft->drawRect(
        60,
        95,
        200,
        85,
        WHITE
    );

    tft->drawTextCentered(
        110,
        "SCAN CARD",
        YELLOW,
        2
    );

    tft->drawTextCentered(
        140,
        "PLACE RFID CARD",
        WHITE,
        1
    );

    tft->drawTextCentered(
        205,
        "READY",
        GREEN,
        1
    );
}

// ============================================================
// UNKNOWN CARD SCREEN
// ============================================================

void uiShowUnknown(const char *uid)
{
    tft->fillScreen(RED);

    tft->drawTextCentered(
        25,
        "UNKNOWN CARD",
        WHITE,
        2
    );

    tft->drawTextCentered(
        65,
        "CARD NOT REGISTERED",
        WHITE,
        1
    );

    tft->drawTextCentered(
        100,
        "UID:",
        YELLOW,
        1
    );

    tft->drawTextCentered(
        120,
        uid,
        WHITE,
        2
    );

    drawButton(
        100,
        180,
        120,
        35,
        "BACK",
        WHITE,
        WHITE
    );
}

// ============================================================
// FAMILY SCREEN
// ============================================================

void uiShowFamily(int cardIndex)
{
    tft->fillScreen(BLUE);

    RationCard &card = cards[cardIndex];

    // --------------------------------------------------------
    // FAMILY NAME
    // --------------------------------------------------------

    tft->drawTextCentered(
        5,
        card.familyName,
        WHITE,
        2
    );

    // --------------------------------------------------------
    // FAMILY MEMBERS
    // --------------------------------------------------------

    for (int i = 0; i < card.memberCount; ++i) {

        int column = i % 2;
        int row = i / 2;

        int x = (column == 0) ? 8 : 165;
        int y = 38 + row * 25;

        // Member name

        tft->drawText(
            x,
            y,
            card.members[i].name,
            WHITE,
            1
        );

        // Adult / child

        tft->drawText(
            x,
            y + 10,
            card.members[i].isAdult ? "ADULT" : "CHILD",
            card.members[i].isAdult ? YELLOW : CYAN,
            1
        );
    }

    // --------------------------------------------------------
    // VIEW RATION BUTTON
    // --------------------------------------------------------

    drawButton(
        75,
        190,
        170,
        38,
        "VIEW RATION",
        WHITE,
        WHITE
    );
}

// ============================================================
// RATION SCREEN
// ============================================================

void uiShowRation(int cardIndex)
{
    tft->fillScreen(BLACK);

    RationCard &card = cards[cardIndex];

    // --------------------------------------------------------
    // HEADER
    // --------------------------------------------------------

    tft->drawText(
        8,
        7,
        card.familyName,
        WHITE,
        2
    );

    // Back button

    drawButton(
        260,
        4,
        55,
        25,
        "BACK",
        WHITE,
        WHITE
    );

    // Separator

    tft->drawFastHLine(
        5,
        34,
        310,
        GRAY
    );

    // --------------------------------------------------------
    // GRAIN NAMES
    // --------------------------------------------------------

    const char *grainNames[4] = {
        "WHEAT",
        "RICE",
        "DAL",
        "SUGAR"
    };

    // --------------------------------------------------------
    // ALLOTMENT / USED ARRAYS
    // --------------------------------------------------------

    float allotted[4] = {
        card.wheatAllotted,
        card.riceAllotted,
        card.dalAllotted,
        card.sugarAllotted
    };

    float used[4] = {
        card.wheatUsed,
        card.riceUsed,
        card.dalUsed,
        card.sugarUsed
    };

    // --------------------------------------------------------
    // DRAW FOUR GRAIN ROWS
    // --------------------------------------------------------

    for (int i = 0; i < 4; ++i) {

        int y = 42 + i * 43;

        float remaining =
            allotted[i] - used[i];

        if (remaining < 0.0f)
            remaining = 0.0f;

        // Grain name

        tft->drawText(
            8,
            y,
            grainNames[i],
            CYAN,
            1
        );

        // Used / allotted

        char amountText[32];

        snprintf(
            amountText,
            sizeof(amountText),
            "%.1f/%.1f KG",
            used[i],
            allotted[i]
        );

        tft->drawText(
            8,
            y + 12,
            amountText,
            WHITE,
            1
        );

        // Remaining

        char remainingText[24];

        snprintf(
            remainingText,
            sizeof(remainingText),
            "LEFT %.1f",
            remaining
        );

        tft->drawText(
            100,
            y + 5,
            remainingText,
            YELLOW,
            1
        );

        // +1 KG button

        drawButton(
            235,
            y - 3,
            78,
            30,
            "+1 KG",
            GREEN,
            GREEN
        );
    }

    // --------------------------------------------------------
    // BOTTOM STATUS
    // --------------------------------------------------------

    tft->drawText(
        8,
        218,
        "SELECT GRAIN TO DISPENSE",
        WHITE,
        1
    );
}

// ============================================================
// FAMILY SCREEN TOUCH
// ============================================================

UIEvent uiUpdateFamily(int cardIndex)
{
    if (!touch)
        return UI_EVENT_NONE;

    if (!touch->touched())
        return UI_EVENT_NONE;

    TouchPoint p = touch->readPoint();

    // --------------------------------------------------------
    // VIEW RATION
    // --------------------------------------------------------

    if (
        p.x >= 75 &&
        p.x <= 245 &&
        p.y >= 185 &&
        p.y <= 235
    ) {
        return UI_EVENT_VIEW_RATION;
    }

    return UI_EVENT_NONE;
}

// ============================================================
// RATION SCREEN TOUCH
// ============================================================

UIEvent uiUpdateRation(int cardIndex)
{
    if (!touch)
        return UI_EVENT_NONE;

    if (!touch->touched())
        return UI_EVENT_NONE;

    TouchPoint p = touch->readPoint();

    // --------------------------------------------------------
    // BACK BUTTON
    // --------------------------------------------------------

    if (
        p.x >= 255 &&
        p.x <= 319 &&
        p.y >= 0 &&
        p.y <= 35
    ) {
        return UI_EVENT_BACK;
    }

    // --------------------------------------------------------
    // GRAIN BUTTONS
    // --------------------------------------------------------

    for (int i = 0; i < 4; ++i) {

        int y = 42 + i * 43;

        if (
            p.x >= 230 &&
            p.x <= 319 &&
            p.y >= y - 5 &&
            p.y <= y + 32
        ) {

            switch (i) {

                case 0:
                    return UI_EVENT_DISPENSE_WHEAT;

                case 1:
                    return UI_EVENT_DISPENSE_RICE;

                case 2:
                    return UI_EVENT_DISPENSE_DAL;

                case 3:
                    return UI_EVENT_DISPENSE_SUGAR;
            }
        }
    }

    return UI_EVENT_NONE;
}
