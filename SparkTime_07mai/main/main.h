#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>  
#include "piloteNeopixel.h"  

#define NP_SEC_GPIO 14
#define NP_SEC_COUNT 60 
#define NP_MIN_HRS_GPIO 25
#define NP_MIN_HRS_COUNT 120
#define ENABLE_SEC 27
#define ENABLE_MIN_HRS 26

typedef struct sNeopixelContexts {
    neopixel_t *npCtxSec;
    neopixel_t *npCtxMinHrs;
} sNeopixelContexts;

typedef struct {
    int heures;
    int minutes;
    int secondes;
    bool estVillePrincipale;  // true pour la ville actuelle, false pour la 2e ville
} sTemps;

typedef enum
{
    MODE_HORLOGE,
    MODE_TEMPERATURE,
    MODE_TEST,
    MODE_ARCENCIEL,
    MODE_ARRET
} eModeAffichage;

typedef struct
{
    eModeAffichage modeActuel;
    int nbVille;
    char heure[16];
    char villeActuelle[16];
    char ville2e[16];
    char couleurHeuresActuelles[16];
    char couleurMinutesActuelles[16];
    char couleurSecondesActuelles[16];
    char couleurHeures2e[16];
    char couleurMinutes2e[16];
    char affichageTemperature[8];
    char affichageType[16];
} sParametresHorloge;

#endif
