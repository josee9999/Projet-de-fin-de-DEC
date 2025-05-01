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

void demarrerServeurWeb(void);

esp_err_t pageAccueilHandler(httpd_req_t *req);
esp_err_t pagePersonnalisationSansWifiHandler(httpd_req_t *req);
esp_err_t pagePersonnalisationAvecWifiHandler(httpd_req_t *req);

esp_err_t setHorlogeSansWifiHandler(httpd_req_t *req);
esp_err_t setHorlogeAvecWifiHandler(httpd_req_t *req);

esp_err_t setModeSansWifiHandler(httpd_req_t *req);
esp_err_t setModeAvecWifiHandler(httpd_req_t *req);

#endif