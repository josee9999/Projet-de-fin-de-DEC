/*
    Nom: processusAPITemps

    Description: Gere la communication avec l'API de fuseau horaire pour 
                 requetes et récupération d'informations.
                 
    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard   
*/

#ifndef PROCESSUS_API_TEMPS_H
#define PROCESSUS_API_TEMPS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "processusGestionHeure.h"

extern QueueHandle_t fileHeure;
extern QueueHandle_t fileParamHorloge;

void tache_APITemps(void *pvParameters);
esp_err_t obtenirHeureVille(const char *timezone);

#endif