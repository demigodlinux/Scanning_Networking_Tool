#include "Arduino.h"
#include "pin_config.h"
#include "TFT_eSPI.h"
#include <Button2.h>
#include "WiFi.h"
#include "time.h"
#include "Objects.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "webPages.h"
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include <ArduinoJson.h>

/*-----------HARDWARE INITIALIZATIONS--------------*/
SSD1306Wire display(0x3c, SDA, SCL); 
TaskHandle_t Task1;
RF24 radio(CS, CSN); 
TFT_eSPI tft;
AsyncWebServer server(80);
/*-------------------------------------------------*/

/*-----------------DISPLAY SPRITES-----------------*/
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
/*--------------------------------------------------*/

/*---------------BUTTON INITIALIZATIONS-------------*/
Button2 down_Button(1);
Button2 up_Button(2);
Button2 select_Button(3);
Button2 menu_Button(21);
/*--------------------------------------------------*/

/*-----------------CONSTANTS------------------------*/
const int numOptionsPerPage = 3;
const int daylightOffset_sec = 3600;
const int selectedNumOptionsPerPage = 4;

const char *optionsPage1[5] = {"Scan Wi-Fi", "Enable Server", "Deauth Attack", "Auth Attack", "Set WI-FI_Str"};
const char *optionsPage2[3] = {"BTE EG", "Option 2b", "Option 3b"};
const char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *ntpServer = "pool.ntp.org";
const char *networkList[10] = {};
const char *pin = "1234"; // Change this to more secure PIN.
const long  gmtOffset_sec = 19800;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
const byte address[6] = "00001"; 
/*--------------------------------------------------*/

/*---------------INTEGERS VARIABLES-----------------*/
int totalPages = 2;
int currentPage = 1;
int currentOption = 0;
int selectedCurrentOption = 0;
int selectedTotalPages = 2;
int selectedCurrentPage = 1;
int totalNetworksFound = 0;
int sizeOfArray = 0;
int innerOptionIndexSelected = 0;
//---- Response codes----//
int attempts = 0;
int successCode = 0;
int peripheralNotEnabledCode = 0;
//-----------------------//
/*--------------------------------------------------*/

/*----------------STRING VARIABLES------------------*/
String Wifi_SSID = WIFI_SSID;
String Wifi_Pass = WIFI_PASSWORD;
String resultTime;
String resultDate;
String optionHovered = "";
String selectedOption = "";
String selectedInnerOption = "";
String innerOptionHovered = "";
String json;
// Variable to store the HTTP request
String header;
String peripheralNotEnabledDevice = "";
/*--------------------------------------------------*/

/*-------------------MISC VARIABLES-----------------*/
bool serverStarted = false;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
/*--------------------------------------------------*/

//----------------Task1code: for Oled Display
void Core0Tasks( void * pvParameters ){
  Serial.print("Core0Tasks running on core ");
  Serial.println(xPortGetCoreID());
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  while(successCode == 0){
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(5, 17, "Hello User!!");
    display.display();
  }
  for(;;){
    oledDisplay();
    delay(500);
  } 
}

void setup() {
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);
    delay(100);

    WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask());
    WiFi.begin(Wifi_SSID, Wifi_Pass);
    
    while(WiFi.status() != WL_CONNECTED && attempts <= 5){
      delay(100);
      attempts += 1;
    }
    attempts = 0;
    Serial.begin(115200);
    Serial.println("Hello T-Display-S3");

    xTaskCreatePinnedToCore(
              Core0Tasks,  /* Task function. */
              "Core0Task", /* name of task. */
              10000,       /* Stack size of task */
              NULL,        /* parameter of the task */
              1,           /* priority of the task */
              &Task1,      /* Task handle to keep track of created task */
              0);          /* pin task to core 0 */ 

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    tft.init();
    tft.setRotation(3);
    tft.setSwapBytes(true);

  #pragma region sprite_Initializations
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
  #pragma endregion 

    backgroundSprite.fillSprite(TFT_DARKGREY);
    kaliSprite.pushImage(0, 0, 96, 96, kali_logo_final);
    kaliSprite.pushToSprite(&backgroundSprite, 105, 40, TFT_BLACK);

    backgroundSprite.pushSprite(0, 0);
    delay(2000);

  #pragma region button_Press_Handlers
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
  #pragma endregion 

    selectedOptionToDisplay(); 
    successCode = 1;
}

