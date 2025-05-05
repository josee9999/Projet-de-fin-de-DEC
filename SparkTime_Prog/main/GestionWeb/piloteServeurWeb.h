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

void demarrerServeurWeb(void);

static void connecterAuWifiStation(void *arg);
esp_err_t setConnectionAuWifiHandler(httpd_req_t *req);
static void *copierSSIDetMotDePasse(const char *ssid, const char *password);

esp_err_t setHorlogeSansWifiHandler(httpd_req_t *req);
esp_err_t setHorlogeAvecWifiHandler(httpd_req_t *req);

esp_err_t setModeSansWifiHandler(httpd_req_t *req);
esp_err_t setModeAvecWifiHandler(httpd_req_t *req);

#endif