/*
    Nom:

    Description:

    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard
*/
#ifndef PROCESSUSAFFICHAGENEOPIXEL_H
#define PROCESSUSAFFICHAGENEOPIXEL_H

#include "interfaceGestionNeopixel.h"
#include "GestionHeure/processusGestionHeure.h"
#include "main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"

#define COULEUR_ETEINTE (tNeopixel){0, 0, 0}

/*typedef enum
{
    MODE_HORLOGE,
    MODE_TEMPERATURE,
    MODE_TEST,
    MODE_ARCENCIEL,
    MODE_ARRET
} eModeAffichage;*/

extern QueueHandle_t fileParamHorloge;

/*typedef struct
{
    eModeAffichage modeActuel;
    int nbVille;
    char heure[16];
    char villeActuelle[16];
    char ville2e[16];
    char couleurHeuresActuelles[16];
    char couleurMinutesActuelles[16];
    char couleurSecondesActuelles[16];
    char couleurHeures2e[16];
    char couleurMinutes2e[16];
    char affichageTemperature[8];
    char affichageType[16];
} sParametresHorloge;*/

void setParametresHorloge(sParametresHorloge *params);
void task_AffichageNeopixel(void *pvParameter);
void choixCouleur(const char *couleur, int position, tNeopixel *pixel, int intensiteLumineuse);
int determinerIntensiteNeopixelHorloge(const sParametresHorloge *parametres);
void initialiserProcessusAffichageNeopixel(void);

void gererModeArcenciel(sNeopixelContexts *npContexts);
void gererModeTest(sNeopixelContexts *npContexts, eModeAffichage *mode);

void affichageHorlogeRegulier(sNeopixelContexts *npContexts, sTemps heureActuelle, sTemps *dernierAffichage, int secondePos, int minutePos, float positionExacte, tNeopixel couleurSecondes, tNeopixel couleurMinutes, tNeopixel couleurHeures);
void affichageHorlogeContinu(sNeopixelContexts *npContexts, sTemps heureActuelle, sTemps *dernierAffichage, int secondePos, int minutePos, tNeopixel couleurSecondes, tNeopixel couleurMinutes, tNeopixel couleurHeures);


#endif