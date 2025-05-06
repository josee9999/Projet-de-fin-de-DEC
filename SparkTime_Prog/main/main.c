#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "Neopixel/processusAffichageNeopixel.h"
#include "Neopixel/interfaceGestionNeopixel.h"
#include "GestionWeb/gestionWifi.h"
#include "GestionWeb/processusServeurWeb.h"
#include "GestionWeb/interfaceServeurWeb.h"
#include "GestionWeb/piloteServeurWeb.h"
#include "esp_log.h"
#include "esp_wifi.h"

static const char *TAG = "Main";

tNeopixelContext np_ctx;
QueueHandle_t fileMode;
eModeAffichage modeActuel = MODE_ARRET;

void app_main(void)
{
    ESP_LOGI(TAG, "Démarrage du Wi-Fi en mode AP");
    demarrerWifiAP();
    
    fileMode = xQueueCreate(5, sizeof(eModeAffichage));
    if (fileMode == NULL) {
        ESP_LOGE(TAG, "Erreur de création de la queue fileMode");
        return;
    }
    ESP_LOGI(TAG, "Queue fileMode créée avec succès.");

    ESP_LOGI(TAG, "Création de la tâche Serveur Web");
    xTaskCreatePinnedToCore(task_serveurWeb, "Tache Serveur Web",4096,NULL,5,NULL,0);
    
    ESP_LOGI(TAG, "Initialisation des Neopixels");
    np_ctx = neopixel_InitInterface(NP_SEC_COUNT,NP_SEC_GPIO);
    if(!np_ctx)
    {
        ESP_LOGE(TAG, "Erreur d'initialisation des Neopixels");
        return;
    }
    ESP_LOGI(TAG, "Création de la tâche d'affichage des Neopixels");
    xTaskCreatePinnedToCore(task_AffichageNeopixel, "Tache affichage Neopixel",6144,np_ctx,5, NULL,1);
}