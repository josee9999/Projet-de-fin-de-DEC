/*
    Nom: 

    Description: 
                 
    Modification: -
    Création: 29/04/2025
    Auteur: Josée Girard   
*/

#include "piloteServeurWeb.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "processusAffichageNeopixel.h"
#include "interfaceServeurWeb.h"
#include <stdio.h>

static const char *TAG = "piloteServeurWeb";

extern QueueHandle_t fileMode;
extern eModeAffichage modeActuel;

void demarrerServeurWeb(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t serveur = NULL;
    if (httpd_start(&serveur, &config) == ESP_OK)
    {
        httpd_uri_t uriPageAccueil = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = pageAccueilHandler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriPageAccueil);

        httpd_uri_t uriPageSansWifi = {
            .uri = "/pageSansWifi",
            .method = HTTP_GET,
            .handler = pagePersonnalisationSansWifiHandler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriPageSansWifi);

        httpd_uri_t uriSetModeSansWifi = {
            .uri = "/setModeSansWifi",
            .method = HTTP_GET,
            .handler = setModeSansWifiHandler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriSetModeSansWifi);

        httpd_uri_t uriSetHorlogeSansWifi = {
            .uri = "/setHorlogeSansWifi",
            .method = HTTP_GET,
            .handler = setHorlogeSansWifiHandler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriSetHorlogeSansWifi);

        httpd_uri_t uriSetConnexionWifi = {
            .uri = "/connectWifi",
            .method = HTTP_GET,
            .handler = setConnectionAuWifiHandler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriSetConnexionWifi);

        httpd_uri_t uriPageAvecWifi = {
            .uri = "/pageAvecWifi",
            .method = HTTP_GET,
            .handler = pagePersonnalisationAvecWifiHandler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriPageAvecWifi);

        httpd_uri_t uriSetModeAvecWifi = {
            .uri = "/setModeAvecWifi",
            .method = HTTP_GET,
            .handler = setModeAvecWifiHandler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriSetModeAvecWifi);

        httpd_uri_t uriSetHorlogeAvecWifi = {
            .uri = "/setHorlogeAvecWifi",
            .method = HTTP_GET,
            .handler = setHorlogeAvecWifiHandler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriSetHorlogeAvecWifi);

        ESP_LOGI(TAG, "Serveur web démarré.");
    }
    else
    {
        ESP_LOGE(TAG, "Erreur au démarrage du serveur web.");
    }
}

esp_err_t setModeSansWifiHandler(httpd_req_t *req)
{
    char query[64];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        char mode_val[8];
        if (httpd_query_key_value(query, "mode", mode_val, sizeof(mode_val)) == ESP_OK)
        {
            ESP_LOGI(TAG, "Mode reçu depuis la page web : %s", mode_val);
            int mode_int = atoi(mode_val);
            if (mode_int <= MODE_ARRET)
            {
                modeActuel = (eModeAffichage)mode_int;
                ESP_LOGI(TAG, "ModeActuel mis à jour à : %d", modeActuel);

                if (xQueueSend(fileMode, &modeActuel, portMAX_DELAY) != pdPASS)
                {
                    ESP_LOGE(TAG, "Erreur d'envoi du mode dans la Queue");
                }
            }
            else
            {
                ESP_LOGW(TAG, "Valeur de mode non valide : %d", mode_int);
            }
        }
    }

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t setHorlogeSansWifiHandler(httpd_req_t *req) // TODO
{
    char query[128];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        char heure[16], couleurHeures[16], couleurMinutes[16], couleurSecondes[16];
        char affichageTemperature[8], affichageType[16];

        if (httpd_query_key_value(query, "heure", heure, sizeof(heure)) == ESP_OK &&
            httpd_query_key_value(query, "couleurHeures", couleurHeures, sizeof(couleurHeures)) == ESP_OK &&
            httpd_query_key_value(query, "couleurMinutes", couleurMinutes, sizeof(couleurMinutes)) == ESP_OK &&
            httpd_query_key_value(query, "couleurSecondes", couleurSecondes, sizeof(couleurSecondes)) == ESP_OK &&
            httpd_query_key_value(query, "affichageTemperature", affichageTemperature, sizeof(affichageTemperature)) == ESP_OK &&
            httpd_query_key_value(query, "affichageType", affichageType, sizeof(affichageType)) == ESP_OK)
        {
            ESP_LOGI(TAG, "Heure: %s, Couleur Heures: %s, Couleur Minutes: %s, Couleur Secondes: %s, Affichage Temp: %s, Affichage Type: %s",
                     heure, couleurHeures, couleurMinutes, couleurSecondes, affichageTemperature, affichageType);

            // Traiter ici les données reçues et mettre à jour l'horloge
            // Par exemple, utiliser affichageTemperature pour activer/désactiver l'affichage de la température
            // et affichageType pour choisir entre régulier et continu.
        }
    }
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t setConnectionAuWifiHandler(httpd_req_t *req)
{
    char ssid[64] = {0};
    char password[64] = {0};

    char buf[128];
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > sizeof(buf)) buf_len = sizeof(buf);
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
        httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid));
        httpd_query_key_value(buf, "password", password, sizeof(password));
    }

    ESP_LOGE(TAG, "Tentative de connexion WiFi avec SSID: %s et mot de passe: %s\n", ssid, password);

    // TODO : ici tu peux tenter la connexion réelle au WiFi si tu veux

    // Rediriger vers la page de personnalisation avec WiFi
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/pageAvecWifi");
    httpd_resp_send(req, NULL, 0);

    return ESP_OK;
}

esp_err_t setModeAvecWifiHandler(httpd_req_t *req) 
{
    return setModeSansWifiHandler(req);
}

esp_err_t setHorlogeAvecWifiHandler(httpd_req_t *req) // TODO
{

    return ESP_OK;
}