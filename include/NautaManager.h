#ifndef NAUTAMANAGER_H
#define NAUTAMANAGER_H

#include <Arduino.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>

#define MAX_LENGTH 250

struct config_struct
{
    char USERNAME[MAX_LENGTH];
    char PASSWORD[MAX_LENGTH];
};

struct session_struct
{
    char CSRFHW[MAX_LENGTH];
    char ATTRIBUTE_UUID[MAX_LENGTH];
    char wlanuserip[MAX_LENGTH];
    char loggerId[MAX_LENGTH];
    char JSESSIONID[MAX_LENGTH];
};

class NautaManager
{
private:
    String _sessionFile;
    String _configFile;
    String _cachedResponse;
    std::function<void(String)> msgCallback;
    bool msgCallbackAssigned = false;

    void GetCookie(String cookie);
    void GetCookies(std::shared_ptr<HTTPClient> httpClient);
    bool InitSession();
    String GetValueFromLine(String line);
    void GetSessionData(std::shared_ptr<HTTPClient> httpClient);
    String GetLoginError();
    bool GetATTRIBUTE_UUID();
    bool PostCredentials();
    String GetLoginPayload();
    String GetLogoutPayload();
    String GetTimePayload();
    void SaveResponseToCache(std::shared_ptr<HTTPClient> httpClient);
    void SendMessage(String message);

public:
    NautaManager(String sessionFile, String configFile);
    ~NautaManager();
    void SetMessageCallback(std::function<void(String)> messageCallback);

    bool LoadConfig();
    void SaveConfig();

    bool LoadSession();
    void SaveSession();
    void ClearSession();

    bool Login();
    bool Logout();
    String GetRemainingTime();

    bool Logout(String username, String csrfhw, String attr, String wlanuserip, String sessionid);

    void setUsername(String username);
    String getUsername();
    void setPassword(String password);
    String getPassword();

    void testRedirect();
};

#endif