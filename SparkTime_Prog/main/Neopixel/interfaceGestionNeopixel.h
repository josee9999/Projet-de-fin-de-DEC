/*
    Nom: interfaceGestionNeopixel

    Description: Interface pour la gestion des Neopixels qui utilise la librairie de Neopixel de Zroxx.
                 Seulement pour les Neopixels RGB (et non les RGBW).

    Modification: -
    Création: 25/04/2025
    Auteur: Josée Girard   
*/

#ifndef INTERFACEGESTIONNEOPIXEL_H
#define INTERFACEGESTIONNEOPIXEL_H

#include "neopixel.h"

#define NP_SEC_GPIO 25
#define NP_SEC_COUNT 2 //POUR TEST: A CHANGER POUR 60 
#define NP_MIN_HRS_GPIO 14
#define NP_MIN_HRS_COUNT 120

extern const uint32_t couleurPixel[];
extern const int countCouleurPixel;

#define COULEUR_ROUGE_PALE 0
#define COULEUR_ORANGE_PALE 1
#define COULEUR_JAUNE_PALE 2
#define COULEUR_VERT_PALE 3
#define COULEUR_BLEU_PALE 4
#define COULEUR_INDIGO_PALE 5
#define COULEUR_VIOLET_PALE 6

tNeopixelContext neopixel_InitInterface(int32_t pixels, int gpio_data);
bool neopixel_setPixelInterface(tNeopixelContext context, tNeopixel* pixels, int32_t pixelCount);
void neopixel_deinitInterface(tNeopixelContext context);

#endif