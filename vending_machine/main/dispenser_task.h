#pragma once

void dispenserInit();

bool dispenserStart(int cardIndex, int grainIndex);

void dispenserUpdate();

bool dispenserBusy();
