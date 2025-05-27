#pragma once

#include <Arduino.h>

#define BUTTON_COUNT 3

#define BTN_1 4
#define BTN_2 5
#define BTN_3 6

#define BUTTON_PINS   {BTN_1, BTN_2, BTN_3}
#define BUTTON_JSONS  {"/buttons/btn1.json", "/buttons/btn2.json", "/buttons/btn3.json"}

void uploadToEpaperTag(const char* jsonFile);
void buttonTask(void* parameter);
void handleButtonPress(int btn);