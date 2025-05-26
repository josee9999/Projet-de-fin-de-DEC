/*
    Nom: processusGestionHeure

    Description: Gere l'heure local de l'horloge et, lorsque necessaire, l'heure d'un autre endroit.

    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard
*/
#include "processusGestionHeure.h"
#include "Neopixel/processusAffichageNeopixel.h"
#include "GestionWeb/piloteServeurWeb.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "GestionHeure";

static sTemps heureActuelle = {0};

static void timer_callback(void* arg)
{
    QueueHandle_t fileHeure = (QueueHandle_t)arg;
    
    // Incrémenter les secondes
    heureActuelle.secondes++;
    if (heureActuelle.secondes >= 60)
    {
        heureActuelle.secondes = 0;
        heureActuelle.minutes++;
        if (heureActuelle.minutes >= 60)
        {
            heureActuelle.minutes = 0;
            heureActuelle.heures++;
            if (heureActuelle.heures >= 24)
            {
                heureActuelle.heures = 0;
            }
        }
    }

    // Vider la file d'abord
    sTemps temp;
    while (xQueueReceive(fileHeure, &temp, 0) == pdPASS) {
        // Vider la file
    }

    // S'assurer que l'heure est marquée comme ville principale
    heureActuelle.estVillePrincipale = true;

    // Envoyer la nouvelle heure
    BaseType_t resultat = xQueueSend(fileHeure, &heureActuelle, 0);
    if (resultat == pdPASS)
    {
        ESP_LOGI(TAG, "Timer: Heure actuelle: %02d:%02d:%02d",
                 heureActuelle.heures, heureActuelle.minutes, heureActuelle.secondes);
    }
    else
    {
        ESP_LOGE(TAG, "Timer: Erreur d'envoi de l'heure dans la file (erreur: %d)", resultat);
    }
}

void task_GestionHeure(void *pvParameters)
{
    QueueHandle_t fileHeure = (QueueHandle_t)pvParameters;
    esp_timer_handle_t timer = NULL;
    const esp_timer_create_args_t timer_args = {
        .callback = &timer_callback,
        .arg = fileHeure,
        .name = "horloge"
    };

    // Initialiser l'heure par défaut
    heureActuelle.heures = 0;
    heureActuelle.minutes = 0;
    heureActuelle.secondes = 0;
    heureActuelle.estVillePrincipale = true;

    // Créer le timer
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    
    // Attendre un peu pour s'assurer que le système est bien initialisé
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Démarrer le timer immédiatement
    ESP_LOGI(TAG, "Démarrage du timer pour l'horloge");
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, 1000000)); // 1 seconde

    // Envoyer l'heure initiale dans la file
    if (xQueueSend(fileHeure, &heureActuelle, 0) == pdPASS) {
        ESP_LOGI(TAG, "Heure initiale envoyée: %02d:%02d:%02d",
                 heureActuelle.heures, heureActuelle.minutes, heureActuelle.secondes);
    }

    while (1)
    {
        // Vérifier s'il y a une mise à jour de l'heure dans la queue
        sTemps nouvelleHeure;
        if (xQueueReceive(fileHeure, &nouvelleHeure, pdMS_TO_TICKS(10)) == pdPASS)
        {
            // Mettre à jour l'heure actuelle si elle vient du web
            if (!nouvelleHeure.estVillePrincipale)
            {
                ESP_LOGI(TAG, "Nouvelle heure reçue du web: %02d:%02d:%02d",
                         nouvelleHeure.heures, nouvelleHeure.minutes, nouvelleHeure.secondes);

                // Mettre à jour l'heure actuelle
                heureActuelle.heures = nouvelleHeure.heures;
                heureActuelle.minutes = nouvelleHeure.minutes;
                heureActuelle.secondes = nouvelleHeure.secondes;
                heureActuelle.estVillePrincipale = true;

                // Renvoyer immédiatement la nouvelle heure dans la file
                if (xQueueSend(fileHeure, &heureActuelle, 0) != pdPASS) {
                    ESP_LOGW(TAG, "Impossible de renvoyer l'heure mise à jour");
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
