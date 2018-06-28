#pragma once

#include <FastAT.h>

//#define DEFAULT_TIMEOUT_TIME 7200

#define ESP_SSL_MODE 0
#define ESP_UDP_MODE 1
#define ESP_TCP_MODE 2

/**
 * Manage an ESP8266 device connected to a serial port.
 */
class FastESP8266 : public FastAT {
public:
    FastESP8266(HardwareSerial* serial);
    virtual ~FastESP8266(void);
    bool reset(void);
    bool test(void);
    bool echoOff(void);
    bool echoOn(void);
    bool getVersionAT(char*);
    bool getVersionSDK(char*);
    
    // Change operational mode
    bool setModeClient(void);
    
    // Connection
    bool connect(const char*, const char*);
    bool connected(void);
    bool getLocalIP(uint8_t* ip_addr);
    bool getRSSI(int16_t& rssi);
    bool enableCustomDNS(uint8_t*, uint8_t*);
    bool disableCustomDNS(void);
    
    // Networking
    bool ping(const char* domain, bool& timeout, uint16_t& ping);
    bool openDataConnection(uint8_t mode, const char* domain, uint16_t port);
    bool writeData(uint8_t*, uint16_t);
    
    // Set Callbacks
    void setWiFiConnectedCallback(void (*)(void));
    void setWiFiDisconnectedCallback(void (*)(void));
    void setWiFiObtainedIPCallback(void (*)(void));
    
protected:
    virtual void parseBufferForCallbacks(void);
    void (*callbackWiFiConnected)(void);
    void (*callbackWiFiDisconnected)(void);
    void (*callbackWiFiObtainedIP)(void);
};