void selectedOptionToDisplay(){
  if(selectedOption.isEmpty() == true){
    if(down_Button.isPressed() == true){nextOption();}
    if(up_Button.isPressed() == true){prevOption();}
    mainMenu();
  }
  else if(strcmp(selectedOption.c_str(), "Scan Wi-Fi") == 0){
    if(down_Button.isPressed() == true){selectedNextOption();}
    if(up_Button.isPressed() == true){selectedPrevOption();}
    displayScannedNetworks(totalNetworksFound);
  }
}

void menuButtonPressed(){
  if(serverStarted == true){
    server.end();
    serverStarted = false;
  }
  optionHovered = "";
  selectedOption = "";
  selectedInnerOption = "";
  innerOptionHovered = "";
  innerOptionIndexSelected = 0;
  selectedCurrentOption = 0;
  selectedOptionToDisplay();
}

void loop() {
    menu_Button.loop();
    up_Button.loop();
    down_Button.loop();
    select_Button.loop();
    if(selectedOption.isEmpty() == true){
      displayTaskBar();
    }
    backgroundSprite.pushSprite(0, 0);
}

void displayTaskBar(){
  batterySymbol();
  wifi_symbol();
  printLocalTime();
}

void batterySymbol(){
   int battValue = (int)((analogRead(PIN_BAT_VOLT) * (3.7 / 1024)) * 10);
   if(battValue > 85){
    batterySprite.pushImage(0, 0, 17, 17, full_battery);
    batterySprite.pushToSprite(&backgroundSprite, 295, 5, TFT_BLACK);
   }
   else if(battValue > 55 && battValue <= 85){
    batterySprite.pushImage(0, 0, 17, 17, half_battery);
    batterySprite.pushToSprite(&backgroundSprite, 295, 5, TFT_BLACK);
   }
   else if(battValue > 20 && battValue <= 55){
    batterySprite.pushImage(0, 0, 17, 17, low_battery);
    batterySprite.pushToSprite(&backgroundSprite, 295, 5, TFT_BLACK);
   }
   else{
    batterySprite.pushImage(0, 0, 17, 17, very_low_battery);
    batterySprite.pushToSprite(&backgroundSprite, 295, 5, TFT_BLACK);
   }
   
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
}

void selectedMenuOptions() {
    selectedOption = optionHovered;
    selectedInnerOption = innerOptionHovered;
    if (strcmp(selectedOption.c_str(), "Scan Wi-Fi") == 0) {
        if(strcmp(selectedInnerOption.c_str(), "") == 0){
          scanWifi();
        }
        else if(strcmp(selectedInnerOption.c_str(), WiFi.SSID(innerOptionIndexSelected).c_str()) == 0){
          displayWifiDetails();
        }
    } 
    else if(strcmp(selectedOption.c_str(), "Enable Server") == 0){
        enableServer();
    }
    else if(strcmp(selectedOption.c_str(), "BTE EG") == 0){
        nrF();
    }
    else if(strcmp(selectedOption.c_str(), "Set WI-FI_Str") == 0){
      setWifi();
    }
    else {
        Serial.println("Build in progress");
        // Your code for Option 2
    }
}

void nextOption() {
    int pageListOptions = sizeOfArray <= numOptionsPerPage ? numOptionsPerPage : sizeOfArray;
    currentOption = (currentOption + 1) % pageListOptions;

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
}

