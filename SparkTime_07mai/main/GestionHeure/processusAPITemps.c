/*
    Nom: processusAPITemps

    Description: Gere la communication avec l'API de fuseau horaire pour 
                 requetes et récupération d'informations.
                 
    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard   
*/

#include "processusAPITemps.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "ProcessusAPITemps";
static const char *BASE_URL_TIMEZONE = "http://worldtimeapi.org/api/timezone/";
static const char *BASE_URL_IP = "http://worldtimeapi.org/api/ip";
static bool est_ville_principale;  // Variable globale pour suivre le contexte

// Déclaration anticipée du gestionnaire d'événements
static esp_err_t http_event_handler(esp_http_client_event_t *evt);

// Structure pour stocker les données de temps reçues de l'API
typedef struct {
    int heures;
    int minutes;
    int secondes;
    char timezone[64];
    bool estVillePrincipale;
} sTempsAPI;

// Déclarations des prototypes de fonctions
esp_err_t obtenirHeureIP(void);
esp_err_t obtenirHeureVille(const char *timezone, bool est_principale);

// Implémentation des fonctions
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;
    static int output_len;

    switch(evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "Réception de données de l'API");
            if (!esp_http_client_is_chunked_response(evt->client)) {
                if (output_buffer == NULL) {
                    output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                    output_len = 0;
                    if (output_buffer == NULL) {
                        ESP_LOGE(TAG, "Échec d'allocation mémoire pour le buffer");
                        return ESP_FAIL;
                    }
                }
                memcpy(output_buffer + output_len, evt->data, evt->data_len);
                output_len += evt->data_len;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "Fin de la réception des données");
            if (output_buffer != NULL) {
                ESP_LOGI(TAG, "Réponse reçue: %.*s", output_len, output_buffer);
                
                // Parse JSON response
                cJSON *root = cJSON_Parse(output_buffer);
                if (root != NULL) {
                    cJSON *datetime = cJSON_GetObjectItem(root, "datetime");
                    cJSON *timezone = cJSON_GetObjectItem(root, "timezone");
                    
                    if (datetime != NULL && datetime->valuestring != NULL) {
                        // Format attendu: "2025-05-23T15:02:05.204717-04:00"
                        char time_str[9];
                        strncpy(time_str, datetime->valuestring + 11, 8);
                        time_str[8] = '\0';
                        
                        sTempsAPI temps = {0};
                        sscanf(time_str, "%d:%d:%d", &temps.heures, &temps.minutes, &temps.secondes);
                        temps.estVillePrincipale = est_ville_principale;
                        
                        if (timezone != NULL && timezone->valuestring != NULL) {
                            strncpy(temps.timezone, timezone->valuestring, sizeof(temps.timezone) - 1);
                            temps.timezone[sizeof(temps.timezone) - 1] = '\0';
                            ESP_LOGI(TAG, "Heure extraite: %02d:%02d:%02d, Fuseau horaire: %s (Ville %s)", 
                                    temps.heures, temps.minutes, temps.secondes, timezone->valuestring,
                                    temps.estVillePrincipale ? "principale" : "secondaire");
                        }
                        
                        // Envoyer l'heure dans la file
                        if (xQueueSend(fileHeure, &temps, pdMS_TO_TICKS(100)) != pdPASS) {
                            ESP_LOGE(TAG, "Erreur d'envoi de l'heure dans la file");
                        } else {
                            ESP_LOGI(TAG, "Heure envoyée avec succès dans la file");
                        }
                    } else {
                        ESP_LOGE(TAG, "Champ datetime non trouvé dans la réponse JSON");
                    }
                    cJSON_Delete(root);
                } else {
                    ESP_LOGE(TAG, "Erreur de parsing JSON");
                }
                free(output_buffer);
                output_buffer = NULL;
                output_len = 0;
            }
            break;
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "Erreur HTTP pendant la communication avec l'API");
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
                output_len = 0;
            }
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "Connexion établie avec l'API");
            break;
        case HTTP_EVENT_HEADERS_SENT:
            ESP_LOGI(TAG, "En-têtes HTTP envoyés");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Déconnexion de l'API");
            break;
        default:
            break;
    }
    return ESP_OK;
}

esp_err_t obtenirHeureIP(void)
{
    ESP_LOGI(TAG, "Tentative d'obtention de l'heure via IP: %s", BASE_URL_IP);
    est_ville_principale = true;  // Par défaut, l'IP est considérée comme ville principale
    
    esp_http_client_config_t config = {
        .url = BASE_URL_IP,
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_GET,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Échec d'initialisation du client HTTP pour l'API IP");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Échec de la requête HTTP IP: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Requête HTTP IP effectuée avec succès");
    }

    esp_http_client_cleanup(client);
    return err;
}

esp_err_t obtenirHeureVille(const char *timezone, bool est_principale)
{
    if (!timezone) {
        ESP_LOGE(TAG, "Timezone invalide");
        return ESP_FAIL;
    }

    est_ville_principale = est_principale;
    
    if (strcmp(timezone, "auto") == 0) {
        ESP_LOGI(TAG, "Mode automatique détecté, utilisation de l'API IP");
        return obtenirHeureIP();
    }

    char url[128];
    snprintf(url, sizeof(url), "%s%s", BASE_URL_TIMEZONE, timezone);
    ESP_LOGI(TAG, "Tentative d'obtention de l'heure pour le fuseau horaire: %s (ville %s)", 
             url, est_principale ? "principale" : "secondaire");

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_GET,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Échec d'initialisation du client HTTP");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Échec de la requête HTTP: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

void task_APITemps(void *pvParameters)
{
    sParametresHorloge params;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(60000); // Mise à jour toutes les minutes

    ESP_LOGI(TAG, "Démarrage de la tâche API Temps");

    while(1) {
        xLastWakeTime = xTaskGetTickCount();

        // Attendre les paramètres de l'horloge
        ESP_LOGI(TAG, "Attente des paramètres de l'horloge...");
        if (xQueueReceive(fileParamHorloge, &params, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "Paramètres reçus - Ville actuelle: %s, Nombre de villes: %d", 
                     params.villeActuelle, params.nbVille);

            // Obtenir l'heure pour la ville actuelle
            ESP_LOGI(TAG, "Requête de l'heure pour la ville actuelle");
            if (obtenirHeureVille(params.villeActuelle, true) != ESP_OK) {
                ESP_LOGE(TAG, "Erreur lors de l'obtention de l'heure pour %s", params.villeActuelle);
            }

            // Si en mode deux villes, obtenir l'heure pour la deuxième ville
            if (params.nbVille == 2) {
                ESP_LOGI(TAG, "Requête de l'heure pour la deuxième ville: %s", params.ville2e);
                if (obtenirHeureVille(params.ville2e, false) != ESP_OK) {
                    ESP_LOGE(TAG, "Erreur lors de l'obtention de l'heure pour %s", params.ville2e);
                }
            }
        }

        // Attendre jusqu'à la prochaine mise à jour
        ESP_LOGI(TAG, "Attente jusqu'à la prochaine mise à jour...");
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}