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
extern QueueHandle_t fileMode;

esp_err_t pageAccueilHandler(httpd_req_t *req);
esp_err_t pagePersonnalisationSansWifiHandler(httpd_req_t *req);
esp_err_t pagePersonnalisationAvecWifiHandler(httpd_req_t *req);

#endif