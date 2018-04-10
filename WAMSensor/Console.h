#ifndef _CONSOLE_H
#define _CONSOLE_H
#include <stdio.h>
#include <Arduino.h>

#define CONSOLE_BUFFER_SIZE     100

inline void consolePrint(const char* tag, const char* format, ...)
{
    static char sbuf[CONSOLE_BUFFER_SIZE];
    va_list varArgs;

    va_start(varArgs, format);
    vsnprintf(sbuf, sizeof(sbuf), format, varArgs);
    va_end(varArgs);
    Serial.printf("[%s] ", tag);
    Serial.println(sbuf);
}

#endif
