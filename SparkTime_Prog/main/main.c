#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "Neopixel/processusAffichageNeopixel.h"
#include "Neopixel/interfaceGestionNeopixel.h"



tNeopixelContext np_ctx;

void app_main(void)
{
    np_ctx = neopixel_InitInterface(NP_SEC_COUNT,NP_SEC_GPIO);
    if(!np_ctx)
    {
        printf("erreur init neopixel\n");
        return;
    }
    xTaskCreatePinnedToCore(task_AffichageNeopixel, "tache affichage neopixel",2048,np_ctx,5, NULL,1);
    
}