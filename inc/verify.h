#ifndef VERIFY_H
#define VERIFY_H

#include <stdint.h>

uint8_t ProgramIsValid(void);
uint8_t CRC_Check_OK(uint32_t receivedCRC);

#endif