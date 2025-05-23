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

extern QueueHandle_t fileHeure;  
extern sParametresHorloge parametresHorloge;

typedef struct {
    int heures;
    int minutes;
    int secondes;
    bool estVillePrincipale;  // true pour la ville actuelle, false pour la 2e ville
} sTemps;

void task_GestionHeure(void *pvParameter);

#endif