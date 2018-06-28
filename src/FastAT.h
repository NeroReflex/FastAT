#pragma once

#include <Arduino.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Connection.h"

#define ESP_MISSING         0
#define ESP_OK              1
#define ESP_FAIL            2
#define ESP_ERROR           3

/* The size used to hold response for AT commands.
 * 
 * PLEASE: note that if 64 is used CIFSR:STAIP gets removed from response,
 * so a minimum value of 128 should be considered.
 */
#define AT_RECV_BUFFER_SIZE (uint8_t)252

class FastAT {
public:
    FastAT(HardwareSerial* serial);
    virtual ~FastAT(void);

protected:
    uint8_t sendCommandAndWaitForResponse(const char*);
    void sendCommand(const char*);
    uint8_t waitForResponse(void);
    uint8_t findResponseOnBuffer(void);
    uint8_t fillBufferWithResponse(void);
    const char* findOnResponse(const char*);
    bool getBetweenFromResponse(uint16_t&, char*, const char*, const char*, uint8_t skip = 0);
    virtual void parseBufferForCallbacks(void);
    
private:
    HardwareSerial* serial = (HardwareSerial*)NULL;
    char buffer[AT_RECV_BUFFER_SIZE];
};

// The default callback action...
void doNothing(void);
