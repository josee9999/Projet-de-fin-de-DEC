#ifndef PILOTETEST_H
#define PILOTETEST_H

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
} neopixel_t;

int neopixel_init(neopixel_t *strip, int gpio_num, int num_leds, int rmt_channel, led_type_t led_type);
void neopixel_set_color(neopixel_t *strip, int index, uint8_t r, uint8_t g, uint8_t b);
void neopixel_show(neopixel_t *strip);
void neopixel_deinit(neopixel_t *strip);
void neopixel_copy_color(neopixel_t *strip, int srcIndex, int dstIndex);
void neopixel_get_color(neopixel_t *strip, int index, uint8_t *r, uint8_t *g, uint8_t *b);

#endif
