#pragma once

enum UIEvent {
    UI_EVENT_NONE,

    UI_EVENT_VIEW_RATION,
    UI_EVENT_BACK,

    UI_EVENT_DISPENSE_WHEAT,
    UI_EVENT_DISPENSE_RICE,
    UI_EVENT_DISPENSE_DAL,
    UI_EVENT_DISPENSE_SUGAR
};

void uiInit();

void uiShowIdle();
void uiShowUnknown(const char *uid);
void uiShowFamily(int cardIndex);
void uiShowRation(int cardIndex);

UIEvent uiUpdateFamily(int cardIndex);
UIEvent uiUpdateRation(int cardIndex);
