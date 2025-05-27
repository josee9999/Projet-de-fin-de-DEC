
/*
    Nom: interfaceGestionNeopixel

    Description: Interface pour la gestion des Neopixels qui utilise la librairie de Neopixel de Zroxx.
                 Seulement pour les Neopixels RGB (et non les RGBW).

    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard
*/
#include "interfaceGestionNeopixel.h"
#include "piloteNeopixel.h"
#include "main.h"        // Pour sNeopixelContexts
#include "esp_log.h"
#include <stdlib.h>


static const char *TAG = "Neopixel_Interface";

// Configuration des couleurs
const tNeopixel couleurPixel[NOMBRE_COULEURS][NOMBRE_NIVEAUX] = {
    {{5, 0, 0}, {127, 0, 0}, {255, 0, 0}},        // Rouge
    {{5, 2, 0}, {127, 51, 0}, {255, 102, 0}},     // Orange
    {{5, 5, 0}, {127, 127, 0}, {255, 255, 0}},    // Jaune
    {{2, 5, 0}, {51, 127, 0}, {102, 255, 0}},     // Jaune-vert
    {{0, 5, 0}, {0, 127, 0}, {0, 255, 0}},        // Vert
    {{0, 5, 2}, {0, 127, 51}, {0, 255, 102}},     // Vert-bleu
    {{0, 5, 5}, {0, 127, 127}, {0, 255, 255}},    // Turquoise
    {{0, 2, 5}, {0, 51, 127}, {0, 102, 255}},     // Bleu
    {{0, 0, 5}, {0, 0, 127}, {0, 0, 255}},        // Indigo
    {{2, 0, 5}, {51, 0, 127}, {102, 0, 255}},     // Violet
    {{5, 0, 5}, {127, 0, 127}, {255, 0, 255}},    // Mauve
    {{5, 0, 2}, {127, 0, 51}, {255, 0, 102}},     // Rose
    {{5, 5, 5}, {127, 127, 127}, {255, 255, 255}} // Blanc
};

const int countCouleurPixel = NOMBRE_COULEURS;

typedef struct
{
    neopixel_t strip;  // Mettre la structure en premier pour un meilleur alignement
    int32_t nbPixels;
    int gpio;
} tNeopixelContextStruct;

tNeopixelContext neopixel_InitInterface(int32_t pixels, int gpio_data)
{
    tNeopixelContextStruct *ctx = malloc(sizeof(tNeopixelContextStruct));
    if (!ctx)
    {
        ESP_LOGE(TAG, "Échec de l'allocation du contexte Neopixel");
        return NULL;
    }

    // Choix du canal RMT en fonction du GPIO
    int rmt_channel;
    if (gpio_data == NP_SEC_GPIO) {
        rmt_channel = 0;  // Canal 0 pour les secondes
    } else if (gpio_data == NP_MIN_HRS_GPIO) {
        rmt_channel = 1;  // Canal 1 pour les minutes/heures
    } else {
        ESP_LOGE(TAG, "GPIO non reconnu: %d", gpio_data);
        free(ctx);
        return NULL;
    }

    // Initialisation de la bande de Neopixels
    if (initialiserNeopixel(&ctx->strip, gpio_data, pixels, rmt_channel, LED_TYPE_WS2812B) != 0)
    {
        ESP_LOGE(TAG, "Échec de l'initialisation des Neopixels");
        free(ctx);
        return NULL;
    }

    ctx->nbPixels = pixels;
    ctx->gpio = gpio_data;

    ESP_LOGI(TAG, "Neopixels initialisés avec succès (GPIO %d, %d LEDs, canal RMT %d)", (int)gpio_data, (int)pixels, rmt_channel);
    return (tNeopixelContext)ctx;
}

bool neopixel_setPixelInterface(tNeopixelContext context, const tNeopixel *pixels, int32_t pixelCount)
{
    if (!context || !pixels)
    {
        ESP_LOGE(TAG, "Contexte ou pixels non définis");
        return false;
    }

    tNeopixelContextStruct *ctx = (tNeopixelContextStruct *)context;

    for (int i = 0; i < pixelCount; i++)
    {
        mettreCouleurNeopixel(&ctx->strip, i, pixels[i].r, pixels[i].g, pixels[i].b);
    }

    afficherNeopixel(&ctx->strip);
    return true;
}

bool neopixel_setPixelSimpleInterface(tNeopixelContext context, int32_t pixelIndex, uint8_t r, uint8_t g, uint8_t b)
{
    if (!context)
        return false;

    tNeopixelContextStruct *ctx = (tNeopixelContextStruct *)context;

    mettreCouleurNeopixel(&ctx->strip, pixelIndex, r, g, b);
    // Pas d'affichage immédiat ici, on peut appeler afficherNeopixel après plusieurs appels
    return true;
}


void neopixel_deinitInterface(tNeopixelContext context)
{
    if (!context)
        return;

    tNeopixelContextStruct *ctx = (tNeopixelContextStruct *)context;

    eteindreNeopixel(&ctx->strip);
    free(ctx);
    ESP_LOGI(TAG, "Neopixels désinitialisés");
}

void neopixel_setPixel(tNeopixelContext ctx, int index, uint8_t r, uint8_t g, uint8_t b)
{
    if (!ctx) return;
    tNeopixelContextStruct *context = (tNeopixelContextStruct *)ctx;
    if (index < 0 || index >= context->nbPixels) return;

    mettreCouleurNeopixel(&context->strip, index, r, g, b);
}

void neopixel_show(tNeopixelContext ctx)
{
    if (!ctx) return;
    tNeopixelContextStruct *context = (tNeopixelContextStruct *)ctx;
    afficherNeopixel(&context->strip);
}

void neopixel_clear(tNeopixelContext ctx)
{
    if (!ctx) return;
    tNeopixelContextStruct *context = (tNeopixelContextStruct *)ctx;
    for (int i = 0; i < context->nbPixels; i++)
        mettreCouleurNeopixel(&context->strip, i, 0, 0, 0);
    afficherNeopixel(&context->strip);
}

void neopixel_clearInterface(tNeopixelContext context)
{
    if (!context)
        return;

    tNeopixelContextStruct *ctx = (tNeopixelContextStruct *)context;

    for (int i = 0; i < ctx->nbPixels; i++)
    {
        mettreCouleurNeopixel(&ctx->strip, i, 0, 0, 0);
    }

    afficherNeopixel(&ctx->strip);
}

void interfaceNeopixel_setToutEteint(sNeopixelContexts *npContexts)
{
    if (npContexts == NULL)
        return;

    neopixel_clearInterface(npContexts->npCtxSec);
    neopixel_clearInterface(npContexts->npCtxMinHrs);
}

void neopixel_showInterface(tNeopixelContext context)
{
    neopixel_show(context);
}