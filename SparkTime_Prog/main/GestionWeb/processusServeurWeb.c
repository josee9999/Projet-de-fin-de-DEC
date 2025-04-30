/*
    Nom:

    Description:

    Modification: -
    Création: 29/04/2025
    Auteur: Josée Girard
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "processusServeurWeb.h"
#include "piloteServeurWeb.h"
#include "gestionWifi.h"

/*void task_serveurWeb(void *pvParameter)
{
    initWifiAP();

    demarrerServeurWeb();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}*/