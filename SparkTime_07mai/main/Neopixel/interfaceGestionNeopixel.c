/*
    Nom: interfaceGestionNeopixel

    Description: Interface pour la gestion des Neopixels qui utilise la librairie de Neopixel de Zroxx.
                 Seulement pour les Neopixels RGB (et non les RGBW).

    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard
*/

#include "interfaceGestionNeopixel.h"
#include <stdio.h>

const uint32_t couleurPixel[NOMBRE_COULEURS][NOMBRE_NIVEAUX] = {
    {NP_RGB(10, 0, 0), NP_RGB(128, 0, 0), NP_RGB(255, 0, 0)},       // rouge
    {NP_RGB(12, 3, 0), NP_RGB(128, 64, 0), NP_RGB(255, 128, 0)},    // orange
    {NP_RGB(11, 9, 0), NP_RGB(128, 128, 0), NP_RGB(255, 255, 0)},   // jaune
    {NP_RGB(0, 10, 0), NP_RGB(0, 128, 0), NP_RGB(0, 255, 0)},       // vert
    {NP_RGB(0, 0, 10), NP_RGB(0, 0, 128), NP_RGB(0, 0, 255)},       // bleu
    {NP_RGB(3, 0, 12), NP_RGB(64, 0, 128), NP_RGB(128, 0, 255)},    // indigo
    {NP_RGB(5, 0, 5), NP_RGB(128, 0, 128), NP_RGB(255, 0, 255)},    // violet
    {NP_RGB(5, 5, 5), NP_RGB(128, 128, 128), NP_RGB(254, 254, 254)} // blanc
};

const int countCouleurPixel = sizeof(couleurPixel) / sizeof(couleurPixel[0]);

tNeopixelContext neopixel_InitInterface(int32_t pixels, int gpio_data)
{
    return (tNeopixelContext *)neopixel_Init(pixels, gpio_data);
}

bool neopixel_setPixelInterface(tNeopixelContext context, tNeopixel *pixel, int32_t pixelCount)
{
    return neopixel_SetPixel(context, pixel, pixelCount);
}

void neopixel_deinitInterface(tNeopixelContext context)
{
    neopixel_Deinit(context);
}
