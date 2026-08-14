#include "stm32f4xx.h"
#include "jump.h"
#include "button_pressed.h"
#include "usart1.h"
#include "usart1_irq.h"
#include "verify.h"
#include "update_process.h"
#include "systick.h"
#include "flash_writer.h"
#include "memory_map.h"

static uint8_t *data;
static uint16_t length;

typedef enum
{
    BOOT_WAIT,
    BOOT_UPDATE,
    BOOT_JUMP
}BootState_t;

static BootState_t bootState;

#define BOOT_TIMEOUT_MS 10000U
#define UPDATE_TIMEOUT_MS 30000U  // 30 seconds to receive update

void BootShowBanner(void)
{
    USART1_SendString("\033[2J");
    USART1_SendString("\033[H");
    USART1_SendString("BOOTLOADER v0.1\r\n");
    USART1_SendString("ARM Cortex-M4\r\n");
    USART1_SendString("STM32F407VGTX\r\n\r\n");
}

int main(void)
{
    ButtonInit();
    USART1_Init();
    SysTickInit();
    FlashInit();
    BootShowBanner();

    uint32_t startTime = SysTickGetTick();
    uint32_t updateStartTime = 0;
    bootState = BOOT_WAIT;

    while(1)
    {
        switch(bootState)
        {
            case BOOT_WAIT:

                
                uint32_t updateFlag = *(uint32_t*)UPDATE_FLAG_ADDRESS;
                
                if(updateFlag == UPDATE_FLAG_VALUE)
                {
                    USART1_RX_Start();
                    updateStartTime = SysTickGetTick();
                    bootState = BOOT_UPDATE;
                    break;
                }

                if(ButtonPressed())
                {
                    USART1_SendString("UPDATE MODE\r\n");
                    USART1_RX_Start();
                    updateStartTime = SysTickGetTick();  // Start update timer
                    bootState = BOOT_UPDATE;
                    break;
                }

                if((SysTickGetTick() - startTime) >= BOOT_TIMEOUT_MS)
                {
                    USART1_SendString("10 SECONDS PASSED\r\n");
                    
                    if(ProgramIsValid())
                    {
                        USART1_SendString("PROGRAM FOUND - JUMPING\r\n");
                        bootState = BOOT_JUMP;
                    }
                    else
                    {
                        USART1_SendString("NO PROGRAM - WAITING FOR UPDATE\r\n");
                        USART1_RX_Start();
                        updateStartTime = SysTickGetTick();
                        bootState = BOOT_UPDATE;
                    }
                }
                break;

            case BOOT_UPDATE:
                // Check for update timeout
                if((SysTickGetTick() - updateStartTime) >= UPDATE_TIMEOUT_MS)
                {
                    USART1_SendString("UPDATE TIMEOUT\r\n");
                    bootState = BOOT_WAIT;
                    startTime = SysTickGetTick();  // Reset boot timer
                    break;
                }

                if(UART_DataReady())
                {
                    UART_GetReceivedData(&data, &length);
                    UpdateProcess(data, length);
                }
                break;

            case BOOT_JUMP:
                JumpToProgram();
                break;

            default:
                bootState = BOOT_WAIT;
                break;
        }
    }
}