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

static const char *TAG = "GestionHeure";

void task_GestionHeure(void *pvParameters)
{
    QueueHandle_t fileHeure = (QueueHandle_t)pvParameters;
    sTemps heureActuelle = {0};

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        // Vérifier s'il y a une mise à jour de l'heure dans la queue
        sTemps nouvelleHeure;
        if (xQueueReceive(fileHeure, &nouvelleHeure, 0) == pdPASS)
        {
            ESP_LOGI(TAG, "Nouvelle heure reçue: %02d:%02d:%02d",
                     nouvelleHeure.heures, nouvelleHeure.minutes, nouvelleHeure.secondes);
            heureActuelle = nouvelleHeure;
        }

        // Incrémenter les secondes toutes les secondes
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
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

        // Envoyer l'heure mise à jour dans la file
        if (xQueueOverwrite(fileHeure, &heureActuelle) != pdPASS)
        {
            ESP_LOGW(TAG, "Impossible d'envoyer l'heure dans la file.");
        }

        ESP_LOGI(TAG, "Heure actuelle: %02d:%02d:%02d",
                 heureActuelle.heures,
                 heureActuelle.minutes,
                 heureActuelle.secondes);
    }
}
