#include <string.h>

#include "FastAT.h"

#define MAX_WAIT_FOR_RESPONSE_IT 0xFFFFFF

#define DEBUG 1

void doNothing(void) {
    // LOL I like this :)
    asm volatile ("nop");
}

const char OK[] = "OK\r\n\0";
const char FAIL[] = "FAIL\r\n\0";
const char ERROR[] = "ERROR\r\n\0";
const char ENTER_DATA[] = ">";
const char DATA_SENT[] = "SEND OK\r\n";

FastAT::FastAT(HardwareSerial* serial) : serial(serial) {

}

FastAT::~FastAT(void) {

}

void FastAT::parseBufferForCallbacks(void) {
    return;
}

const char* FastAT::findOnResponse(const char* needle) {
    return strstr((const char*)buffer, needle);
}

bool FastAT::getBetweenFromResponse(uint16_t& length, char* bufferToFill, const char* beginDelim, const char* endingDelim, uint8_t skip) {
    skip++;
    
    uint16_t beginLength = strlen(beginDelim);
    const char* begin = buffer;
    while (skip) {
        const char* match = strstr(begin, beginDelim);
        begin = (match != NULL) ? match + beginLength : NULL;
        skip--;
    }
    
    if (begin == NULL)
        return false;
    
    const char* end = strstr(begin, endingDelim);
    if (end == NULL)
        return false;
    
    length = 0;
    while (begin < end) {
        *(bufferToFill + length) = *(begin++);
        ++length;
    }
    
    return length > 0;
}

uint8_t FastAT::findResponseOnBuffer(void) {
    if (findOnResponse(DATA_SENT) != (char*)NULL)
        return ESP_DATA_SENT;
    else if (findOnResponse(OK) != (char*)NULL)
        return ESP_OK;
    else if (findOnResponse(FAIL) != (char*)NULL)
        return ESP_FAIL;
    else if (findOnResponse(ERROR) != (char*)NULL)
        return ESP_ERROR;
    else if (findOnResponse(ENTER_DATA) != (char*)NULL)
        return ESP_ENTER_DATA;
    
    return ESP_MISSING;
}

uint8_t FastAT::fillBufferWithResponse(void) {
    uint8_t length = strlen(buffer);
    char* tmp = buffer + length;
    
    if (length == (AT_RECV_BUFFER_SIZE - 1)) {
        // Free space in buffer
        for (uint8_t j = 0; j < (AT_RECV_BUFFER_SIZE / 2); j++)
            buffer[j] = buffer[(AT_RECV_BUFFER_SIZE / 2) + j];
        // Put a string terminator to make the rest of the buffer free
        buffer[(AT_RECV_BUFFER_SIZE / 2)] = 0x00;
        // Re-evaluate the current buffer position
        tmp = buffer + (AT_RECV_BUFFER_SIZE / 2);
    }
    
    while (( uint8_t(tmp - buffer) < (AT_RECV_BUFFER_SIZE - 1)) && (serial->available())) {
        *(tmp++) = (char)serial->read();
    }
    *tmp = 0x00;
    
    return (uint8_t)(tmp - buffer);
}

uint8_t FastAT::waitForResponse(bool waiting_for_enter_data) {
    //clean the buffer from previous messages
    *buffer = 0x00;
    
    uint32_t it = 0;
    uint8_t response = 0x00;
    
    if (!waiting_for_enter_data) {
        do {
            fillBufferWithResponse();
        } while ((!(response = findResponseOnBuffer())) && ((++it) < MAX_WAIT_FOR_RESPONSE_IT));
    } else {
        do {
            fillBufferWithResponse();
        } while ((!findOnResponse(">")) && ((++it) < MAX_WAIT_FOR_RESPONSE_IT));
    }

#if defined( DEBUG ) && (DEBUG == 1)
    Serial.println(buffer);
#endif
    
    // parse the obtained result buffer for something device-dependant
    parseBufferForCallbacks();
    
    return response;
}

void FastAT::flush(void) {
    // parse messages arrived when I was not listening
    while (serial->available()) {
        fillBufferWithResponse();
        parseBufferForCallbacks();
    }
    
    // Flush serial for a clean response to the following command
    serial->flush();
    
    //flush the response buffer
    *buffer = 0x00;
}

void FastAT::sendCommand(const char* cmdBuffer) {
    flush();
    
#if defined( DEBUG ) && (DEBUG == 1)
    Serial.print("$: ");
#endif
    
    // Send the first part of the command
    rawSend((uint8_t*)cmdBuffer, strlen(cmdBuffer));
    
    //this is like pressing "Enter"
    rawSend("\r\n", 2);
} 

uint8_t FastAT::sendCommandAndWaitForResponse(const char* cmd) {
    //send command...
    sendCommand( cmd );
    
    //... and wait for the response
    return waitForResponse();
}

void FastAT::rawSend(const uint8_t* data, uint8_t length) {
    while (length--) {
        char current_byte = (char)(*(data++));
        
        serial->write(current_byte);
#if defined( DEBUG ) && (DEBUG == 1)
        Serial.print(current_byte);
#endif
    }
}
