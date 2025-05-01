/*
    Nom:

    Description:

    Modification: -
    Création: 29/04/2025
    Auteur: Josée Girard
*/

#include "interfaceServeurWeb.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "processusAffichageNeopixel.h"
#include <stdio.h>

static const char *TAG = "ServeurWeb";

const char *styleCSS =
   "<style>"
    "html, body {"
    "  height: 100%;"
    "  margin: 0;"
    "  padding: 0;"
    "  overflow: hidden;"
    "  font-family: Arial, sans-serif;"
    "  background-color: #f0f0f0;"
    "  display: flex;"
    "  justify-content: center;"
    "  align-items: center;"
    "}"
    "form {"
    "  background-color: white;"
    "  padding: 2em;"
    "  border-radius: 10px;"
    "  box-shadow: 0 0 10px rgba(0, 0, 0, 0.2);"
    "  width: 90%;"
    "  max-width: 400px;"
    "  text-align: center;"
    "}"
    "h1 {"
    "  font-size: 1.4em;"
    "  margin-bottom: 0.5em;"
    "}"
    "h2 {"
    "  font-size: 1.2em;"
    "  margin-bottom: 0.5em;"
    "}"
    "select {"
    "  width: 100%;"  
    "  padding: 0.8em;"
    "  font-size: 1.2em;"
    "  margin-bottom: 1em;"
    "  border: 1px solid #ccc;"
    "  border-radius: 5px;"
    "  background-color: white;" 
    "}"
    "input[type=button], input[type=submit] {"
    "  width: 100%;"
    "  padding: 0.8em;"
    "  font-size: 1.2em;"
    "  margin-bottom: 1em;"
    "  border: none;"
    "  border-radius: 5px;"
    "  background-color:rgb(52, 147, 235);"
    "  color: white;"
    "  cursor: pointer;"
    "}"
    "input[type=button]:hover, input[type=submit]:hover {"
    "  background-color:rgb(3, 109, 186);"  
    "}"
    "input[type=button]:focus, input[type=submit]:focus {"
    "  outline: none;"
    "@media (max-width: 480px) {"
    "  form {"
    "    width: 100vw;"
    "    max-width: none;"
    "    padding: 1.5em;"
    "    margin: 0;"
    "  }"
    "  input[type=button] {"
    "    font-size: 1em;"
    "    padding: 0.7em;"
    "  }"
    "}"
    "</style>";

esp_err_t pageAccueilHandler(httpd_req_t *req)
{
    char *pageAccueil = malloc(8192);
    if (pageAccueil == NULL)
    {
        ESP_LOGE(TAG, "Erreur d'allocation de mémoire pour la page d'accueil");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    snprintf(pageAccueil, 8192,
             "<html>"
             "<head>"
             "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
             "%s"
             "</head>"
             "<body>"
             "<div class=\"form-container\">"
             "<h1>SparkTime - Connexion</h1>"
             "<h2>TODO: connexion au wifi</h2>"
             "<input type=\"button\" value=\"Connexion avec Wi-Fi\" onclick=\"location.href='/pageWifi'\">"
             "<input type=\"button\" value=\"Connexion sans Wi-Fi\" onclick=\"location.href='/pageSansWifi'\">"
             "</div>"
             "</body>"
             "</html>",
             styleCSS);
    httpd_resp_send(req, pageAccueil, HTTPD_RESP_USE_STRLEN);
    free(pageAccueil);
    return ESP_OK;
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
    // TODO
    return ESP_OK;
}

esp_err_t pagePersonnalisationSansWifiHandler(httpd_req_t *req)
{
    char *pageSansWifi = malloc(8192);
    if (pageSansWifi == NULL)
    {
        ESP_LOGE(TAG, "Erreur d'allocation de mémoire pour la page sans wifi");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    snprintf(pageSansWifi, 8192,
             "<html>"
             "<head>"
             "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
             "%s"
             "</head>"
             "<body>"
             "<form id=\"form\" action=\"/setModeSansWifi\" method=\"GET\">"
             "<h1>SparkTime - version sans wifi</h1>"
             "<h2>Tests</h2>"
             "<label for=\"mode\">Selectionner le mode :</label><br>"
             "<select id=\"mode\" name=\"mode\">"
             "<option value=\"2\">test de leds</option>"
             "<option value=\"3\">arc-en-ciel</option>"
             "<option value=\"4\">rien</option>"
             "</select><br>"
             "<input type=\"submit\" value=\"GO\">"
             "</form>"
             "<script>"
             "document.getElementById('form').onsubmit = function(event) {"
             "  event.preventDefault();"
             "  var xhr = new XMLHttpRequest();"
             "  var mode = document.getElementById('mode').value;"
             "  xhr.open('GET', '/setModeSansWifi?mode=' + mode, true);"
             "  xhr.send();"
             "};"
             "</script>"
             "</body>"
             "</html>",
             styleCSS);
    httpd_resp_send(req, pageSansWifi, HTTPD_RESP_USE_STRLEN);
    free(pageSansWifi);
    return ESP_OK;
}

esp_err_t setModeAvecWifiHandler(httpd_req_t *req)
{
    // TODO
    return ESP_OK;
}

esp_err_t setHorlogeAvecWifiHandler(httpd_req_t *req)
{
    // TODO
    return ESP_OK;
}

esp_err_t pagePersonnalisationAvecWifiHandler(httpd_req_t *req)
{
    // TODO
    return ESP_OK;
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

        ESP_LOGI(TAG, "Serveur web démarré.");
    }
    else
    {
        ESP_LOGE(TAG, "Erreur au démarrage du serveur web.");
    }
}
