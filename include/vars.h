#ifndef VARS_H
#define VARS_H

#include <Arduino.h>
#include <JC_Button.h>
#include <WiFiManager.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <TickerScheduler.h>

#include "NautaManager.h"
#include "UdpBroadcaster.h"

const int max_length = 255;

#define TASK_CHECK_WIFI 0
#define TASK_UPDATE_TIME 1

struct COOKIES
{
    String JSESSIONID;
} cookies;

struct SESSIONDATA
{
    String username;
    String CSRFHW;
    String wlanuserip;
    String ATTRIBUTE_UUID;
    String loggerId;
} sessionData;

uint8_t BUTTON = D5;
auto readyForOTA = false;
auto loggedIn = false;

Button btn(BUTTON, 50U, true, true);

WiFiManagerParameter wmUsernameParam("username", "username", "", 250);
WiFiManagerParameter wmPasswordParam("password", "password", "", 250);

std::shared_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
std::shared_ptr<HTTPClient> httpClient;

std::shared_ptr<NautaManager> nautaManager(new NautaManager("/nautaConfig.bin", "/nautaSession.bin"));
std::shared_ptr<UdpBroadcaster> udpBroadcaster(new UdpBroadcaster());

std::shared_ptr<TickerScheduler> tickerScheduler(new TickerScheduler(10));

#endif