void prevOption() {
  int pageListOptions = sizeOfArray <= numOptionsPerPage ? numOptionsPerPage : sizeOfArray;
  Serial.println(pageListOptions);
    if (currentPage > 1 || currentOption > 0) {
        currentOption = (currentOption - 1 + pageListOptions) % pageListOptions;

        if (currentOption == pageListOptions - 1 && currentPage > 1) {
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
}

void mainMenu() {
    menuBgSprite.pushImage(0, 0, 320, 170, menuBackground);
    menuBgSprite.pushToSprite(&backgroundSprite, 0, 0);

    textSprite.fillSprite(TFT_BLACK);
    textSprite.setTextColor(TFT_WHITE, TFT_BLACK);

    if (currentPage == 1) {
      Serial.println("Page 1");
        textSprite.drawString("Wi-Fi:", 0, 0, 4);
        textSprite.pushToSprite(&backgroundSprite, 10, 35, TFT_BLACK);
        sizeOfArray = sizeof(optionsPage1)/sizeof(optionsPage1[0]);
        drawOptions(optionsPage1, sizeOfArray);
        // int arrayLength = sizeof(optionsPage1) / sizeof(optionsPage1[0]);
        // Serial.println(arrayLength);
    } else if (currentPage == 2) {
      Serial.println("Page 2");
        textSprite.drawString("nRF:", 0, 0, 4);
        textSprite.pushToSprite(&backgroundSprite, 10, 35, TFT_BLACK);
        sizeOfArray = sizeof(optionsPage2)/sizeof(optionsPage2[0]);
        drawOptions(optionsPage2, sizeOfArray);
    }
}

void drawOptions(const char **options, int size) {
  int pageListOptions = size <= numOptionsPerPage ? numOptionsPerPage : size;

  Serial.println(size);
  Serial.println(pageListOptions);
    int j = 0;
    for (int i = 0; i < pageListOptions; ++i) {
      textSprite.fillSprite(TFT_BLACK);

        if(i < numOptionsPerPage){
          Serial.println("First Column");
          int y = 70 + i * 30;
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
        else{
          Serial.println("Second Column");
          int y = 35 + j * 30;
          if (i == currentOption) {
            textSprite.setTextColor(TFT_CYAN, TFT_BLACK);
            arrowSprite.pushImage(0, 0, 16, 16, arrowicons8_arrow_16);
            arrowSprite.pushToSprite(&backgroundSprite, 165, y, TFT_BLACK);
            optionHovered = (String)options[i];
        } else {
            textSprite.setTextColor(TFT_WHITE, TFT_BLACK);
        }

        textSprite.setTextSize(1);
        textSprite.setFreeFont(&FreeSans9pt7b);
        textSprite.drawString(options[i], 0, 0, 1);
        textSprite.pushToSprite(&backgroundSprite, 186, y, TFT_BLACK);
        textSprite.setFreeFont(NULL);
        j += 1 ;

        }
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
        textSprite.pushToSprite(&backgroundSprite, 60, 70, TFT_BLACK);
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
            innerOptionHovered = WiFi.SSID(i);
            innerOptionIndexSelected = i;
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

void displayWifiDetails(){
  int n = innerOptionIndexSelected;
  backgroundSprite.fillSprite(TFT_DARKGREY);

  textSprite.setTextColor(TFT_GOLD, TFT_BLACK);
  textSprite.fillSprite(TFT_BLACK);
  textSprite.setTextSize(1);
  textSprite.setFreeFont(&FreeSans9pt7b);
  String resultList = (String)(n + 1) + ": " + WiFi.SSID(n) + " Str: (" + (WiFi.RSSI(n) > -70 ? "GOOD" : "FAIR") + ") " + ((WiFi.encryptionType(n) == WIFI_AUTH_OPEN) ? " " : "*");
  textSprite.drawString(resultList.c_str(), 5, 5, 1);
  textSprite.pushToSprite(&backgroundSprite, 0, 0, TFT_BLACK);

  textSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  textSprite.fillSprite(TFT_BLACK);
  textSprite.setTextSize(1);
  textSprite.setFreeFont(&FreeSans9pt7b);

  String BSSID = "MAC Addr: " + macAddressToString(WiFi.BSSID(n));
  textSprite.drawString(BSSID.c_str(), 10, 40, 1);

  String Channel = "Channel: " + (String)WiFi.channel(n);
  textSprite.drawString(Channel.c_str(), 10, 65, 1);

  String encryptType = "Encryp Type: " + getEncryptionType(WiFi.encryptionType(n));

  textSprite.drawString(encryptType.c_str(), 10, 90, 1);

  String RSSI = "Signal Str: " + (String)(WiFi.RSSI(n) > -70 ? "GOOD " : (WiFi.RSSI(n) < -70 && WiFi.RSSI(n) > -80 ? "FAIR " : "POOR ")) + "( " + (String)WiFi.RSSI(n) + " dBm )" ;

  textSprite.drawString(RSSI.c_str(), 10, 115, 1);

  textSprite.pushToSprite(&backgroundSprite, 0, 0, TFT_BLACK);
  textSprite.setFreeFont(NULL);
}

void setWifi(){  
  bool sdState = initSDCard();
  String ipAdd;

  delay(10);
  loaderSprite.fillSprite(TFT_BLACK);
  loaderSprite.pushImage(145, 70, 30, 30, loading_0);
  loaderSprite.pushSprite(0, 0, blendColors(0x0000, 0xFFFF, 64));

  if(sdState == true){
    peripheralDeviceSet(0, "");

    if(WiFi.status() == WL_CONNECTED){
    ipAdd = WiFi.localIP().toString();
  }
  else{
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MyESP32AP", "aa221100");
    ipAdd = WiFi.softAPIP().toString();
  }

  displayServerDetails(ipAdd);

  //wifiHtmlString
  File file = SD.open("/ConnectionString.json");

  if(!file){
    file = SD.open("/ConnectionString.json", FILE_WRITE);
    const char* connectionString = R"({
      "Wifi_SSID": "Aadinandika 2.4G",
      "Wifi_Pass": "aa221100"
    })";
    file.print(connectionString);
    file.close();
    Serial.println("Connection String saved to file: ConnectionString.json" );
  }

  file.close();

  file = SD.open("/wifiIndex.html", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.print(wifiHtmlString);
  file.close();
  Serial.println("HTML saved to file: wifiIndex.html");

  wifiServerInitialization();

  }
  else{
    peripheralDeviceSet(1, "SD");
    menuButtonPressed();
  }
}

void nrF(){
  radio.begin();    

  if(radio.begin()){
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN); 
    radio.stopListening(); 

    bool buttonPressed = select_Button.isPressed();
    Serial.println("Button pressed");
    Serial.println(buttonPressed);
    radio.write(&buttonPressed, sizeof(buttonPressed)); 
    if(buttonPressed == true)
    {
    Serial.println("Button High");
    const char text[] = "Your Button State is HIGH";
    radio.write(&text, sizeof(text));                
    }
    else
    {
    Serial.println("Button Low");
    const char text[] = "Your Button State is LOW";
    radio.write(&text, sizeof(text)); 
    }
  }
  else{
    peripheralDeviceSet(1, "RF");
    menuButtonPressed();
  }
}

void enableServer(){
  bool sdState = initSDCard();
  String ipAdd;

  delay(10);
  loaderSprite.fillSprite(TFT_BLACK);
  loaderSprite.pushImage(145, 70, 30, 30, loading_0);
  loaderSprite.pushSprite(0, 0, blendColors(0x0000, 0xFFFF, 64));

  if(sdState == true){
    peripheralDeviceSet(0, "");

    if(WiFi.status() == WL_CONNECTED){
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    ipAdd = WiFi.localIP().toString();
  }
  else{
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MyESP32AP", "aa221100");
    Serial.println("");
    Serial.println("AP Created.");
    Serial.println("IP address: ");
    ipAdd = WiFi.softAPIP().toString();
  }

  delay(10);
  loaderSprite.fillSprite(TFT_BLACK);
  loaderSprite.pushImage(145, 70, 30, 30, loading_30);
  loaderSprite.pushSprite(0, 0, blendColors(0x0000, 0xFFFF, 64));

  File file = SD.open("/main.html", FILE_WRITE);
  Serial.println(file);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.print(mainHtml);
  file.close();

  Serial.println("HTML saved to file: main.html");

  file = SD.open("/index.html", FILE_WRITE);
  Serial.println(file);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.print(htmlString);
  file.close();
  
  Serial.println("HTML saved to file: index.html");

  loaderSprite.fillSprite(TFT_BLACK);
  loaderSprite.pushImage(145, 70, 30, 30, loading_60);
  loaderSprite.pushSprite(0, 0, blendColors(0x0000, 0xFFFF, 64));

  serverInitiation();

  delay(10);
  loaderSprite.fillSprite(TFT_BLACK);
  loaderSprite.pushImage(145, 70, 30, 30, loading_90);
  loaderSprite.pushSprite(0, 0, blendColors(0x0000, 0xFFFF, 64));

  delay(50);

  displayServerDetails(ipAdd);

  }
  else{
    peripheralDeviceSet(1, "SD");
    menuButtonPressed();
  }

}

void displayServerDetails(String ipAdd){
  backgroundSprite.fillSprite(TFT_DARKGREY);

  textSprite.setTextColor(TFT_GOLD, TFT_BLACK);
  textSprite.fillSprite(TFT_BLACK);
  textSprite.setTextSize(1);
  textSprite.setFreeFont(&FreeSans9pt7b);
  String serverEnabled = "Server Enabled";
  textSprite.drawString(serverEnabled, 5, 5, 1);
  textSprite.pushToSprite(&backgroundSprite, 0, 0, TFT_BLACK);

  textSprite.setTextColor(TFT_WHITE, TFT_BLACK);
  textSprite.fillSprite(TFT_BLACK);
  textSprite.setTextSize(1);
  textSprite.setFreeFont(&FreeSans9pt7b);
  
  if(WiFi.status() == WL_CONNECTED){
    String message = "Connect to server to continue.." ;
    textSprite.drawString(message, 10, 40, 1);

    textSprite.setTextSize(2);
    textSprite.drawString(ipAdd, 50, 85, 1);
  }
  else{
    String message1 = "Connect to wifi: MyESP32AP";
    textSprite.drawString(message1, 10, 40, 1);

    String message2 = "and then connect to server..";
    textSprite.drawString(message2, 10, 60, 1);

    textSprite.setTextSize(2);
    textSprite.drawString(ipAdd, 50, 85, 1);
  }
  
  textSprite.pushToSprite(&backgroundSprite, 0, 0, TFT_BLACK);
  textSprite.setTextSize(1);
  textSprite.setFreeFont(NULL);
}

void peripheralDeviceSet(int code, String device){
  peripheralNotEnabledCode = code;
  peripheralNotEnabledDevice = device;
}

bool initSDCard(){
  SD.begin();
  if(!SD.begin()){
    Serial.println("Card Mount Failed");
    return false;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return false;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  return true;
}

void oledDisplay(){

    delay(10);
    int sdPulse = (int)(analogRead(17) * (5.0 / 2056));
    int rfPulse = (int)(analogRead(18) * (5.0 / 1786));

    if(peripheralNotEnabledCode == 1 && (sdPulse <= 8 || rfPulse <= 10)){
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_16);
      display.drawString(5, 14, "Enable " + peripheralNotEnabledDevice + " to");
      display.drawString(5, 30, "continue...");
      display.display();
    }
    else if(sdPulse > 8 && rfPulse <= 10){
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_16);
      display.drawString(5, 17, "SD Enabled");
      display.display();
    }
    else if((strcmp(selectedOption.c_str(), "Enable Server") == 0) && sdPulse <= 8 && peripheralNotEnabledCode == 0){
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_16);
      display.drawString(5, 14, "Oops.. SD device");
      display.drawString(5, 30, "disabled!!");
      display.display();
    }
    else if((strcmp(selectedOption.c_str(), "BTE EG") == 0) && rfPulse <= 10 && peripheralNotEnabledCode == 0){
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_16);
      display.drawString(5, 14, "Oops.. RF device");
      display.drawString(5, 30, "disabled!!");
      display.display();
    }
    else if(rfPulse > 10 && sdPulse <= 8){
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_16);
      display.drawString(5, 17, "RF Enabled");
      display.display();
    }
    else if(rfPulse <= 10 && sdPulse <= 8){
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_16);
      display.drawString(5, 14, "Peripherals not");
      display.drawString(5, 30, "enabled??");
      display.display();
    }
    else{
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_16);
      display.drawString(5, 14, "You can't run");
      display.drawString(5, 30, "both at a time!!");
      display.display();
    }
}

