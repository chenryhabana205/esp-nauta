#include <LittleFS.h>
#include "DebugUtils.h"
#include "NautaManager.h"

const char *headerKeys[] = {"Set-Cookie", "Location", "Cookie", "Date"};
const size_t numberOfHeaders = 4;

session_struct _session;
config_struct _config;

unsigned char h2int(char c)
{
    if (c >= '0' && c <= '9')
    {
        return ((unsigned char)c - '0');
    }
    if (c >= 'a' && c <= 'f')
    {
        return ((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F')
    {
        return ((unsigned char)c - 'A' + 10);
    }
    return (0);
}

void myStrCpy(char *dest, const char *src, const size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        dest[i] = src[i];
    }
}

String urldecode(String str)
{

    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (size_t i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (c == '+')
        {
            encodedString += ' ';
        }
        else if (c == '%')
        {
            i++;
            code0 = str.charAt(i);
            i++;
            code1 = str.charAt(i);
            c = (h2int(code0) << 4) | h2int(code1);
            encodedString += c;
        }
        else
        {

            encodedString += c;
        }

        yield();
    }

    return encodedString;
}

String urlencode(String str)
{
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (size_t i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (c == ' ')
        {
            encodedString += '+';
        }
        else if (isalnum(c))
        {
            encodedString += c;
        }
        else
        {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9)
            {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9)
            {
                code0 = c - 10 + 'A';
            }
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
        }
        yield();
    }
    return encodedString;
}

NautaManager::NautaManager(String sessionFile, String configFile)
{
    _sessionFile = sessionFile;
    _configFile = configFile;

    // _wifiClient = std::make_shared<BearSSL::WiFiClientSecure>();
    // _httpClient = std::make_shared<HTTPClient>();
    // _wifiClient->setInsecure();
}

NautaManager::~NautaManager()
{
}

void NautaManager::SetMessageCallback(std::function<void(String)> messageCallback)
{
    msgCallback = messageCallback;
    msgCallbackAssigned = true;
}
void NautaManager::SendMessage(String message)
{
    if (msgCallbackAssigned)
    {
        msgCallback(message);
    }
}

void NautaManager::SaveConfig()
{
    if (LittleFS.begin())
    {
        File configFile = LittleFS.open(this->_configFile, "w");
        if (configFile)
        {
            configFile.write((uint8_t *)&(_config), sizeof(config_struct));
            configFile.close();
        }
    }
    else
    {
        // DEBUG_MSG("failed to mount FS");
    }
}

void NautaManager::SaveSession()
{
    if (LittleFS.begin())
    {
        File sessionFile = LittleFS.open(this->_sessionFile, "w");
        if (sessionFile)
        {
            DebugPrintln("saving...");
            sessionFile.write((uint8_t *)&(_session), sizeof(session_struct));
            sessionFile.close();
            ESP.reset();
        }
    }
    else
    {
        // DEBUG_MSG("failed to mount FS");
    }
}

void NautaManager::ClearSession()
{
    if (LittleFS.begin())
    {
        if (LittleFS.exists(this->_sessionFile))
        {
            LittleFS.remove(this->_sessionFile);
        }
    }
    else
    {
        DebugPrintln("failed to mount FS");
    }
}

bool NautaManager::LoadConfig()
{
    if (LittleFS.begin())
    {
        if (LittleFS.exists(this->_configFile))
        {
            File configFile = LittleFS.open(this->_configFile, "r");
            configFile.read((uint8_t *)&(_config), sizeof(config_struct));
            configFile.close();
        }
        else
        {
            return false;
        }
    }
    else
    {
        DebugPrintln("failed to mount FS");
    }
    return true;
}

bool NautaManager::LoadSession()
{
    if (LittleFS.begin())
    {
        if (LittleFS.exists(this->_sessionFile))
        {
            File sessionFile = LittleFS.open(this->_sessionFile, "r");
            sessionFile.read((uint8_t *)&(_session), sizeof(session_struct));
            sessionFile.close();
        }
        else
        {
            return false;
        }
    }
    else
    {
        DebugPrintln("failed to mount FS");
    }
    return true;
}

void NautaManager::GetCookie(String cookie)
{
    int idx = cookie.indexOf(';');
    if (idx > 0)
    {
        idx = cookie.length();
    }
    auto subValue = cookie.substring(0, idx);
    idx = subValue.indexOf('=');
    if (idx <= 0)
    {
        DebugPrintln("[ERROR]... invalid cookie => " + cookie);
        return;
    }
    auto cookieName = cookie.substring(0, idx);
    auto idxC = cookie.indexOf(';');
    if (idxC < 0)
    {
        idxC = cookie.length();
    }
    auto cookieValue = cookie.substring(idx + 1, idxC);
    // DebugPrintln(cookieName + " => " + cookieValue);
    if (cookieName.equals("JSESSIONID"))
    {
        DebugPrintln(cookieName + " => " + cookieValue);
        strcpy(_session.JSESSIONID, cookieValue.c_str());
    }
}

void NautaManager::GetCookies(std::shared_ptr<HTTPClient> httpClient)
{
    auto rawCookies = httpClient->header("Set-Cookie");
    DebugPrintln(rawCookies);
    int idx = 0;
    String cookie;
    while (rawCookies.length() > 0)
    {
        idx = rawCookies.indexOf(',');
        if (idx > 0)
        {
            cookie = rawCookies.substring(0, idx);
            rawCookies = rawCookies.substring(idx + 1);
        }
        else
        {
            cookie = rawCookies;
            rawCookies = "";
        }
        GetCookie(cookie);
    }
}

String NautaManager::GetValueFromLine(String line)
{
    String result = "";

    auto vI = line.indexOf("value");
    if (vI == -1)
    {
        return result;
    }
    auto vS = line.indexOf('"', vI);
    auto vE = line.indexOf('"', vS + 1);
    if ((vS != -1) && (vE != -1))
    {
        result = line.substring(vS + 1, vE);
    }
    // DebugPrintln("value: " + result);
    return result;
}

void NautaManager::GetSessionData(std::shared_ptr<HTTPClient> httpClient)
{
    auto stream = httpClient->getStreamPtr();
    while (stream->available())
    {
        auto line = stream->readStringUntil('\n');
        line.replace("'", "\"");
        if (line.indexOf("name=\"wlanuserip\"") != -1)
        {
            strcpy(_session.wlanuserip, GetValueFromLine(line).c_str());
        }
        if (line.indexOf("name=\"loggerId\"") != -1)
        {
            strcpy(_session.loggerId, GetValueFromLine(line).c_str());
        }
        if (line.indexOf("name=\"CSRFHW\"") != -1)
        {
            strcpy(_session.CSRFHW, GetValueFromLine(line).c_str());
        }
    }
}

bool NautaManager::InitSession()
{
    auto wifiClient = std::make_shared<BearSSL::WiFiClientSecure>();
    auto httpClient = std::make_shared<HTTPClient>();
    wifiClient->setInsecure();

    if (httpClient->begin(*wifiClient, "https://secure.etecsa.net:8443/"))
    {
        httpClient->collectHeaders(headerKeys, numberOfHeaders);
        httpClient->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

        DebugPrintln("[HTTP] GET...");
        int httpCode = httpClient->GET();

        if (httpCode > 0)
        {
            GetCookies(httpClient);
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                // auto content = _httpClient->getString();
                // GetSessionData(&content);
                GetSessionData(httpClient);
            }
        }
        else
        {
            DebugPrintln(String("[HTTP] GET... failed, error: ") + httpClient->errorToString(httpCode));
            return false;
        }

        httpClient->end();
    }
    else
    {
        DebugPrintln("[HTTP} Unable to connect\n");
        return false;
    }
    return true;
}

