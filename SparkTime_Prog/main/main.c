#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "neopixel.h"

#define NEOPIXEL_GPIO 25
#define NEOPIXEL_COUNT 2

tNeopixelContext np_ctx;

void app_main(void)
{
    np_ctx = neopixel_Init(NEOPIXEL_COUNT, NEOPIXEL_GPIO) ;

    if(!np_ctx)
    {
        printf("nope\n");
        return;
    };

    tNeopixel pixel[2]= {
    {.index = 0, .rgb = NP_RGB(0,5,0)},
    {.index = 1, .rgb = NP_RGB(5,0,5)}
    };
    neopixel_SetPixel(np_ctx, pixel,2);
}