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
void demarrer_serveur_web(void);

esp_err_t page_racine_handler(httpd_req_t *req);
esp_err_t set_mode_handler(httpd_req_t *req);

#endif