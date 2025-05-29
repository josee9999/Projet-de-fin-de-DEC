/*
    Nom: 

    Description: 
                 
    Modification: -
    Création: 29/04/2025
    Auteur: Josée Girard   
*/


#ifndef PILOTE_SERVEUR_WEB_H
#define PILOTE_SERVEUR_WEB_H

#include "esp_err.h"        
#include "esp_http_server.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"

void demarrerServeurWeb(void);
esp_err_t setConnectionAuWifiHandler(httpd_req_t *req);

void urlDecode(char *src, size_t srcSize);

esp_err_t setHorlogeSansWifiHandler(httpd_req_t *req);
esp_err_t setHorlogeAvecWifiHandler(httpd_req_t *req);

esp_err_t setModeSansWifiHandler(httpd_req_t *req);
esp_err_t setModeAvecWifiHandler(httpd_req_t *req);

esp_err_t redemarrerSystemeHandler(httpd_req_t *req);

#endif