String NautaManager::GetLoginPayload()
{
    return "username=" + urlencode(_config.USERNAME) + "&password=" + urlencode(_config.PASSWORD) + "&CSRFHW=" + urlencode(_session.CSRFHW) +
           "&wlanuserip=" + urlencode(_session.wlanuserip) + "&loggerId=" + urlencode(_session.loggerId) +
           "&wlanacname=" + "&wlanmac=" + "&ssid=nauta_hogar" + "&usertype=";
}
String NautaManager::GetLogoutPayload()
{
    return "username=" + urlencode(_config.USERNAME) + "&CSRFHW=" + urlencode(_session.CSRFHW) + "&wlanuserip=" + urlencode(_session.wlanuserip) +
           "&ATTRIBUTE_UUID=" + urlencode(_session.ATTRIBUTE_UUID) + "&loggerId=" + urlencode(_session.loggerId) + "+" + urlencode(_config.USERNAME) +
           "&ssid=nauta_hogar" + "&remove=1";
}
String NautaManager::GetTimePayload()
{
    return "op=getLeftTime&ATTRIBUTE_UUID=" + urlencode(_session.ATTRIBUTE_UUID) + "&CSRFHW=" + urlencode(_session.CSRFHW) +
           "&wlanuserip=" + urlencode(_session.wlanuserip) + "&ssid=nauta_hogar" + "&loggerId=" + urlencode(_session.loggerId) + "+" + urlencode(_config.USERNAME) +
           "&username=" + urlencode(_config.USERNAME);
}

