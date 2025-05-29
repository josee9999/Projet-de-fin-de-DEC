#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "piloteTest.h"

#define WS2812B_LED_COUNT 120
#define SK6812MINI_LED_COUNT 60
#define TOTAL_LED_COUNT (WS2812B_LED_COUNT + SK6812MINI_LED_COUNT)

neopixel_t ws2812b;    // minutes (WS2812B)
neopixel_t sk6812mini; // secondes (SK6812MINI)

void afficherChenille(int position, const uint8_t chenille[][3], int chenilleTaille)
{
    // Pour chaque LED sur la 1ère strip (SK6812MINI)
    for (int i = 0; i < SK6812MINI_LED_COUNT; i++)
    {
        int ledPos = position - i;
        // Allume la LED uniquement si elle est dans la longueur chenilleTaille
        if (ledPos >= 0 && ledPos < chenilleTaille)
        {
            neopixel_set_color(&sk6812mini, i,
                               chenille[ledPos][0],
                               chenille[ledPos][1],
                               chenille[ledPos][2]);
        }
        else
        {
            neopixel_set_color(&sk6812mini, i, 0, 0, 0);
        }
    }

    // Pour chaque LED sur la 2ème strip (WS2812B)
for (int i = 0; i < WS2812B_LED_COUNT; i++)
{
    int ledPos = position - (SK6812MINI_LED_COUNT + i);
        // Allume la LED uniquement si elle est dans la longueur chenilleTaille
        if (ledPos >= 0 && ledPos < chenilleTaille)
        {
            neopixel_set_color(&ws2812b, i,
                               chenille[ledPos][0],
                               chenille[ledPos][1],
                               chenille[ledPos][2]);
        }
        else
        {
            neopixel_set_color(&ws2812b, i, 0, 0, 0);
        }
    }

    neopixel_show(&sk6812mini);
    neopixel_show(&ws2812b);
}

void app_main(void)
{
    gpio_reset_pin(26);
    gpio_reset_pin(27);
    gpio_set_direction(26, GPIO_MODE_OUTPUT);
    gpio_set_direction(27, GPIO_MODE_OUTPUT);
    gpio_set_level(26, 0);
    gpio_set_level(27, 0);

    if (neopixel_init(&ws2812b, 25, WS2812B_LED_COUNT, 0, LED_TYPE_WS2812B) != 0)
    {
        printf("Erreur d'initialisation WS2812B\n");
        return;
    }

    if (neopixel_init(&sk6812mini, 14, SK6812MINI_LED_COUNT, 1, LED_TYPE_SK6812MINI) != 0)
    {
        printf("Erreur d'initialisation SK6812MINI\n");
        neopixel_deinit(&ws2812b);
        return;
    }

    const uint8_t chenille[][3] = {
        {5, 0, 0},   // Rouge
        {5, 2, 0},   // Rouge-orangé
        {5, 5, 0},   // Jaune
        {2, 5, 0},   // Jaune-vert
        {0, 5, 0},   // Vert
        {0, 5, 2},   // Vert-bleu
        {0, 5, 5},   // Turquoise
        {0, 2, 5},   // Bleu clair
        {0, 0, 5},   // Bleu
        {2, 0, 5},   // Bleu-violet
        {5, 0, 5},   // Magenta
        {5, 0, 2}    // Rose-rouge
    };

    const int chenilleTaille = sizeof(chenille) / sizeof(chenille[0]);

    int pos = 0;
    while (1)
    {
        afficherChenille(pos, chenille, chenilleTaille);
        vTaskDelay(pdMS_TO_TICKS(175));
        pos++;
        if (pos >= TOTAL_LED_COUNT + chenilleTaille)
        {
            pos = 0; // Redémarre la chenille en boucle infinie
        }
    }

    neopixel_deinit(&ws2812b);
    neopixel_deinit(&sk6812mini);
}
