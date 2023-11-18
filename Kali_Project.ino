#include "Arduino.h"
#include "TFT_eSPI.h"
#include "pin_config.h"
#include <Button2.h>
#include "WiFi.h"
#include "time.h"
#include "Objects.h"


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
TFT_eSprite loaderSprite(&tft);

Button2 down_Button(1);
Button2 up_Button(3);
Button2 select_Button(2);
Button2 menu_Button(10);

const int numOptionsPerPage = 3;
const int daylightOffset_sec = 3600;
const int selectedNumOptionsPerPage = 4;

const char *optionsPage1[] = {"Scan Wi-Fi", "Enable AP", "Deauth Atck", "Auth Atck"};
const char *optionsPage2[numOptionsPerPage] = {"Option 1b", "Option 2b", "Option 3b"};
const char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *ntpServer = "pool.ntp.org";
const char *networkList[10] = {};

const long  gmtOffset_sec = 19800;

int totalPages = 2;
int currentPage = 1;
int currentOption = 0;
int selectedCurrentOption = 0;
int selectedTotalPages = 2;
int selectedCurrentPage = 1;
int totalNetworksFound = 0;

String resultTime;
String resultDate;
String optionHovered;
String selectedOption;

void setup() {
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.begin(115200);
    Serial.println("Hello T-Display-S3");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(true);

    backgroundSprite.createSprite(320, 170);
    loaderSprite.createSprite(320, 170);
    loaderSprite.setSwapBytes(true);
    kaliSprite.createSprite(96, 96);
    kaliSprite.setSwapBytes(true);
    menuBgSprite.createSprite(320, 170);
    menuBgSprite.setSwapBytes(true);
    wifiSprite.createSprite(20, 20);
    wifiSprite.setSwapBytes(true);
    batterySprite.createSprite(20, 20);
    batterySprite.setSwapBytes(true);
    timeSprite.createSprite(70, 30);
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
        selectedOptionToDisplay();
    });

    up_Button.setPressedHandler([](Button2& btn) {
        selectedOptionToDisplay();
    });

    select_Button.setPressedHandler([](Button2& btn) {
        selectedMenuOptions();
    });
    menu_Button.setPressedHandler([](Button2& btn) {
        menuButtonPressed();
    });
    selectedOptionToDisplay(); 
}

void selectedOptionToDisplay(){
  if(selectedOption.isEmpty() == true){
    if(down_Button.isPressed() == true){nextOption();}
    if(up_Button.isPressed() == true){prevOption();}
    mainMenu(); 
    displayTaskBar();
  }
  else if(strcmp(selectedOption.c_str(), "Scan Wi-Fi") == 0){
    if(down_Button.isPressed() == true){selectedNextOption();}
    if(up_Button.isPressed() == true){selectedPrevOption();}
    displayScannedNetworks(totalNetworksFound);
  }
}

void menuButtonPressed(){
  optionHovered = "";
  selectedOption = "";
  selectedOptionToDisplay();
}

void loop() {
    menu_Button.loop();
    up_Button.loop();
    down_Button.loop();
    select_Button.loop();
    backgroundSprite.pushSprite(0, 0);
}

