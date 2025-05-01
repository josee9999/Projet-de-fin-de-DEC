/*
    Nom:

    Description:

    Modification: -
    Création: 29/04/2025
    Auteur: Josée Girard
*/
#include "processusServeurWeb.h"
#include "interfaceServeurWeb.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "ProcessusServeurWeb";

void task_serveurWeb(void *pvParameters)
{
    ESP_LOGI(TAG, "Tâche serveur web démarrée sur le core 0");
    demarrerServeurWeb(); 

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}