void wifiServerInitialization(){
  server.on("/SetWifi", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/wifiIndex.html", "text/html");
  });

  server.on(
    "/SaveWifi",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {
        String connectionString = request->header("Content-Body");

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, connectionString);
          // Check for parsing errors
        if (error) {
          Serial.print(F("Error parsing JSON: "));
          Serial.println(error.c_str());
          AsyncWebServerResponse *response = request->beginResponse(400);
          request->send(response);
          return;
        }
        else{
          const char* ssid = doc["Wifi_SSID"];
          const char* pass = doc["Wifi_Pass"];
          WiFi.begin(ssid, pass);

          while (WiFi.status() != WL_CONNECTED && attempts <= 5){
            delay(50);
            attempts++;
          }
          if(attempts >= 5){
            WiFi.begin(Wifi_SSID, Wifi_Pass);
            AsyncWebServerResponse *response = request->beginResponse(500);
            request->send(response);
            attempts = 0;
          }
          else{
            // Send the response
            File file = SD.open("/ConnectionString.json", FILE_WRITE);
            file.print(connectionString);
            file.close();

            AsyncWebServerResponse *response = request->beginResponse(200);
            request->send(response);
            attempts = 0;
            menuButtonPressed();
          }
      }
  });
}

void serverInitiation(){
  delay(50);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD, "/main.html", "text/html");
  });

  server.serveStatic("/", SD, "/").setDefaultFile("main.html");

  server.on("/Directory", HTTP_GET, [](AsyncWebServerRequest *request){
    json = getDirectoryJSON("/"); // Get directory as JSON
    saveJSONToFile(json, "/data.json");
    request->send(SD, "/index.html", "text/html");
  });

  wifiServerInitialization();

  server.on(
    "/Directory",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {
    // Check if the request has a file attached
    String fileData = request->header("File");
    String buffData = request->header("Buffer");

    Serial.println(fileData);
    Serial.println(buffData);

    StaticJsonDocument<200> fileW;
    deserializeJson(fileW, fileData);

    StaticJsonDocument<200> buffW;
    deserializeJson(buffW, buffData);

      const char* name = fileW["name"];
      const char* type = fileW["type"];
      JsonArray buff = buffW["buffer"];
      size_t size = fileW["size"];

      // Save the file to the SD card
      String path = (String)"/" + name;
      File uploadFile = SD.open(path.c_str(), FILE_WRITE);
      if (uploadFile) {
          uint8_t byteValue;
          for (size_t i = 0; i < size; i++) {
            byteValue = buff[i];
            uploadFile.write(byteValue);
          }
        uploadFile.close();
        // Optionally, you can do something with the uploaded file here
        Serial.printf("Uploaded file: %s, size: %u\n", name, size);
      } else {
        Serial.println("Error opening file on SD card.");
        AsyncWebServerResponse *response = request->beginResponse(500, "text/plain", "Error writing file!");
        // Send the response
        request->send(response);
      }
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "File uploaded successfully!");
      // Send the response
      request->send(response);
  });
  
  server.begin();
  serverStarted = true;
  delay(50);
}

