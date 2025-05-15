/*
    Nom:

    Description:

    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard
*/

#include "processusAffichageNeopixel.h"
#include "interfaceGestionNeopixel.h"
#include "GestionHeure/processusGestionHeure.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver\gpio.h"
#include "main.h"

static SemaphoreHandle_t npMutex = NULL;
static const char *TAG = "processusAffichageNeopixel";
extern QueueHandle_t fileParamHorloge;
extern QueueHandle_t fileHeure;

// eModeAffichage modeActuel = MODE_ARCENCIEL;
sParametresHorloge parametresHorloge = {0};

void setParametresHorloge(sParametresHorloge *params)
{
    if (params)
    {
        memcpy(&parametresHorloge, params, sizeof(sParametresHorloge));
    }
}

sParametresHorloge getParametresHorloge(void)
{
    return parametresHorloge;
}

void task_AffichageNeopixel(void *pvParameter)
{
    ESP_LOGI(TAG, "Tâche Neopixel démarrée");
    sNeopixelContexts *npContexts = (sNeopixelContexts *)pvParameter;

    if (npContexts == NULL || npContexts->npCtxSec == NULL || npContexts->npCtxMinHrs == NULL)
    {
        ESP_LOGE(TAG, "Erreur : Contextes Neopixel non initialisés.");
        vTaskDelete(NULL);
        return;
    }
    tNeopixel pixelSec[NP_SEC_COUNT];
    tNeopixel pixelMinHrs[NP_MIN_HRS_COUNT];

    int offsetSec = 0;
    int indexCouleurSec = 0;
    int ledIndexSec = 0;

    int offsetMinHrs = 0;
    int indexCouleurMinHrs = 0;
    int ledIndexMinHrs = 0;

    eModeAffichage mode = MODE_ARRET;

    for (int i = 0; i < NP_SEC_COUNT; i++)
    {
        pixelSec[i].index = i;
        pixelSec[i].rgb = COULEUR_ETEINTE;
    };
    for (int i = 0; i < NP_MIN_HRS_COUNT; i++)
    {
        pixelMinHrs[i].index = i;
        pixelMinHrs[i].rgb = COULEUR_ETEINTE;
    };

    neopixel_setPixelInterface(npContexts->npCtxSec, pixelSec, NP_SEC_COUNT);

    while (1)
    {
        sParametresHorloge nouveauxParametres;
        if (xQueueReceive(fileParamHorloge, &nouveauxParametres, pdMS_TO_TICKS(10)) == pdPASS)
        {
            ESP_LOGI(TAG, "Nouveaux paramètres reçus dans task_AffichageNeopixel");
            setParametresHorloge(&nouveauxParametres); // Met à jour les paramètres globaux
            mode = nouveauxParametres.modeActuel;      // Met à jour le mode actuel
        }

        if (xSemaphoreTake(npMutex, portMAX_DELAY) == pdTRUE)
        {
            switch (mode)
            {
            case MODE_ARRET:
                gpio_set_level(GPIO_NUM_27, 0);
                gpio_set_level(GPIO_NUM_26, 0);
                for (int i = 0; i < NP_SEC_COUNT; i++)
                {
                    pixelSec[i].index = i;
                    pixelSec[i].rgb = COULEUR_ETEINTE;
                };
                for (int i = 0; i < NP_MIN_HRS_COUNT; i++)
                {
                    pixelMinHrs[i].index = i;
                    pixelMinHrs[i].rgb = COULEUR_ETEINTE;
                };
                neopixel_setPixelInterface(npContexts->npCtxSec, pixelSec, NP_SEC_COUNT);
                neopixel_setPixelInterface(npContexts->npCtxMinHrs, pixelMinHrs, NP_MIN_HRS_COUNT);

                gpio_set_level(GPIO_NUM_27, 1);
                gpio_set_level(GPIO_NUM_26, 1);
                vTaskDelay(pdMS_TO_TICKS(1000));

                break;
            case MODE_ARCENCIEL:
                /* gpio_set_level(GPIO_NUM_27, 0);
                 gpio_set_level(GPIO_NUM_26, 0);
                 for (int i = NP_SEC_COUNT - 1; i > 0; i--)
                 {
                     pixelSec[i].rgb = pixelSec[i - 1].rgb;
                 }

                 // Ajoute la nouvelle couleur en tête (ou éteint si on est à la fin)
                 if (offset < countCouleurPixel)
                 {
                     pixelSec[0].rgb = couleurPixel[offset][NIVEAU_PALE];
                 }
                 else
                 {
                     pixelSec[0].rgb = COULEUR_ETEINTE;
                 }

                 // Met à jour les index (nécessaire pour certaines libs Neopixel)
                 for (int i = 0; i < NP_SEC_COUNT; i++)
                 {
                     pixelSec[i].index = i;
                 }

                 neopixel_setPixelInterface(np_ctx, pixelSec, NP_SEC_COUNT);

                 offset++;

                 if (offset >= countCouleurPixel + NP_SEC_COUNT)
                 {
                     gpio_set_level(GPIO_NUM_27, 1);
                     gpio_set_level(GPIO_NUM_26, 1);
                     ESP_LOGI(TAG, "ENABLE_SEC: %d", gpio_get_level(ENABLE_SEC));
                     ESP_LOGI(TAG, "Arc-en-ciel terminé, retour au mode ARRET");
                     offset = 0;

                     mode = MODE_ARRET;
                 }
 */
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
                    couleurPixel[COULEUR_BLANC][NIVEAU_VIF],

                    COULEUR_ETEINTE};

                //const int nbEtapes = sizeof(sequenceTest) / sizeof(sequenceTest[0]);
                gpio_set_level(GPIO_NUM_26, 0);
                gpio_set_level(GPIO_NUM_27, 0);

                for (int i = 0; i < NP_SEC_COUNT; i++)
                {
                    pixelSec[i].index = i;
                    pixelSec[i].rgb = COULEUR_ETEINTE;
                }
                for (int i = 0; i < NP_MIN_HRS_COUNT; i++)
                {
                    pixelMinHrs[i].index = i;
                    pixelMinHrs[i].rgb = COULEUR_ETEINTE;
                }

                ESP_LOGI(TAG, "Début du test : ledIndexSec = %d, indexCouleurSec = %d", ledIndexSec, indexCouleurSec);
                if (ledIndexSec < NP_SEC_COUNT)
                {
                    if (indexCouleurSec < 13)
                    {
                        pixelSec[ledIndexSec].rgb = sequenceTest[indexCouleurSec];
                        ESP_LOGI(TAG, "LED %d, couleur appliquée : 0x%08lX", ledIndexSec, pixelSec[ledIndexSec].rgb);

                        neopixel_setPixelInterface(npContexts->npCtxSec, pixelSec, NP_SEC_COUNT);
                        indexCouleurSec++;
                    }
                    else
                    {
                        indexCouleurSec = 0;
                        ledIndexSec++;
                        ESP_LOGI(TAG, "Passage à la LED suivante : ledIndex = %d", ledIndexSec);
                    }
                }else if(ledIndexMinHrs < NP_MIN_HRS_COUNT)
                {
                    if (indexCouleurMinHrs < 13)
                    {
                        pixelMinHrs[ledIndexMinHrs].rgb = sequenceTest[indexCouleurMinHrs];
                        ESP_LOGI(TAG, "LED %d, couleur appliquée : 0x%08lX", ledIndexMinHrs, pixelMinHrs[ledIndexMinHrs].rgb);

                        neopixel_setPixelInterface(npContexts->npCtxMinHrs, pixelMinHrs, NP_MIN_HRS_COUNT);
                        indexCouleurMinHrs++;
                    }
                    else
                    {
                        indexCouleurMinHrs = 0;
                        ledIndexMinHrs++;
                        ESP_LOGI(TAG, "Passage à la LED suivante : ledIndex = %d", ledIndexMinHrs);
                    }
                }
                else
                {
                    gpio_set_level(GPIO_NUM_26, 1);
                    gpio_set_level(GPIO_NUM_27, 1);

                    ledIndexSec = 0;
                    indexCouleurSec = 0;
                    ledIndexMinHrs = 0;
                    indexCouleurMinHrs = 0;
                    mode = MODE_ARRET;
                    ESP_LOGI(TAG, "Test terminé, retour au mode ARRET");
                }
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;

            case MODE_HORLOGE:
                /*ESP_LOGI(TAG, "Entrée dans MODE_HORLOGE");
                vTaskDelay(pdMS_TO_TICKS(1));

                sTemps heureActuelle = {0};
                if (xQueueReceive(fileHeure, &heureActuelle, pdMS_TO_TICKS(500)) == pdPASS)
                {
                    ESP_LOGI(TAG, "Heure actuelle dans Neopixel: %02d:%02d:%02d",
                             heureActuelle.heures, heureActuelle.minutes, heureActuelle.secondes);
                }
                else
                {
                    ESP_LOGW(TAG, "Impossible de lire l'heure.");
                }

                int intensiteLumineuse = determinerIntensiteNeopixelHorloge(&parametresHorloge);
                choixCouleur(parametresHorloge.couleurHeuresActuelles, 0, pixelSec, intensiteLumineuse);
                if (heureActuelle.secondes % 2 == 0)
                {
                    choixCouleur(parametresHorloge.couleurMinutesActuelles, 1, pixelSec, intensiteLumineuse);
                }
                else
                {
                    choixCouleur(parametresHorloge.couleurSecondesActuelles, 1, pixelSec, intensiteLumineuse);
                }
                neopixel_setPixelInterface(np_ctx, pixelSec, NP_SEC_COUNT);

                ESP_LOGI(TAG, "Sortie du MODE_HORLOGE");*/
                vTaskDelay(pdMS_TO_TICKS(500));
                break;

            case MODE_TEMPERATURE:
                break;
            }
            xSemaphoreGive(npMutex);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void choixCouleur(const char *couleur, int position, tNeopixel *pixel, int intensiteLumineuse)
{
    if (strcmp(couleur, "rouge") == 0)
    {
        pixel[position].rgb = couleurPixel[COULEUR_ROUGE][intensiteLumineuse];
    }
    else if (strcmp(couleur, "orange") == 0)
    {
        pixel[position].rgb = couleurPixel[COULEUR_ORANGE][intensiteLumineuse];
    }
    else if (strcmp(couleur, "jaune") == 0)
    {
        pixel[position].rgb = couleurPixel[COULEUR_JAUNE][intensiteLumineuse];
    }
    else if (strcmp(couleur, "vert") == 0)
    {
        pixel[position].rgb = couleurPixel[COULEUR_VERT][intensiteLumineuse];
    }
    else if (strcmp(couleur, "bleu") == 0)
    {
        pixel[position].rgb = couleurPixel[COULEUR_BLEU][intensiteLumineuse];
    }
    else if (strcmp(couleur, "indigo") == 0)
    {
        pixel[position].rgb = couleurPixel[COULEUR_INDIGO][intensiteLumineuse];
    }
    else if (strcmp(couleur, "violet") == 0)
    {
        pixel[position].rgb = couleurPixel[COULEUR_VIOLET][intensiteLumineuse];
    }
    else if (strcmp(couleur, "blanc") == 0)
    {
        pixel[position].rgb = couleurPixel[COULEUR_BLANC][intensiteLumineuse];
    }
    else
    {
        pixel[position].rgb = COULEUR_ETEINTE;
    }
}

int determinerIntensiteNeopixelHorloge(const sParametresHorloge *parametres)
{
    int intensiteLumineuse;

    if (parametres->nbVille == 2)
    {
        intensiteLumineuse = NIVEAU_PALE;
    }
    else if (strcmp(parametres->affichageType, "continu") == 0)
    {
        intensiteLumineuse = NIVEAU_PALE;
    }
    else
    {
        intensiteLumineuse = NIVEAU_MOYEN;
    }
    return intensiteLumineuse;
}

void initialiserProcessusAffichageNeopixel(void)
{
    if (npMutex == NULL)
    {
        npMutex = xSemaphoreCreateMutex();
        if (npMutex == NULL)
        {
            ESP_LOGE(TAG, "Erreur de création du sémaphore Neopixel");
            return;
        }
    }
}