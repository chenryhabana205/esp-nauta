#include <ESP8266WiFi.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <JC_Button.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <regex>

#include "DebugUtils.h"

#include "vars.h"

void saveConfigCallback()
{
  nautaManager->setUsername(wmUsernameParam.getValue());
  nautaManager->setPassword(wmPasswordParam.getValue());
  nautaManager->SaveConfig();
}

void setupWifiAndConfig()
{
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&wmUsernameParam);
  wifiManager.addParameter(&wmPasswordParam);

  String chipID = String(ESP.getChipId(), HEX);
  chipID.toUpperCase();
  String AP_SSID = "IOT_" + chipID + "_AutoConnectAP";
  String AP_PASS = "IOT_" + chipID;

  wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str());

  // Serial.println("connected...yeey :)");
  DebugPrintln("connected...yeey :)");
}

void setupButtons()
{
  pinMode(BUTTON, INPUT);
}

void setup()
{
  Serial.begin(115200);
  if (!nautaManager->LoadConfig())
  {
    WiFiManager wm;
    wm.resetSettings();
  }
  else
  {
    wmUsernameParam.setValue(nautaManager->getUsername().c_str(), nautaManager->getUsername().length());
    wmPasswordParam.setValue(nautaManager->getPassword().c_str(), nautaManager->getPassword().length());
  }
  setupWifiAndConfig();
  setupButtons();
  loggedIn = nautaManager->LoadSession();
  DebugPrintln("loggedId: " + String(loggedIn));
  // loggedIn = false;
}

void checkIfSessionChangeIsNeeded()
{
  if (btn.wasReleased())
  {
    DebugPrintln("Changing session state");
    // nautaManager->Logout("celiap88@nauta.com.cu", "d3c4438df803a3f6a94cefa845f6b196", "399CC7A68DEDB0EBF6CFA74CFE7983A8", "10.224.224.181", "3AC5F90824E46FE9D9B42B450B819222");
    if (!loggedIn)
    {
      // nautaManager->testRedirect();
      DebugPrintln("login");
      if (nautaManager->Login())
      {
        loggedIn = true;
      }
    }
    else
    {
      DebugPrintln("logout");
      if (nautaManager->Logout())
      {
        loggedIn = false;
      }
    }
  }
}

void checkIfCleanDataIsNeeded()
{
  if (btn.pressedFor(2000))
  {
    DebugPrintln("Clearing saved data!");
    WiFiManager wm;
    wm.erase();
    wm.reboot();
  }
}

void checkIfWifiIsActive() {
  WiFiManager wm;
}

void loop()
{
  btn.read();
  checkIfCleanDataIsNeeded();
  checkIfSessionChangeIsNeeded();
  checkIfWifiIsActive();
}