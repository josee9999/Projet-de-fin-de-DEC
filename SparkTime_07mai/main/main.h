#ifndef MAIN_H
#define MAIN_H

#include "piloteNeopixel.h"

#define NP_SEC_GPIO 14
#define NP_SEC_COUNT 60 
#define NP_MIN_HRS_GPIO 25
#define NP_MIN_HRS_COUNT 120
#define ENABLE_SEC 27
#define ENABLE_MIN_HRS 26

//typedef void* tNeopixelContext;

typedef struct sNeopixelContexts {
    neopixel_t *npCtxSec;
neopixel_t *npCtxMinHrs;
   
} sNeopixelContexts;

#endif