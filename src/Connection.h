#pragma once

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The size used to hold unread received data.
 */
#define AT_RECV_BUFFER_SIZE (uint8_t)64

#define CONNECTION_NONE     0
#define CONNECTION_UDP      1
#define CONNECTION_TCP      2
#define CONNECTION_SSL      3

class Connection {
    Connection(void);
    virtual ~Connection(void);
    virtual void write(uint8_t*, const uint16_t&) const;
    virtual uint16_t read(uint8_t*) const;
    virtual bool connected(void) const;
    virtual uint8_t getType(void) const;
    
private:
    uint8_t* buffer = (uint8_t*)NULL;
};
