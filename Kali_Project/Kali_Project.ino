#include "Arduino.h"
#include "TFT_eSPI.h"
#include "kali_logo.h"
#include "pin_config.h"
#include "arrow.h"
#include <Button2.h>
#include "menu_background.h"
#include "WiFi.h"
#include "strongWifi.h"
#include "weakWifi.h"
#include "noBattery.h"
#include "time.h"

TFT_eSPI tft;

TFT_eSprite backgroundSprite(&tft);
TFT_eSprite kaliSprite(&tft);
TFT_eSprite arrowSprite(&tft);
TFT_eSprite menuBgSprite(&tft);
TFT_eSprite wifiSprite(&tft);
TFT_eSprite batterySprite(&tft);
TFT_eSprite textSprite(&tft);
TFT_eSprite timeSprite(&tft);
TFT_eSprite dateSprite(&tft);

Button2 down_Button(1);
Button2 select_Button(2);

const int numOptionsPerPage = 3;
const char *optionsPage1[numOptionsPerPage] = {"Scan Wi-Fi", "Option 2a", "Option 3a"};
const char *optionsPage2[numOptionsPerPage] = {"Option 1b", "Option 2b", "Option 3b"};
const char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 3600;

int totalPages = 2;
int currentPage = 1;
int currentOption = 0;

const char *selectedOption = nullptr;

void setup() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);

    Serial.begin(115200);
    Serial.println("Hello T-Display-S3");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(true);

    backgroundSprite.createSprite(320, 170);
    kaliSprite.createSprite(96, 96);
    kaliSprite.setSwapBytes(true);
    menuBgSprite.createSprite(320, 170);
    menuBgSprite.setSwapBytes(true);
    wifiSprite.createSprite(20, 20);
    wifiSprite.setSwapBytes(true);
    batterySprite.createSprite(20, 20);
    batterySprite.setSwapBytes(true);
    timeSprite.createSprite(50, 20);
    timeSprite.setSwapBytes(true);
    dateSprite.createSprite(273, 20);
    dateSprite.setSwapBytes(true);
    arrowSprite.createSprite(16, 16);
    arrowSprite.setSwapBytes(true);
    textSprite.createSprite(320, 170);
    textSprite.setSwapBytes(true);

    backgroundSprite.fillSprite(TFT_DARKGREY);
    kaliSprite.pushImage(0, 0, 96, 96, kali_logo_final);
    kaliSprite.pushToSprite(&backgroundSprite, 105, 40, TFT_BLACK);

    backgroundSprite.pushSprite(0, 0);
    delay(2000);

    down_Button.setPressedHandler([](Button2& btn) {
        nextOption();
        mainMenu();
    });

    select_Button.setPressedHandler([](Button2& btn) {
        selectedMenuOptions();
    });

    mainMenu();
}

void loop() {
    down_Button.loop();
    select_Button.loop();
    printLocalTime();
    wifi_symbol();
    backgroundSprite.pushSprite(0, 0);
}

void wifi_symbol() {
    int rssi = WiFi.RSSI();
    wifiSprite.fillSprite(TFT_BLACK);

    if (WiFi.status() == WL_CONNECTED) {
        if (rssi > -70) {
            wifiSprite.pushImage(0, 0, 17, 17, strong_wifi);
        } else {
            wifiSprite.pushImage(0, 0, 17, 17, weak_wifi);
        }
    }

    wifiSprite.pushToSprite(&backgroundSprite, 273, 5, TFT_BLACK);
    batterySprite.pushImage(0, 0, 17, 17, no_battery);
    batterySprite.pushToSprite(&backgroundSprite, 297, 5, TFT_BLACK);
}

void selectedMenuOptions() {
    Serial.println("Select button pressed");
    Serial.println(selectedOption);

    if (strcmp(selectedOption, "Scan Wi-Fi") == 0) {
        scanWifi();
    } else {
        Serial.println("Build in progress");
        // Your code for Option 2
    }
}

void nextOption() {
    currentOption = (currentOption + 1) % numOptionsPerPage;

    if (currentOption == 0) {
        currentPage = (currentPage % totalPages) + 1;
    }
}

void mainMenu() {
    menuBgSprite.pushImage(0, 0, 320, 170, menuBackground);
    menuBgSprite.pushToSprite(&backgroundSprite, 0, 0);

    textSprite.fillSprite(TFT_BLACK);
    textSprite.setTextColor(TFT_WHITE, TFT_BLACK);

    if (currentPage == 1) {
        textSprite.drawString("Page 1", 0, 0, 4);
        textSprite.pushToSprite(&backgroundSprite, 10, 35, TFT_BLACK);
        drawOptions(optionsPage1);
    } else if (currentPage == 2) {
        textSprite.drawString("Page 2", 0, 0, 4);
        textSprite.pushToSprite(&backgroundSprite, 10, 35, TFT_BLACK);
        drawOptions(optionsPage2);
    }
}

void drawOptions(const char **options) {
    for (int i = 0; i < numOptionsPerPage; ++i) {
        int y = 70 + i * 30;
        textSprite.fillSprite(TFT_BLACK);

        if (i == currentOption) {
            textSprite.setTextColor(TFT_CYAN, TFT_BLACK);
            arrowSprite.pushImage(0, 0, 16, 16, arrowicons8_arrow_16);
            arrowSprite.pushToSprite(&backgroundSprite, 5, y, TFT_BLACK);
            selectedOption = options[i];
        } else {
            textSprite.setTextColor(TFT_WHITE, TFT_BLACK);
        }

        textSprite.setTextSize(1);
        textSprite.setFreeFont(&FreeSans9pt7b);
        textSprite.drawString(options[i], 0, 0, 1);
        textSprite.pushToSprite(&backgroundSprite, 26, y, TFT_BLACK);
        textSprite.setFreeFont(NULL);
    }
}

void scanWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("scan start");

    int n = WiFi.scanNetworks();
    Serial.println("scan done");

    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");

        for (int i = 0; i < n; ++i) {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
        }
    }

    Serial.println("");
    WiFi.reconnect();
}

void printLocalTime() {
    dateSprite.fillSprite(TFT_BLACK);
    timeSprite.fillSprite(TFT_BLACK);
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    String resultTime = String(timeinfo.tm_hour < 10 ? "0" + String(timeinfo.tm_hour) : timeinfo.tm_hour) +
                        ":" +
                        String(timeinfo.tm_min < 10 ? "0" + String(timeinfo.tm_min) : timeinfo.tm_min);

    String resultDate = String(timeinfo.tm_mday) + " " + String(monthNames[timeinfo.tm_mon]) + ", " +
                        String(timeinfo.tm_year + 1900);

    dateSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    dateSprite.setTextSize(1);
    dateSprite.setFreeFont(&FreeSans9pt7b);
    dateSprite.drawString(resultDate.c_str(), 0, 0, 1);
    dateSprite.pushToSprite(&backgroundSprite, 110, 5, TFT_BLACK);
    dateSprite.setFreeFont(NULL);

    timeSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    timeSprite.setTextSize(1);
    timeSprite.setFreeFont(&FreeSans9pt7b);
    timeSprite.drawString(resultTime.c_str(), 0, 0, 1);
    timeSprite.pushToSprite(&backgroundSprite, 5, 5, TFT_BLACK);
    timeSprite.setFreeFont(NULL);
}
