#include "buttons.h"

#include <ArduinoJson.h>

#include "commstructs.h"
#include "contentmanager.h"
#include "settings.h"
#include "storage.h"
#include "tag_db.h"
#include "web.h"

const int buttonPins[BUTTON_COUNT] = BUTTON_PINS;
const char* buttonJsons[BUTTON_COUNT] = BUTTON_JSONS;

void uploadToEpaperTag(const char* jsonFile) {
    File file = contentFS->open(jsonFile, "r");
    if (!file) {
        // Failed to open file
        return;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) {
        // Failed to parse JSON
        return;
    }
    auto macArray = doc["mac"].as<JsonArray>();
    if (!macArray.isNull()) {
        String jsonString;
        serializeJson(doc["json"], jsonString);
        for (JsonVariant v : macArray) {
            String dst = v.as<const char*>();
            if (dst.length() == 16) {
                uint8_t mac[8];
                if (hex2mac(dst, mac)) {
                    xSemaphoreTake(fsMutex, portMAX_DELAY);
                    File file = contentFS->open("/current/" + dst + ".json", "w");
                    if (!file) {
                        // Failed to create file
                        xSemaphoreGive(fsMutex);
                        continue;
                    }
                    file.print(jsonString);
                    file.close();
                    xSemaphoreGive(fsMutex);
                    tagRecord *taginfo = tagRecord::findByMAC(mac);
                    if (taginfo != nullptr) {
                        uint32_t ttl = doc["ttl"].as<uint32_t>();
                        taginfo->modeConfigJson = "{\"filename\":\"/current/" + dst + ".json\",\"interval\":\"" + String(ttl) + "\"}";
                        taginfo->contentMode = 19;
                        taginfo->nextupdate = 0;
                        wsSendTaginfo(mac, SYNC_USERCFG);
                    }
                }
            }
        }
    }
}

void buttonTask(void* parameter) {
    bool btnPrev[BUTTON_COUNT];
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        btnPrev[i] = HIGH;
    }

    while (1) {
        for (int i = 0; i < BUTTON_COUNT; ++i) {
            bool btnCurr = digitalRead(buttonPins[i]);
            if (btnPrev[i] == HIGH && btnCurr == LOW) {
                uploadToEpaperTag(buttonJsons[i]);
            }
            btnPrev[i] = btnCurr;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}