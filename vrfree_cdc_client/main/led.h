#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

typedef enum {
	LED_CLR_R = 5,
	LED_CLR_G = 4,
	LED_CLR_B = 18
} led_color_t;

void led_init();
void led_on(led_color_t led);
void led_off(led_color_t led);
void led_toggle(led_color_t led);
