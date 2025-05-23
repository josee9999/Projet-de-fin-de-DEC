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

    // Envoyer la nouvelle heure
    if (xQueueSend(fileHeure, &heureActuelle, 0) == pdPASS)
    {
        ESP_LOGI(TAG, "Heure actuelle: %02d:%02d:%02d",
                 heureActuelle.heures, heureActuelle.minutes, heureActuelle.secondes);
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
    sTemps temp;

    // Créer le timer une seule fois
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    
    // Attendre un peu pour s'assurer que le système est bien initialisé
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Attendre de recevoir l'heure initiale du site web
    ESP_LOGI(TAG, "En attente de l'heure initiale du site web...");
    bool heureInitialisee = false;
    
    while (!heureInitialisee) {
        if (xQueueReceive(fileHeure, &temp, pdMS_TO_TICKS(100)) == pdPASS) {
            if (temp.estVillePrincipale) {
                heureActuelle = temp;
                heureInitialisee = true;
                ESP_LOGI(TAG, "Heure initiale reçue du site web: %02d:%02d:%02d",
                         heureActuelle.heures, heureActuelle.minutes, heureActuelle.secondes);
            } else {
                // Si ce n'est pas la ville principale, la remettre dans la file
                if (xQueueSend(fileHeure, &temp, 0) != pdPASS) {
                    ESP_LOGW(TAG, "Impossible de remettre l'heure de la ville secondaire dans la file");
                }
            }
        }
    }

    // Démarrer le timer une fois l'heure initialisée
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, 1000000)); // 1 seconde

    while (1)
    {
        // Vérifier s'il y a une mise à jour de l'heure dans la queue
        sTemps nouvelleHeure;
        if (xQueueReceive(fileHeure, &nouvelleHeure, 0) == pdPASS)
        {
            if (nouvelleHeure.estVillePrincipale) {
                // Vérifier si c'est une heure différente
                if (nouvelleHeure.heures != heureActuelle.heures ||
                    nouvelleHeure.minutes != heureActuelle.minutes ||
                    nouvelleHeure.secondes != heureActuelle.secondes)
                {
                    ESP_LOGI(TAG, "Nouvelle heure reçue du site web: %02d:%02d:%02d",
                             nouvelleHeure.heures, nouvelleHeure.minutes, nouvelleHeure.secondes);

                    // Mettre à jour l'heure actuelle
                    heureActuelle = nouvelleHeure;
                }
            }
            
            // Toujours renvoyer l'heure dans la file (qu'elle soit principale ou secondaire)
            if (xQueueSend(fileHeure, &nouvelleHeure, 0) != pdPASS)
            {
                ESP_LOGW(TAG, "Impossible de remettre l'heure dans la file");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
