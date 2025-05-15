#ifndef MAIN_H
#define MAIN_H
#include "Neopixel/interfaceGestionNeopixel.h"

#define NP_SEC_GPIO 14
#define NP_SEC_COUNT 2 //POUR TEST: A CHANGER POUR 60 
#define NP_MIN_HRS_GPIO 25
#define NP_MIN_HRS_COUNT 2 //POUR TEST :A CHANGER POUR 120
#define ENABLE_SEC 27
#define ENABLE_MIN_HRS 26

typedef struct {
    tNeopixelContext *npCtxSec;
    tNeopixelContext *npCtxMinHrs;
} sNeopixelContexts;

#endif