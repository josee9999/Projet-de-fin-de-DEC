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

/*#define NP_SEC_GPIO 25
#define NP_SEC_COUNT 2 //POUR TEST: A CHANGER POUR 60 
#define NP_MIN_HRS_GPIO 14
#define NP_MIN_HRS_COUNT 120*/

#define NOMBRE_COULEURS 8
#define NOMBRE_NIVEAUX 3

extern const uint32_t couleurPixel[NOMBRE_COULEURS][NOMBRE_NIVEAUX];
extern const int countCouleurPixel;

typedef enum {
    COULEUR_ROUGE,
    COULEUR_ORANGE,
    COULEUR_JAUNE,
    COULEUR_VERT,
    COULEUR_BLEU,
    COULEUR_INDIGO,
    COULEUR_VIOLET,
    COULEUR_BLANC
} eCouleurNom;

typedef enum {
    NIVEAU_PALE,
    NIVEAU_MOYEN,
    NIVEAU_VIF
} eLuminosite;

tNeopixelContext neopixel_InitInterface(int32_t pixels, int gpio_data);
bool neopixel_setPixelInterface(tNeopixelContext context, tNeopixel* pixels, int32_t pixelCount);
void neopixel_deinitInterface(tNeopixelContext context);

#endif