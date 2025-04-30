#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "Neopixel/processusAffichageNeopixel.h"
#include "Neopixel/interfaceGestionNeopixel.h"
#include "GestionWeb/gestionWifi.h"
#include "GestionWeb/processusServeurWeb.h"
#include "GestionWeb/interfaceServeurWeb.h"


tNeopixelContext np_ctx;

void app_main(void)
{
    //test
    demarrer_wifi_ap();
    demarrer_serveur_web();
    //fin test

    np_ctx = neopixel_InitInterface(NP_SEC_COUNT,NP_SEC_GPIO);
    if(!np_ctx)
    {
        printf("erreur init neopixel\n");
        return;
    }
    xTaskCreatePinnedToCore(task_AffichageNeopixel, "Tache affichage Neopixel",2048,np_ctx,5, NULL,1);
   // xTaskCreatePinnedToCore(task_serveurWeb,"Tache Serveur Web",4096,NULL,5,NULL,0);

}