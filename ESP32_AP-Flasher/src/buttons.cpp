#include "buttons.h"

#include <ArduinoJson.h>

#include "commstructs.h"
#include "contentmanager.h"
#include "settings.h"
#include "storage.h"
#include "tag_db.h"
#include "web.h"

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
    String dst = doc["mac"] | "";
    if (dst.length() != 16) {
        // Invalid MAC address length
        return;
    }
    uint8_t mac[8];
    if (hex2mac(dst, mac)) {
        xSemaphoreTake(fsMutex, portMAX_DELAY);
        File file = contentFS->open("/current/" + dst + ".json", "w");
        if (!file) {
            // Failed to create file
            xSemaphoreGive(fsMutex);
            return;
        }
        String jsonString;
        serializeJson(doc["json"], jsonString);
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

void buttonTask(void* parameter) {
    pinMode(BTN_1, INPUT_PULLUP);
    pinMode(BTN_2, INPUT_PULLUP);
    pinMode(BTN_3, INPUT_PULLUP);

    while (1) {
        if (digitalRead(BTN_1) == LOW) {
            uploadToEpaperTag("/buttons/btn1.json");
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        if (digitalRead(BTN_2) == LOW) {
            uploadToEpaperTag("/buttons/btn2.json");
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        if (digitalRead(BTN_3) == LOW) {
            uploadToEpaperTag("/buttons/btn3.json");
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}