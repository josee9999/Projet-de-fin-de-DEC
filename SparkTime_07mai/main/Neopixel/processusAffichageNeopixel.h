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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"

#define COULEUR_ETEINTE NP_RGB(0, 0, 0)

typedef enum
{
    MODE_HORLOGE,
    MODE_TEMPERATURE,
    MODE_TEST,
    MODE_ARCENCIEL,
    MODE_ARRET
} eModeAffichage;

extern eModeAffichage modeActuel;
extern QueueHandle_t fileMode;

typedef struct {
    char heure[16];
    char couleurHeures[16];
    char couleurMinutes[16];
    char couleurSecondes[16];
    char affichageTemperature[8];
    char affichageType[16];
} sParametresHorloge;


void setParametresHorloge(sParametresHorloge *params);
void task_AffichageNeopixel(void *pvParameter);
void choixCouleur(const char *couleur, int position, tNeopixel *pixel);
#endif