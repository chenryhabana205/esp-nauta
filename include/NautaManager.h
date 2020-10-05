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

// struct cookies_struct
// {
//     char JSESSIONID[MAX_LENGTH];
// };

class NautaManager
{
private:
    String _sessionFile;
    String _configFile;
    String _cachedResponse;

    config_struct _config;
    session_struct _session;
    // cookies_struct _cookies;

    // std::shared_ptr<BearSSL::WiFiClientSecure> _wifiClient;
    // std::shared_ptr<HTTPClient> _httpClient;

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
    void SaveResponseToCache(std::shared_ptr<HTTPClient> httpClient);

public:
    NautaManager(String sessionFile, String configFile);
    ~NautaManager();

    bool LoadConfig();
    void SaveConfig();

    bool LoadSession();
    void SaveSession();
    void ClearSession();

    bool Login();
    bool Logout();

    bool Logout(String username, String csrfhw, String attr, String wlanuserip, String sessionid);

    void setUsername(String username);
    String getUsername();
    void setPassword(String password);
    String getPassword();

    void testRedirect();
};

#endif