#include "stm32f4xx.h"
#include "flash_writer.h"
#include "memory_map.h"

static uint32_t address = PROGRAM_ADDRESS;

static void FlashUnlock(void)
{
    if(FLASH->CR & FLASH_CR_LOCK)
    {
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;
    }
}

void FlashLock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}

void FlashInit(void)
{
    FlashUnlock();
    FLASH->ACR |= FLASH_ACR_PRFTEN;
    FLASH->ACR |= FLASH_ACR_ICEN;
    FLASH->ACR |= FLASH_ACR_DCEN;
}

static void FlashWaitBusy(void)
{
    while(FLASH->SR & FLASH_SR_BSY);
}

void FlashEraseProgram(void)
{
    FlashUnlock();
    FlashWaitBusy();
    FLASH->CR &= ~FLASH_CR_SNB;
    FLASH->CR |= FLASH_CR_SER;
    FLASH->CR |= (2 << FLASH_CR_SNB_Pos);
    FLASH->CR |= FLASH_CR_STRT;
    FlashWaitBusy();
    FLASH->CR &= ~FLASH_CR_SER;
    address = PROGRAM_ADDRESS;
}


uint8_t FlashWrite(uint8_t *data, uint16_t length)
{
    FlashUnlock();

    for(uint16_t i = 0; i < length; i += 4)
    {
        uint32_t word = 0xFFFFFFFF;

        if(i < length)
            word &= ~(0xFF << 0), word |= data[i];
        if(i + 1 < length)
            word |= data[i+1] << 8;
        if(i + 2 < length)
            word |= data[i+2] << 16;
        if(i + 3 < length)
            word |= data[i+3] << 24;

        FlashWaitBusy();
        FLASH->CR &= ~FLASH_CR_PSIZE;
        FLASH->CR |= FLASH_CR_PSIZE_1;
        FLASH->CR |= FLASH_CR_PG;

        *(__IO uint32_t*)address = word;

        FlashWaitBusy();

        if(FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | 
                        FLASH_SR_PGPERR | FLASH_SR_PGSERR))
        {
            return 0;
        }

        FLASH->CR &= ~FLASH_CR_PG;
        address += 4;
    }

    FlashLock();
    return 1;
}


uint8_t FlashWriteAtAddress(uint32_t flashAddress, uint8_t *data, uint16_t length)
{
    FlashUnlock();

    for(uint16_t i = 0; i < length; i += 4)
    {
        uint32_t word = 0xFFFFFFFF;

        if(i < length)
            word &= ~(0xFF << 0), word |= data[i];
        if(i + 1 < length)
            word |= data[i+1] << 8;
        if(i + 2 < length)
            word |= data[i+2] << 16;
        if(i + 3 < length)
            word |= data[i+3] << 24;

        FlashWaitBusy();
        FLASH->CR &= ~FLASH_CR_PSIZE;
        FLASH->CR |= FLASH_CR_PSIZE_1;
        FLASH->CR |= FLASH_CR_PG;

        *(__IO uint32_t*)(flashAddress + i) = word;  

        FlashWaitBusy();

        if(FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | 
                        FLASH_SR_PGPERR | FLASH_SR_PGSERR))
        {
            return 0;
        }

        FLASH->CR &= ~FLASH_CR_PG;
    }

    FlashLock();
    return 1;
}