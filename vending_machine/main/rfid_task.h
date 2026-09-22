#pragma once

#include <stdint.h>

void rfidInit();

bool rfidPoll(int *cardIndex, char *uidText, int uidTextSize);
