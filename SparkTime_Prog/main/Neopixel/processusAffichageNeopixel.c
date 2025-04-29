/*
    Nom: 

    Description: 
                 
    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard   
*/

#include "processusAffichageNeopixel.h"
#include "interfaceGestionNeopixel.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

//static tNeopixelContext np_ctx;

void task_AffichageNeopixel(void *pvParameter)
{
    tNeopixelContext np_ctx = (tNeopixelContext)pvParameter;
    tNeopixel pixel[NP_SEC_COUNT];
    int offset = 0;
    int couleurActuelle = 0;

    for(int i = 0; i < NP_SEC_COUNT;i++)
    {
        pixel[i].index = i;
        pixel[i].rgb=COULEUR_ETEINTE;
    };

    neopixel_setPixelInterface(np_ctx, pixel,NP_SEC_COUNT);
    
    while(1) //POUR TEST: while qqchose en realite??
    {
        for(int i = 0; i<NP_SEC_COUNT;i++)
        {
            pixel[i].index = i;
            couleurActuelle = (offset + i) % countCouleurPixel;
            pixel[i].rgb=couleurPixel[couleurActuelle];
        }
        neopixel_setPixelInterface(np_ctx, pixel,NP_SEC_COUNT);

        offset++;
        if (offset>=couleurActuelle)
        {
            couleurActuelle=0;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}