// ###########################################################################################################################################
// #
// # WordClock code for the 2 printables WordClock 16x16 matrix projects:
// # https://www.printables.com/de/model/350568-wordclock-16x16-led-matrix-2023-v1
// # https://www.printables.com/de/model/361861-wordclock-16x16-led-matrix-2023-v2
// # https://www.printables.com/de/model/450556-wordclock-16x16-led-matrix-2023-v3
// #
// # Code by https://github.com/AWSW-de
// #
// # Released under license: GNU General Public License v3.0: https://github.com/AWSW-de/WordClock-16x16-LED-matrix-2023/blob/main/LICENSE
// #
// ###########################################################################################################################################
/*

      ___           ___           ___           ___           ___           ___       ___           ___           ___     
     /\__\         /\  \         /\  \         /\  \         /\  \         /\__\     /\  \         /\  \         /\__\    
    /:/ _/_       /::\  \       /::\  \       /::\  \       /::\  \       /:/  /    /::\  \       /::\  \       /:/  /    
   /:/ /\__\     /:/\:\  \     /:/\:\  \     /:/\:\  \     /:/\:\  \     /:/  /    /:/\:\  \     /:/\:\  \     /:/__/     
  /:/ /:/ _/_   /:/  \:\  \   /::\~\:\  \   /:/  \:\__\   /:/  \:\  \   /:/  /    /:/  \:\  \   /:/  \:\  \   /::\__\____ 
 /:/_/:/ /\__\ /:/__/ \:\__\ /:/\:\ \:\__\ /:/__/ \:|__| /:/__/ \:\__\ /:/__/    /:/__/ \:\__\ /:/__/ \:\__\ /:/\:::::\__\
 \:\/:/ /:/  / \:\  \ /:/  / \/_|::\/:/  / \:\  \ /:/  / \:\  \  \/__/ \:\  \    \:\  \ /:/  / \:\  \  \/__/ \/_|:|~~|~   
  \::/_/:/  /   \:\  /:/  /     |:|::/  /   \:\  /:/  /   \:\  \        \:\  \    \:\  /:/  /   \:\  \          |:|  |    
   \:\/:/  /     \:\/:/  /      |:|\/__/     \:\/:/  /     \:\  \        \:\  \    \:\/:/  /     \:\  \         |:|  |    
    \::/  /       \::/  /       |:|  |        \::/__/       \:\__\        \:\__\    \::/  /       \:\__\        |:|  |    
     \/__/         \/__/         \|__|         ~~            \/__/         \/__/     \/__/         \/__/         \|__|    

*/


// ###########################################################################################################################################
// # Includes:
// #
// # You will need to add the following libraries to your Arduino IDE to use the project:
// # - Adafruit NeoPixel      // by Adafruit:                     https://github.com/adafruit/Adafruit_NeoPixel
// # - WiFiManager            // by tablatronix / tzapu:          https://github.com/tzapu/WiFiManager
// # - AsyncTCP               // by me-no-dev:                    https://github.com/me-no-dev/AsyncTCP
// # - ESPAsyncWebServer      // by me-no-dev:                    https://github.com/me-no-dev/ESPAsyncWebServer
// # - ESPUI                  // by s00500:                       https://github.com/s00500/ESPUI
// # - ArduinoJson            // by bblanchon:                    https://github.com/bblanchon/ArduinoJson
// # - LITTLEFS               // by lorol:                        https://github.com/lorol/LITTLEFS
// #
// ###########################################################################################################################################
#include <WiFi.h>               // Used to connect the ESP32 to your WiFi
#include <WiFiManager.h>        // Used for the WiFi Manager option to be able to connect the WordClock to your WiFi without code changes
#include <Adafruit_NeoPixel.h>  // Used to drive the NeoPixel LEDs
#include "time.h"               // Used for NTP time requests
#include <AsyncTCP.h>           // Used for the internal web server
#include <ESPAsyncWebServer.h>  // Used for the internal web server
#include <DNSServer.h>          // Used for the internal web server
#include <ESPUI.h>              // Used for the internal web server
#include <Preferences.h>        // Used to save the configuration to the ESP32 flash
#include <WiFiClient.h>         // Used for update function
#include <WebServer.h>          // Used for update function
#include <Update.h>             // Used for update function
#include "settings.h"           // Settings are stored in a seperate file to make to code better readable and to be able to switch to other settings faster


// ###########################################################################################################################################
// # Version number of the code:
// ###########################################################################################################################################
const char* WORD_CLOCK_VERSION = "V2.2.0";


// ###########################################################################################################################################
// # Internal web server settings:
// ###########################################################################################################################################
AsyncWebServer server(80);       // Web server for config
WebServer updserver(2022);       // Web server for OTA updates
AsyncWebServer ledserver(2023);  // Web server for HTML commands
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 44, 1);
DNSServer dnsServer;


// ###########################################################################################################################################
// # Declartions and variables used in the functions:
// ###########################################################################################################################################
Preferences preferences;
int langLEDlayout;
int iHour = 0;
int iMinute = 0;
int iSecond = 0;
bool updatedevice = true;
bool updatenow = false;
bool updatemode = false;
bool changedvalues = false;
int WiFiManFix = 0;
String iStartTime = "Failed to obtain time on startup... Please restart...";
int redVal_back, greenVal_back, blueVal_back;
int redVal_time, greenVal_time, blueVal_time;
int intensity, intensity_day, intensity_night, intensity_web;
int set_web_intensity = 0;
int usenightmode, day_time_start, day_time_stop, statusNightMode;
int useshowip, usesinglemin;
int statusLabelID, statusNightModeID, statusLanguageID, intensity_web_HintID, DayNightSectionID, LEDsettingsSectionID;
int sliderBrightnessDayID, switchNightModeID, sliderBrightnessNightID, call_day_time_startID, call_day_time_stopID;
char* selectLang;
int RandomColor;
uint16_t text_colour_background;
uint16_t text_colour_time;
int switchRandomColorID, switchSingleMinutesID;


// ###########################################################################################################################################
// # Setup function that runs once at startup of the ESP:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);
  delay(500);
  preferences.begin("wordclock", false);  // Init ESP32 flash
  Serial.println("######################################################################");
  Serial.print("# WordClock startup of version: ");
  Serial.println(WORD_CLOCK_VERSION);
  Serial.println("######################################################################");
  getFlashValues();                // Read settings from flash
  strip.begin();                   // Init the LEDs
  strip.show();                    // Init the LEDs --> Set them to OFF
  intensity = intensity_day;       // Set the intenity to day mode for startup
  strip.setBrightness(intensity);  // Set LED brightness
  if (testTime == 0) {             // If time text test mode is not used:
    WIFI_login();                  // WiFiManager
    WiFiManager1stBootFix();       // WiFi Manager 1st connect fix
    ShowIPaddress();               // Display the current IP-address
    configNTPTime();               // NTP time setup
    setupWebInterface();           // Generate the configuration page
    updatenow = true;              // Update the display 1x after startup
    update_display();              // Update LED display
    handleOTAupdate();             // Start the ESP32 OTA update server
    handleLEDupdate();             // LED update via web
    Serial.println("######################################################################");
    Serial.println("# Web interface online at: http://" + IpAddress2String(WiFi.localIP()));
    Serial.println("# HTTP controls online at: http://" + IpAddress2String(WiFi.localIP()) + ":2023");
  }
  Serial.println("######################################################################");
  Serial.println("# WordClock startup finished...");
  Serial.println("######################################################################");
}


// ###########################################################################################################################################
// # Loop function which runs all the time after the startup was done:
// ###########################################################################################################################################
void loop() {
  printLocalTime();                               // Locally get the time (NTP server requests done 1x per hour)
  if (updatedevice == true) {                     // Allow display updates (normal usage)
    if (changedvalues == true) setFlashValues();  // Write settings to flash
    update_display();                             // Update display (1x per minute regulary)
  }
  dnsServer.processNextRequest();                    // Update web server
  if (updatemode == true) updserver.handleClient();  // ESP32 OTA updates
}


// ###########################################################################################################################################
// # Setup the internal web server configuration page:
// ###########################################################################################################################################
void setupWebInterface() {
  dnsServer.start(DNS_PORT, "*", apIP);


  // Section General:
  // ################
  ESPUI.separator("General:");

  // Status label:
  statusLabelID = ESPUI.label("Status:", ControlColor::Dark, "Operational");

  // WordClock version:
  ESPUI.label("Version", ControlColor::None, WORD_CLOCK_VERSION);



  // Section LED settings:
  // #####################
  LEDsettingsSectionID = ESPUI.separator("LED settings:");

  // Time color selector:
  char hex_time[7] = { 0 };
  sprintf(hex_time, "#%02X%02X%02X", redVal_time, greenVal_time, blueVal_time);
  text_colour_time = ESPUI.text("Time", colCallTIME, ControlColor::Dark, hex_time);
  ESPUI.setInputType(text_colour_time, "color");

  // Background color selector:
  char hex_back[7] = { 0 };
  sprintf(hex_back, "#%02X%02X%02X", redVal_back, greenVal_back, blueVal_back);
  text_colour_background = ESPUI.text("Background", colCallBACK, ControlColor::Dark, hex_back);
  ESPUI.setInputType(text_colour_background, "color");

  // Use random color mode:
  switchRandomColorID = ESPUI.switcher("Use random text color every new minute", &switchRandomColor, ControlColor::Dark, RandomColor);
  if (RandomColor == 1) {
    ESPUI.updateVisibility(text_colour_time, false);
    ESPUI.updateVisibility(text_colour_background, false);
  }

  // Show single minutes to display the minute exact time:
  switchSingleMinutesID = ESPUI.switcher("Show single minutes to display the minute exact time", &switchSingleMinutes, ControlColor::Dark, usesinglemin);

  // Show note when intensity is currently controlled via web-url usage and these internal settings get disabled:
  intensity_web_HintID = ESPUI.label("Manual settings disabled due to web URL usage:", ControlColor::Alizarin, "Restart WordClock or deactivate web control usage via http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=1");
  ESPUI.updateVisibility(intensity_web_HintID, false);



  // Section LED night mode settings:
  // ################################
  DayNightSectionID = ESPUI.separator("Day/Night LED brightness mode settings:");

  // Use night mode function:
  switchNightModeID = ESPUI.switcher("Use night mode to reduce brightness", &switchNightMode, ControlColor::Dark, usenightmode);

  // Intensity DAY slider selector: !!! DEFAULT LIMITED TO 64 of 255 !!!
  sliderBrightnessDayID = ESPUI.slider("Brightness during the day", &sliderBrightnessDay, ControlColor::Dark, intensity_day, 0, LEDintensityLIMIT);

  // Intensity NIGHT slider selector: !!! DEFAULT LIMITED TO 64 of 255 !!!
  sliderBrightnessNightID = ESPUI.slider("Brightness at night", &sliderBrightnessNight, ControlColor::Dark, intensity_night, 0, LEDintensityLIMIT);

  // Night mode status:
  statusNightModeID = ESPUI.label("Night mode status", ControlColor::Dark, "Night mode not used");

  // Day mode start time:
  call_day_time_startID = ESPUI.number("Day time starts at", call_day_time_start, ControlColor::Dark, day_time_start, 0, 11);

  // Day mode stop time:
  call_day_time_stopID = ESPUI.number("Day time ends after", call_day_time_stop, ControlColor::Dark, day_time_stop, 12, 23);



  // Section Startup:
  // ################
  ESPUI.separator("Startup:");

  // Startup LED test function:
  // ESPUI.switcher("Show LED test on startup", &switchLEDTest, ControlColor::Dark, useledtest);

  // Show IP-address on startup:
  ESPUI.switcher("Show IP-address on startup", &switchShowIP, ControlColor::Dark, useshowip);



  // Section WiFi:
  // #############
  ESPUI.separator("WiFi:");

  // WiFi SSID:
  ESPUI.label("SSID", ControlColor::Dark, WiFi.SSID());

  // WiFi signal strength:
  ESPUI.label("Signal", ControlColor::Dark, String(WiFi.RSSI()) + "dBm");

  // Hostname:
  ESPUI.label("Hostname", ControlColor::Dark, hostname);

  // WiFi ip-address:
  ESPUI.label("IP-address", ControlColor::Dark, IpAddress2String(WiFi.localIP()));

  // WiFi MAC-address:
  ESPUI.label("MAC address", ControlColor::Dark, WiFi.macAddress());



  // Section smart home control via web URLs:
  // ########################################
  ESPUI.separator("Smart home control via web URLs:");

  // About note:
  ESPUI.label("About note", ControlColor::Dark, "Control WordClock from your smart home environment via web URLs.");

  // Functions note:
  ESPUI.label("Functions", ControlColor::Dark, "You can turn the LEDs off or on via http commands to reduce energy consumption.");

  // Usage note:
  ESPUI.label("Usage hints and examples", ControlColor::Dark, "http://" + IpAddress2String(WiFi.localIP()) + ":2023");



  // Section Time settings:
  // ######################
  ESPUI.separator("Time settings:");

  // NTP server:
  ESPUI.label("NTP server", ControlColor::Dark, NTPserver);

  // Time zone:
  ESPUI.label("Time zone", ControlColor::Dark, Timezone);

  // Time:
  ESPUI.label("Startup time", ControlColor::Dark, iStartTime);



  // Section Update:
  // ###############
  ESPUI.separator("Update:");

  // Update WordClock:
  ESPUI.button("Activate update mode", &buttonUpdate, ControlColor::Dark, "Activate update mode", (void*)1);

  // Update URL
  ESPUI.label("Update URL", ControlColor::Dark, "http://" + IpAddress2String(WiFi.localIP()) + ":2022/ota");

  // Update User account
  ESPUI.label("Update account", ControlColor::Dark, "Username: WordClock   /   Password: 16x16");



  // Section Language:
  // #################
  ESPUI.separator("Language:");

  // Set layout language:
  if (langLEDlayout == 0) selectLang = "German";
  if (langLEDlayout == 1) selectLang = "English";
  if (langLEDlayout == 2) selectLang = "Dutch";
  if (langLEDlayout == 3) selectLang = "Swedish";
  if (langLEDlayout == 4) selectLang = "Italian";
  if (langLEDlayout == 5) selectLang = "French";
  if (langLEDlayout == 6) selectLang = "Swiss German";
  if (langLEDlayout == 7) selectLang = "Chinese";
  if (langLEDlayout == 8) selectLang = "Swabian German";

  // Language overview:
  ESPUI.addControl(ControlType::Label, "Available languages", "<center><table border='3' class='center' width='100%'><tr><th>Value:</th><th>Language:</th><th>Value:</th><th>Language:</th></tr><tr align='center'><td>0</td><td>German</td><td>1</td><td>English</td></tr><tr align='center'><td>2</td><td>Dutch</td><td>3</td><td>Swedish</td></tr><tr align='center'><td>4</td><td>Italian</td><td>5</td><td>French</td></tr><tr align='center'><td>6</td><td>Swiss German</td><td>7</td><td>Chinese</td></tr><tr align='center'><td>8</td><td>Swabian German</td><td></td><td></td></tr></table>", ControlColor::Dark, Control::noParent, 0);

  // Change language:
  ESPUI.number("Select your language", call_langauge_select, ControlColor::Dark, langLEDlayout, 0, 8);

  // Current language:
  statusLanguageID = ESPUI.label("Current layout language", ControlColor::Dark, selectLang);



  // Section Maintenance:
  // ####################
  ESPUI.separator("Maintenance:");

  // Restart WordClock:
  ESPUI.button("Restart", &buttonRestart, ControlColor::Dark, "Restart", (void*)1);

  // Reset WiFi settings:
  ESPUI.button("Reset WiFi settings", &buttonWiFiReset, ControlColor::Dark, "Reset WiFi settings", (void*)2);

  // Reset WordClock settings:
  ESPUI.button("Reset WordClock settings", &buttonWordClockReset, ControlColor::Dark, "Reset WordClock settings", (void*)3);



  // Update night mode status text on startup:
  if (usenightmode == 1) {
    if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
      ESPUI.print(statusNightModeID, "Day time");
      if ((iHour == 0) && (day_time_stop == 23)) ESPUI.print(statusNightModeID, "Night time");  // Special function if day_time_stop set to 23 and time is 24, so 0...
    } else {
      ESPUI.print(statusNightModeID, "Night time");
    }
  }


  // Deploy the page:
  ESPUI.begin("WordClock");
}


