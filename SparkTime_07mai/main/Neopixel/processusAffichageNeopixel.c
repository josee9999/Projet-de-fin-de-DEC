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
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <string.h>
#include <stdio.h>
#include "main.h"

static SemaphoreHandle_t npMutex = NULL;
static const char *TAG = "processusAffichageNeopixel";
extern QueueHandle_t fileParamHorloge;
extern QueueHandle_t fileHeure;

sParametresHorloge parametresHorloge = {0};

/*static int ledIndexSec = 0;
static int indexCouleurSec = 0;
static int ledIndexMinHrs = 0;
static int indexCouleurMinHrs = 0;*/

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

// Fonction d'aide pour éteindre toutes les LEDs
static void eteindreToutesLesLEDs(sNeopixelContexts *npContexts)
{
    if (npContexts == NULL || npContexts->npCtxSec == NULL || npContexts->npCtxMinHrs == NULL)
        return;

    // Éteindre les LEDs des secondes
    for (int i = 0; i < NP_SEC_COUNT; i++)
    {
        mettreCouleurNeopixel(npContexts->npCtxSec, i, 0, 0, 0);
    }
    // Éteindre les LEDs des minutes/heures
    for (int i = 0; i < NP_MIN_HRS_COUNT; i++)
    {
        mettreCouleurNeopixel(npContexts->npCtxMinHrs, i, 0, 0, 0);
    }

    afficherNeopixel(npContexts->npCtxSec);
    afficherNeopixel(npContexts->npCtxMinHrs);
}

