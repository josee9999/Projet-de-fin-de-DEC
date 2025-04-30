/*
    Nom: 

    Description: 
                 
    Modification: -
    Création: 29/04/2025
    Auteur: Josée Girard   
*/

/*#include "interfaceServeurWeb.h"
#include "piloteServeurWeb.h"
#include "esp_http_server.h"

httpd_handle_t  serveur = NULL;

esp_err_t demarrerServeurWeb(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    esp_err_t err = httpd_start(&serveur, &config);

    if(err == ESP_OK)
    {
        httpd_uri_t pageConnection =  
        {
            .uri = "/connection",
            .method = HTTP_GET,
            .handler = handlerPageConnection,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(&serveur, &pageConnection);
    }
    return err;
}*/