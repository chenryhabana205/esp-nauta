#ifndef UDPBROADCASTER_H
#define UDPBROADCASTER_H

#include <memory>
#include <WiFiUdp.h>

enum MESSAGE_TYPES {
    NAUTA_TIME_REMAINING,
    NAUTA_SHARED_SESSION,
    NAUTA_MESSAGE
};

class UdpBroadcaster
{
private:
    std::shared_ptr<WiFiUDP> udp;
public:
    UdpBroadcaster(/* args */);
    ~UdpBroadcaster();

    void BroadcastMessage(MESSAGE_TYPES messageType, String message);
};


#endif