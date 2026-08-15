#include "stm32f4xx.h"

#include "flash_writer.h"
#include "update_process.h"
#include "memory_map.h"
#include "header.h"
#include "jump.h"
#include "verify.h"


typedef enum{
    UPDATE_WAIT_HEADER,
    UPDATE_RECEIVE_PAYLOAD,
    UPDATE_RECEIVE_CRC,
    UPDATE_VERIFY,
    UPDATE_FINISHED,
    UPDATE_ERROR
}UpdateState_t;


static volatile UpdateState_t updateState = UPDATE_WAIT_HEADER;
static uint32_t bytesReceived = 0;
static uint32_t receivedCRC = 0;
static uint32_t headerSize = sizeof(ImageHeader_t);
static uint32_t FlagAddress = UPDATE_FLAG_ADDRESS; 


static uint8_t crcBuffer[4];
static uint8_t crcBytesReceived = 0;


void UpdateProcess(uint8_t *data, uint16_t length)
{

    switch(updateState)
    {

    case UPDATE_WAIT_HEADER:
    if(HeaderReceived(data, length))
    {
        const ImageHeader_t *header = HeaderGet();
        
        if( sizeof(ImageHeader)+header->image_size <= PROGRAM_FLASH_SIZE)
        {
            uint32_t flagValue = UPDATE_FLAG_VALUE;

            FlashEraseProgram();
            
            FlashWriteAtAddress(FlagAddress,(uint8_t*)&flagValue, 4); 
        
            bytesReceived = 0;
            FlashWrite((uint8_t*)header, headerSize);
            
            updateState = UPDATE_RECEIVE_PAYLOAD;
        }
        else
        {
            updateState = UPDATE_ERROR;
        }
    }
    break;


    case UPDATE_RECEIVE_PAYLOAD:
    {
        const ImageHeader_t *header = HeaderGet();

        uint32_t remaining = header->image_size - bytesReceived;

        if(length > remaining)
        {
            length = remaining;
        }

        if(FlashWrite(data, length))
        {
            bytesReceived += length;

            if(bytesReceived >= header->image_size)
            {
                updateState = UPDATE_RECEIVE_CRC;
            }
        }
        else
        {

            updateState = UPDATE_ERROR;
        }

        break;
    }


    case UPDATE_RECEIVE_CRC:
    {
        uint16_t i = 0;

        while (i < length && crcBytesReceived < 4)
        {
            crcBuffer[crcBytesReceived++] = data[i++];
        }

        if (crcBytesReceived == 4)
        {
            receivedCRC = (uint32_t)crcBuffer[0]
                        | ((uint32_t)crcBuffer[1] << 8)
                        | ((uint32_t)crcBuffer[2] << 16)
                        | ((uint32_t)crcBuffer[3] << 24);

            updateState = UPDATE_VERIFY;
        }

        break;
     }



    case UPDATE_VERIFY:

        if(CRC_Check_OK(receivedCRC))
        {
            updateState = UPDATE_FINISHED;
        }
        else
        {

            updateState = UPDATE_ERROR;
        }

        break;


    case UPDATE_FINISHED:
    {
        uint32_t flagValue = UPDATE_VALID_VALUE;

        FlashWriteAtAddress(FlagAddress,(uint8_t*)&flagValue, 4);

        NVIC_SystemReset();
    }
    break;


    case UPDATE_ERROR:

        while(1);

        break;


    default:

        updateState = UPDATE_ERROR;

        break;
    }
}
