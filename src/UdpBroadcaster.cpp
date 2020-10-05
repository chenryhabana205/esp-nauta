#include "ESP8266WiFi.h"

#include "UdpBroadcaster.h"

UdpBroadcaster::UdpBroadcaster(/* args */)
{
    udp = std::make_shared<WiFiUDP>(WiFiUDP());
}

UdpBroadcaster::~UdpBroadcaster()
{
}

void UdpBroadcaster::BroadcastMessage(MESSAGE_TYPES messageType, String message)
{
    auto broadcastAddress = WiFi.localIP();
    broadcastAddress[3] = 255;
    Serial.println("Sending packet to " + broadcastAddress.toString());
    udp->beginPacket(broadcastAddress, 6400);
    udp->write((short)messageType);
    udp->print(message);
    udp->endPacket();
}
