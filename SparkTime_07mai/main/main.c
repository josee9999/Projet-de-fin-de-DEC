#include <stdio.h>
#include <stdlib.h> // pour malloc et free
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "main.h"
#include "Neopixel/processusAffichageNeopixel.h"
#include "Neopixel/interfaceGestionNeopixel.h"
#include "GestionWeb/gestionWifi.h"
#include "GestionWeb/processusServeurWeb.h"
#include "GestionWeb/interfaceServeurWeb.h"
#include "GestionWeb/piloteServeurWeb.h"
#include "GestionHeure/processusGestionHeure.h"
#include "GestionHeure/processusAPITemps.h"

static const char *TAG = "Main";

// Déclaration des queues globales
QueueHandle_t fileParamHorloge = NULL;
QueueHandle_t fileHeure = NULL;
eModeAffichage modeActuel = MODE_ARRET;

// Structure pour les contextes Neopixel (pointeurs simples)
sNeopixelContexts *npContexts = NULL;

void app_main(void)
{
    // Initialiser NVS en premier
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Attendre un peu avant de continuer
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Initialisation des GPIOs pour les sections de Neopixels
    gpio_reset_pin(ENABLE_MIN_HRS);
    gpio_reset_pin(ENABLE_SEC);
    gpio_set_direction(ENABLE_MIN_HRS, GPIO_MODE_OUTPUT);
    gpio_set_direction(ENABLE_SEC, GPIO_MODE_OUTPUT);
    gpio_set_level(ENABLE_MIN_HRS, 1);
    gpio_set_level(ENABLE_SEC, 1);

    // Initialisation du processus d'affichage Neopixel
    initialiserProcessusAffichageNeopixel();

    ESP_LOGI(TAG, "Démarrage du Wi-Fi en mode AP");
    demarrerWifiAP();

    // Création de la file pour les paramètres de l'horloge
    fileParamHorloge = xQueueCreate(5, sizeof(sParametresHorloge));
    if (fileParamHorloge == NULL)
    {
        ESP_LOGE(TAG, "Erreur de création de la queue fileParamHorloge");
        return;
    }

    // Création de la file pour l'heure
    fileHeure = xQueueCreate(5, sizeof(sTemps));
    if (fileHeure == NULL)
    {
        ESP_LOGE(TAG, "Erreur de création de la queue fileHeure");
        return;
    }

    // Création de la tâche Serveur Web sur le core 0
    ESP_LOGI(TAG, "Création de la tâche Serveur Web");
    xTaskCreatePinnedToCore(task_serveurWeb, "Tache Serveur Web", 4096, NULL, 5, NULL, 0);

    // Création de la tâche API Temps sur le core 0
    ESP_LOGI(TAG, "Création de la tâche API Temps");
    xTaskCreatePinnedToCore(task_APITemps, "Tache API Temps", 8192, NULL, 5, NULL, 0);

    // Allocation mémoire pour les contextes Neopixels (Secondes, Minutes/Heures)
    npContexts = malloc(sizeof(sNeopixelContexts));
    if (npContexts == NULL)
    {
        ESP_LOGE(TAG, "Erreur d'allocation mémoire pour les contextes Neopixel");
        return;
    }

    // Initialisation des contextes Neopixels (pointeurs simples)
    npContexts->npCtxSec = neopixel_InitInterface(NP_SEC_COUNT, NP_SEC_GPIO);
    npContexts->npCtxMinHrs = neopixel_InitInterface(NP_MIN_HRS_COUNT, NP_MIN_HRS_GPIO);

    if (npContexts->npCtxSec == NULL || npContexts->npCtxMinHrs == NULL)
    {
        ESP_LOGE(TAG, "Erreur d'initialisation des Neopixels");
        free(npContexts);
        npContexts = NULL;
        return;
    }

    ESP_LOGI(TAG, "Neopixels initialisés avec succès");

    // Création de la tâche d'affichage des Neopixels sur le core 1
    ESP_LOGI(TAG, "Création de la tâche d'affichage des Neopixels");
    xTaskCreatePinnedToCore(task_AffichageNeopixel, "Tache affichage Neopixel", 6144, npContexts, 4, NULL, 1);

    // Création de la tâche de gestion de l'heure sur le core 0
    ESP_LOGI(TAG, "Création de la tâche de gestion de l'heure");
    xTaskCreatePinnedToCore(task_GestionHeure, "TaskGestionHeure", 4096, (void *)fileHeure, 6, NULL, 0);
}