// #include "SedProt.h"

#ifndef SED_PROTOCOL_H
#define SED_PROTOCOL_H

void sedProtSlaveInit();

void sedProtSlaveWrite(unsigned char addr, unsigned char data);
unsigned char sedProtSlaveRead(unsigned char addr);

unsigned char sedProtSlaveReadNoD0(unsigned char addr);

#endif
