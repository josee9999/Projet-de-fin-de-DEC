/*
    Nom:

    Description:

    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard
*/

#ifndef PROCESSUSAFFICHAGENEOPIXEL_H
#define PROCESSUSAFFICHAGENEOPIXEL_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"

typedef enum
{
    MODE_HORLOGE,
    MODE_TEMPERATURE,
    MODE_TEST,
    MODE_ARCENCIEL,
    MODE_ARRET
} eModeAffichage;

extern volatile eModeAffichage modeActuel;

void task_AffichageNeopixel(void *pvParameter);

#endif