void task_AffichageNeopixel(void *pvParameter)
{
    ESP_LOGI(TAG, "Tâche Neopixel démarrée");
    sNeopixelContexts *npContexts = (sNeopixelContexts *)pvParameter;

    if (!npContexts || !npContexts->npCtxSec || !npContexts->npCtxMinHrs)
    {
        ESP_LOGE(TAG, "Erreur : Contextes Neopixel non initialisés.");
        vTaskDelete(NULL);
        return;
    }

    eModeAffichage mode = MODE_ARCENCIEL;
    int offset = 0;
    sTemps derniereHeure = {0};

    while (1)
    {
        // Vérifier les nouveaux paramètres
        sParametresHorloge nouveauxParametres;
        if (xQueueReceive(fileParamHorloge, &nouveauxParametres, pdMS_TO_TICKS(10)) == pdPASS)
        {
            ESP_LOGI(TAG, "Nouveaux paramètres reçus - Mode: %d", nouveauxParametres.modeActuel);
            setParametresHorloge(&nouveauxParametres);
            mode = nouveauxParametres.modeActuel;
            eteindreToutesLesLEDs(npContexts);
        }

        if (xSemaphoreTake(npMutex, portMAX_DELAY) == pdTRUE)
        {
            switch (mode)
            {
            case MODE_HORLOGE:
            {
                sTemps heureActuelle = {0};
                bool heureRecue = false;

                // Vider la file et garder la dernière valeur
                while (xQueueReceive(fileHeure, &heureActuelle, 0) == pdPASS)
                {
                    heureRecue = true;
                }

                // Si on a reçu une nouvelle heure
                if (heureRecue)
                {
                    ESP_LOGI(TAG, "Mode Horloge - Nouvelle heure: %02d:%02d:%02d",
                             heureActuelle.heures, heureActuelle.minutes, heureActuelle.secondes);

                    derniereHeure = heureActuelle;

                    static sTemps dernierAffichage = {-1, -1, -1}; // Pour suivre le dernier état

                    if (npContexts != NULL && npContexts->npCtxSec != NULL && npContexts->npCtxMinHrs != NULL)
                    {
                        tNeopixel couleurSecondes = {0};
                        tNeopixel couleurMinutes = {0};
                        tNeopixel couleurHeures = {0};

                        // Déterminer les couleurs en fonction des paramètres web
                        // Déterminer l'intensité en fonction du mode
                        int intensite = determinerIntensiteNeopixelHorloge(&parametresHorloge);
                        choixCouleur(parametresHorloge.couleurSecondesActuelles, 0, &couleurSecondes, intensite);
                        choixCouleur(parametresHorloge.couleurMinutesActuelles, 0, &couleurMinutes, intensite);
                        choixCouleur(parametresHorloge.couleurHeuresActuelles, 0, &couleurHeures, intensite);

                        // Position des secondes (décalage de 45 pour aligner 0 sur la position 45)
                        int secondePos = (heureActuelle.secondes + 45) % 60;
                        ESP_LOGI(TAG, "Position LED seconde: %d (seconde: %d)", secondePos, heureActuelle.secondes);

                        // Position des minutes
                        int minutePos = (heureActuelle.minutes + 45) % 60;
                        ESP_LOGI(TAG, "Position LED minute: %d (minute: %d)", minutePos, heureActuelle.minutes);

                        // Position des heures (conversion en format 12h)
                        int heures12 = heureActuelle.heures % 12;
                        if (heures12 == 0)
                            heures12 = 12; // Convertir 0h en 12h

                        // Calcul de la position précise des heures en tenant compte des minutes
                        float progression = heureActuelle.minutes / 60.0f;
                        if (heures12 == 0)
                            heures12 = 12;

                        // Calcul de la position exacte avec interpolation
                        float positionExacte;
                        if (heures12 == 12)
                        {
                            positionExacte = 105; // Midi/Minuit reste fixe à 105
                        }
                        else
                        {
                            // Calcul de la position de base pour l'heure
                            // 1h -> 110, 2h -> 115, 3h -> 60, 4h -> 65, etc.
                            float posBase;
                            if (heures12 <= 2)
                            {
                                // Pour 1h et 2h, on part de 110
                                posBase = 110 + ((heures12 - 1) * 5);
                            }
                            else
                            {
                                // Pour 3h à 11h, on commence à 60
                                posBase = 60 + ((heures12 - 3) * 5);
                            }

                            // Ajout de la progression des minutes
                            positionExacte = posBase + (progression * 5);

                            // Gestion du débordement
                            if (positionExacte >= 120)
                            {
                                positionExacte = 60 + (positionExacte - 120);
                            }
                        }

                        // Affichage selon le type d'affichage choisi
                        if (strcmp(parametresHorloge.affichageType, "regulier") == 0)
                        {
                            affichageHorlogeRegulier(npContexts, heureActuelle, &dernierAffichage, secondePos, minutePos, positionExacte, couleurSecondes, couleurMinutes, couleurHeures);
                        }
                        else if (strcmp(parametresHorloge.affichageType, "continu") == 0)
                        {
                            affichageHorlogeContinu(npContexts, heureActuelle, &dernierAffichage, secondePos, minutePos, couleurSecondes, couleurMinutes, couleurHeures);
                        }

                        // Sauvegarder l'état actuel
                        dernierAffichage = heureActuelle;

                        // Afficher les changements
                        ESP_LOGI(TAG, "Affichage des LEDs... Type: %s", parametresHorloge.affichageType);
                        afficherNeopixel(npContexts->npCtxSec);
                        afficherNeopixel(npContexts->npCtxMinHrs);
                    }
                }
                break;
            }
            case MODE_ARRET:
                eteindreToutesLesLEDs(npContexts);
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;

            case MODE_ARCENCIEL:
                gererModeArcenciel(npContexts);
                vTaskDelay(pdMS_TO_TICKS(75));
                break;

            case MODE_TEST:
                gererModeTest(npContexts, &mode);
                vTaskDelay(pdMS_TO_TICKS(50));
                break;

            case MODE_TEMPERATURE:
                // TODO .....................
                break;
            }
            xSemaphoreGive(npMutex);
        }

        // Délai raisonnable entre les mises à jour
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void gererModeArcenciel(sNeopixelContexts *npContexts)
{
    static int pos = 0;
    const int nbCouleurs = NOMBRE_COULEURS - 1; // Exclure le blanc
    const int totalLEDs = NP_SEC_COUNT + NP_MIN_HRS_COUNT;

    eteindreToutesLesLEDs(npContexts);
    gpio_set_level(ENABLE_MIN_HRS, 0);
    gpio_set_level(ENABLE_SEC, 0);

    if (npContexts != NULL && npContexts->npCtxSec != NULL && npContexts->npCtxMinHrs != NULL)
    {
        // Allumer seulement les 12 LEDs de la chenille
        for (int i = 0; i < nbCouleurs; i++)
        {
            int ledPos = pos - i;
            if (ledPos >= 0 && ledPos < totalLEDs)
            {
                if (ledPos < NP_SEC_COUNT)
                {
                    mettreCouleurNeopixel(npContexts->npCtxSec, ledPos,
                                          couleurPixel[i][NIVEAU_PALE].r,
                                          couleurPixel[i][NIVEAU_PALE].g,
                                          couleurPixel[i][NIVEAU_PALE].b);
                }
                else
                {
                    mettreCouleurNeopixel(npContexts->npCtxMinHrs, ledPos - NP_SEC_COUNT,
                                          couleurPixel[i][NIVEAU_PALE].r,
                                          couleurPixel[i][NIVEAU_PALE].g,
                                          couleurPixel[i][NIVEAU_PALE].b);
                }
            }
        }

        afficherNeopixel(npContexts->npCtxSec);
        afficherNeopixel(npContexts->npCtxMinHrs);

        pos++;
        if (pos >= totalLEDs + nbCouleurs)
        {
            pos = 0;
        }
    }
}

void gererModeTest(sNeopixelContexts *npContexts, eModeAffichage *mode)
{
    eteindreToutesLesLEDs(npContexts); // Éteindre avant de commencer
    gpio_set_level(ENABLE_MIN_HRS, 0);
    gpio_set_level(ENABLE_SEC, 0);

    static int ledIndex = 0;
    static int indexCouleur = 0;
    static bool testingSeconds = true; // Pour savoir quelle bande on teste

    if (npContexts != NULL && npContexts->npCtxSec != NULL && npContexts->npCtxMinHrs != NULL)
    {
        if (testingSeconds)
        {
            // Test des LEDs des secondes
            if (ledIndex < NP_SEC_COUNT)
            {
                if (indexCouleur < 12)
                {
                    int couleur;
                    int niveau;

                    if (indexCouleur < 3)
                    {
                        couleur = COULEUR_ROUGE;
                        niveau = indexCouleur;
                    }
                    else if (indexCouleur < 6)
                    {
                        couleur = COULEUR_VERT;
                        niveau = indexCouleur - 3;
                    }
                    else if (indexCouleur < 9)
                    {
                        couleur = COULEUR_INDIGO;
                        niveau = indexCouleur - 6;
                    }
                    else
                    {
                        couleur = COULEUR_BLANC;
                        niveau = indexCouleur - 9;
                    }

                    mettreCouleurNeopixel(npContexts->npCtxSec, ledIndex,
                                          couleurPixel[couleur][niveau].r,
                                          couleurPixel[couleur][niveau].g,
                                          couleurPixel[couleur][niveau].b);

                    // ESP_LOGI(TAG, "LED Secondes %d: Couleur %d, Niveau %d", ledIndex, couleur, niveau);

                    indexCouleur++;
                }
                else
                {
                    indexCouleur = 0;
                    ledIndex++;
                }
            }
            else
            {
                // Passage aux minutes/heures
                testingSeconds = false;
                ledIndex = 0;
                indexCouleur = 0;
                eteindreToutesLesLEDs(npContexts);
            }
            afficherNeopixel(npContexts->npCtxSec);
        }
        else
        {
            // Test des LEDs minutes/heures
            if (ledIndex < NP_MIN_HRS_COUNT)
            {
                if (indexCouleur < 12)
                {
                    int couleur;
                    int niveau;

                    if (indexCouleur < 3)
                    {
                        couleur = COULEUR_ROUGE;
                        niveau = indexCouleur;
                    }
                    else if (indexCouleur < 6)
                    {
                        couleur = COULEUR_VERT;
                        niveau = indexCouleur - 3;
                    }
                    else if (indexCouleur < 9)
                    {
                        couleur = COULEUR_INDIGO;
                        niveau = indexCouleur - 6;
                    }
                    else
                    {
                        couleur = COULEUR_BLANC;
                        niveau = indexCouleur - 9;
                    }

                    mettreCouleurNeopixel(npContexts->npCtxMinHrs, ledIndex,
                                          couleurPixel[couleur][niveau].r,
                                          couleurPixel[couleur][niveau].g,
                                          couleurPixel[couleur][niveau].b);

                    indexCouleur++;
                }
                else
                {
                    indexCouleur = 0;
                    ledIndex++;
                }
            }
            else
            {
                // Fin du test
                testingSeconds = true;
                ledIndex = 0;
                indexCouleur = 0;
                *mode = MODE_ARRET;
                eteindreToutesLesLEDs(npContexts);
            }
            afficherNeopixel(npContexts->npCtxMinHrs);
        }
    }
}

void affichageHorlogeRegulier(sNeopixelContexts *npContexts, sTemps heureActuelle, sTemps *dernierAffichage,
                              int secondePos, int minutePos, float positionExacte,
                              tNeopixel couleurSecondes, tNeopixel couleurMinutes, tNeopixel couleurHeures)
{
    // Éteindre l'ancienne LED de secondes
    if (dernierAffichage->secondes >= 0)
    {
        int anciennePosSec = (dernierAffichage->secondes + 45) % 60;
        mettreCouleurNeopixel(npContexts->npCtxSec, anciennePosSec, 0, 0, 0);
    }

    // Éteindre l'ancienne LED de minutes si elle a changé
    if (dernierAffichage->minutes != heureActuelle.minutes && dernierAffichage->minutes >= 0)
    {
        int anciennePosMin = (dernierAffichage->minutes + 45) % 60;
        mettreCouleurNeopixel(npContexts->npCtxMinHrs, anciennePosMin, 0, 0, 0);
    }

    // Calculer l'ancienne position exacte de l'aiguille des heures
    if (dernierAffichage->heures >= 0 && dernierAffichage->minutes >= 0)
    {
        int anciennesHeures12 = dernierAffichage->heures % 12;
        if (anciennesHeures12 == 0)
            anciennesHeures12 = 12;

        float ancienneProgression = dernierAffichage->minutes / 60.0f;
        float anciennePositionExacte;
        if (anciennesHeures12 == 12)
        {
            anciennePositionExacte = 105;
        }
        else
        {
            float anciennePosBase;
            if (anciennesHeures12 <= 2)
            {
                anciennePosBase = 110 + ((anciennesHeures12 - 1) * 5);
            }
            else
            {
                anciennePosBase = 60 + ((anciennesHeures12 - 3) * 5);
            }
            anciennePositionExacte = anciennePosBase + (ancienneProgression * 5);
            if (anciennePositionExacte >= 120)
            {
                anciennePositionExacte = 60 + (anciennePositionExacte - 120);
            }
        }

        int ancienneLedHeure = (int)(anciennePositionExacte + 0.5f);
        int ledHeure = (int)(positionExacte + 0.5f);

        // Éteindre l'ancienne LED si elle est différente de la nouvelle
        if (ancienneLedHeure != ledHeure)
        {
            mettreCouleurNeopixel(npContexts->npCtxMinHrs, ancienneLedHeure, 0, 0, 0);
        }
    }

    // Allumer les nouvelles LEDs
    mettreCouleurNeopixel(npContexts->npCtxSec, secondePos,
                          couleurSecondes.r, couleurSecondes.g, couleurSecondes.b);

    mettreCouleurNeopixel(npContexts->npCtxMinHrs, minutePos,
                          couleurMinutes.r, couleurMinutes.g, couleurMinutes.b);

    int ledHeure = (int)(positionExacte + 0.5f);
    mettreCouleurNeopixel(npContexts->npCtxMinHrs, ledHeure,
                          couleurHeures.r, couleurHeures.g, couleurHeures.b);
}


void affichageHorlogeContinu(sNeopixelContexts *npContexts, sTemps heureActuelle, sTemps *dernierAffichage, int secondePos,
                             int minutePos, tNeopixel couleurSecondes, tNeopixel couleurMinutes, tNeopixel couleurHeures)
{
    static bool premierAffichage = true;
    static int derniereSeconde = -1;
    static int derniereMinute = -1;
    static int derniereHeure = -1;
    static float dernierePosHeure = -1;
    bool needUpdate = false;

    // Au premier affichage ou lors d'un changement de mode
    if (premierAffichage)
    {
        eteindreToutesLesLEDs(npContexts);
        derniereSeconde = -1;
        derniereMinute = -1;
        derniereHeure = -1;
        dernierePosHeure = -1;
        premierAffichage = false;
        needUpdate = true;
    }

    // Gestion des secondes
    if (heureActuelle.secondes == 0 && derniereSeconde != 0)
    {
        // Réinitialisation complète des LEDs des secondes
        for (int i = 0; i < NP_SEC_COUNT; i++)
        {
            mettreCouleurNeopixel(npContexts->npCtxSec, i, 0, 0, 0);
        }
        needUpdate = true;
    }

    // Affichage des secondes
    if (heureActuelle.secondes != derniereSeconde)
    {
        int startPos = 45;
        int endPos = secondePos;

        if (endPos < startPos)
        {
            // Si on a dépassé la position 45, on allume jusqu'à 60 puis de 0 à la position actuelle
            for (int i = startPos; i < 60; i++)
            {
                mettreCouleurNeopixel(npContexts->npCtxSec, i,
                                      couleurSecondes.r, couleurSecondes.g, couleurSecondes.b);
            }
            for (int i = 0; i <= endPos; i++)
            {
                mettreCouleurNeopixel(npContexts->npCtxSec, i,
                                      couleurSecondes.r, couleurSecondes.g, couleurSecondes.b);
            }
        }
        else
        {
            // Sinon on allume simplement de 45 à la position actuelle
            for (int i = startPos; i <= endPos; i++)
            {
                mettreCouleurNeopixel(npContexts->npCtxSec, i,
                                      couleurSecondes.r, couleurSecondes.g, couleurSecondes.b);
            }
        }
        needUpdate = true;
    }

    // Gestion des minutes
    if (heureActuelle.minutes == 0 && derniereMinute != 0)
    {
        // Réinitialisation complète des LEDs des minutes
        for (int i = 0; i < 60; i++)
        {
            mettreCouleurNeopixel(npContexts->npCtxMinHrs, i, 0, 0, 0);
        }
        needUpdate = true;
    }

    // Affichage des minutes
    if (heureActuelle.minutes != derniereMinute)
    {
        int startPos = 45;
        int endPos = minutePos;

        if (endPos < startPos)
        {
            // Si on a dépassé la position 45, on allume jusqu'à 60 puis de 0 à la position actuelle
            for (int i = startPos; i < 60; i++)
            {
                mettreCouleurNeopixel(npContexts->npCtxMinHrs, i,
                                      couleurMinutes.r, couleurMinutes.g, couleurMinutes.b);
            }
            for (int i = 0; i <= endPos; i++)
            {
                mettreCouleurNeopixel(npContexts->npCtxMinHrs, i,
                                      couleurMinutes.r, couleurMinutes.g, couleurMinutes.b);
            }
        }
        else
        {
            // Sinon on allume simplement de 45 à la position actuelle
            for (int i = startPos; i <= endPos; i++)
            {
                mettreCouleurNeopixel(npContexts->npCtxMinHrs, i,
                                      couleurMinutes.r, couleurMinutes.g, couleurMinutes.b);
            }
        }
        needUpdate = true;
    }

    // Calcul de la position précise des heures en tenant compte des minutes
    float progression = heureActuelle.minutes / 60.0f;
    int heures12 = heureActuelle.heures % 12;
    if (heures12 == 0)
        heures12 = 12;

    // Calcul de la position exacte avec interpolation
    float positionExacte;
    if (heures12 == 12)
    {
        positionExacte = 105; // Midi/Minuit reste fixe à 105
    }
    else
    {
        // Calcul de la position de base pour l'heure
        // 1h -> 110, 2h -> 115, 3h -> 60, 4h -> 65, etc.
        float posBase;
        if (heures12 <= 2)
        {
            // Pour 1h et 2h, on part de 110
            posBase = 110 + ((heures12 - 1) * 5);
        }
        else
        {
            // Pour 3h à 11h, on commence à 60
            posBase = 60 + ((heures12 - 3) * 5);
        }

        // Ajout de la progression des minutes
        positionExacte = posBase + (progression * 5);

        // Gestion du débordement
        if (positionExacte >= 120)
        {
            positionExacte = 60 + (positionExacte - 120);
        }
    }

    // Mise à jour de l'affichage des heures si la position a changé
    if (positionExacte != dernierePosHeure || heureActuelle.heures != derniereHeure)
    {
        // Réinitialisation des LEDs des heures
        for (int i = 60; i < NP_MIN_HRS_COUNT; i++)
        {
            mettreCouleurNeopixel(npContexts->npCtxMinHrs, i, 0, 0, 0);
        }

        if (heures12 == 12)
        {
            // Cas spécial pour midi/minuit : seulement la LED 105
            mettreCouleurNeopixel(npContexts->npCtxMinHrs, 105,
                                  couleurHeures.r, couleurHeures.g, couleurHeures.b);
        }
        else
        {
            // Pour les autres heures
            int startPos = 105; // Position de départ (12h)
            int endPos = (int)positionExacte;

            if (endPos < startPos)
            {
                // Si on est entre 1h et 11h
                // D'abord de 105 jusqu'à la fin
                for (int i = startPos; i < NP_MIN_HRS_COUNT; i++)
                {
                    mettreCouleurNeopixel(npContexts->npCtxMinHrs, i,
                                          couleurHeures.r, couleurHeures.g, couleurHeures.b);
                }
                // Puis de 60 jusqu'à la position actuelle
                for (int i = 60; i <= endPos; i++)
                {
                    mettreCouleurNeopixel(npContexts->npCtxMinHrs, i,
                                          couleurHeures.r, couleurHeures.g, couleurHeures.b);
                }
            }
            else if (endPos > startPos)
            {
                // Si on est entre 13h et 23h
                for (int i = startPos; i <= endPos; i++)
                {
                    mettreCouleurNeopixel(npContexts->npCtxMinHrs, i,
                                          couleurHeures.r, couleurHeures.g, couleurHeures.b);
                }
            }
        }
        needUpdate = true;
        dernierePosHeure = positionExacte;
    }

    // Mise à jour des dernières valeurs
    derniereSeconde = heureActuelle.secondes;
    derniereMinute = heureActuelle.minutes;
    derniereHeure = heureActuelle.heures;

    // Si on change d'heure, réinitialiser premierAffichage
    if (dernierAffichage->heures != heureActuelle.heures)
    {
        premierAffichage = true;
    }

    // Afficher les changements seulement si nécessaire
    if (needUpdate)
    {
        // S'assurer que toutes les couleurs sont bien configurées avant l'affichage
        vTaskDelay(pdMS_TO_TICKS(2));
        afficherNeopixel(npContexts->npCtxSec);
        vTaskDelay(pdMS_TO_TICKS(2));
        afficherNeopixel(npContexts->npCtxMinHrs);
    }
}

void choixCouleur(const char *couleur, int position, tNeopixel *pixel, int intensiteLumineuse)
{
    if (strcmp(couleur, "rouge") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_ROUGE][intensiteLumineuse];
    }
    else if (strcmp(couleur, "orange") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_ORANGE][intensiteLumineuse];
    }
    else if (strcmp(couleur, "jaune") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_JAUNE][intensiteLumineuse];
    }
    else if (strcmp(couleur, "jaune-vert") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_JAUNE_VERT][intensiteLumineuse];
    }
    else if (strcmp(couleur, "vert") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_VERT][intensiteLumineuse];
    }
    else if (strcmp(couleur, "vert-bleu") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_VERT_BLEU][intensiteLumineuse];
    }
    else if (strcmp(couleur, "turquoise") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_TURQUOISE][intensiteLumineuse];
    }
    else if (strcmp(couleur, "bleu") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_BLEU][intensiteLumineuse];
    }
    else if (strcmp(couleur, "indigo") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_INDIGO][intensiteLumineuse];
    }
    else if (strcmp(couleur, "violet") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_VIOLET][intensiteLumineuse];
    }
    else if (strcmp(couleur, "mauve") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_MAUVE][intensiteLumineuse];
    }
    else if (strcmp(couleur, "rose") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_ROSE][intensiteLumineuse];
    }
    else if (strcmp(couleur, "blanc") == 0)
    {
        pixel[position] = couleurPixel[COULEUR_BLANC][intensiteLumineuse];
    }
    else
    {
        pixel[position] = COULEUR_ETEINTE;
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