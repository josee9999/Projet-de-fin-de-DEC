#ifndef PILOTENEOPIXEL_H
#define PILOTENEOPIXEL_H

#include "driver/rmt.h"

typedef enum {
    LED_TYPE_WS2812B,
    LED_TYPE_SK6812MINI
} led_type_t;

typedef struct {
    int gpio_num;
    int num_leds;
    int rmt_channel;
    led_type_t led_type;
    uint8_t *buffer;  // données RGB (G,R,B)
    rmt_item32_t *rmt_items;
} neopixel_t;

int initialiserNeopixel(neopixel_t *strip, int gpio_num, int num_leds, int rmt_channel, led_type_t led_type);

void mettreCouleurNeopixel(neopixel_t *strip, int index, uint8_t r, uint8_t g, uint8_t b);
void afficherNeopixel(neopixel_t *strip);
void eteindreNeopixel(neopixel_t *strip);
void copierCouleurNeopixel(neopixel_t *strip, int srcIndex, int dstIndex);
void lireCouleurNeopixel(neopixel_t *strip, int index, uint8_t *r, uint8_t *g, uint8_t *b);
void deinitialiserNeopixel(void);
#endif