// ###########################################################################################################################################
// # Read settings from flash:
// ###########################################################################################################################################
void getFlashValues() {
  if (debugtexts == 1) Serial.println("Read settings from flash: START");
  langLEDlayout = preferences.getUInt("langLEDlayout", langLEDlayout_default);
  redVal_time = preferences.getUInt("redVal_time", redVal_time_default);
  greenVal_time = preferences.getUInt("greenVal_time", greenVal_time_default);
  blueVal_time = preferences.getUInt("blueVal_time", blueVal_time_default);
  redVal_back = preferences.getUInt("redVal_back", redVal_back_default);
  greenVal_back = preferences.getUInt("greenVal_back", greenVal_back_default);
  blueVal_back = preferences.getUInt("blueVal_back", blueVal_back_default);
  intensity_day = preferences.getUInt("intensity_day", intensity_day_default);
  intensity_night = preferences.getUInt("intensity_night", intensity_night_default);
  usenightmode = preferences.getUInt("usenightmode", usenightmode_default);
  day_time_start = preferences.getUInt("day_time_start", day_time_start_default);
  day_time_stop = preferences.getUInt("day_time_stop", day_time_stop_default);
  useshowip = preferences.getUInt("useshowip", useshowip_default);
  usesinglemin = preferences.getUInt("usesinglemin", usesinglemin_default);
  RandomColor = preferences.getUInt("RandomColor", RandomColor_default);
  if (debugtexts == 1) Serial.println("Read settings from flash: END");
}


// ###########################################################################################################################################
// # Write settings to flash:
// ###########################################################################################################################################
void setFlashValues() {
  if (debugtexts == 1) Serial.println("Write settings to flash: START");
  changedvalues = false;
  preferences.putUInt("langLEDlayout", langLEDlayout);
  preferences.putUInt("redVal_time", redVal_time);
  preferences.putUInt("greenVal_time", greenVal_time);
  preferences.putUInt("blueVal_time", blueVal_time);
  preferences.putUInt("redVal_back", redVal_back);
  preferences.putUInt("greenVal_back", greenVal_back);
  preferences.putUInt("blueVal_back", blueVal_back);
  preferences.putUInt("intensity_day", intensity_day);
  preferences.putUInt("intensity_night", intensity_night);
  preferences.putUInt("usenightmode", usenightmode);
  preferences.putUInt("day_time_start", day_time_start);
  preferences.putUInt("day_time_stop", day_time_stop);
  preferences.putUInt("useshowip", useshowip);
  preferences.putUInt("usesinglemin", usesinglemin);
  preferences.putUInt("RandomColor", RandomColor);
  if (debugtexts == 1) Serial.println("Write settings to flash: END");
  if (usenightmode == 1) {
    if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
      ESPUI.print(statusNightModeID, "Day time");
      if ((iHour == 0) && (day_time_stop == 23)) ESPUI.print(statusNightModeID, "Night time");  // Special function if day_time_stop set to 23 and time is 24, so 0...
    } else {
      ESPUI.print(statusNightModeID, "Night time");
    }
  } else {
    ESPUI.print(statusNightModeID, "Night mode not used");
  }
  updatenow = true;  // Update display now...
}


// ###########################################################################################################################################
// # GUI: Reset the WordClock settings:
// ###########################################################################################################################################
int WordClockResetCounter = 0;
void buttonWordClockReset(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(250);
  if (WordClockResetCounter == 0) ResetTextLEDs(strip.Color(255, 0, 0));
  if (WordClockResetCounter == 1) ResetTextLEDs(strip.Color(0, 255, 0));
  switch (type) {
    case B_DOWN:
      break;
    case B_UP:
      if (WordClockResetCounter == 1) {
        Serial.println("Status: WORDCLOCK SETTINGS RESET REQUEST EXECUTED");
        preferences.clear();
        delay(250);
        preferences.putUInt("langLEDlayout", langLEDlayout_default);
        preferences.putUInt("redVal_time", redVal_time_default);
        preferences.putUInt("greenVal_time", greenVal_time_default);
        preferences.putUInt("blueVal_time", blueVal_time_default);
        preferences.putUInt("redVal_back", redVal_back_default);
        preferences.putUInt("greenVal_back", greenVal_back_default);
        preferences.putUInt("blueVal_back", blueVal_back_default);
        preferences.putUInt("intensity_day", intensity_day_default);
        preferences.putUInt("intensity_night", intensity_night_default);
        preferences.putUInt("useshowip", useshowip_default);
        preferences.putUInt("usenightmode", usenightmode_default);
        preferences.putUInt("day_time_stop", day_time_stop_default);
        preferences.putUInt("day_time_stop", day_time_stop_default);
        preferences.putUInt("usesinglemin", usesinglemin_default);
        preferences.putUInt("RandomColor", RandomColor_default);
        delay(250);
        preferences.end();
        Serial.println("####################################################################################################");
        Serial.println("# WORDCLOCK SETTING WERE SET TO DEFAULT... WORDCLOCK WILL NOW RESTART... PLEASE CONFIGURE AGAIN... #");
        Serial.println("####################################################################################################");
        for (int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        delay(250);
        ESP.restart();
      } else {
        Serial.println("Status: WORDCLOCK SETTINGS RESET REQUEST");
        ESPUI.print(statusLabelID, "WORDCLOCK SETTINGS RESET REQUESTED");
        ESPUI.updateButton(sender->id, "! Press button once more to apply settings reset !");
        WordClockResetCounter = WordClockResetCounter + 1;
      }
      break;
  }
}


// ###########################################################################################################################################
// # GUI: Language selection
// ###########################################################################################################################################
void call_langauge_select(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  langLEDlayout = sender->value.toInt();
  // Set layout language text in gui:
  if (langLEDlayout == 0) selectLang = "German";
  if (langLEDlayout == 1) selectLang = "English";
  if (langLEDlayout == 2) selectLang = "Dutch";
  if (langLEDlayout == 3) selectLang = "Swedish";
  if (langLEDlayout == 4) selectLang = "Italian";
  if (langLEDlayout == 5) selectLang = "French";
  if (langLEDlayout == 6) selectLang = "Swiss German";
  if (langLEDlayout == 7) selectLang = "Chinese";
  if (langLEDlayout == 8) selectLang = "Swabian German";
  ESPUI.print(statusLanguageID, selectLang);
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # Show the IP-address on the display:
// ###########################################################################################################################################
void ShowIPaddress() {
  if (useshowip == 1) {
    Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.localIP()));
    int ipdelay = 2000;

    // Testing the digits:
    // for (int i = 0; i < 10; i++) {
    //   back_color();
    //   numbers(i, 3);
    //   numbers(i, 2);
    //   numbers(i, 1);
    //   strip.show();
    //   delay(ipdelay);
    // }

    // Octet 1:
    back_color();
    numbers(getDigit(int(WiFi.localIP()[0]), 2), 3);
    numbers(getDigit(int(WiFi.localIP()[0]), 1), 2);
    numbers(getDigit(int(WiFi.localIP()[0]), 0), 1);
    setLED(160, 160, 1);
    setLED(191, 191, 1);  // 2nd row
    setLED(236, 239, 1);
    setLED(240, 243, 1);  // 2nd row
    strip.show();
    delay(ipdelay);

    // // Octet 2:
    back_color();
    numbers(getDigit(int(WiFi.localIP()[1]), 2), 3);
    numbers(getDigit(int(WiFi.localIP()[1]), 1), 2);
    numbers(getDigit(int(WiFi.localIP()[1]), 0), 1);
    setLED(160, 160, 1);
    setLED(191, 191, 1);  // 2nd row
    setLED(232, 239, 1);
    setLED(240, 247, 1);  // 2nd row
    strip.show();
    delay(ipdelay);

    // // Octet 3:
    back_color();
    numbers(getDigit(int(WiFi.localIP()[2]), 2), 3);
    numbers(getDigit(int(WiFi.localIP()[2]), 1), 2);
    numbers(getDigit(int(WiFi.localIP()[2]), 0), 1);
    setLED(160, 160, 1);
    setLED(191, 191, 1);  // 2nd row
    setLED(228, 239, 1);
    setLED(240, 251, 1);  // 2nd row
    strip.show();
    delay(ipdelay);

    // // Octet 4:
    back_color();
    numbers(getDigit(int(WiFi.localIP()[3]), 2), 3);
    numbers(getDigit(int(WiFi.localIP()[3]), 1), 2);
    numbers(getDigit(int(WiFi.localIP()[3]), 0), 1);
    setLED(224, 239, 1);
    setLED(240, 255, 1);  // 2nd row
    strip.show();
    delay(ipdelay);
  }
}


