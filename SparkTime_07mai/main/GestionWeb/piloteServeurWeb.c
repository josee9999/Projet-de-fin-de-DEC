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
#include "GestionHeure/processusGestionHeure.h"
#include <stdio.h>
#include "esp_system.h"

static void connecterAuWifiStation(void *arg);
static void *copierSSIDetMotDePasse(const char *ssid, const char *password);
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

static const char *TAG = "piloteServeurWeb";

extern QueueHandle_t fileParamHorloge;
extern QueueHandle_t fileHeure;
extern eModeAffichage modeActuel;

// Gestionnaire d'événements WiFi
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "Station WiFi démarrée, tentative de connexion...");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Connecté au réseau WiFi!");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Déconnecté du réseau WiFi, nouvelle tentative...");
                esp_wifi_connect();
                break;
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "Adresse IP obtenue: " IPSTR, IP2STR(&event->ip_info.ip));
        }
    }
}

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

static httpd_handle_t serveur = NULL;
static bool systeme_en_redemarrage = false;

void demarrerServeurWeb(void)
{
    // Enregistrer le gestionnaire d'événements WiFi
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;  // Augmenter le nombre maximum de handlers
    
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

        // Ajouter la route de redémarrage
        httpd_uri_t uriRedemarrer = {
            .uri = "/redemarrer",
            .method = HTTP_GET,
            .handler = redemarrerSystemeHandler,
            .user_ctx = NULL
        };
        esp_err_t ret = httpd_register_uri_handler(serveur, &uriRedemarrer);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Erreur lors de l'enregistrement du handler de redémarrage: %d", ret);
            return;
        }
        
        ESP_LOGI(TAG, "Serveur web démarré avec route de redémarrage.");
    }
    else
    {
        ESP_LOGE(TAG, "Erreur au démarrage du serveur web.");
    }
}

