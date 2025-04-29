/*
    Nom: 

    Description: 
                 
    Modification: -
    Création: 29/04/2025
    Auteur: Josée Girard   
*/

#ifndef INTERFACE_SERVEUR_WEB_H
#define INTERFACE_SERVEUR_WEB_H

#include "esp_http_server.h"
#include "processusAffichageNeopixel.h"

extern volatile eModeAffichage modeActuel;

esp_err_t handlerPageConnection(httpd_req_t *req);
esp_err_t handlerPageSansWifi(httpd_req_t *req);

#endif 