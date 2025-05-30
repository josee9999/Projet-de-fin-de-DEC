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

extern QueueHandle_t fileParamHorloge;

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