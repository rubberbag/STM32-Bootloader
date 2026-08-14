#ifndef FLASH_WRITER_H
#define FLASH_WRITER_H

#include "stdint.h"

void FlashInit(void);

void FlashEraseProgram(void);

uint8_t FlashWrite( uint8_t *data, uint16_t length);

uint8_t FlashWriteAtAddress(uint32_t flashAddress, uint8_t *data, uint16_t length);

void FlashLock(void);

#endif