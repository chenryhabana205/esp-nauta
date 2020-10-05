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

    DebugPrintln("connected...yeey :)");
}

void updateStatusLed()
{
    if (WiFi.isConnected())
    {
        // DebugPrintln("WiFi ON");
        digitalWrite(LED_BUILTIN, LOW);
    }
    else
    {
        // DebugPrintln("WiFi OFF");
        digitalWrite(LED_BUILTIN, HIGH);
    }
    if (loggedIn)
    {
        // DebugPrintln("Nauta is ON");
        digitalWrite(LED_BUILTIN_AUX, LOW);
    }
    else
    {
        // DebugPrintln("Nauta is OFF");
        digitalWrite(LED_BUILTIN_AUX, HIGH);
    }
}

void checkIfWifiIsActive(void *p)
{
    updateStatusLed();
    // if (!WiFi.isConnected())
    // {
    //     DebugPrintln("Wifi disconected, restarting...");
    //     ESP.reset();
    // }
}

void blink(void *p)
{
    DebugPrintln("blink");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    digitalWrite(LED_BUILTIN_AUX, !digitalRead(LED_BUILTIN_AUX));
}

void setupButtons()
{
    pinMode(BUTTON, INPUT_PULLUP);
}

void setupLeds()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_BUILTIN_AUX, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_BUILTIN_AUX, HIGH);
}

void updateRemainingTime(void *)
{
    if (loggedIn)
    {
        auto time = nautaManager->GetRemainingTime();
        udpBroadcaster->BroadcastMessage(NAUTA_TIME_REMAINING, time);
    }
}

void messageCallback(String message)
{
    udpBroadcaster->BroadcastMessage(NAUTA_MESSAGE, message);
}

void setupTimers()
{
    tickerScheduler->add(TASK_CHECK_WIFI, 5000, checkIfWifiIsActive, nullptr, false);
    tickerScheduler->add(TASK_UPDATE_TIME, 60000, updateRemainingTime, nullptr, false);
}

void setup()
{
    Serial.begin(115200);
    nautaManager->SetMessageCallback(messageCallback);
    if (!nautaManager->LoadConfig())
    {
        WiFiManager wm;
        wm.resetSettings();
    }
    else
    {
        wmUsernameParam.setValue(nautaManager->getUsername().c_str(), 250);
        wmPasswordParam.setValue(nautaManager->getPassword().c_str(), 250);
    }
    setupWifiAndConfig();
    setupButtons();
    setupLeds();
    setupTimers();
    loggedIn = nautaManager->LoadSession();
    updateStatusLed();
    DebugPrintln("loggedId: " + String(loggedIn));
}

void checkIfSessionChangeIsNeeded()
{
    if (btn.wasReleased())
    {
        DebugPrintln("Changing session state");
        if (!loggedIn)
        {
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
        updateStatusLed();
    }
}

void checkIfCleanDataIsNeeded()
{
    if (btn.pressedFor(10000))
    {
        DebugPrintln("Clearing saved data!");
        WiFiManager wm;
        wm.erase();
        wm.reboot();
    }
}

void loop()
{
    btn.read();
    tickerScheduler->update();
    checkIfCleanDataIsNeeded();
    checkIfSessionChangeIsNeeded();
}