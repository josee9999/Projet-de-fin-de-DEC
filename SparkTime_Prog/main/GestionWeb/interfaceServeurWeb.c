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

static const char *TAG = "ServeurWeb";

esp_err_t set_mode_handler(httpd_req_t *req)
{
    char query[64];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK)
    {
        char mode_val[8];
        if (httpd_query_key_value(query, "mode", mode_val, sizeof(mode_val)) == ESP_OK)
        {
            ESP_LOGI(TAG, "Mode reçu depuis la page web : %s", mode_val);
            int mode_int = atoi(mode_val);
            if (mode_int >= MODE_HORLOGE && mode_int <= MODE_ARRET)
            {
                modeActuel = (eModeAffichage)mode_int;
                ESP_LOGI(TAG, "ModeActuel mis à jour à : %d", modeActuel);

                // Envoie le mode dans la Queue
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

esp_err_t page_racine_handler(httpd_req_t *req)
{
    const char *pageAcceuil =
        "<html>"
        "<head>"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
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
        "  box-shadow: 0 0 10px rgba(0,0,0,0.2);"
        "  width: 90%;"
        "  max-width: 400px;"
        "  text-align: center;"
        "}"
        "h1, h2 {"
        "  font-size: 1.4em;"
        "  margin-bottom: 0.5em;"
        "}"
        "label {"
        "  font-size: 1.2em;"
        "  display: block;"
        "  margin: 1em 0 0.5em;"
        "}"
        "select, input[type=submit] {"
        "  width: 100%;"
        "  padding: 0.8em;"
        "  font-size: 1em;"
        "  margin-bottom: 1em;"
        "  border: 1px solid #ccc;"
        "  border-radius: 5px;"
        "}"
        "@media (max-width: 480px) {"
        "  form {"
        "    width: 100vw;"
        "    max-width: none;"
        "    padding: 1.5em;"
        "    margin: 0;"
        "  }"
        "  select, input[type=submit] {"
        "    font-size: 0.9em;"
        "    padding: 0.7em;"
        "  }"
        "}"
        "</style>"
        "</head>"
        "<body>"
        "<form id=\"form\" action=\"/set_mode\" method=\"GET\">"
        "<h1>Bienvenue sur l'horloge</h1>"
        "<h2>Page de tests</h2>"
        "<label for=\"mode\">Selectionner le mode :</label><br>"
        "<select id=\"mode\" name=\"mode\">"
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
        "  xhr.open('GET', '/set_mode?mode=' + mode, true);"
        "  xhr.send();"
        "};"
        "</script>"
        "</body>"
        "</html>";
    httpd_resp_send(req, pageAcceuil, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void demarrer_serveur_web(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t serveur = NULL;
    if (httpd_start(&serveur, &config) == ESP_OK)
    {
        httpd_uri_t uriRacine = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = page_racine_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriRacine);

        httpd_uri_t uriSetMode = {
            .uri = "/set_mode",
            .method = HTTP_GET,
            .handler = set_mode_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(serveur, &uriSetMode);

        ESP_LOGI(TAG, "Serveur web démarré.");
    }
    else
    {
        ESP_LOGE(TAG, "Erreur au démarrage du serveur web.");
    }
}