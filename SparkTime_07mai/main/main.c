#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver\gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "main.h"
#include "Neopixel/processusAffichageNeopixel.h"
#include "Neopixel/interfaceGestionNeopixel.h"
#include "GestionWeb/gestionWifi.h"
#include "GestionWeb/processusServeurWeb.h"
#include "GestionWeb/interfaceServeurWeb.h"
#include "GestionWeb/piloteServeurWeb.h"
#include "GestionHeure/processusGestionHeure.h"

static const char *TAG = "Main";

tNeopixelContext np_ctx;
QueueHandle_t fileParamHorloge;
QueueHandle_t fileHeure;
eModeAffichage modeActuel = MODE_ARRET;

void app_main(void)
{
    gpio_reset_pin(ENABLE_SEC);
    //gpio_reset_pin(ENABLE_MIN_HRS);
    gpio_set_direction(ENABLE_SEC, GPIO_MODE_OUTPUT);
    //gpio_set_direction(ENABLE_MIN_HRS, GPIO_MODE_OUTPUT);
    gpio_set_level(ENABLE_SEC, 1);
    //gpio_set_level(ENABLE_MIN_HRS, 1);
        
    initialiserProcessusAffichageNeopixel();
    ESP_LOGI(TAG, "Démarrage du Wi-Fi en mode AP");
    demarrerWifiAP();

    // sParametresHorloge
    fileParamHorloge = xQueueCreate(5, sizeof(sParametresHorloge));
    if (fileParamHorloge == NULL)
    {
        ESP_LOGE(TAG, "Erreur de création de la queue fileParamHorloge");
        return;
    }
    ESP_LOGI(TAG, "Queue fileParamHorloge créée avec succès.");

    // fileHeure
    fileHeure = xQueueCreate(1, sizeof(sTemps));
    if (fileHeure == NULL)
    {
        ESP_LOGE(TAG, "Erreur de création de la queue fileHeure");
        return;
    }
    ESP_LOGI(TAG, "Queue fileHeure créée avec succès.");

    ESP_LOGI(TAG, "Création de la tâche Serveur Web");
    xTaskCreatePinnedToCore(task_serveurWeb, "Tache Serveur Web", 4096, NULL, 5, NULL, 0);

    ESP_LOGI(TAG, "Initialisation des Neopixels");
    np_ctx = neopixel_InitInterface(NP_SEC_COUNT, NP_SEC_GPIO);
    if (!np_ctx)
    {
        ESP_LOGE(TAG, "Erreur d'initialisation des Neopixels");
        return;
    }
    ESP_LOGI(TAG, "Création de la tâche d'affichage des Neopixels");
    xTaskCreatePinnedToCore(task_AffichageNeopixel, "Tache affichage Neopixel", 6144, np_ctx, 7, NULL, 1);

    xTaskCreatePinnedToCore(task_GestionHeure, "TaskGestionHeure", 4096, (void *)fileHeure, 6, NULL, 0);

    ESP_LOGI("Main", "Tâche GestionHeure démarrée sur le Core 0");
}
