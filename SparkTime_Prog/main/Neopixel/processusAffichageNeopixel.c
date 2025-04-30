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
#include "esp_log.h"

extern QueueHandle_t fileMode;

// eModeAffichage modeActuel = MODE_ARCENCIEL;

void task_AffichageNeopixel(void *pvParameter)
{
    ESP_LOGI("Neopixel", "Tâche Neopixel démarrée");
    tNeopixelContext np_ctx = (tNeopixelContext)pvParameter;
    tNeopixel pixel[NP_SEC_COUNT];
    int offset = 0;
    // int couleurActuelle = 0;
    int indexCouleur = 0;

    eModeAffichage mode = MODE_ARRET;

    for (int i = 0; i < NP_SEC_COUNT; i++)
    {
        pixel[i].index = i;
        pixel[i].rgb = COULEUR_ETEINTE;
    };

    neopixel_setPixelInterface(np_ctx, pixel, NP_SEC_COUNT);

    while (1)
    {
        eModeAffichage nouveauMode;
        if (xQueueReceive(fileMode, &nouveauMode, 0) == pdPASS)
        {
            mode = nouveauMode;
        }

        switch (mode)
        {
        case MODE_ARRET:
            for (int i = 0; i < NP_SEC_COUNT; i++)
            {
                pixel[i].index = i;
                pixel[i].rgb = COULEUR_ETEINTE;
            };
            neopixel_setPixelInterface(np_ctx, pixel, NP_SEC_COUNT);
            ESP_LOGI("Neopixel", "Mode ARRET");
            vTaskDelay(pdMS_TO_TICKS(500));

            break;
        case MODE_ARCENCIEL:
            for (int i = 0; i < NP_SEC_COUNT; i++)
            {
                indexCouleur = offset + i;
                pixel[i].index = i;
                if (indexCouleur < countCouleurPixel)
                {
                    pixel[i].rgb = couleurPixel[indexCouleur];
                }
                else
                {
                    pixel[i].rgb = COULEUR_ETEINTE;
                }
            }
            neopixel_setPixelInterface(np_ctx, pixel, NP_SEC_COUNT);
            offset++;
            ESP_LOGI("Neopixel", "Mode ARCENCIEL - offset %d", offset);

            offset++;
            if (offset >= countCouleurPixel)
            {
                ESP_LOGI("Neopixel", "Arc-en-ciel terminé, retour au mode ARRET");
                offset = 0;
                mode = MODE_ARRET;
            }

            vTaskDelay(pdMS_TO_TICKS(250));
            break;

        case MODE_TEST: // TODO
            break;

        case MODE_HORLOGE: // TODO
            break;

        case MODE_TEMPERATURE: // TODO
            break;
        }
    }
}