void DebugHeaders(std::shared_ptr<HTTPClient> client)
{
    for (int i = 0; i < client->headers(); i++)
    {
        DebugPrintln(client->headerName(i) + " : " + client->header(i));
    }
}

bool NautaManager::GetATTRIBUTE_UUID()
{
    auto pos = -1;
    if ((pos = _cachedResponse.indexOf("ATTRIBUTE_UUID=")) >= 0)
    {
        auto ampPos = _cachedResponse.indexOf('&', pos + 15);
        if (ampPos >= pos)
        {
            auto value = _cachedResponse.substring(pos + 15, ampPos);
            value.trim();
            DebugPrintln("ATTR value: " + value);
            DebugPrintln("ATTR value size:" + String(value.length()));
            memccpy(_session.ATTRIBUTE_UUID, value.c_str(), 0, 32);
            // myStrCpy(_session.ATTRIBUTE_UUID, value.c_str(), value.length());
            DebugPrintln(_session.ATTRIBUTE_UUID);
            return true;
        }
    }
    return false;
}

void NautaManager::SaveResponseToCache(std::shared_ptr<HTTPClient> httpClient)
{
    _cachedResponse.clear();
    auto stream = httpClient->getStreamPtr();
    while (stream->available())
    {
        auto line = stream->readStringUntil('\n');
        _cachedResponse += line;
        line.replace("\n", "");
        line.replace("\r", "");
    }
}

String NautaManager::GetLoginError()
{
    auto pos = -1;
    if ((pos = _cachedResponse.indexOf("alert(\"")) >= 0)
    {
        return _cachedResponse.substring(pos + 7, _cachedResponse.indexOf("\"", pos + 7));
    }
    return String();
}