void displayTaskBar(){
  wifi_symbol();
  printLocalTime();
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
    selectedOption = optionHovered;
    Serial.println(selectedOption);

    if (strcmp(selectedOption.c_str(), "Scan Wi-Fi") == 0) {
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

void selectedNextOption() {
  int selectedPages = selectedNumOptionsPerPage;

  if(strcmp(selectedOption.c_str(), "Scan Wi-Fi") == 0){ selectedPages = totalNetworksFound <= selectedNumOptionsPerPage ? totalNetworksFound : selectedNumOptionsPerPage; }
    selectedCurrentOption = (selectedCurrentOption + 1) % selectedPages;

    if (currentOption == 0) {
        selectedCurrentPage = (selectedCurrentPage % selectedTotalPages) + 1;
    }
    Serial.println(selectedPages);
    Serial.println(selectedCurrentPage);
    Serial.println(selectedCurrentOption);
    Serial.println(selectedTotalPages);
}

void prevOption() {
    if (currentPage > 1 || currentOption > 0) {
        currentOption = (currentOption - 1 + numOptionsPerPage) % numOptionsPerPage;

        if (currentOption == numOptionsPerPage - 1 && currentPage > 1) {
            currentPage = (currentPage - 1 + totalPages) % totalPages;
        }
    }
}

void selectedPrevOption() {
  int selectedPages = selectedNumOptionsPerPage;

  if(strcmp(selectedOption.c_str(), "Scan Wi-Fi") == 0){ selectedPages = totalNetworksFound <= selectedNumOptionsPerPage ? totalNetworksFound : selectedNumOptionsPerPage; }
    if (selectedCurrentPage > 1 || selectedCurrentOption > 0) {
        selectedCurrentOption = (selectedCurrentOption - 1 + selectedPages) % selectedPages;

        if (selectedCurrentOption == selectedPages - 1 && selectedCurrentPage > 1) {
            selectedCurrentPage = (selectedCurrentPage - 1 + selectedTotalPages) % selectedTotalPages;
        }
    }
    Serial.println(selectedPages);
    Serial.println(selectedCurrentPage);
    Serial.println(selectedCurrentOption);
    Serial.println(selectedTotalPages);
}


void mainMenu() {
    menuBgSprite.pushImage(0, 0, 320, 170, menuBackground);
    menuBgSprite.pushToSprite(&backgroundSprite, 0, 0);

    textSprite.fillSprite(TFT_BLACK);
    textSprite.setTextColor(TFT_WHITE, TFT_BLACK);

    if (currentPage == 1) {
        textSprite.drawString("Wi-Fi:", 0, 0, 4);
        textSprite.pushToSprite(&backgroundSprite, 10, 35, TFT_BLACK);
        drawOptions(optionsPage1);
        // int arrayLength = sizeof(optionsPage1) / sizeof(optionsPage1[0]);
        // Serial.println(arrayLength);
    } else if (currentPage == 2) {
        textSprite.drawString("BLTE:", 0, 0, 4);
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
            optionHovered = (String)options[i];
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
    delay(10);
    loaderSprite.fillSprite(TFT_BLACK);
    loaderSprite.pushImage(145, 70, 30, 30, loading_0);
    loaderSprite.pushSprite(0, 0, blendColors(0x0000, 0xFFFF, 64));

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(50);

    loaderSprite.fillSprite(TFT_BLACK);
    loaderSprite.pushImage(145, 70, 30, 30, loading_30);
    loaderSprite.pushSprite(0, 0, blendColors(0x0000, 0xFFFF, 64));

    Serial.println("scan start");
    int n = WiFi.scanNetworks();

    loaderSprite.fillSprite(TFT_BLACK);
    loaderSprite.pushImage(145, 70, 30, 30, loading_60);
    loaderSprite.pushSprite(0, 0, blendColors(0x0000, 0xFFFF, 64));

    Serial.println("scan done");

    Serial.println("");
    WiFi.reconnect();
    
    totalNetworksFound = n;

    loaderSprite.fillSprite(TFT_BLACK);
    loaderSprite.pushImage(145, 70, 30, 30, loading_90);
    loaderSprite.pushSprite(0, 0, blendColors(0x0000, 0xFFFF, 4));

    delay(10);

    dateSprite.fillSprite(TFT_BLACK);
    timeSprite.fillSprite(TFT_BLACK);
    backgroundSprite.fillSprite(TFT_DARKGREY);

    displayScannedNetworks(n);
}

void displayScannedNetworks(int n){
    backgroundSprite.fillSprite(TFT_DARKGREY);
    if (n == 0) {
        textSprite.setTextColor(TFT_GOLD, TFT_BLACK);
        textSprite.fillSprite(TFT_BLACK);
        textSprite.setTextSize(1);
        textSprite.setFreeFont(&FreeSans9pt7b);
        textSprite.drawString("No Networks found", 0, 0, 1);
        textSprite.pushToSprite(&backgroundSprite, 145, 70, TFT_BLACK);
        textSprite.setFreeFont(NULL);
    } else {
        textSprite.setTextColor(TFT_GOLD, TFT_BLACK);
        textSprite.fillSprite(TFT_BLACK);
        textSprite.setTextSize(1);
        textSprite.setFreeFont(&FreeSans9pt7b);
        String header = "Scan Complete, " + (String)n + " Network"+ (n > 1 ? "s " : " ") + "found..";
        textSprite.drawString(header, 0, 0, 1);
        textSprite.pushToSprite(&backgroundSprite, 20, 5, TFT_BLACK);
        textSprite.setFreeFont(NULL);

        int length = n <= selectedNumOptionsPerPage ? n : selectedNumOptionsPerPage;
        for (int i = 0; i < length; ++i) {

          int y = 40 + i * 30;
          String resultList = (String)(i + 1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");

          if (i == selectedCurrentOption) {
            textSprite.setTextColor(TFT_CYAN, TFT_BLACK);
            arrowSprite.pushImage(0, 0, 16, 16, arrowicons8_arrow_16);
            arrowSprite.pushToSprite(&backgroundSprite, 5, y, TFT_BLACK);
          } else {
            textSprite.setTextColor(TFT_WHITE, TFT_BLACK);
          }
          textSprite.fillSprite(TFT_BLACK);
          textSprite.setTextSize(1);
          textSprite.setFreeFont(&FreeSans9pt7b);
          textSprite.drawString(resultList.c_str(), 0, 0, 1);
          textSprite.pushToSprite(&backgroundSprite, 30, y, TFT_BLACK);
          textSprite.setFreeFont(NULL);

          delay(10);
        }
    }
}

void printLocalTime() {
  if (WiFi.status() == WL_CONNECTED){
    dateSprite.fillSprite(TFT_BLACK);
    timeSprite.fillSprite(TFT_BLACK);
    delay(10);
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    resultTime = String(timeinfo.tm_hour < 10 ? "0" + String(timeinfo.tm_hour) : timeinfo.tm_hour) +
                        ":" +
                        String(timeinfo.tm_min < 10 ? "0" + String(timeinfo.tm_min) : timeinfo.tm_min);

    resultDate = String(timeinfo.tm_mday) + " " + String(monthNames[timeinfo.tm_mon]) + ", " +
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
}


uint16_t blendColors(uint16_t color1, uint16_t color2, uint8_t opacity) {
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;

    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;

    uint8_t r = ((r1 * (255 - opacity)) + (r2 * opacity)) / 255;
    uint8_t g = ((g1 * (255 - opacity)) + (g2 * opacity)) / 255;
    uint8_t b = ((b1 * (255 - opacity)) + (b2 * opacity)) / 255;

    return tft.color565(r, g, b);
}
