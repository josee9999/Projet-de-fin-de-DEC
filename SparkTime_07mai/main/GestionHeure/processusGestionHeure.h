/*
    Nom: processusGestionHeure

    Description: Gere l'heure local de l'horloge et, lorsque necessaire, l'heure d'un autre endroit.
                 
    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard   
*/
#ifndef PROCESSUSGESTIONHEURE_H
#define PROCESSUSGESTIONHEURE_H

#include "Neopixel/processusAffichageNeopixel.h" 
#include "main.h"

extern QueueHandle_t fileHeure;  
extern sParametresHorloge parametresHorloge;

void task_GestionHeure(void *pvParameter);

#endif