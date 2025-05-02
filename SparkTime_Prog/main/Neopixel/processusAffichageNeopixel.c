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
    int indexCouleur = 0;
    int ledIndex = 0;

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
            vTaskDelay(pdMS_TO_TICKS(500));

            break;
        case MODE_ARCENCIEL:
        for (int i = NP_SEC_COUNT - 1; i > 0; i--)
        {
            pixel[i].rgb = pixel[i - 1].rgb;
        }
    
        // Ajoute la nouvelle couleur en tête (ou éteint si on est à la fin)
        if (offset < countCouleurPixel)
        {
            pixel[0].rgb = couleurPixel[offset][NIVEAU_PALE];
        }
        else
        {
            pixel[0].rgb = COULEUR_ETEINTE;
        }
    
        // Met à jour les index (nécessaire pour certaines libs Neopixel)
        for (int i = 0; i < NP_SEC_COUNT; i++)
        {
            pixel[i].index = i;
        }
    
        neopixel_setPixelInterface(np_ctx, pixel, NP_SEC_COUNT);
    
        offset++;
    
        if (offset >= countCouleurPixel + NP_SEC_COUNT)
        {
            ESP_LOGI("Neopixel", "Arc-en-ciel terminé, retour au mode ARRET");
            offset = 0;
            mode = MODE_ARRET;
        }
    
        vTaskDelay(pdMS_TO_TICKS(1000));
        break;

        case MODE_TEST:
        uint32_t sequenceTest[] = {
            couleurPixel[COULEUR_ROUGE][NIVEAU_PALE],
            couleurPixel[COULEUR_ROUGE][NIVEAU_MOYEN],
            couleurPixel[COULEUR_ROUGE][NIVEAU_VIF],
    
            couleurPixel[COULEUR_VERT][NIVEAU_PALE],
            couleurPixel[COULEUR_VERT][NIVEAU_MOYEN],
            couleurPixel[COULEUR_VERT][NIVEAU_VIF],
    
            couleurPixel[COULEUR_BLEU][NIVEAU_PALE],
            couleurPixel[COULEUR_BLEU][NIVEAU_MOYEN],
            couleurPixel[COULEUR_BLEU][NIVEAU_VIF],
    
            couleurPixel[COULEUR_BLANC][NIVEAU_PALE],
            couleurPixel[COULEUR_BLANC][NIVEAU_MOYEN],
            couleurPixel[COULEUR_BLANC][NIVEAU_MOYEN], //REMETTRE A VIF QUAND 5V DISPO
    
            COULEUR_ETEINTE};
            
            const int nbÉtapes = sizeof(sequenceTest) / sizeof(sequenceTest[0]);

            for (int i = 0; i < NP_SEC_COUNT; i++)
            {
                pixel[i].index = i;
                pixel[i].rgb = COULEUR_ETEINTE;
            }
            ESP_LOGI("Neopixel", "Début du test : ledIndex = %d, indexCouleur = %d", ledIndex, indexCouleur);
            if (ledIndex < NP_SEC_COUNT)
            {
                if (indexCouleur < 13)
                {
                    pixel[ledIndex].rgb = sequenceTest[indexCouleur];
                    ESP_LOGI("Neopixel", "LED %d, couleur appliquée : 0x%08lX", ledIndex, pixel[ledIndex].rgb);

                    neopixel_setPixelInterface(np_ctx, pixel, NP_SEC_COUNT);
                    indexCouleur++;
                }
                else
                {
                    indexCouleur = 0;
                    ledIndex++;
                    ESP_LOGI("Neopixel", "Passage à la LED suivante : ledIndex = %d", ledIndex);
                }
            }
            else
            {
                ledIndex = 0;
                indexCouleur = 0;
                mode = MODE_ARRET;
                ESP_LOGI("Neopixel", "Test terminé, retour au mode ARRET");
            }
            vTaskDelay(pdMS_TO_TICKS(500));
            break;

        case MODE_HORLOGE: // TODO
            break;

        case MODE_TEMPERATURE: // TODO
            break;
        }
    }
}