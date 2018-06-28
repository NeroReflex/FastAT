#include "FastESP8266.h"

FastESP8266::FastESP8266(HardwareSerial* serial) : FastAT(serial) {
    callbackWiFiConnected = &doNothing;
    callbackWiFiDisconnected = &doNothing;
    callbackWiFiObtainedIP = &doNothing;
}

FastESP8266::~FastESP8266(void) { }

void FastESP8266::parseBufferForCallbacks(void) {
    if (findOnResponse("WIFI CONNECTED"))
        callbackWiFiConnected();
    
    if (findOnResponse("WIFI GOT IP"))
        callbackWiFiObtainedIP();
    
    if (findOnResponse("WIFI DISCONNECT"))
        callbackWiFiDisconnected();
        
}

void FastESP8266::setWiFiConnectedCallback(void (*callback)(void)) {
    callbackWiFiConnected = callback;
}

void FastESP8266::setWiFiDisconnectedCallback(void (*callback)(void)) {
    callbackWiFiDisconnected = callback;
}

void FastESP8266::setWiFiObtainedIPCallback(void (*callback)(void)) {
    callbackWiFiObtainedIP = callback;
}

bool FastESP8266::test(void) {
    return sendCommandAndWaitForResponse( "AT" ) == ESP_OK;
}

bool FastESP8266::reset(void) {
    return sendCommandAndWaitForResponse( "AT+RST" ) == ESP_OK;
}

bool FastESP8266::echoOff(void) {
    return sendCommandAndWaitForResponse( "ATE0" ) == ESP_OK;
}

bool FastESP8266::echoOn(void) {
    return sendCommandAndWaitForResponse( "ATE1" ) == ESP_OK;
}

bool FastESP8266::getVersionAT(char* version_str) {
    uint8_t response = sendCommandAndWaitForResponse( "AT+GMR" );
    
    uint16_t length = 0;
    if (getBetweenFromResponse(length, version_str, "AT version:", "(")) {
        version_str[length] = 0x00;
    }
    
    return response == ESP_OK;
}

bool FastESP8266::getVersionSDK(char* version_str) { 
    uint8_t response = sendCommandAndWaitForResponse( "AT+GMR" );
    
    uint16_t length = 0;
    if (getBetweenFromResponse(length, version_str, "SDK version:", "(")) {
        version_str[length] = 0x00;
    }
    
    return response == ESP_OK;
}

bool FastESP8266::setModeClient(void) {
    return sendCommandAndWaitForResponse("AT+CWMODE=1") == ESP_OK;
}

bool FastESP8266::connect(const char* ssid, const char* password) {
    char cmdBuffer[92] = "";
    sprintf(cmdBuffer, "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    /*
    ctrcpy(cmdBuffer, "AT+CWJAP=\"");
    strcat(cmdBuffer, ssid);
    strcat(cmdBuffer, "\",\"");
    strcat(cmdBuffer, password);
    strcat(cmdBuffer, "\"");
    */
    
    return (sendCommandAndWaitForResponse( cmdBuffer ) == ESP_OK);
}

bool FastESP8266::connected(void) {
    int16_t rssi;
    
    // The command for getting RSSI and the "No AP" string is the same, so I am calling it now 
    uint8_t response = getRSSI(rssi);
    
    return (findOnResponse("No AP") == NULL) && (response == ESP_OK);
}

bool FastESP8266::ping(const char* domain, bool& timeout, uint16_t& ping) {
    char cmdBuffer[64] = "";
    
    sprintf(cmdBuffer, "AT+PING=\"%s\"", domain);
    /*
    strcpy(cmdBuffer, "AT+PING=\"");
    strcat(cmdBuffer, domain);
    strcat(cmdBuffer, "\"");
    */
    uint8_t response = sendCommandAndWaitForResponse( cmdBuffer );
    
    timeout = true; // default value
    ping = 0xFFFF; // default value
    uint16_t length = 0;
    if ((response == ESP_OK) && (getBetweenFromResponse(length, &cmdBuffer[0], "+", "\n\r"))) {
        cmdBuffer[length] = 0x00; // append string terminator
        timeout = (strstr(cmdBuffer, "timeout") != NULL);
        ping = (!timeout) ? atoi(cmdBuffer) : 0xFFFF;
    }
    
    return response == ESP_OK;
}

bool FastESP8266::getLocalIP(uint8_t* ip_addr) {
    uint8_t response = sendCommandAndWaitForResponse( "AT+CIFSR" );
    
    // 0.0.0.0 obviously not a valid IP
    memset(ip_addr, 0x00, 4 * sizeof(uint8_t));
       
    uint16_t length = 0;
    char ip_str[20] = { 0x00 };
    if (getBetweenFromResponse(length, ip_str, "+CIFSR:STAIP,\"", "\"\r\n")) {
        ip_str[length] = 0x00;
        sscanf(ip_str, "%d.%d.%d.%d", ip_addr, ip_addr + 1, ip_addr + 2, ip_addr + 3);
    }
    
    return response == ESP_OK;
}

bool FastESP8266::getRSSI(int16_t& rssi) {
    uint8_t response = sendCommandAndWaitForResponse( "AT+CWJAP_CUR?" );
    
    // 0 obviously not a valid RSSI
    memset(&rssi, 0x00, sizeof(int16_t));
    
    uint16_t length = 0;
    char rssi_str[4] = { 0x00 };
    if (getBetweenFromResponse(length, rssi_str, ",-", "\n\r")) {
        rssi_str[length] = 0x00;
        sscanf(rssi_str, "%d", &rssi);
        rssi *= -1;
    }
    
    return response == ESP_OK;
}

bool FastESP8266::enableCustomDNS(uint8_t* dns_p, uint8_t* dns_s) {
    char cmdBuffer[92] = "";
    sprintf(cmdBuffer, "AT+CIPDNS_CUR=1,\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"", dns_p[0], dns_p[1], dns_p[2], dns_p[3], dns_s[0], dns_s[1], dns_s[2], dns_s[3]);
    
    //remove unneeded secondary dns
    if ((!dns_s[0])  && (!dns_s[1]) && (!dns_s[2]) && (!dns_s[3]))
        cmdBuffer[strlen(cmdBuffer) - 10] = 0x00;
    
    return sendCommandAndWaitForResponse( cmdBuffer ) == ESP_OK;
}

bool FastESP8266::disableCustomDNS(void) {
    return sendCommandAndWaitForResponse( "AT+CIPDNS_CUR=0" ) == ESP_OK;
}
