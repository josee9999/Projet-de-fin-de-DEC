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

const uint32_t couleurPixel[]={
    NP_RGB(10,0,0), //rouge
    NP_RGB(10,5,0),//orange
    NP_RGB(10,10,0),//jaune
    NP_RGB(0,10,0),//vert
    NP_RGB(0,0,10),//bleu
    NP_RGB(5,0,10),//indigo
    NP_RGB(5,0,5),//violet
};

const int countCouleurPixel = sizeof(couleurPixel)/ sizeof(couleurPixel[0]);

tNeopixelContext neopixel_InitInterface(int32_t pixels, int gpio_data)
{
    return (tNeopixelContext *)neopixel_Init(pixels, gpio_data);
}

bool neopixel_setPixelInterface(tNeopixelContext context, tNeopixel* pixel, int32_t pixelCount)
{
    return neopixel_SetPixel(context, pixel, pixelCount);
}

void neopixel_deinitInterface(tNeopixelContext context)
{
    neopixel_Deinit(context);
}


