#ifndef _MULTIPLEXER_H
#define _MULTIPLEXER_H

#include <Arduino.h>
#include <Wire.h>

#define MULTIPLEXER_ADDRESS     0x70


static uint8_t currentChannel = 0;

static inline bool multiplexerSetChannel(uint8_t channel)
{
    if (channel > 7) return false;
    currentChannel = channel;
    Wire.beginTransmission(MULTIPLEXER_ADDRESS);
    Wire.write(1 << channel);
    Wire.endTransmission();
    return true;
}

static inline uint8_t multiplexerChannel()
{
    return currentChannel;
}

#endif