String getDirectoryJSON(const String &path) {

  File root = SD.open(path);
  Serial.println(root);
  if (!root) {
    Serial.println("Failed to open directory");
    return "{}";
  }
  String json = "[";
  while (File file = root.openNextFile()) {
    String fileType = (String)file.name();
    json += (String) "{ \"name\": \"" + fileType + "\", ";
     json += (String) "\"size\": \"" + String((String)((int)file.size() / 1024) + " kB") + "\", ";
    json += (String) "\"type\": \"" + ((fileType.indexOf(".") == -1) ? "directory" : ((fileType.indexOf(".") > 0) ? fileType.substring(fileType.indexOf(".")) : "hidden")) + "\" },";
    file.close();
  }

  root.close();
  Serial.println(json);

  // Remove the trailing comma and close the JSON array
  json.remove(json.length() - 1);  // Remove the trailing comma
  json += "]";
  Serial.println(json);
  return json;
}

void saveJSONToFile(const String &json, const char *filename) {
  File file = SD.open(filename, FILE_WRITE);
  Serial.println(file);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.print(json);
  file.close();
  Serial.println("JSON data saved to file: " + String(filename));
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

String macAddressToString(const uint8_t* mac) {
  char buf[18];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

String getEncryptionType(int encryptionType) {
  switch (encryptionType) {
    case 0:
      return "OPEN";
    case 1:
      return "WEP";
    case 2:
      return "WPA";
    case 3:
      return "WPA2";
    case 4:
      return "WPA_WPA2";
    case 5:
      return "MAX";
    default:
      return "UNKNOWN";
  }
}
