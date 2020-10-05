#ifndef VARS_H
#define VARS_H

#include <Arduino.h>
#include <JC_Button.h>
#include <WiFiManager.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>

#include "NautaManager.h"

const int max_length = 255;

char nauta_username[40];
char nauta_password[40];

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

uint8_t BUTTON = 0;
auto readyForOTA = false;
auto loggedIn = false;

Button btn(BUTTON, 50U, true, true);

WiFiManagerParameter wmUsernameParam("username", "username", nauta_username, 40);
WiFiManagerParameter wmPasswordParam("password", "password", nauta_password, 40);

std::shared_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
std::shared_ptr<HTTPClient> httpClient;

std::shared_ptr<NautaManager> nautaManager(new NautaManager("/nautaConfig.bin", "/nautaSession.bin"));

#endif