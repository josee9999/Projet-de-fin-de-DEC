/*
    Nom: 

    Description: 
                 
    Modification: -
    Création: 29/04/2025
    Auteur: Josée Girard   
*/

/*#include "esp_http_server.h"
#include "interfaceServeurWeb.h"

volatile eModeAffichage modeActuel = MODE_ARRET;

esp_err_t handlerPageConnection(httpd_req_t *req) //A CHANGER: Code pour test
{
    // Récupérer la requête GET et le paramètre "mode"
    char* mode = NULL;
    size_t len = httpd_req_get_url_query_len(req) + 1; // Inclut le '\0'
    if (len > 1) {
        mode = malloc(len);
        if (httpd_req_get_url_query_str(req, mode, len) == ESP_OK) {
            // Recherche du paramètre "mode" dans la requête
            char param[10];
            if (httpd_query_key_value(mode, "mode", param, sizeof(param)) == ESP_OK) {
                // Convertir la valeur du paramètre "mode" en entier
                modeActuel = atoi(param);  // Mise à jour de modeActuel
            }
        }
        free(mode);
    }
    const char *pageConnection = 
        "<html>"
        "<body>"
        "<h1>Mode Arc-en-ciel</h1>"
        "<form action=\"/set_mode\" method=\"GET\">"
        "<label for=\"mode\">Selectionner le mode:</label>"
        "<select id=\"mode\" name=\"mode\">"
        "<option value =\"0\">1 arc-en-ciel</option>"
        "<option value =\"1\">rien</option>"
        "</select><br><br>"
        "<input type=\"submit\" value=\"GO\">"
        "</form>"
        "</body>"
        "</html>";
        
    httpd_resp_send(req, pageConnection, HTTPD_RESP_USE_STRLEN);   
    return ESP_OK; 
}

esp_err_t handlerPageSansWifi(httpd_req_t *req) 
{
    httpd_resp_send(req, "Page sans WiFi (pas encore implémentée)", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t handlerPageAvecWifi(httpd_req_t *req) 
{
    httpd_resp_send(req, "Page avec WiFi (pas encore implémentée)", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}*/

#include "interfaceServeurWeb.h"
#include "esp_log.h"
#include "esp_http_server.h"

static const char *TAG = "ServeurWeb";

esp_err_t page_racine_handler(httpd_req_t *req) {
    const char* reponse = "<html><body><h1>Bienvenue sur l'horloge</h1></body></html>";
    httpd_resp_send(req, reponse, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void demarrer_serveur_web(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t serveur = NULL;
    if (httpd_start(&serveur, &config) == ESP_OK) {
        httpd_uri_t uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = page_racine_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(serveur, &uri);
        ESP_LOGI(TAG, "Serveur web démarré.");
    } else {
        ESP_LOGE(TAG, "Erreur au démarrage du serveur web.");
    }
}