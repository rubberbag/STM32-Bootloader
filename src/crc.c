#include "memory_map.h"
#include "header.h"
#include <stdint.h>


uint32_t CRC_Calculate(uint8_t *data, uint32_t size)
{
    uint32_t crc = 0xFFFFFFFF;

    for(uint32_t i = 0; i < size; i++)
    {
        crc ^= data[i];

        for(int j = 0; j < 8; j++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }

    return crc ^ 0xFFFFFFFF;
}
