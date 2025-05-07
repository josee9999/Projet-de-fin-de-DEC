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
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "processusAffichageNeopixel.h"
#include "interfaceServeurWeb.h"
#include <stdio.h>

static const char *TAG = "piloteServeurWeb";

extern QueueHandle_t fileMode;
extern eModeAffichage modeActuel;

typedef struct
{
    char ssid[32];
    char password[64];
} infoConnWifi;

static void *copierSSIDetMotDePasse(const char *ssid, const char *password)
{
    infoConnWifi *copy = malloc(sizeof(infoConnWifi));
    if (!copy)
    {
        ESP_LOGE(TAG, "Échec de l'allocation mémoire pour les infos Wi-Fi.");
        return NULL;
    }
    strncpy(copy->ssid, ssid, sizeof(copy->ssid) - 1);
    copy->ssid[sizeof(copy->ssid) - 1] = '\0';
    strncpy(copy->password, password, sizeof(copy->password) - 1);
    copy->password[sizeof(copy->password) - 1] = '\0';
    return copy;
}

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

esp_err_t setHorlogeSansWifiHandler(httpd_req_t *req)
{
    char query[128];
    sParametresHorloge params = {0};

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        if (httpd_query_key_value(query, "heure", params.heure, sizeof(params.heure)) == ESP_OK)
            ESP_LOGI(TAG, "Heure: %s", params.heure);
        else
            ESP_LOGW(TAG, "Paramètre 'heure' non reçu.");

        if (httpd_query_key_value(query, "couleurHeures", params.couleurHeures, sizeof(params.couleurHeures)) == ESP_OK)
            ESP_LOGI(TAG, "Couleur Heures: %s", params.couleurHeures);
        else
            ESP_LOGW(TAG, "Paramètre 'couleurHeures' non reçu.");

        if (httpd_query_key_value(query, "couleurMinutes", params.couleurMinutes, sizeof(params.couleurMinutes)) == ESP_OK)
            ESP_LOGI(TAG, "Couleur Minutes: %s", params.couleurMinutes);
        else
            ESP_LOGW(TAG, "Paramètre 'couleurMinutes' non reçu.");

        if (httpd_query_key_value(query, "couleurSecondes", params.couleurSecondes, sizeof(params.couleurSecondes)) == ESP_OK)
            ESP_LOGI(TAG, "Couleur Secondes: %s", params.couleurSecondes);
        else
            ESP_LOGW(TAG, "Paramètre 'couleurSecondes' non reçu.");

        if (httpd_query_key_value(query, "affichageTemperature", params.affichageTemperature, sizeof(params.affichageTemperature)) == ESP_OK)
            ESP_LOGI(TAG, "Affichage Temp: %s", params.affichageTemperature);
        else
            ESP_LOGW(TAG, "Paramètre 'affichageTemperature' non reçu.");

        if (httpd_query_key_value(query, "affichageType", params.affichageType, sizeof(params.affichageType)) == ESP_OK)
            ESP_LOGI(TAG, "Affichage Type: %s", params.affichageType);
        else
            ESP_LOGW(TAG, "Paramètre 'affichageType' non reçu.");

        setParametresHorloge(&params);
        ESP_LOGI(TAG, "Les paramètres de l'horloge ont été mis à jour.");
        modeActuel = MODE_HORLOGE;
        if (xQueueSend(fileMode, &modeActuel, portMAX_DELAY) != pdPASS)
        {
            ESP_LOGE(TAG, "Erreur d'envoi du mode dans la Queue");
        }
    }
    else
    {
        ESP_LOGE(TAG, "Impossible de récupérer la chaîne de requête.");
    }

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static void connecterAuWifiStation(void *arg)
{
    infoConnWifi *infos = (infoConnWifi *)arg;

    esp_wifi_stop();
    esp_wifi_deinit();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Activer les deux modes : STA + AP
    esp_wifi_set_mode(WIFI_MODE_APSTA);

    // Configurer le mode STA (client Wi-Fi)
    wifi_config_t wifi_config_sta = {0};
    strncpy((char *)wifi_config_sta.sta.ssid, infos->ssid, sizeof(wifi_config_sta.sta.ssid) - 1);
    strncpy((char *)wifi_config_sta.sta.password, infos->password, sizeof(wifi_config_sta.sta.password) - 1);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta);

    // Configurer le mode AP (point d’accès local)
    wifi_config_t wifi_config_ap = {
        .ap = {
            .ssid = "Horloge-AP",
            .ssid_len = 0,
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK}};
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap);

    esp_wifi_start();

    esp_wifi_connect();

    free(infos);
}