esp_err_t setModeSansWifiHandler(httpd_req_t *req)
{
    char query[64];
    sParametresHorloge params = {0}; // Utilisation de la structure correcte
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        char mode_val[8];
        if (httpd_query_key_value(query, "mode", mode_val, sizeof(mode_val)) == ESP_OK)
        {
            ESP_LOGI(TAG, "Mode reçu depuis la page web : %s", mode_val);
            int mode_int = atoi(mode_val);
            if (mode_int <= MODE_ARRET)
            {
                params.modeActuel = (eModeAffichage)mode_int; // Utilisation de la structure
                ESP_LOGI(TAG, "ModeActuel mis à jour à : %d", params.modeActuel);

                // Envoi de la structure complète dans la queue
                if (xQueueSend(fileParamHorloge, &params, portMAX_DELAY) != pdPASS)
                {
                    ESP_LOGE(TAG, "Erreur d'envoi des paramètres dans la queue fileParamHorloge.");
                }
                else
                {
                    ESP_LOGI(TAG, "Paramètres envoyés dans la queue fileParamHorloge.");
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

void urlDecode(char *src, size_t srcSize)
{
    char *dest = src;
    while (*src && srcSize--)
    {
        if (*src == '%' && src[1] && src[2])
        {
            int value;
            sscanf(src + 1, "%2x", &value);
            *dest++ = (char)value;
            src += 3;
        }
        else if (*src == '+')
        {
            *dest++ = ' ';
            src++;
        }
        else
        {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

esp_err_t setHorlogeSansWifiHandler(httpd_req_t *req)
{
    char query[512];  // Augmentation de la taille du buffer
    sParametresHorloge params = {0};
    params.nbVille = 1;
    params.modeActuel = MODE_HORLOGE;  // Forcer le mode horloge

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        // Extraction de l'heure avec décodage URL
        if (httpd_query_key_value(query, "heure", params.heure, sizeof(params.heure)) == ESP_OK)
        {
            urlDecode(params.heure, sizeof(params.heure)); // Décodage de l'URL
            ESP_LOGI(TAG, "Heure reçue (décodée): %s", params.heure);

            // Extraire l'heure et l'envoyer dans fileHeure
            sTemps heureActuelle = {0};
            if (sscanf(params.heure, "%d:%d", &heureActuelle.heures, &heureActuelle.minutes) == 2)
            {
                heureActuelle.secondes = 0; // Forcer les secondes à 0
                heureActuelle.estVillePrincipale = true;  // Marquer comme ville principale
                if (xQueueSend(fileHeure, &heureActuelle, pdMS_TO_TICKS(10)) != pdPASS)
                {
                    ESP_LOGE(TAG, "Erreur d'envoi de l'heure dans la queue fileHeure.");
                }
                else
                {
                    ESP_LOGI(TAG, "Heure envoyée dans la queue fileHeure : %02d:%02d:%02d", 
                             heureActuelle.heures, heureActuelle.minutes, heureActuelle.secondes);
                }
            }
            else
            {
                ESP_LOGW(TAG, "Format d'heure non valide après décodage : %s", params.heure);
            }
        }
        else
        {
            ESP_LOGW(TAG, "Paramètre 'heure' non reçu.");
        }

        // Extraction des autres paramètres d'affichage
        if (httpd_query_key_value(query, "couleurHeures", params.couleurHeuresActuelles, sizeof(params.couleurHeuresActuelles)) == ESP_OK)
            ESP_LOGI(TAG, "Couleur Heures: %s", params.couleurHeuresActuelles);
        
        if (httpd_query_key_value(query, "couleurMinutes", params.couleurMinutesActuelles, sizeof(params.couleurMinutesActuelles)) == ESP_OK)
            ESP_LOGI(TAG, "Couleur Minutes: %s", params.couleurMinutesActuelles);
        
        if (httpd_query_key_value(query, "couleurSecondes", params.couleurSecondesActuelles, sizeof(params.couleurSecondesActuelles)) == ESP_OK)
            ESP_LOGI(TAG, "Couleur Secondes: %s", params.couleurSecondesActuelles);
        
        if (httpd_query_key_value(query, "affichageTemperature", params.affichageTemperature, sizeof(params.affichageTemperature)) == ESP_OK)
            ESP_LOGI(TAG, "Affichage Temp: %s", params.affichageTemperature);
        
        if (httpd_query_key_value(query, "affichageType", params.affichageType, sizeof(params.affichageType)) == ESP_OK)
            ESP_LOGI(TAG, "Affichage Type: %s", params.affichageType);
        
        // Envoi des paramètres d'affichage dans fileParamHorloge
        if (xQueueSend(fileParamHorloge, &params, pdMS_TO_TICKS(10)) != pdPASS)
        {
            ESP_LOGE(TAG, "Erreur d'envoi des paramètres dans la queue fileParamHorloge.");
        }
        else
        {
            ESP_LOGI(TAG, "Paramètres envoyés dans la queue fileParamHorloge.");
        }

        ESP_LOGI(TAG, "Les paramètres de l'horloge ont été mis à jour.");
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
    // On ignore les paramètres passés et on utilise des valeurs fixes
    const char *ssid_fixe = "IPhoneJosee";  // SSID du hotspot (avec I majuscule)
    const char *password_fixe = "qwertyuiop";     // Mot de passe du hotspot

    // Ne pas arrêter le WiFi, juste configurer le mode station
    wifi_config_t wifi_config_sta = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        }
    };
    strncpy((char *)wifi_config_sta.sta.ssid, ssid_fixe, sizeof(wifi_config_sta.sta.ssid) - 1);
    strncpy((char *)wifi_config_sta.sta.password, password_fixe, sizeof(wifi_config_sta.sta.password) - 1);
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Libérer la mémoire si des paramètres ont été passés
    if (arg != NULL) {
        free(arg);
    }

    ESP_LOGI(TAG, "Tentative de connexion au réseau WiFi: %s", ssid_fixe);
}

esp_err_t setConnectionAuWifiHandler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Démarrage de la connexion WiFi avec les paramètres hardcodés");

    esp_timer_handle_t delay_timer;
    esp_timer_create_args_t timer_args = {
        .callback = (void *)connecterAuWifiStation,
        .arg = NULL,  // Pas besoin de passer les paramètres car ils sont hardcodés
        .dispatch_method = ESP_TIMER_TASK,
        .name = "wifi_reconnect"
    };
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
    char query[512];
    sParametresHorloge params = {0};
    params.nbVille = 2; // Mode deux villes
    params.modeActuel = MODE_HORLOGE;  // Forcer le mode horloge
    strncpy(params.affichageType, "regulier", sizeof(params.affichageType) - 1); // Valeur par défaut

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        char villeActuelle[32], ville2e[32];
        char couleurHeuresActuels[32], couleurMinutesActuels[32], couleurSecondes[32];
        char couleurHeures2e[32], couleurMinutes2e[32];
        char affichageTemperature[8];
        char affichageType[16];

        // Récupération et décodage des paramètres
        if (httpd_query_key_value(query, "villeActuelle", villeActuelle, sizeof(villeActuelle)) == ESP_OK &&
            httpd_query_key_value(query, "ville2e", ville2e, sizeof(ville2e)) == ESP_OK &&
            httpd_query_key_value(query, "couleurHeuresActuels", couleurHeuresActuels, sizeof(couleurHeuresActuels)) == ESP_OK &&
            httpd_query_key_value(query, "couleurMinutesActuels", couleurMinutesActuels, sizeof(couleurMinutesActuels)) == ESP_OK &&
            httpd_query_key_value(query, "couleurSecondes", couleurSecondes, sizeof(couleurSecondes)) == ESP_OK &&
            httpd_query_key_value(query, "couleurHeures2e", couleurHeures2e, sizeof(couleurHeures2e)) == ESP_OK &&
            httpd_query_key_value(query, "couleurMinutes2e", couleurMinutes2e, sizeof(couleurMinutes2e)) == ESP_OK &&
            httpd_query_key_value(query, "affichageTemperature", affichageTemperature, sizeof(affichageTemperature)) == ESP_OK &&
            httpd_query_key_value(query, "affichageType", affichageType, sizeof(affichageType)) == ESP_OK)
        {
            // Décodage URL des valeurs
            urlDecode(villeActuelle, sizeof(villeActuelle));
            urlDecode(ville2e, sizeof(ville2e));
            urlDecode(couleurHeuresActuels, sizeof(couleurHeuresActuels));
            urlDecode(couleurMinutesActuels, sizeof(couleurMinutesActuels));
            urlDecode(couleurSecondes, sizeof(couleurSecondes));
            urlDecode(couleurHeures2e, sizeof(couleurHeures2e));
            urlDecode(couleurMinutes2e, sizeof(couleurMinutes2e));
            urlDecode(affichageType, sizeof(affichageType));

            ESP_LOGI(TAG, "Paramètres reçus - Ville actuelle: %s, Ville 2e: %s", villeActuelle, ville2e);
            ESP_LOGI(TAG, "Couleurs - Heures: %s, Minutes: %s, Secondes: %s", couleurHeuresActuels, couleurMinutesActuels, couleurSecondes);
            ESP_LOGI(TAG, "Couleurs 2e ville - Heures: %s, Minutes: %s", couleurHeures2e, couleurMinutes2e);
            ESP_LOGI(TAG, "Affichage température: %s", affichageTemperature);
            ESP_LOGI(TAG, "Type d'affichage: %s", affichageType);

            // Configuration des paramètres
            strncpy(params.villeActuelle, villeActuelle, sizeof(params.villeActuelle) - 1);
            strncpy(params.ville2e, ville2e, sizeof(params.ville2e) - 1);
            strncpy(params.couleurHeuresActuelles, couleurHeuresActuels, sizeof(params.couleurHeuresActuelles) - 1);
            strncpy(params.couleurMinutesActuelles, couleurMinutesActuels, sizeof(params.couleurMinutesActuelles) - 1);
            strncpy(params.couleurSecondesActuelles, couleurSecondes, sizeof(params.couleurSecondesActuelles) - 1);
            strncpy(params.couleurHeures2e, couleurHeures2e, sizeof(params.couleurHeures2e) - 1);
            strncpy(params.couleurMinutes2e, couleurMinutes2e, sizeof(params.couleurMinutes2e) - 1);
            strncpy(params.affichageTemperature, affichageTemperature, sizeof(params.affichageTemperature) - 1);
            strncpy(params.affichageType, affichageType, sizeof(params.affichageType) - 1);

            // Envoi des paramètres dans la file
            if (xQueueSend(fileParamHorloge, &params, portMAX_DELAY) != pdPASS)
            {
                ESP_LOGE(TAG, "Erreur lors de l'envoi des paramètres dans la file");
                httpd_resp_set_type(req, "text/plain");
                httpd_resp_set_status(req, HTTPD_500);
                httpd_resp_sendstr(req, "Erreur interne du serveur");
                return ESP_FAIL;
            }

            ESP_LOGI(TAG, "Paramètres envoyés avec succès dans la file");
        }
        else
        {
            ESP_LOGE(TAG, "Erreur lors de la récupération des paramètres");
            httpd_resp_set_type(req, "text/plain");
            httpd_resp_set_status(req, HTTPD_400);
            httpd_resp_sendstr(req, "Requête invalide");
            return ESP_FAIL;
        }
    }
    else
    {
        ESP_LOGE(TAG, "Erreur lors de la récupération de la query string");
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_set_status(req, HTTPD_400);
        httpd_resp_sendstr(req, "Requête invalide");
        return ESP_FAIL;
    }

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

esp_err_t redemarrerSystemeHandler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Handler de redémarrage appelé");

    // Envoyer la réponse HTTP immédiatement
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "Redémarrage en cours...");
    
    // Petit délai pour s'assurer que la réponse est envoyée
    vTaskDelay(pdMS_TO_TICKS(100));

    // Arrêter tous les processus immédiatement
    vTaskSuspendAll();
    
    // Désactiver les interruptions
    portDISABLE_INTERRUPTS();
    
    // Forcer le redémarrage matériel
    esp_system_abort("Redémarrage forcé");

    // Ces lignes ne seront jamais exécutées
    return ESP_OK;
}