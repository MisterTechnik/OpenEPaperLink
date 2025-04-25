#pragma once

#include <Arduino.h>

#define BTN_1 4
#define BTN_2 5
#define BTN_3 6

void uploadToEpaperTag(const char* jsonFile);
void buttonTask(void* parameter);
void handleButtonPress(int btn);