#ifndef IHEADER_H
#define IHEADER_H

#include <stdint.h>

typedef struct
{
    uint32_t magic;
    uint32_t image_size;
    uint32_t version;
}ImageHeader_t;


uint8_t HeaderReceived(uint8_t *data, uint16_t length);

const ImageHeader_t *HeaderGet(void);

#endif  