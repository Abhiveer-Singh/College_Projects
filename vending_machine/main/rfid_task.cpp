#include "rfid_task.h"

#include "main.h"
#include "mf_rc522.h"
#include "ration_data.h"

#include <stdio.h>
#include <string.h>

static MFRC522Driver *rfid = nullptr;

void rfidInit()
{
    if (rfid != nullptr) {
        return;
    }

    rfid = new MFRC522Driver(g_rfidSpi);

    rfid->begin(RFID_RST);

    printf("RFID initialized\n");
}

bool rfidPoll(int *cardIndex, char *uidText, int uidTextSize)
{
    if (rfid == nullptr) {
        return false;
    }

    if (cardIndex == nullptr || uidText == nullptr) {
        return false;
    }

    if (!rfid->isNewCardPresent()) {
        return false;
    }

    uint8_t uid[10];
    uint8_t uidLen = 0;

    if (!rfid->readCardSerial(uid, &uidLen)) {
        return false;
    }

    rfid->haltA();
    rfid->stopCrypto1();

    uidText[0] = '\0';

    for (int i = 0; i < uidLen; i++) {
        char temp[4];

        snprintf(
            temp,
            sizeof(temp),
            "%02X",
            uid[i]
        );

        strncat(
            uidText,
            temp,
            uidTextSize - strlen(uidText) - 1
        );
    }

    int foundCard = findCardByUID(uid, uidLen);

    if (foundCard < 0) {
        *cardIndex = -1;
        return true;
    }

    *cardIndex = foundCard;

    printf(
        "RFID detected: %s -> card %d\n",
        uidText,
        foundCard
    );

    return true;
}