bool NautaManager::Login()
{
    auto wifiClient = std::make_shared<BearSSL::WiFiClientSecure>();
    auto httpClient = std::make_shared<HTTPClient>();
    wifiClient->setInsecure();

    if (!InitSession())
    {
        return false;
    }
    auto loginPayload = GetLoginPayload();
    String location;
    DebugPrintln("loginPayload: " + loginPayload);
    if (httpClient->begin(*wifiClient, "https://secure.etecsa.net:8443/LoginServlet"))
    {
        httpClient->addHeader("Content-Type", "application/x-www-form-urlencoded");
        httpClient->addHeader("Cookie", "JSESSIONID=" + String(_session.JSESSIONID));
        httpClient->collectHeaders(headerKeys, numberOfHeaders);
        httpClient->setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);

        auto httpCode = httpClient->POST(loginPayload);
        DebugPrintln("Login httpCode: " + String(httpCode));
        DebugPrintln("Login location:" + httpClient->getLocation());
        DebugPrintln("Login size: " + String(httpClient->getSize()));
        if (httpCode > 0)
        {
            GetCookies(httpClient);
            SaveResponseToCache(httpClient);
            if (httpCode == HTTP_CODE_OK)
            {
                auto loginError = GetLoginError();
                Serial.println("LOGIN ERROR: " + loginError);
                SendMessage(loginError);
                return false;
            }
            else if (httpCode != HTTP_CODE_FOUND)
            {
                return false;
            }
            location = httpClient->getLocation();
        }
        else
        {
            DebugPrintln(String("[HTTP] POST... failed, error: ") + httpClient->errorToString(httpCode));
            SendMessage("ERROR: no se puede contactar el portal de autenticación de ETECSA");
            return false;
        }
        httpClient->end();
        DebugPrintln("Next step: " + location);
        if (httpClient->begin(*wifiClient, location))
        {
            httpClient->addHeader("Cookie", "JSESSIONID=" + String(_session.JSESSIONID));
            httpClient->setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);

            auto httpCode = httpClient->GET();
            SaveResponseToCache(httpClient);

            DebugPrintln("Login httpCode: " + String(httpCode));
            DebugPrintln("Login location:" + httpClient->getLocation());
            DebugPrintln("Login size: " + String(httpClient->getSize()));

            if (httpCode == HTTP_CODE_OK)
            {
                if (GetATTRIBUTE_UUID())
                {
                    SaveSession();
                    return true;
                }
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

bool NautaManager::Logout()
{
    auto wifiClient = std::make_shared<BearSSL::WiFiClientSecure>();
    auto httpClient = std::make_shared<HTTPClient>();
    wifiClient->setInsecure();

    auto logoutPayload = GetLogoutPayload();

    DebugPrintln("logoutPayload: " + logoutPayload);
    if (httpClient->begin(*wifiClient, "https://secure.etecsa.net:8443/LogoutServlet"))
    {
        httpClient->addHeader("Content-Type", "application/x-www-form-urlencoded");
        httpClient->addHeader("Cookie", "JSESSIONID=" + String(_session.JSESSIONID));

        auto httpCode = httpClient->POST(logoutPayload);
        DebugPrintln("Logout httpCode: " + String(httpCode));
        if (httpCode > 0)
        {
            SaveResponseToCache(httpClient);
            Serial.println(_cachedResponse);
            ClearSession();
            ESP.reset();
            return true;
        }
        else
        {
            SendMessage("ERROR: no se puede contactar el portal de autenticación de ETECSA");
        }
    }
    return false;
}

String NautaManager::GetRemainingTime()
{
    auto wifiClient = std::make_shared<BearSSL::WiFiClientSecure>();
    auto httpClient = std::make_shared<HTTPClient>();
    wifiClient->setInsecure();

    auto timePayload = GetTimePayload();

    DebugPrintln("timePayload: " + timePayload);
    if (httpClient->begin(*wifiClient, "https://secure.etecsa.net:8443/EtecsaQueryServlet"))
    {
        httpClient->addHeader("Content-Type", "application/x-www-form-urlencoded");
        httpClient->addHeader("Cookie", "JSESSIONID=" + String(_session.JSESSIONID));

        auto httpCode = httpClient->POST(timePayload);
        DebugPrintln("EtecsaQueryServlet httpCode: " + String(httpCode));
        if (httpCode > 0)
        {
            SaveResponseToCache(httpClient);
            Serial.println(_cachedResponse);
            return _cachedResponse;
        }
        else
        {
            SendMessage("ERROR: no se puede contactar el portal de autenticación de ETECSA");
        }
    }
    return String("CONNECTION ERROR");
}

bool NautaManager::Logout(String username, String csrfhw, String attr, String wlanuserip, String sessionid)
{
    strcpy(_session.JSESSIONID, sessionid.c_str());
    strcpy(_config.USERNAME, username.c_str());
    strcpy(_session.ATTRIBUTE_UUID, attr.c_str());
    strcpy(_session.CSRFHW, csrfhw.c_str());
    strcpy(_session.wlanuserip, wlanuserip.c_str());
    return Logout();
}

void NautaManager::setUsername(String username)
{
    strcpy(_config.USERNAME, username.c_str());
}

String NautaManager::getUsername()
{
    return _config.USERNAME;
}

void NautaManager::setPassword(String password)
{
    strcpy(_config.PASSWORD, password.c_str());
}

String NautaManager::getPassword()
{
    return _config.PASSWORD;
}

void NautaManager::testRedirect()
{
    // _httpClient->setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
    // if (_httpClient->begin("http://google.com"))
    // {
    //     auto code = _httpClient->GET();
    //     Serial.println("http code: " + String(code));
    //     Serial.println(String("[HTTP] GET... failed, error: ") + _httpClient->errorToString(code));
    //     DebugPrintln(_httpClient->getLocation());
    //     _httpClient->end();
    // }
}