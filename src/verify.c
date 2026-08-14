#include "verify.h"
#include "memory_map.h"
#include "header.h"
#include "crc.h" 


uint8_t ProgramIsValid(void)
{
    uint32_t stack;
    uint32_t reset;


    stack = *(volatile uint32_t *)PROGRAM_ADDRESS;
    reset = *(volatile uint32_t *)(PROGRAM_ADDRESS + 4);


    if(stack == 0xFFFFFFFF || reset == 0xFFFFFFFF)
    {
        return 0;
    }


    if(stack < SRAM_START || stack > SRAM_END)
    {
        return 0;
    }


    if(reset < PROGRAM_ADDRESS ||
       reset > (PROGRAM_ADDRESS + PROGRAM_FLASH_SIZE))
    {
        return 0;
    }


    if((reset & 1U) == 0U)
    {
        return 0;
    }


    return 1;
}

uint8_t CRC_Check_OK(uint32_t receivedCRC)
{
    const ImageHeader_t *header;

    header = HeaderGet();
    

    uint32_t total_size = sizeof(ImageHeader_t) + header->image_size;

    uint32_t calculated = CRC_Calculate((uint8_t*)PROGRAM_ADDRESS, total_size);


    if(calculated == receivedCRC)
    {
        return 1;
    }

    return 0;
}