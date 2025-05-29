#include "piloteTest.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "pilote";

// Timings en ns et reset en µs
typedef struct {
    int t0h;
    int t0l;
    int t1h;
    int t1l;
    int reset_us;
} led_timing_t;

static void getLedTiming(led_type_t type, led_timing_t *timing) {
    if (type == LED_TYPE_WS2812B) {
        timing->t0h = 400;
        timing->t0l = 800;
        timing->t1h = 850;
        timing->t1l = 450;
        timing->reset_us = 80;
    } else if (type == LED_TYPE_SK6812MINI) {
        timing->t0h = 300;
        timing->t0l = 900;
        timing->t1h = 600;
        timing->t1l = 600;
        timing->reset_us = 90;
    }
}

static void rgb_to_rmt(const uint8_t *data, int length, rmt_item32_t *items, led_timing_t *timing) {
    int idx = 0;

    for (int i = 0; i < length; i++) {
        uint8_t byte = data[i];
        for (int j = 0; j < 8; j++) {
            if (byte & 0x80) {
                items[idx].duration0 = (timing->t1h * 80) / 1000; // 12.5 ns par tick
                items[idx].level0 = 1;
                items[idx].duration1 = (timing->t1l * 80) / 1000;
                items[idx].level1 = 0;
            } else {
                items[idx].duration0 = (timing->t0h * 80) / 1000;
                items[idx].level0 = 1;
                items[idx].duration1 = (timing->t0l * 80) / 1000;
                items[idx].level1 = 0;
            }
            byte <<= 1;
            idx++;
        }
    }
}

int neopixel_init(neopixel_t *strip, int gpio_num, int num_leds, int rmt_channel, led_type_t led_type) {
    if (!strip || num_leds <= 0 || rmt_channel < 0 || rmt_channel > 7) return -1;

    strip->gpio_num = gpio_num;
    strip->num_leds = num_leds;
    strip->rmt_channel = rmt_channel;
    strip->led_type = led_type;

    strip->buffer = malloc(num_leds * 3);
    if (!strip->buffer) {
        ESP_LOGE(TAG, "Malloc failed");
        return -1;
    }
    memset(strip->buffer, 0, num_leds * 3);

    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = rmt_channel,
        .gpio_num = gpio_num,
        .clk_div = 1,  // 12.5 ns par tick (80 MHz)
        .mem_block_num = 1,
        .tx_config = {
            .loop_en = false,
            .carrier_en = false,
            .idle_output_en = true,
            .idle_level = RMT_IDLE_LEVEL_LOW,
        }
    };

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(rmt_channel, 0, 0));

    return 0;
}

void neopixel_set_color(neopixel_t *strip, int index, uint8_t r, uint8_t g, uint8_t b) {
    if (!strip || !strip->buffer || index < 0 || index >= strip->num_leds) return;
    strip->buffer[3 * index + 0] = g;
    strip->buffer[3 * index + 1] = r;
    strip->buffer[3 * index + 2] = b;
}

void neopixel_show(neopixel_t *strip) {
    if (!strip || !strip->buffer) return;

    int num_bits = strip->num_leds * 24;
    rmt_item32_t *items = malloc(sizeof(rmt_item32_t) * num_bits);
    if (!items) {
        ESP_LOGE(TAG, "Malloc RMT items failed");
        return;
    }

    led_timing_t timing;
    getLedTiming(strip->led_type, &timing);

    rgb_to_rmt(strip->buffer, strip->num_leds * 3, items, &timing);

    ESP_ERROR_CHECK(rmt_write_items(strip->rmt_channel, items, num_bits, true));
    ESP_ERROR_CHECK(rmt_wait_tx_done(strip->rmt_channel, pdMS_TO_TICKS(100)));

    free(items);

    vTaskDelay(pdMS_TO_TICKS(timing.reset_us / 1000 + 1));
}

void neopixel_deinit(neopixel_t *strip) {
    if (!strip) return;
    if (strip->buffer) free(strip->buffer);
    rmt_driver_uninstall(strip->rmt_channel);
}

void neopixel_copy_color(neopixel_t *strip, int srcIndex, int dstIndex) {
    if (!strip || !strip->buffer) return;
    if (srcIndex < 0 || srcIndex >= strip->num_leds) return;
    if (dstIndex < 0 || dstIndex >= strip->num_leds) return;

    strip->buffer[3 * dstIndex + 0] = strip->buffer[3 * srcIndex + 0]; // G
    strip->buffer[3 * dstIndex + 1] = strip->buffer[3 * srcIndex + 1]; // R
    strip->buffer[3 * dstIndex + 2] = strip->buffer[3 * srcIndex + 2]; // B
}

void neopixel_get_color(neopixel_t *strip, int index, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (!strip || !strip->buffer || index < 0 || index >= strip->num_leds) return;
    if (g) *g = strip->buffer[3 * index + 0];
    if (r) *r = strip->buffer[3 * index + 1];
    if (b) *b = strip->buffer[3 * index + 2];
}
