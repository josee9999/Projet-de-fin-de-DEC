
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

#include <stdint.h>
#include <stdbool.h>
#include "piloteNeopixel.h"

#define NOMBRE_COULEURS 13
#define NOMBRE_NIVEAUX 3

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} tNeopixel;

extern const tNeopixel couleurPixel[NOMBRE_COULEURS][NOMBRE_NIVEAUX];
extern const int countCouleurPixel;

typedef enum {
    COULEUR_ROUGE,
    COULEUR_ORANGE,
    COULEUR_JAUNE,
    COULEUR_JAUNE_VERT,
    COULEUR_VERT,
    COULEUR_VERT_BLEU,
    COULEUR_TURQUOISE,
    COULEUR_BLEU,
    COULEUR_INDIGO,
    COULEUR_VIOLET,
    COULEUR_MAUVE,
    COULEUR_ROSE,
    COULEUR_BLANC
} eCouleurNom;

typedef enum {
    NIVEAU_PALE,
    NIVEAU_MOYEN,
    NIVEAU_VIF
} eLuminosite;

typedef void *tNeopixelContext;

struct sNeopixelContexts;

tNeopixelContext neopixel_InitInterface(int32_t pixels, int gpio_data);
bool neopixel_setPixelInterface(tNeopixelContext context, const tNeopixel *pixels, int32_t pixelCount);
bool neopixel_setPixelSimpleInterface(tNeopixelContext context, int32_t pixelIndex, uint8_t r, uint8_t g, uint8_t b);
void neopixel_deinitInterface(tNeopixelContext context);
void neopixel_clearInterface(tNeopixelContext context);
void interfaceNeopixel_setToutEteint(struct sNeopixelContexts *npContexts);
void neopixel_showInterface(tNeopixelContext context);
void neopixel_clear(tNeopixelContext ctx);

#endif