esp_err_t setConnectionAuWifiHandler(httpd_req_t *req)
{
    char ssid[64] = {0};
    char password[64] = {0};

    char buf[128];
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > sizeof(buf))
        buf_len = sizeof(buf);
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
    {
        httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid));
        httpd_query_key_value(buf, "password", password, sizeof(password));
    }

    ESP_LOGI(TAG, "Tentative de connexion WiFi avec SSID: %s", ssid);

    void *copy = copierSSIDetMotDePasse(ssid, password);

    esp_timer_handle_t delay_timer;
    esp_timer_create_args_t timer_args = {
        .callback = (void *)connecterAuWifiStation,
        .arg = copy,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "wifi_reconnect"};
    esp_timer_create(&timer_args, &delay_timer);
    esp_timer_start_once(delay_timer, 500 * 1000);

    const char *reponse_html = "<html><head><meta http-equiv='refresh' content='2;url=/pageAvecWifi'></head><body>Connexion en cours...<br>Redirection dans 2 secondes.</body></html>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, reponse_html, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t setModeAvecWifiHandler(httpd_req_t *req)
{
    return setModeSansWifiHandler(req);
}

esp_err_t setHorlogeAvecWifiHandler(httpd_req_t *req)
{
    char query[256];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        char villeActuelle[16], ville2e[16];
        char couleurHeuresActuels[16], couleurMinutesActuels[16], couleurSecondes[16];
        char couleurHeures2e[16], couleurMinutes2e[16];
        char affichageTemperature[8];

        if (httpd_query_key_value(query, "villeActuelle", villeActuelle, sizeof(villeActuelle)) == ESP_OK &&
            httpd_query_key_value(query, "ville2e", ville2e, sizeof(ville2e)) == ESP_OK &&
            httpd_query_key_value(query, "couleurHeuresActuels", couleurHeuresActuels, sizeof(couleurHeuresActuels)) == ESP_OK &&
            httpd_query_key_value(query, "couleurMinutesActuels", couleurMinutesActuels, sizeof(couleurMinutesActuels)) == ESP_OK &&
            httpd_query_key_value(query, "couleurSecondes", couleurSecondes, sizeof(couleurSecondes)) == ESP_OK &&
            httpd_query_key_value(query, "couleurHeures2e", couleurHeures2e, sizeof(couleurHeures2e)) == ESP_OK &&
            httpd_query_key_value(query, "couleurMinutes2e", couleurMinutes2e, sizeof(couleurMinutes2e)) == ESP_OK &&
            httpd_query_key_value(query, "affichageTemperature", affichageTemperature, sizeof(affichageTemperature)) == ESP_OK)
        {
            ESP_LOGI(TAG, "Ville actuelle: %s, Ville 2e: %s, Couleur Heures Actuels: %s, Couleur Minutes Actuels: %s, Couleur Secondes: %s, Couleur Heures 2e: %s, Couleur Minutes 2e: %s, Affichage Temp: %s",
                     villeActuelle, ville2e, couleurHeuresActuels, couleurMinutesActuels, couleurSecondes, couleurHeures2e, couleurMinutes2e, affichageTemperature);

            // Traiter ici les données reçues et mettre à jour l'horloge
            // Par exemple, mettre à jour les couleurs et les villes, afficher la température, etc.
        }
    }
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}