// ###########################################################################################################################################
// # Set the numbers on the display in each single row:
// ###########################################################################################################################################
void numbers(int wert, int segment) {

  // Serial.println(wert);

  switch (segment) {
    case 3:
      {
        switch (wert) {
          case 0:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 108, 1);
              setLED(115, 115, 1);  // 2nd row
              setLED(111, 111, 1);
              setLED(112, 112, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(143, 143, 1);
              setLED(144, 144, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 1:
            {
              setLED(44, 44, 1);
              setLED(51, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(108, 108, 1);
              setLED(115, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 172, 1);
              setLED(179, 179, 1);  // 2nd row
              break;
            }
          case 2:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(143, 143, 1);
              setLED(144, 144, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 3:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 4:
            {
              setLED(44, 44, 1);
              setLED(51, 51, 1);  // 2nd row
              setLED(47, 47, 1);
              setLED(48, 48, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 172, 1);
              setLED(179, 179, 1);  // 2nd row
              break;
            }
          case 5:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 6:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(143, 143, 1);
              setLED(144, 144, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 7:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(108, 108, 1);
              setLED(115, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 172, 1);
              setLED(179, 179, 1);  // 2nd row
              break;
            }
          case 8:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(143, 143, 1);
              setLED(144, 144, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
          case 9:
            {
              setLED(44, 47, 1);
              setLED(48, 51, 1);  // 2nd row
              setLED(76, 76, 1);
              setLED(83, 83, 1);  // 2nd row
              setLED(79, 79, 1);
              setLED(80, 80, 1);  // 2nd row
              setLED(108, 111, 1);
              setLED(112, 115, 1);  // 2nd row
              setLED(140, 140, 1);
              setLED(147, 147, 1);  // 2nd row
              setLED(172, 175, 1);
              setLED(176, 179, 1);  // 2nd row
              break;
            }
        }
        break;
      }

    case 2:
      {
        switch (wert) {
          case 0:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 103, 1);
              setLED(120, 120, 1);  // 2nd row
              setLED(106, 106, 1);
              setLED(117, 117, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(138, 138, 1);
              setLED(149, 149, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 1:
            {
              setLED(39, 39, 1);
              setLED(56, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(103, 103, 1);
              setLED(120, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 167, 1);
              setLED(184, 184, 1);  // 2nd row
              break;
            }
          case 2:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(138, 138, 1);
              setLED(149, 149, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 3:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 4:
            {
              setLED(39, 39, 1);
              setLED(56, 56, 1);  // 2nd row
              setLED(42, 42, 1);
              setLED(53, 53, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 167, 1);
              setLED(184, 184, 1);  // 2nd row
              break;
            }
          case 5:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 6:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(138, 138, 1);
              setLED(149, 149, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 7:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(103, 103, 1);
              setLED(120, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 167, 1);
              setLED(184, 184, 1);  // 2nd row
              break;
            }
          case 8:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(138, 138, 1);
              setLED(149, 149, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
          case 9:
            {
              setLED(39, 42, 1);
              setLED(53, 56, 1);  // 2nd row
              setLED(71, 71, 1);
              setLED(88, 88, 1);  // 2nd row
              setLED(74, 74, 1);
              setLED(85, 85, 1);  // 2nd row
              setLED(103, 106, 1);
              setLED(117, 120, 1);  // 2nd row
              setLED(135, 135, 1);
              setLED(152, 152, 1);  // 2nd row
              setLED(167, 170, 1);
              setLED(181, 184, 1);  // 2nd row
              break;
            }
        }
        break;
      }

    case 1:
      {
        switch (wert) {
          case 0:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 98, 1);
              setLED(125, 125, 1);  // 2nd row
              setLED(101, 101, 1);
              setLED(122, 122, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(133, 133, 1);
              setLED(154, 154, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 1:
            {
              setLED(34, 34, 1);
              setLED(61, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(98, 98, 1);
              setLED(125, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 162, 1);
              setLED(189, 189, 1);  // 2nd row
              break;
            }
          case 2:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(133, 133, 1);
              setLED(154, 154, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 3:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 4:
            {
              setLED(34, 34, 1);
              setLED(61, 61, 1);  // 2nd row
              setLED(37, 37, 1);
              setLED(58, 58, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 162, 1);
              setLED(189, 189, 1);  // 2nd row
              break;
            }
          case 5:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 6:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(133, 133, 1);
              setLED(154, 154, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 7:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(98, 98, 1);
              setLED(125, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 162, 1);
              setLED(189, 189, 1);  // 2nd row
              break;
            }
          case 8:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(133, 133, 1);
              setLED(154, 154, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
          case 9:
            {
              setLED(34, 37, 1);
              setLED(58, 61, 1);  // 2nd row
              setLED(66, 66, 1);
              setLED(93, 93, 1);  // 2nd row
              setLED(69, 69, 1);
              setLED(90, 90, 1);  // 2nd row
              setLED(98, 101, 1);
              setLED(122, 125, 1);  // 2nd row
              setLED(130, 130, 1);
              setLED(157, 157, 1);  // 2nd row
              setLED(162, 165, 1);
              setLED(186, 189, 1);  // 2nd row
              break;
            }
        }
        break;
      }
  }
}


// ###########################################################################################################################################
// # Get a digit from a number at position pos: (Split IP-address octets in single digits)
// ###########################################################################################################################################
int getDigit(int number, int pos) {
  return (pos == 0) ? number % 10 : getDigit(number / 10, --pos);
}


// ###########################################################################################################################################
// # GUI: Restart the WordClock:
// ###########################################################################################################################################
void buttonRestart(Control* sender, int type, void* param) {
  updatedevice = false;
  delay(250);
  ResetTextLEDs(strip.Color(255, 0, 0));
  if (changedvalues == true) setFlashValues();  // Write settings to flash
  delay(250);
  preferences.end();
  delay(250);
  ResetTextLEDs(strip.Color(0, 255, 0));
  delay(750);
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
  delay(250);
  ESP.restart();
}


// ###########################################################################################################################################
// # GUI: Reset the WiFi settings of the WordClock:
// ###########################################################################################################################################
int WIFIResetCounter = 0;
void buttonWiFiReset(Control* sender, int type, void* param) {
  updatedevice = false;
  if (WIFIResetCounter == 0) ResetTextLEDs(strip.Color(255, 0, 0));
  if (WIFIResetCounter == 1) ResetTextLEDs(strip.Color(0, 255, 0));
  switch (type) {
    case B_DOWN:
      break;
    case B_UP:
      if (WIFIResetCounter == 1) {
        Serial.println("Status: WIFI SETTINGS RESET REQUEST EXECUTED");
        WiFi.disconnect();
        delay(1000);
        WiFiManager manager;
        manager.resetSettings();
        Serial.println("####################################################################################################");
        Serial.println("# WIFI SETTING WERE SET TO DEFAULT... WORDCLOCK WILL NOW RESTART... PLEASE CONFIGURE WIFI AGAIN... #");
        Serial.println("####################################################################################################");
        for (int i = 0; i < NUMPIXELS; i++) {
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        delay(250);
        ESP.restart();
      } else {
        preferences.putUInt("WiFiManFix", 0);                 // WiFi Manager Fix Reset
        preferences.putUInt("useshowip", useshowip_default);  // Show IP-address again
        delay(100);
        preferences.end();
        Serial.println("Status: WIFI SETTINGS RESET REQUEST");
        ESPUI.print(statusLabelID, "WORDCLOCK WIFI SETTINGS RESET REQUEST");
        ESPUI.updateButton(sender->id, "! Press button once more to apply WiFi reset !");
        WIFIResetCounter = WIFIResetCounter + 1;
      }
      break;
  }
}


// ###########################################################################################################################################
// # GUI: Update the WordClock:
// ###########################################################################################################################################
void buttonUpdate(Control* sender, int type, void* param) {
  preferences.end();
  updatedevice = false;
  delay(1000);
  updatemode = true;
  delay(1000);
  back_color();
  strip.show();
  Serial.println(String("param: ") + String(int(param)));
  switch (type) {
    case B_DOWN:
      Serial.println("Status: Update request");
      ESPUI.print(statusLabelID, "Update requested");
      break;
    case B_UP:
      Serial.println("Status: Update executed");
      ESPUI.updateButton(sender->id, "Update mode active now - Use the update url: >>>");
      break;
  }
}


// ###########################################################################################################################################
// # Show a LED output for RESET in the different languages:
// ###########################################################################################################################################
void ResetTextLEDs(uint32_t color) {
  updatedevice = false;
  delay(1000);
  back_color();

  if (langLEDlayout == 0) {      // DE:
    setLEDcol(137, 138, color);  // RE
    setLEDcol(149, 150, color);  // 2nd row
    setLEDcol(167, 168, color);  // SE
    setLEDcol(183, 184, color);  // 2nd row
    setLEDcol(227, 227, color);  // T
    setLEDcol(252, 252, color);  // 2nd row
  }

  if (langLEDlayout == 1) {      // EN:
    setLEDcol(100, 101, color);  // RE
    setLEDcol(122, 123, color);  // 2nd row
    setLEDcol(174, 175, color);  // SE
    setLEDcol(176, 177, color);  // 2nd row
    setLEDcol(227, 227, color);  // T
    setLEDcol(252, 252, color);  // 2nd row
  }

  if (langLEDlayout == 2) {      // NL:
    setLEDcol(33, 33, color);    // R
    setLEDcol(62, 62, color);    // 2nd row
    setLEDcol(96, 97, color);    // ES
    setLEDcol(126, 127, color);  // 2nd row
    setLEDcol(164, 164, color);  // E
    setLEDcol(187, 187, color);  // 2nd row
    setLEDcol(227, 227, color);  // T
    setLEDcol(252, 252, color);  // 2nd row
  }

  if (langLEDlayout == 3) {    // SWE:
    setLEDcol(67, 71, color);  // R
    setLEDcol(88, 92, color);  // 2nd row
  }

  if (langLEDlayout == 4) {    // IT:
    setLEDcol(11, 11, color);  // R
    setLEDcol(20, 20, color);  // 2nd row
    setLEDcol(9, 9, color);    // E
    setLEDcol(22, 22, color);  // 2nd row
    setLEDcol(45, 47, color);  // SET
    setLEDcol(48, 50, color);  // 2nd row
  }

  if (langLEDlayout == 5) {    // FR:
    setLEDcol(11, 13, color);  // RES
    setLEDcol(18, 20, color);  // 2nd row
    setLEDcol(5, 5, color);    // E
    setLEDcol(26, 26, color);  // 2nd row
    setLEDcol(36, 36, color);  // T
    setLEDcol(59, 59, color);  // 2nd row
  }

  if (langLEDlayout == 6) {    // GSW:
    setLEDcol(11, 15, color);  // RESET
    setLEDcol(16, 20, color);  // 2nd row
  }

  if (langLEDlayout == 7) {    // CN:
    setLEDcol(38, 39, color);  // RESET 重置
    setLEDcol(56, 57, color);  // 2nd row
  }

  if (langLEDlayout == 8) {      // SWABIAN GERMAN:
    setLEDcol(40, 41, color);    // RE
    setLEDcol(54, 55, color);    // 2nd row
    setLEDcol(133, 134, color);  // SE
    setLEDcol(153, 154, color);  // 2nd row
    setLEDcol(204, 204, color);  // T
    setLEDcol(211, 211, color);  // 2nd row
  }

  strip.show();
}


// ###########################################################################################################################################
// # Actual function, which controls 1/0 of the LED with color value:
// ###########################################################################################################################################
void setLEDcol(int ledNrFrom, int ledNrTo, uint32_t color) {
  if (ledNrFrom > ledNrTo) {
    setLED(ledNrTo, ledNrFrom, 1);  // Sets LED numbers in correct order
  } else {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS))
        strip.setPixelColor(i, color);
    }
  }
}


// ###########################################################################################################################################
// # GUI: Night mode switch:
// ###########################################################################################################################################
void switchNightMode(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      usenightmode = 1;
      if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
        intensity = intensity_day;
        if ((iHour == 0) && (day_time_stop == 23)) intensity = intensity_night;  // Special function if day_time_stop set to 23 and time is 24, so 0...
      } else {
        intensity = intensity_night;
      }
      break;
    case S_INACTIVE:
      intensity = intensity_day;
      usenightmode = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Single minutes switch:
// ###########################################################################################################################################
void switchSingleMinutes(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      usesinglemin = 1;
      break;
    case S_INACTIVE:
      usesinglemin = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Use random color mode:
// ###########################################################################################################################################
void switchRandomColor(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      RandomColor = 1;
      ESPUI.updateVisibility(text_colour_background, false);
      ESPUI.updateVisibility(text_colour_time, false);
      redVal_back = 0;
      greenVal_back = 0;
      blueVal_back = 0;
      break;
    case S_INACTIVE:
      RandomColor = 0;
      ESPUI.updateVisibility(text_colour_background, true);
      ESPUI.updateVisibility(text_colour_time, true);
      ESPUI.jsonReload();
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Show IP-ADdress switch:
// ###########################################################################################################################################
void switchShowIP(Control* sender, int value) {
  updatedevice = false;
  delay(1000);
  switch (value) {
    case S_ACTIVE:
      useshowip = 1;
      break;
    case S_INACTIVE:
      useshowip = 0;
      break;
  }
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # Update the display / time on it:
// ###########################################################################################################################################
void update_display() {
  // if (debugtexts == 1) Serial.println("Time: " + iStartTime);

  // Show the current time or use the time text test function:
  if (testTime == 0) {  // Show the current time:
    show_time(iHour, iMinute);
  } else {  // TEST THE DISPLAY TIME OUTPUT:
    strip.setBrightness(5);
    for (int i = 1; i <= 12; i++) {  // 12 hours only:
      show_time(i, 0);
      delay(1000);
    }
    for (int i = 0; i <= 55; i += 5) {  // 5 minutes steps only:
      show_time(9, i);
      delay(1000);
    }
    for (int i = 9; i < 10; i++) {  // Hours 0 to 3 with all minute texts:
      for (int y = 0; y < 60; y++) {
        show_time(i, y);
        delay(10);
      }
    }
  }
}


// ###########################################################################################################################################
// # Display hours and minutes text function:
// ###########################################################################################################################################
uint32_t colorRGB;
static int lastHourSet = -1;
static int lastMinutesSet = -1;
void show_time(int hours, int minutes) {

  if ((lastHourSet == hours && lastMinutesSet == minutes) && updatenow == false) {  // Reduce display updates to new minutes and new config updates
    return;
  }

  updatenow = false;
  lastHourSet = hours;
  lastMinutesSet = minutes;

  // Show current time of display update:
  if (debugtexts == 1) Serial.println("Update display now: " + String(hours) + ":" + String(minutes) + ":" + String(iSecond));

  // Night/Day mode intensity setting:
  if ((usenightmode == 1) && (set_web_intensity == 0)) {
    if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
      intensity = intensity_day;
      if ((iHour == 0) && (day_time_stop == 23)) intensity = intensity_night;  // Special function if day_time_stop set to 23 and time is 24, so 0...
    } else {
      intensity = intensity_night;
    }
    if (testDayNightmode == 1) {  // Test day/night times function:
      Serial.println("############################################################################################");
      Serial.println("Current time day/night test: " + String(hours) + ":" + String(minutes) + ":" + String(iSecond));
      Serial.println("Current settings: day_time_start: " + String(day_time_start) + " day_time_stop: " + String(day_time_stop));
      for (int i = 0; i < 24; i++) {
        String daynightvar = "-";
        if ((i >= day_time_start) && (i <= day_time_stop)) {
          daynightvar = "Day time";
          if ((i == 0) && (day_time_stop == 23)) daynightvar = "Night time";
        } else {
          daynightvar = "Night time";
        }
        Serial.println("Current hour: " + String(i) + " --> " + daynightvar);
      }
      testDayNightmode = 0;  // Show the list 1x only
      Serial.println("############################################################################################");
    }
  } else {  // Control intensity by WordClock settings or via HTML command:
    if (set_web_intensity == 0) intensity = intensity_day;
    if (set_web_intensity == 1) intensity = 0;
  }
  strip.setBrightness(intensity);

  // Set background color:
  back_color();

  // Static text color or random color mode:
  if (RandomColor == 0) colorRGB = strip.Color(redVal_time, greenVal_time, blueVal_time);
  if (RandomColor == 1) colorRGB = strip.Color(random(255), random(255), random(255));

  // Display time:
  iHour = hours;
  iMinute = minutes;

  // Test a special time:
  if (testspecialtime == 1) {
    Serial.println("Special time test active: " + String(test_hour) + ":" + String(test_minute));
    iHour = test_hour;
    iMinute = test_minute;
  }

  // divide minute by 5 to get value for display control
  int minDiv = iMinute / 5;
  if (usesinglemin == 1) showMinutes(iMinute);

  // ########################################################### DE:
  if (langLEDlayout == 0) {  // DE:

    // ES IST:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(16, 17, colorRGB);  // 2nd row
    setLEDcol(10, 12, colorRGB);
    setLEDcol(19, 21, colorRGB);  // 2nd row
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES IST ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(76, 79, colorRGB);
      setLEDcol(80, 83, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }
    // VIERTEL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(69, 75, colorRGB);
      setLEDcol(84, 90, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("VIERTEL ");
    }
    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(32, 35, colorRGB);
      setLEDcol(60, 63, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }
    // ZWANZIG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(41, 47, colorRGB);
      setLEDcol(48, 54, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("ZWANZIG ");
    }
    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(64, 67, colorRGB);
      setLEDcol(92, 95, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("NACH ");
    }
    // VOR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(109, 111, colorRGB);
      setLEDcol(112, 114, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("VOR ");
    }
    // HALB:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(104, 107, colorRGB);
      setLEDcol(116, 119, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("HALB ");
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(169, 171, colorRGB);  // EIN
            setLEDcol(180, 182, colorRGB);  // 2nd row
            if (testPrintTimeTexts == 1) Serial.print("EIN ");
          }
          if ((xHour == 1) && (iMinute > 4)) {
            setLEDcol(168, 171, colorRGB);  // EINS (S in EINS) (just used if not point 1 o'clock)
            setLEDcol(180, 183, colorRGB);  // 2nd row
            if (testPrintTimeTexts == 1) Serial.print("EINS ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(140, 143, colorRGB);  // ZWEI
          setLEDcol(144, 147, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ZWEI ");
          break;
        }
      case 3:
        {
          setLEDcol(136, 139, colorRGB);  // DREI
          setLEDcol(148, 151, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("DREI ");
          break;
        }
      case 4:
        {
          setLEDcol(128, 131, colorRGB);  // VIER
          setLEDcol(156, 159, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("VIER ");
          break;
        }
      case 5:
        {
          setLEDcol(160, 163, colorRGB);  // FUENF
          setLEDcol(188, 191, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
          break;
        }
      case 6:
        {
          setLEDcol(164, 168, colorRGB);  // SECHS
          setLEDcol(183, 187, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("SECHS ");
          break;
        }
      case 7:
        {
          setLEDcol(202, 207, colorRGB);  // SIEBEN
          setLEDcol(208, 213, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("SIEBEN ");
          break;
        }
      case 8:
        {
          setLEDcol(172, 175, colorRGB);  // ACHT
          setLEDcol(176, 179, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ACHT ");
          break;
        }
      case 9:
        {
          setLEDcol(132, 135, colorRGB);  // NEUN
          setLEDcol(152, 155, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("NEUN ");
          break;
        }
      case 10:
        {
          setLEDcol(99, 102, colorRGB);   // ZEHN (Stunden)
          setLEDcol(121, 124, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
          break;
        }
      case 11:
        {
          setLEDcol(96, 98, colorRGB);    // ELF
          setLEDcol(125, 127, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ELF ");
          break;
        }
      case 12:
        {
          setLEDcol(197, 201, colorRGB);  // ZWÖLF
          setLEDcol(214, 218, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ZWÖLF ");
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(192, 194, colorRGB);  // UHR
      setLEDcol(221, 223, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("UHR ");
    }
  }

  // ########################################################### EN:
  if (langLEDlayout == 1) {  // EN:

    // IT IS:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(16, 17, colorRGB);  // 2nd row
    setLEDcol(11, 12, colorRGB);
    setLEDcol(19, 20, colorRGB);  // 2nd row

    // FIVE: (Minutes)                         // x:05 + x:25 + x:35 + x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(38, 41, colorRGB);
      setLEDcol(54, 57, colorRGB);  // 2nd row
    }
    // QUARTER:                                // x:15 + X:45
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(72, 78, colorRGB);
      setLEDcol(81, 87, colorRGB);  // 2nd row
    }
    // A:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(5, 5, colorRGB);
      setLEDcol(26, 26, colorRGB);  // 2nd row
    }
    // TEN: (Minutes)                          // x:10 + x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(0, 2, colorRGB);
      setLEDcol(29, 31, colorRGB);  // 2nd row
    }
    // TWENTY:                                 // x:20 + x:25 + x:35 + x:40
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(42, 47, colorRGB);
      setLEDcol(48, 53, colorRGB);  // 2nd row
    }
    // PAST:                                   // x:05 + x:10 + x:15 + x:20 + x:25 + x:30
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 5) || (minDiv == 6)) {
      setLEDcol(66, 69, colorRGB);
      setLEDcol(90, 93, colorRGB);  // 2nd row
    }
    // TO:                                     // x:35 + x:40 + x:45 + x:50 + x:55
    if ((minDiv == 7) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(65, 66, colorRGB);
      setLEDcol(93, 94, colorRGB);  // 2nd row
    }
    // HALF:                                   // x:30
    if ((minDiv == 6)) {
      setLEDcol(3, 6, colorRGB);
      setLEDcol(25, 28, colorRGB);  // 2nd row
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 35 hour needs to be counted up:
    // Twenty five to two = 13:35
    if (iMinute >= 35) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(201, 203, colorRGB);  // ONE
          setLEDcol(212, 214, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(105, 107, colorRGB);  // TWO
          setLEDcol(116, 118, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(99, 103, colorRGB);   // THREE
          setLEDcol(120, 124, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(128, 131, colorRGB);  // FOUR
          setLEDcol(156, 159, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(108, 111, colorRGB);  // FIVE
          setLEDcol(112, 115, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(163, 165, colorRGB);  // SIX
          setLEDcol(186, 188, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(171, 175, colorRGB);  // SEVEN
          setLEDcol(176, 180, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(166, 170, colorRGB);  // EIGHT
          setLEDcol(181, 185, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(204, 207, colorRGB);  // NINE
          setLEDcol(208, 211, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(96, 98, colorRGB);    // TEN
          setLEDcol(125, 127, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(138, 143, colorRGB);  // ELEVEN
          setLEDcol(144, 149, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          setLEDcol(132, 137, colorRGB);  // TWELVE
          setLEDcol(150, 155, colorRGB);  // 2nd row
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(193, 199, colorRGB);  // O'CLOCK
      setLEDcol(216, 222, colorRGB);  // 2nd row
    }
  }

  // ########################################################### NL:
  if (langLEDlayout == 2) {  // NL:

    // HET IS:
    setLEDcol(13, 15, colorRGB);
    setLEDcol(16, 18, colorRGB);  // 2nd row
    setLEDcol(10, 11, colorRGB);
    setLEDcol(20, 21, colorRGB);  // 2nd row

    // VIJF: (Minuten) x:05, x:25, x:35, x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(0, 3, colorRGB);
      setLEDcol(28, 31, colorRGB);  // 2nd row
    }
    // KWART: x:15, x:45
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(38, 42, colorRGB);
      setLEDcol(53, 57, colorRGB);  // 2nd row
    }
    // TIEN: (Minuten) x:10, x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(44, 47, colorRGB);
      setLEDcol(48, 51, colorRGB);  // 2nd row
    }
    // TIEN: (TIEN VOOR HALF, TIEN OVER HALF) x:20, x:40 (on request not set to TWINTIG OVER)
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(44, 47, colorRGB);
      setLEDcol(48, 51, colorRGB);  // 2nd row
    }
    // OVER: x:05, x:10, x:15, x:35, x:40
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(33, 36, colorRGB);
      setLEDcol(59, 62, colorRGB);  // 2nd row
    }
    // VOOR: x:20, x:25, x:45, x:50, x:55
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(64, 67, colorRGB);
      setLEDcol(92, 95, colorRGB);  // 2nd row
    }
    // HALF:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(107, 110, colorRGB);
      setLEDcol(113, 116, colorRGB);  // 2nd row
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 20 hour needs to be counted up:
    // tien voor half 2 = 13:20
    if (iMinute >= 20) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(99, 101, colorRGB);   // EEN
          setLEDcol(122, 124, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(203, 206, colorRGB);  // TWEE
          setLEDcol(209, 212, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(164, 167, colorRGB);  // DRIE
          setLEDcol(184, 187, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(198, 201, colorRGB);  // VIER
          setLEDcol(214, 217, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(160, 163, colorRGB);  // VIJF
          setLEDcol(188, 191, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(96, 98, colorRGB);    // ZES
          setLEDcol(125, 127, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(129, 133, colorRGB);  // ZEVEN
          setLEDcol(154, 158, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(102, 105, colorRGB);  // ACHT
          setLEDcol(118, 121, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(171, 175, colorRGB);  // NEGEN
          setLEDcol(176, 180, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(140, 143, colorRGB);  // TIEN (Stunden)
          setLEDcol(144, 147, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(168, 170, colorRGB);  // ELF
          setLEDcol(181, 183, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          setLEDcol(134, 139, colorRGB);  // TWAALF
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(193, 195, colorRGB);  // UUR
      setLEDcol(220, 222, colorRGB);  // 2nd row
    }
  }

  // ########################################################### SWE:
  if (langLEDlayout == 3) {  // SWE:

    // KLOCKAN ÄR:
    setLEDcol(9, 15, colorRGB);
    setLEDcol(16, 22, colorRGB);  // 2nd row
    setLEDcol(5, 6, colorRGB);
    setLEDcol(25, 26, colorRGB);  // 2nd row

    // FEM: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(64, 66, colorRGB);
      setLEDcol(93, 95, colorRGB);  // 2nd row
    }
    // KVART:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(36, 40, colorRGB);
      setLEDcol(55, 59, colorRGB);  // 2nd row
    }
    // TIO: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(72, 74, colorRGB);
      setLEDcol(85, 87, colorRGB);  // 2nd row
    }
    // TJUGO:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(75, 79, colorRGB);
      setLEDcol(80, 84, colorRGB);  // 2nd row
    }
    // ÖVER:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(108, 111, colorRGB);
      setLEDcol(112, 115, colorRGB);  // 2nd row
    }
    // I:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(97, 97, colorRGB);
      setLEDcol(126, 126, colorRGB);  // 2nd row
    }
    // HALV:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(140, 143, colorRGB);
      setLEDcol(144, 147, colorRGB);  // 2nd row
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(170, 172, colorRGB);  // ETT
            setLEDcol(179, 181, colorRGB);  // 2nd row
          }
          break;
        }
      case 2:
        {
          setLEDcol(160, 162, colorRGB);  // TVÅ
          setLEDcol(189, 191, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(132, 134, colorRGB);  // TRE
          setLEDcol(153, 155, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(197, 200, colorRGB);  // FYRA
          setLEDcol(218, 218, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(194, 196, colorRGB);  // FEM
          setLEDcol(219, 221, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(201, 203, colorRGB);  // SEX
          setLEDcol(212, 214, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(173, 175, colorRGB);  // SJU
          setLEDcol(176, 178, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(128, 131, colorRGB);  // ÅTTA
          setLEDcol(156, 159, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(135, 137, colorRGB);  // NIO
          setLEDcol(150, 152, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(168, 170, colorRGB);  // TIO (Stunden)
          setLEDcol(181, 183, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(204, 207, colorRGB);  // ELVA
          setLEDcol(208, 211, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          setLEDcol(163, 166, colorRGB);  // TOLV
          setLEDcol(185, 188, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ########################################################### IT:
  if (langLEDlayout == 4) {  // IT:

    // SONO LE:
    setLEDcol(9, 10, colorRGB);   // LE
    setLEDcol(21, 22, colorRGB);  // 2nd row
    setLEDcol(12, 15, colorRGB);  // SONO
    setLEDcol(16, 19, colorRGB);  // 2nd row
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> SONO LE ");
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 40 hour needs to be counted up:
    if (iMinute >= 40) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(0, 0, colorRGB);      // È
          setLEDcol(31, 31, colorRGB);    // 2nd row
          setLEDcol(104, 108, colorRGB);  // L’UNA
          setLEDcol(115, 119, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("È L’UNA ");
          break;
        }
      case 2:
        {
          setLEDcol(101, 103, colorRGB);  // DUE
          setLEDcol(120, 122, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("DUE ");
          break;
        }
      case 3:
        {
          setLEDcol(109, 111, colorRGB);  // TRE
          setLEDcol(112, 114, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("TRE ");
          break;
        }
      case 4:
        {
          setLEDcol(73, 79, colorRGB);  // QUATTRO
          setLEDcol(80, 86, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("QUATTRO ");
          break;
        }
      case 5:
        {
          setLEDcol(64, 69, colorRGB);  // CINQUE
          setLEDcol(90, 95, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("CINQUE ");
          break;
        }
      case 6:
        {
          setLEDcol(40, 42, colorRGB);  // SEI
          setLEDcol(53, 55, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("SEI ");
          break;
        }
      case 7:
        {
          setLEDcol(43, 47, colorRGB);  // SETTE
          setLEDcol(48, 52, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("SETTE ");
          break;
        }
      case 8:
        {
          setLEDcol(70, 73, colorRGB);  // OTTO
          setLEDcol(86, 89, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("OTTO ");
          break;
        }
      case 9:
        {
          setLEDcol(97, 100, colorRGB);   // NOVE
          setLEDcol(123, 126, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("NOVE ");
          break;
        }
      case 10:
        {
          setLEDcol(138, 142, colorRGB);  // DIECI
          setLEDcol(145, 149, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("DIECI ");
          break;
        }
      case 11:
        {
          setLEDcol(1, 6, colorRGB);    // UNDICI
          setLEDcol(25, 30, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("UNDICI ");
          break;
        }
      case 12:
        {
          setLEDcol(34, 39, colorRGB);  // DODICI
          setLEDcol(56, 61, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("DODICI ");
          break;
        }
    }

    // E:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(134, 134, colorRGB);
      setLEDcol(153, 153, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("E ");
    }
    // MENO:
    if ((minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(132, 135, colorRGB);
      setLEDcol(152, 155, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("MENO ");
    }
    // 5/55: CINQUE
    if ((minDiv == 1) || (minDiv == 11)) {
      setLEDcol(162, 167, colorRGB);
      setLEDcol(184, 189, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("CINQUE ");
    }
    // 15/45: UN QUARTO
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(128, 129, colorRGB);  // UN
      setLEDcol(158, 159, colorRGB);  // 2nd row
      setLEDcol(234, 239, colorRGB);  // QUARTO
      setLEDcol(240, 245, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("UN QUARTO ");
    }
    // 10/50: DIECI
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(192, 196, colorRGB);
      setLEDcol(219, 223, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("DIECI ");
    }
    // 20/40: VENTI
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(203, 207, colorRGB);
      setLEDcol(208, 212, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("VENTI ");
    }
    // 25: VENTICINQUE
    if (minDiv == 5) {
      setLEDcol(197, 207, colorRGB);
      setLEDcol(208, 218, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("VENTICINQUE ");
    }
    // 30: TRENTA
    if (minDiv == 6) {
      setLEDcol(168, 173, colorRGB);
      setLEDcol(178, 183, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("TRENTA ");
    }
    // 35: TRENTACINQUE
    if (minDiv == 7) {
      setLEDcol(162, 173, colorRGB);
      setLEDcol(178, 189, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("TRENTACINQUE ");
    }
  }

  // ########################################################### FR:
  if (langLEDlayout == 5) {  // FR:

    // IL EST:
    setLEDcol(14, 15, colorRGB);  // IL
    setLEDcol(16, 17, colorRGB);  // 2nd row
    setLEDcol(10, 12, colorRGB);  // EST
    setLEDcol(19, 21, colorRGB);  // 2nd row

    // CINQ: (Minutes) x:05, x:25, x:35, x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(197, 200, colorRGB);
      setLEDcol(215, 218, colorRGB);  // 2nd row
    }
    // ET QUART: x:15,
    if ((minDiv == 3)) {
      setLEDcol(171, 172, colorRGB);  // ET
      setLEDcol(179, 180, colorRGB);  // 2nd row
      setLEDcol(193, 197, colorRGB);  // QUART
      setLEDcol(218, 222, colorRGB);  // 2nd row
    }
    // LE QUART: x:45
    if ((minDiv == 9)) {
      setLEDcol(172, 173, colorRGB);  // LE
      setLEDcol(178, 179, colorRGB);  // 2nd row
      setLEDcol(193, 197, colorRGB);  // QUART
      setLEDcol(218, 222, colorRGB);  // 2nd row
    }
    // DIX: (Minutes) x:10, x:50
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(167, 169, colorRGB);
      setLEDcol(182, 184, colorRGB);  // 2nd row
    }
    // VINGT: x:20, x:40
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(202, 206, colorRGB);
      setLEDcol(209, 213, colorRGB);  // 2nd row
    }
    // VINGT-: x:25, x:35
    if ((minDiv == 5) || (minDiv == 7)) {
      setLEDcol(201, 206, colorRGB);
      setLEDcol(209, 214, colorRGB);  // 2nd row
    }
    // MOINS: x:35, x:40 x:45, x:50, x:55
    if ((minDiv == 7) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(128, 132, colorRGB);
      setLEDcol(155, 159, colorRGB);  // 2nd row
    }
    // ET DEMIE: x:30
    if ((minDiv == 6)) {
      setLEDcol(171, 172, colorRGB);  // ET
      setLEDcol(179, 180, colorRGB);  // 2nd row
      setLEDcol(161, 165, colorRGB);  // DEMIE
      setLEDcol(186, 190, colorRGB);  // 2nd row
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 35 hour needs to be counted up:
    if (iMinute >= 35) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(141, 143, colorRGB);  // UNE
          setLEDcol(144, 146, colorRGB);  // 2nd row
          setLEDcol(135, 139, colorRGB);  // HEURE
          setLEDcol(148, 152, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(96, 99, colorRGB);    // DEUX
          setLEDcol(124, 127, colorRGB);  // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(107, 111, colorRGB);  // TROIS
          setLEDcol(112, 116, colorRGB);  // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(42, 47, colorRGB);    // QUATRE
          setLEDcol(48, 53, colorRGB);    // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(1, 4, colorRGB);      // CINQ
          setLEDcol(27, 30, colorRGB);    // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(64, 66, colorRGB);    // SIX
          setLEDcol(93, 95, colorRGB);    // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(104, 107, colorRGB);  // SEPT
          setLEDcol(116, 119, colorRGB);  // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(32, 35, colorRGB);    // HUIT
          setLEDcol(60, 63, colorRGB);    // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(100, 103, colorRGB);  // NEUF
          setLEDcol(120, 123, colorRGB);  // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(77, 79, colorRGB);    // DIX
          setLEDcol(80, 82, colorRGB);    // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(5, 8, colorRGB);      // ONZE
          setLEDcol(23, 26, colorRGB);    // 2nd row
          setLEDcol(134, 139, colorRGB);  // HEURES
          setLEDcol(148, 153, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          // MINUIT (0) or MIDI (12)
          if (iHour == 0 || (iHour == 23 && iMinute >= 35)) setLEDcol(36, 41, colorRGB);   // MINUIT (0)
          if (iHour == 0 || (iHour == 23 && iMinute >= 35)) setLEDcol(54, 59, colorRGB);   // 2nd row
          if (iHour == 12 || (iHour == 11 && iMinute >= 35)) setLEDcol(73, 76, colorRGB);  // MIDI (12)
          if (iHour == 12 || (iHour == 11 && iMinute >= 35)) setLEDcol(83, 86, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ########################################################### GSW:
  if (langLEDlayout == 6) {  // GSW:

    // ES ISCH:
    setLEDcol(13, 14, colorRGB);  // ES
    setLEDcol(17, 18, colorRGB);  // 2nd row
    setLEDcol(4, 7, colorRGB);    // ISCH
    setLEDcol(24, 27, colorRGB);  // 2nd row

    // FÜÜF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(44, 47, colorRGB);
      setLEDcol(48, 51, colorRGB);  // 2nd row
    }
    // VIERTEL:
    if ((minDiv == 3) || (minDiv == 9)) {
      setLEDcol(72, 78, colorRGB);
      setLEDcol(81, 87, colorRGB);  // 2nd row
    }
    // ZÄH: (Minuten)
    if ((minDiv == 2) || (minDiv == 10)) {
      setLEDcol(34, 36, colorRGB);
      setLEDcol(59, 61, colorRGB);  // 2nd row
    }
    // ZWÄNZG:
    if ((minDiv == 4) || (minDiv == 8)) {
      setLEDcol(65, 70, colorRGB);
      setLEDcol(89, 94, colorRGB);  // 2nd row
    }
    // AB:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 4) || (minDiv == 7)) {
      setLEDcol(110, 111, colorRGB);
      setLEDcol(112, 113, colorRGB);  // 2nd row
    }
    // VOR:
    if ((minDiv == 5) || (minDiv == 8) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(107, 109, colorRGB);
      setLEDcol(114, 116, colorRGB);  // 2nd row
    }
    // HALBI:
    if ((minDiv == 5) || (minDiv == 6) || (minDiv == 7)) {
      setLEDcol(101, 105, colorRGB);
      setLEDcol(118, 122, colorRGB);  // 2nd row
    }


    // set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:25
    if (iMinute >= 25) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(128, 130, colorRGB);  // EIS
          setLEDcol(157, 159, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(136, 139, colorRGB);  // ZWEI
          setLEDcol(148, 151, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(96, 99, colorRGB);    // DRÜÜ
          setLEDcol(124, 127, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(160, 164, colorRGB);  // VIERI
          setLEDcol(187, 191, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(171, 175, colorRGB);  // FÜÜFI
          setLEDcol(176, 180, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(165, 170, colorRGB);  // SÄCHSI
          setLEDcol(181, 186, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(202, 207, colorRGB);  // SIEBNI
          setLEDcol(208, 213, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(192, 196, colorRGB);  // ACHTI
          setLEDcol(219, 223, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(131, 135, colorRGB);  // NÜÜNI
          setLEDcol(152, 156, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(197, 201, colorRGB);  // ZÄHNI (Stunden)
          setLEDcol(214, 218, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(140, 143, colorRGB);  // ELFI
          setLEDcol(144, 147, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          setLEDcol(233, 238, colorRGB);  // ZWÖLFI
          setLEDcol(241, 246, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ########################################################### CN:
  if (langLEDlayout == 7) {  // CN:

    // IT IS: 现在 时间
    setLEDcol(44, 45, colorRGB);
    setLEDcol(50, 51, colorRGB);  // 2nd row
    setLEDcol(40, 41, colorRGB);
    setLEDcol(54, 55, colorRGB);  // 2nd row

    // 零五分                         // x:05
    if ((minDiv == 1)) {
      setLEDcol(101, 103, colorRGB);
      setLEDcol(120, 122, colorRGB);  // 2nd row
    }
    // 十分                         // x:10
    if ((minDiv == 2)) {
      setLEDcol(98, 99, colorRGB);
      setLEDcol(124, 125, colorRGB);  // 2nd row
    }
    // 十五分                         // x:15
    if ((minDiv == 3)) {
      setLEDcol(138, 140, colorRGB);
      setLEDcol(147, 149, colorRGB);  // 2nd row
    }
    // 二十分                         // x:20
    if ((minDiv == 4)) {
      setLEDcol(98, 100, colorRGB);
      setLEDcol(123, 125, colorRGB);  // 2nd row
    }
    // 二十五分                         // x:25
    if ((minDiv == 5)) {
      setLEDcol(138, 141, colorRGB);
      setLEDcol(146, 149, colorRGB);  // 2nd row
    }
    // 三十分                         // x:30
    if ((minDiv == 6)) {
      setLEDcol(135, 137, colorRGB);
      setLEDcol(150, 152, colorRGB);  // 2nd row
    }
    // 三十五分                         // x:35
    if ((minDiv == 7)) {
      setLEDcol(170, 173, colorRGB);
      setLEDcol(178, 181, colorRGB);  // 2nd row
    }
    // 四十分                         // x:40
    if ((minDiv == 8)) {
      setLEDcol(132, 134, colorRGB);
      setLEDcol(153, 155, colorRGB);  // 2nd row
    }
    // 四十五分                         // x:45
    if ((minDiv == 9)) {
      setLEDcol(166, 169, colorRGB);
      setLEDcol(182, 185, colorRGB);  // 2nd row
    }
    // 五十分                         // x:50
    if ((minDiv == 10)) {
      setLEDcol(163, 165, colorRGB);
      setLEDcol(186, 188, colorRGB);  // 2nd row
    }
    // 五十五分                         // x:55
    if ((minDiv == 11)) {
      setLEDcol(202, 205, colorRGB);
      setLEDcol(210, 213, colorRGB);  // 2nd row
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0) {
      xHour = 12;
    }


    switch (xHour) {
      case 1:
        {
          setLEDcol(75, 76, colorRGB);  // 一点
          setLEDcol(83, 84, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(72, 73, colorRGB);  // 二点
          setLEDcol(86, 87, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(36, 37, colorRGB);  // 三点
          setLEDcol(58, 59, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(34, 35, colorRGB);  // 四点
          setLEDcol(60, 61, colorRGB);  // 2nd row
          break;
        }
      case 5:
        {
          setLEDcol(70, 71, colorRGB);  // 五点
          setLEDcol(88, 89, colorRGB);  // 2nd row
          break;
        }
      case 6:
        {
          setLEDcol(68, 69, colorRGB);  // 六点
          setLEDcol(90, 91, colorRGB);  // 2nd row
          break;
        }
      case 7:
        {
          setLEDcol(66, 67, colorRGB);  // 七点
          setLEDcol(92, 93, colorRGB);  // 2nd row
          break;
        }
      case 8:
        {
          setLEDcol(108, 109, colorRGB);  // 八点
          setLEDcol(114, 115, colorRGB);  // 2nd row
          break;
        }
      case 9:
        {
          setLEDcol(106, 107, colorRGB);  // 九点
          setLEDcol(116, 117, colorRGB);  // 2nd row
          break;
        }
      case 10:
        {
          setLEDcol(104, 105, colorRGB);  // 十点
          setLEDcol(118, 119, colorRGB);  // 2nd row
          break;
        }
      case 11:
        {
          setLEDcol(75, 77, colorRGB);  // 十一点
          setLEDcol(82, 84, colorRGB);  // 2nd row
          break;
        }
      case 12:
        {
          setLEDcol(72, 74, colorRGB);  // 十二点
          setLEDcol(85, 87, colorRGB);  // 2nd row
          break;
        }
    }

    if (iMinute < 5) {
      setLEDcol(162, 162, colorRGB);  // 整
      setLEDcol(189, 189, colorRGB);  // 2nd row
    }
  }

  // ########################################################### SWABIAN GERMAN:
  if (langLEDlayout == 8) {  // SWABIAN GERMAN:

    // ES ISCH:
    setLEDcol(14, 15, colorRGB);
    setLEDcol(16, 17, colorRGB);  // 2nd row
    setLEDcol(9, 12, colorRGB);
    setLEDcol(19, 22, colorRGB);  // 2nd row
    if (testPrintTimeTexts == 1) {
      Serial.println("");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(" --> ES ISCH ");
    }

    // FÜNF: (Minuten)
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      setLEDcol(76, 79, colorRGB);
      setLEDcol(80, 83, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("FÜNF ");
    }

    // VIERTL:
    if (minDiv == 3) {
      setLEDcol(0, 5, colorRGB);
      setLEDcol(26, 31, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("VIERTL ");
    }

    // DREIVIERTL:
    if (minDiv == 9) {
      setLEDcol(33, 42, colorRGB);
      setLEDcol(53, 62, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("DREIVIERTL ");
    }

    // ZEHN: (Minuten)
    if ((minDiv == 2) || (minDiv == 4) || (minDiv == 8) || (minDiv == 10)) {
      setLEDcol(71, 74, colorRGB);
      setLEDcol(85, 88, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("ZEHN ");
    }

    // NACH:
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(65, 68, colorRGB);
      setLEDcol(91, 94, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("NACH ");
    }

    // VOR:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 10) || (minDiv == 11)) {
      setLEDcol(109, 111, colorRGB);
      setLEDcol(112, 114, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("VOR ");
    }

    // HALB:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      setLEDcol(102, 105, colorRGB);
      setLEDcol(118, 121, colorRGB);  // 2nd row
      if (testPrintTimeTexts == 1) Serial.print("HALB ");
    }

    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 25 hour needs to be counted up:
    // fuenf vor halb 2 = 13:15
    if (iMinute >= 15) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }

    switch (xHour) {
      case 1:
        {
          if (xHour == 1) {
            setLEDcol(165, 168, colorRGB);  // OISE
            setLEDcol(183, 186, colorRGB);  // 2nd row
            if (testPrintTimeTexts == 1) Serial.print("OISE ");
          }
          break;
        }
      case 2:
        {
          setLEDcol(160, 164, colorRGB);  // ZWOIE
          setLEDcol(187, 191, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ZWOIE ");
          break;
        }
      case 3:
        {
          setLEDcol(235, 239, colorRGB);  // DREIE
          setLEDcol(240, 244, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("DREIE ");
          break;
        }
      case 4:
        {
          setLEDcol(128, 132, colorRGB);  // VIERE
          setLEDcol(155, 159, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("VIERE ");
          break;
        }
      case 5:
        {
          setLEDcol(139, 143, colorRGB);  // FÜNFE
          setLEDcol(144, 148, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("FÜNFE ");
          break;
        }
      case 6:
        {
          setLEDcol(133, 138, colorRGB);  // SECHSE
          setLEDcol(149, 154, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("SECHSE ");
          break;
        }
      case 7:
        {
          setLEDcol(169, 175, colorRGB);  // SIEBENE
          setLEDcol(176, 182, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("SIEBENE ");
          break;
        }
      case 8:
        {
          setLEDcol(203, 207, colorRGB);  // ACHTE
          setLEDcol(208, 212, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ACHTE ");
          break;
        }
      case 9:
        {
          setLEDcol(192, 196, colorRGB);  // NEUNE
          setLEDcol(219, 223, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("NEUNE ");
          break;
        }
      case 10:
        {
          setLEDcol(96, 100, colorRGB);   // ZEHNE (Stunden)
          setLEDcol(123, 127, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ZEHNE ");
          break;
        }
      case 11:
        {
          setLEDcol(232, 235, colorRGB);  // ELFE
          setLEDcol(244, 247, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ELFE ");
          break;
        }
      case 12:
        {
          setLEDcol(197, 202, colorRGB);  // ZWÖLFE
          setLEDcol(213, 218, colorRGB);  // 2nd row
          if (testPrintTimeTexts == 1) Serial.print("ZWÖLFE ");
          break;
        }
    }
  }

  strip.show();
}


// ###########################################################################################################################################
// # Display extra minutes function:
// ###########################################################################################################################################
void showMinutes(int minutes) {
  int minMod = (minutes % 5);
  // Serial.println(minMod);

  // ##################################################### DE:
  if (langLEDlayout == 0) {  // DE:

    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(243, 243, colorRGB);  // 2nd row
          setLEDcol(226, 231, colorRGB);  // MINUTE
          setLEDcol(248, 253, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(244, 244, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(246, 246, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### EN:
  if (langLEDlayout == 1) {  // EN:
    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(243, 243, colorRGB);  // 2nd row
          setLEDcol(226, 231, colorRGB);  // MINUTE
          setLEDcol(248, 253, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(244, 244, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTES
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTES
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(246, 246, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTES
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### NL:
  if (langLEDlayout == 2) {  // NL:

    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(243, 243, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN (set to this on request, because there was no space for the extra word "minuut")
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(244, 244, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(246, 246, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTEN
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### SWE:
  if (langLEDlayout == 3) {  // SWE:

    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(243, 243, colorRGB);  // 2nd row
          setLEDcol(227, 231, colorRGB);  // MINUT
          setLEDcol(248, 252, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(244, 244, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTER
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTER
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(246, 246, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTER
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### IT:
  if (langLEDlayout == 4) {  // IT:

    switch (minMod) {
      case 1:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(247, 247, colorRGB);  // 2nd row
          setLEDcol(230, 230, colorRGB);  // 1
          setLEDcol(249, 249, colorRGB);  // 2nd row
          setLEDcol(225, 225, colorRGB);  // M
          setLEDcol(254, 254, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(247, 247, colorRGB);  // 2nd row
          setLEDcol(229, 229, colorRGB);  // 2
          setLEDcol(250, 250, colorRGB);  // 2nd row
          setLEDcol(225, 225, colorRGB);  // M
          setLEDcol(254, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(247, 247, colorRGB);  // 2nd row
          setLEDcol(228, 228, colorRGB);  // 3
          setLEDcol(251, 251, colorRGB);  // 2nd row
          setLEDcol(225, 225, colorRGB);  // M
          setLEDcol(254, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(232, 232, colorRGB);  // +
          setLEDcol(247, 247, colorRGB);  // 2nd row
          setLEDcol(227, 227, colorRGB);  // 4
          setLEDcol(252, 252, colorRGB);  // 2nd row
          setLEDcol(225, 225, colorRGB);  // M
          setLEDcol(254, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### FR:
  if (langLEDlayout == 5) {  // FR:
    switch (minMod) {
      case 1:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(236, 236, colorRGB);  // 1
          setLEDcol(243, 243, colorRGB);  // 2nd row
          setLEDcol(226, 231, colorRGB);  // MINUTE
          setLEDcol(248, 253, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(235, 235, colorRGB);  // 2
          setLEDcol(244, 244, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTES
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(234, 234, colorRGB);  // 3
          setLEDcol(245, 245, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTES
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(238, 238, colorRGB);  // +
          setLEDcol(241, 241, colorRGB);  // 2nd row
          setLEDcol(233, 233, colorRGB);  // 4
          setLEDcol(246, 246, colorRGB);  // 2nd row
          setLEDcol(225, 231, colorRGB);  // MINUTES
          setLEDcol(248, 254, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### GSW:
  if (langLEDlayout == 6) {  // GSW:

    switch (minMod) {
      case 1:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(248, 248, colorRGB);  // 2nd row
          setLEDcol(229, 229, colorRGB);  // 1
          setLEDcol(250, 250, colorRGB);  // 2nd row
          setLEDcol(224, 224, colorRGB);  // M
          setLEDcol(255, 255, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(248, 248, colorRGB);  // 2nd row
          setLEDcol(228, 228, colorRGB);  // 2
          setLEDcol(251, 251, colorRGB);  // 2nd row
          setLEDcol(224, 224, colorRGB);  // M
          setLEDcol(255, 255, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(248, 248, colorRGB);  // 2nd row
          setLEDcol(227, 227, colorRGB);  // 3
          setLEDcol(252, 252, colorRGB);  // 2nd row
          setLEDcol(224, 224, colorRGB);  // M
          setLEDcol(255, 255, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(231, 231, colorRGB);  // +
          setLEDcol(248, 248, colorRGB);  // 2nd row
          setLEDcol(226, 226, colorRGB);  // 4
          setLEDcol(253, 253, colorRGB);  // 2nd row
          setLEDcol(224, 224, colorRGB);  // M
          setLEDcol(255, 255, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ########################################################### CN:
  if (langLEDlayout == 7) {  // CN:
    switch (minMod) {
      case 1:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(215, 215, colorRGB);  // 2nd row
          setLEDcol(199, 199, colorRGB);  // 一
          setLEDcol(216, 216, colorRGB);  // 2nd row
          setLEDcol(194, 195, colorRGB);  // 分钟
          setLEDcol(220, 221, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(215, 215, colorRGB);  // 2nd row
          setLEDcol(198, 198, colorRGB);  // 二
          setLEDcol(217, 217, colorRGB);  // 2nd row
          setLEDcol(194, 195, colorRGB);  // 分钟
          setLEDcol(220, 221, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(215, 215, colorRGB);  // 2nd row
          setLEDcol(197, 197, colorRGB);  // 三
          setLEDcol(218, 218, colorRGB);  // 2nd row
          setLEDcol(194, 195, colorRGB);  // 分钟
          setLEDcol(220, 221, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(200, 200, colorRGB);  // 加
          setLEDcol(215, 215, colorRGB);  // 2nd row
          setLEDcol(196, 196, colorRGB);  // 四
          setLEDcol(219, 219, colorRGB);  // 2nd row
          setLEDcol(194, 195, colorRGB);  // 分钟
          setLEDcol(220, 221, colorRGB);  // 2nd row
          break;
        }
    }
  }

  // ##################################################### SWABIAN:
  if (langLEDlayout == 8) {  // SWABIAN:

    switch (minMod) {
      case 1:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(249, 249, colorRGB);  // 2nd row
          setLEDcol(229, 229, colorRGB);  // 1
          setLEDcol(250, 250, colorRGB);  // 2nd row
          setLEDcol(224, 224, colorRGB);  // M
          setLEDcol(255, 255, colorRGB);  // 2nd row
          break;
        }
      case 2:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(249, 249, colorRGB);  // 2nd row
          setLEDcol(228, 228, colorRGB);  // 2
          setLEDcol(251, 251, colorRGB);  // 2nd row
          setLEDcol(224, 224, colorRGB);  // M
          setLEDcol(255, 255, colorRGB);  // 2nd row
          break;
        }
      case 3:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(249, 249, colorRGB);  // 2nd row
          setLEDcol(227, 227, colorRGB);  // 3
          setLEDcol(252, 252, colorRGB);  // 2nd row
          setLEDcol(224, 224, colorRGB);  // M
          setLEDcol(255, 255, colorRGB);  // 2nd row
          break;
        }
      case 4:
        {
          setLEDcol(230, 230, colorRGB);  // +
          setLEDcol(249, 249, colorRGB);  // 2nd row
          setLEDcol(226, 226, colorRGB);  // 4
          setLEDcol(253, 253, colorRGB);  // 2nd row
          setLEDcol(224, 224, colorRGB);  // M
          setLEDcol(255, 255, colorRGB);  // 2nd row
          break;
        }
    }
  }
}


// ###########################################################################################################################################
// # Background color function: SET ALL LEDs OFF
// ###########################################################################################################################################
void back_color() {
  uint32_t c0 = strip.Color(redVal_back, greenVal_back, blueVal_back);  // Background color
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, c0);
  }
  delay(500);
}


// ###########################################################################################################################################
// # Startup WiFi text function:
// ###########################################################################################################################################
void SetWLAN(uint32_t color) {
  Serial.println("Show text WLAN/WIFI...");

  if (langLEDlayout == 0) {  // DE:
    for (uint16_t i = 5; i < 9; i++) {
      strip.setPixelColor(i, color);
    }
    for (uint16_t i = 23; i < 27; i++) {  // 2nd row
      strip.setPixelColor(i, color);
    }
  }

  if (langLEDlayout == 1) {  // EN:
    for (uint16_t i = 7; i < 11; i++) {
      strip.setPixelColor(i, color);
    }
    for (uint16_t i = 21; i < 25; i++) {  // 2nd row
      strip.setPixelColor(i, color);
    }
  }

  if (langLEDlayout == 2) {  // NL:
    for (uint16_t i = 75; i < 79; i++) {
      strip.setPixelColor(i, color);
    }
    for (uint16_t i = 81; i < 85; i++) {  // 2nd row
      strip.setPixelColor(i, color);
    }
  }

  if (langLEDlayout == 3) {  // SWE:
    for (uint16_t i = 0; i < 4; i++) {
      strip.setPixelColor(i, color);
    }
    for (uint16_t i = 28; i < 32; i++) {  // 2nd row
      strip.setPixelColor(i, color);
    }
  }

  if (langLEDlayout == 4) {      // IT:
    setLEDcol(233, 233, color);  // W
    setLEDcol(246, 246, color);  // 2nd row
    setLEDcol(231, 231, color);  // I
    setLEDcol(248, 248, color);  // 2nd row
    setLEDcol(226, 226, color);  // F
    setLEDcol(253, 253, color);  // 2nd row
    setLEDcol(224, 224, color);  // I
    setLEDcol(255, 255, color);  // 2nd row
  }

  if (langLEDlayout == 5) {      // FR:
    setLEDcol(239, 239, color);  // W
    setLEDcol(240, 240, color);  // 2nd row
    setLEDcol(237, 237, color);  // I
    setLEDcol(242, 242, color);  // 2nd row
    setLEDcol(232, 232, color);  // F
    setLEDcol(247, 247, color);  // 2nd row
    setLEDcol(224, 224, color);  // I
    setLEDcol(255, 255, color);  // 2nd row
  }

  if (langLEDlayout == 6) {    // GSW:
    setLEDcol(7, 10, color);   // WIFI
    setLEDcol(21, 24, color);  // 2nd row
  }

  if (langLEDlayout == 7) {    // CN:
    setLEDcol(42, 43, color);  // WIFI
    setLEDcol(52, 53, color);  // 2nd row
  }

  if (langLEDlayout == 8) {    // SWABIAN:
    setLEDcol(12, 13, color);  // WI
    setLEDcol(18, 19, color);  // 2nd row
    setLEDcol(7, 8, color);    // FI
    setLEDcol(23, 24, color);  // 2nd row
  }

  strip.show();
}


// ###########################################################################################################################################
// # Wifi Manager setup and reconnect function that runs once at startup and during the loop function of the ESP:
// ###########################################################################################################################################
void WIFI_login() {
  Serial.print("Try to connect to WiFi: ");
  Serial.println(WiFi.SSID());
  SetWLAN(strip.Color(0, 0, 255));
  WiFi.setHostname(hostname);
  bool WiFires;
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(AP_TIMEOUT);
  WiFires = wifiManager.autoConnect(DEFAULT_AP_NAME);
  if (!WiFires) {
    Serial.print("Failed to connect to WiFi: ");
    Serial.println(WiFi.SSID());
    SetWLAN(strip.Color(255, 0, 0));
    delay(1000);
  } else {
    Serial.print("Connected to WiFi: ");
    Serial.println(WiFi.SSID());
    SetWLAN(strip.Color(0, 255, 0));
    delay(1000);
  }
}


// ###########################################################################################################################################
// # WiFi Manager 1st connect fix: (Needed after the 1st login to your router - Restart the device once to be able to reach the web page...)
// ###########################################################################################################################################
void WiFiManager1stBootFix() {
  WiFiManFix = preferences.getUInt("WiFiManFix", 0);
  if (WiFiManFix == 0) {
    Serial.println("######################################################################");
    Serial.println("# ESP restart needed because of WiFi Manager Fix");
    Serial.println("######################################################################");
    SetWLAN(strip.Color(0, 255, 0));
    preferences.putUInt("WiFiManFix", 1);
    delay(1000);
    preferences.end();
    delay(1000);
    ESP.restart();
  }
}


// ###########################################################################################################################################
// # Actual function, which controls 1/0 of the LED:
// ###########################################################################################################################################
void setLED(int ledNrFrom, int ledNrTo, int switchOn) {
  if (ledNrFrom > ledNrTo) {
    setLED(ledNrTo, ledNrFrom, switchOn);  // Sets LED numbers in correct order
  } else {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS))
        strip.setPixelColor(i, strip.Color(redVal_time, greenVal_time, blueVal_time));
    }
  }
  if (switchOn == 0) {
    for (int i = ledNrFrom; i <= ledNrTo; i++) {
      if ((i >= 0) && (i < NUMPIXELS))
        strip.setPixelColor(i, strip.Color(0, 0, 0));  // Switch LEDs off
    }
  }
}


// ###########################################################################################################################################
// # NTP time functions:
// ###########################################################################################################################################
void configNTPTime() {
  initTime(Timezone);
  printLocalTime();
}
// ###########################################################################################################################################
void setTimezone(String timezone) {
  Serial.printf("Setting timezone to %s\n", timezone.c_str());
  setenv("TZ", timezone.c_str(), 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}
// ###########################################################################################################################################
void initTime(String timezone) {
  struct tm timeinfo;
  Serial.println("Setting up time");
  configTime(0, 0, NTPserver);
  if (!getLocalTime(&timeinfo)) {
    back_color();
    Serial.println("Failed to obtain time");
    ESPUI.print(statusLabelID, "Failed to obtain time");

    if (langLEDlayout == 0) {  // DE:
      setLEDcol(1, 4, strip.Color(255, 0, 0));
      setLEDcol(27, 30, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 1) {  // EN:
      setLEDcol(33, 36, strip.Color(255, 0, 0));
      setLEDcol(59, 62, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 2) {  // NL:
      setLEDcol(69, 72, strip.Color(255, 0, 0));
      setLEDcol(87, 90, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 3) {  // SWE:
      setLEDcol(96, 98, strip.Color(255, 0, 0));
      setLEDcol(125, 127, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 4) {                       // IT:
      setLEDcol(111, 111, strip.Color(255, 0, 0));  // T
      setLEDcol(112, 112, strip.Color(255, 0, 0));  // 2nd row
      setLEDcol(96, 97, strip.Color(255, 0, 0));    // EM
      setLEDcol(126, 127, strip.Color(255, 0, 0));  // 2nd row
      setLEDcol(131, 131, strip.Color(255, 0, 0));  // P
      setLEDcol(156, 156, strip.Color(255, 0, 0));  // 2nd row
      setLEDcol(234, 234, strip.Color(255, 0, 0));  // O
      setLEDcol(245, 245, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 5) {                       // FR:
      setLEDcol(10, 10, strip.Color(255, 0, 0));    // T
      setLEDcol(21, 21, strip.Color(255, 0, 0));    // 2nd row
      setLEDcol(41, 42, strip.Color(255, 0, 0));    // EM
      setLEDcol(53, 54, strip.Color(255, 0, 0));    // 2nd row
      setLEDcol(105, 105, strip.Color(255, 0, 0));  // p
      setLEDcol(118, 118, strip.Color(255, 0, 0));  // 2nd row
      setLEDcol(128, 128, strip.Color(255, 0, 0));  // S
      setLEDcol(159, 159, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 6) {                     // GSW:
      setLEDcol(0, 3, strip.Color(255, 0, 0));    // ZIIT
      setLEDcol(28, 31, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 7) {  // CN:
      setLEDcol(40, 41, strip.Color(255, 0, 0));
      setLEDcol(54, 55, strip.Color(255, 0, 0));  // 2nd row
    }

    if (langLEDlayout == 8) {                       // SWABIAN:
      setLEDcol(73, 74, strip.Color(255, 0, 0));    // ZE
      setLEDcol(85, 86, strip.Color(255, 0, 0));    // 2nd row
      setLEDcol(131, 131, strip.Color(255, 0, 0));  // I
      setLEDcol(156, 156, strip.Color(255, 0, 0));  // 2nd row
      setLEDcol(204, 204, strip.Color(255, 0, 0));  // T
      setLEDcol(211, 211, strip.Color(255, 0, 0));  // 2nd row
    }

    strip.show();
    delay(1000);
    ESP.restart();
    return;
  } else {
    back_color();
    Serial.println("Failed to obtain time");
    ESPUI.print(statusLabelID, "Failed to obtain time");

    if (langLEDlayout == 0) {  // DE:
      setLEDcol(1, 4, strip.Color(0, 255, 0));
      setLEDcol(27, 30, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 1) {  // EN:
      setLEDcol(33, 36, strip.Color(0, 255, 0));
      setLEDcol(59, 62, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 2) {  // NL:
      setLEDcol(69, 72, strip.Color(0, 255, 0));
      setLEDcol(87, 90, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 3) {  // SWE:
      setLEDcol(96, 98, strip.Color(0, 255, 0));
      setLEDcol(125, 127, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 4) {                       // IT:
      setLEDcol(111, 111, strip.Color(0, 255, 0));  // T
      setLEDcol(112, 112, strip.Color(0, 255, 0));  // 2nd row
      setLEDcol(96, 97, strip.Color(0, 255, 0));    // EM
      setLEDcol(126, 127, strip.Color(0, 255, 0));  // 2nd row
      setLEDcol(131, 131, strip.Color(0, 255, 0));  // P
      setLEDcol(156, 156, strip.Color(0, 255, 0));  // 2nd row
      setLEDcol(234, 234, strip.Color(0, 255, 0));  // O
      setLEDcol(245, 245, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 5) {                       // FR:
      setLEDcol(10, 10, strip.Color(0, 255, 0));    // T
      setLEDcol(21, 21, strip.Color(0, 255, 0));    // 2nd row
      setLEDcol(41, 42, strip.Color(0, 255, 0));    // EM
      setLEDcol(53, 54, strip.Color(0, 255, 0));    // 2nd row
      setLEDcol(105, 105, strip.Color(0, 255, 0));  // p
      setLEDcol(118, 118, strip.Color(0, 255, 0));  // 2nd row
      setLEDcol(128, 128, strip.Color(0, 255, 0));  // S
      setLEDcol(159, 159, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 6) {                     // GSW:
      setLEDcol(0, 3, strip.Color(0, 255, 0));    // ZIIT
      setLEDcol(28, 31, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 7) {  // CN:
      setLEDcol(40, 41, strip.Color(0, 255, 0));
      setLEDcol(54, 55, strip.Color(0, 255, 0));  // 2nd row
    }

    if (langLEDlayout == 8) {                       // SWABIAN:
      setLEDcol(73, 74, strip.Color(0, 255, 0));    // ZE
      setLEDcol(85, 86, strip.Color(0, 255, 0));    // 2nd row
      setLEDcol(131, 131, strip.Color(0, 255, 0));  // I
      setLEDcol(156, 156, strip.Color(0, 255, 0));  // 2nd row
      setLEDcol(204, 204, strip.Color(0, 255, 0));  // T
      setLEDcol(211, 211, strip.Color(0, 255, 0));  // 2nd row
    }

    strip.show();
    delay(1000);
  }
  Serial.println("Got the time from NTP");
  setTimezone(timezone);
}
// ###########################################################################################################################################
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time 1");
    return;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  iStartTime = String(timeStringBuff);
  // Serial.println(iStartTime);
  iHour = timeinfo.tm_hour;
  iMinute = timeinfo.tm_min;
  iSecond = timeinfo.tm_sec;
  // Serial.print("Time: ");
  // Serial.print(iHour);
  // Serial.print(":");
  // Serial.print(iMinute);
  // Serial.print(":");
  // Serial.println(iSecond);
  delay(1000);
}
// ###########################################################################################################################################
void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst) {
  struct tm tm;
  tm.tm_year = yr - 1900;  // Set date
  tm.tm_mon = month - 1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;  // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}
// ###########################################################################################################################################


// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - TIME:
// ###########################################################################################################################################
void getRGBTIME(String hexvalue) {
  updatedevice = false;
  delay(1000);
  hexvalue.toUpperCase();
  char c[7];
  hexvalue.toCharArray(c, 8);
  int red = hexcolorToInt(c[1], c[2]);
  int green = hexcolorToInt(c[3], c[4]);
  int blue = hexcolorToInt(c[5], c[6]);
  redVal_time = red;
  greenVal_time = green;
  blueVal_time = blue;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - BACKGROUND:
// ###########################################################################################################################################
void getRGBBACK(String hexvalue) {
  updatedevice = false;
  delay(1000);
  hexvalue.toUpperCase();
  char c[7];
  hexvalue.toCharArray(c, 8);
  int red = hexcolorToInt(c[1], c[2]);
  int green = hexcolorToInt(c[3], c[4]);
  int blue = hexcolorToInt(c[5], c[6]);
  redVal_back = red;
  greenVal_back = green;
  blueVal_back = blue;
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Convert hex color value to RGB int values - helper function:
// ###########################################################################################################################################
int hexcolorToInt(char upper, char lower) {
  int uVal = (int)upper;
  int lVal = (int)lower;
  uVal = uVal > 64 ? uVal - 55 : uVal - 48;
  uVal = uVal << 4;
  lVal = lVal > 64 ? lVal - 55 : lVal - 48;
  return uVal + lVal;
}


// ###########################################################################################################################################
// # GUI: Color change for time color:
// ###########################################################################################################################################
void colCallTIME(Control* sender, int type) {
  getRGBTIME(sender->value);
}


// ###########################################################################################################################################
// # GUI: Color change for background color:
// ###########################################################################################################################################
void colCallBACK(Control* sender, int type) {
  getRGBBACK(sender->value);
}


// ###########################################################################################################################################
// # GUI: Slider change for LED intensity: DAY
// ###########################################################################################################################################
void sliderBrightnessDay(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  intensity_day = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Slider change for LED intensity: NIGHT
// ###########################################################################################################################################
void sliderBrightnessNight(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  intensity_night = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Time Day Mode Start
// ###########################################################################################################################################
void call_day_time_start(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  day_time_start = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Time Day Mode Stop
// ###########################################################################################################################################
void call_day_time_stop(Control* sender, int type) {
  updatedevice = false;
  delay(1000);
  day_time_stop = sender->value.toInt();
  changedvalues = true;
  updatedevice = true;
}


// ###########################################################################################################################################
// # GUI: Convert IP-address value to string:
// ###########################################################################################################################################
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]);
}


// ###########################################################################################################################################
// # ESP32 OTA update:
// ###########################################################################################################################################
const char* loginIndex =
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>WordClock Update Login Page</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td>Username:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Password:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  "function check(form)"
  "{"
  "if(form.userid.value=='WordClock' && form.pwd.value=='16x16')"
  "{"
  "window.open('/serverIndex')"
  "}"
  "else"
  "{"
  " alert('Error Password or Username')/*displays error message*/"
  "}"
  "}"
  "</script>";

const char* serverIndex =
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>progress: 0%</div>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
  "},"
  "error: function (a, b, c) {"
  "}"
  "});"
  "});"
  "</script>";

void handleOTAupdate() {
  // OTA update server pages urls:
  updserver.on("/", HTTP_GET, []() {
    updserver.sendHeader("Connection", "close");
    updserver.send(200, "text/html", "WordClock web server on port 2022 is up. Please use the shown url and account credentials to update...");
  });

  updserver.on("/ota", HTTP_GET, []() {
    updserver.sendHeader("Connection", "close");
    updserver.send(200, "text/html", loginIndex);
  });

  updserver.on("/serverIndex", HTTP_GET, []() {
    updserver.sendHeader("Connection", "close");
    updserver.send(200, "text/html", serverIndex);
  });

  // handling uploading firmware file:
  updserver.on(
    "/update", HTTP_POST, []() {
      updserver.sendHeader("Connection", "close");
      updserver.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    },
    []() {
      HTTPUpload& upload = updserver.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {  //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        // flashing firmware to ESP
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {  //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          delay(1000);
        } else {
          Update.printError(Serial);
        }
      }
    });
  updserver.begin();
}


// ###########################################################################################################################################
// # HTML command web server:
// ###########################################################################################################################################
int ew = 0;  // Current extra word
String ledstatus = "ON";
void handleLEDupdate() {  // LED server pages urls:

  ledserver.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {  // Show a manual how to use these links:
    String message = "WordClock web configuration and querry options examples:\n\n";
    message = message + "General:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023 --> Shows this text\n\n";
    message = message + "Get the status of the WordClock LEDs:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/status --> Show the status of the LEDs (0 = OFF and 1 = ON).\n\n";
    message = message + "Turn the LEDs OFF or ON:\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=0 --> LED intensity is set to OFF which will turn the display off.\n";
    message = message + "http://" + IpAddress2String(WiFi.localIP()) + ":2023/config?LEDs=1 --> LED intensity is set to ON which will turn the display on again...\n";
    request->send(200, "text/plain", message);
  });

  ledserver.on("/config", HTTP_GET, [](AsyncWebServerRequest* request) {  // Configure background and time texts color and intensity:
    int paramsNr = request->params();
    // Serial.println(paramsNr);
    for (int i = 0; i < paramsNr; i++) {
      AsyncWebParameter* p = request->getParam(i);
      // Serial.print("Param name: ");
      // Serial.println(p->name());
      // Serial.print("Param value: ");
      // Serial.println(p->value());
      // Serial.println("------------------");
      if ((p->value().toInt() >= 0) && (p->value().toInt() <= 1)) {
        if ((String(p->name()) == "LEDs") && (p->value().toInt() == 0)) {
          set_web_intensity = 1;
          ledstatus = "OFF";
          ESPUI.updateVisibility(intensity_web_HintID, true);
          ESPUI.updateVisibility(statusNightModeID, false);
          ESPUI.updateVisibility(sliderBrightnessDayID, false);
          ESPUI.updateVisibility(switchNightModeID, false);
          ESPUI.updateVisibility(sliderBrightnessNightID, false);
          ESPUI.updateVisibility(call_day_time_startID, false);
          ESPUI.updateVisibility(call_day_time_stopID, false);
          ESPUI.updateVisibility(text_colour_time, false);
          ESPUI.updateVisibility(text_colour_background, false);
          ESPUI.updateVisibility(switchRandomColorID, false);
          ESPUI.updateVisibility(DayNightSectionID, false);
          ESPUI.updateVisibility(switchSingleMinutesID, false);
          ESPUI.jsonReload();
        }
        if ((String(p->name()) == "LEDs") && (p->value().toInt() == 1)) {
          set_web_intensity = 0;
          ledstatus = "ON";
          ESPUI.updateVisibility(intensity_web_HintID, false);
          ESPUI.updateVisibility(statusNightModeID, true);
          ESPUI.updateVisibility(sliderBrightnessDayID, true);
          ESPUI.updateVisibility(switchNightModeID, true);
          ESPUI.updateVisibility(sliderBrightnessNightID, true);
          ESPUI.updateVisibility(call_day_time_startID, true);
          ESPUI.updateVisibility(call_day_time_stopID, true);
          ESPUI.updateVisibility(text_colour_time, true);
          ESPUI.updateVisibility(text_colour_background, true);
          ESPUI.updateVisibility(switchRandomColorID, true);
          ESPUI.updateVisibility(DayNightSectionID, true);
          ESPUI.updateVisibility(switchSingleMinutesID, true);
        }
        changedvalues = true;
        updatenow = true;
      } else {
        request->send(200, "text/plain", "INVALID VALUES - MUST BE BETWEEN 0 and 1");
      }
    }
    request->send(200, "text/plain", "WordClock LEDs set to: " + ledstatus);
  });

  ledserver.on("/status", HTTP_GET, [](AsyncWebServerRequest* request) {  // Show the status of all extra words and the color for the background and time texts:
    String message = ledstatus;
    request->send(200, "text/plain", message);
  });

  ledserver.begin();
}

// ###########################################################################################################################################
// # EOF - You have successfully reached the end of the code - well done ;-)
// ###########################################################################################################################################