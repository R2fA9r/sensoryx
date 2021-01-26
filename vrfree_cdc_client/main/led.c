#include "led.h"

bool b_ledOn[3];

void led_init() {

	b_ledOn[0] = false;
    b_ledOn[1] = false;
    b_ledOn[2] = false;

    gpio_reset_pin(LED_CLR_R);
    gpio_reset_pin(LED_CLR_G);
    gpio_reset_pin(LED_CLR_B);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED_CLR_R, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_CLR_G, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_CLR_B, GPIO_MODE_OUTPUT);

}

void led_on(led_color_t led) {
    switch(led) {
        case LED_CLR_R:
        gpio_set_level(LED_CLR_R, 1);
        break;
        case LED_CLR_G:
        gpio_set_level(LED_CLR_G, 1);
        break;
        case LED_CLR_B:
        gpio_set_level(LED_CLR_B, 1);
        break;
    }

    b_ledOn[(uint8_t)led] = true;
}

void led_off(led_color_t led) {
	switch(led) {
        case LED_CLR_R:
        gpio_set_level(LED_CLR_R, 0);
        break;
        case LED_CLR_G:
        gpio_set_level(LED_CLR_R, 0);
        break;
        case LED_CLR_B:
        gpio_set_level(LED_CLR_R, 0);
        break;
    }
    b_ledOn[(uint8_t)led] = false;
}

void led_toggle(led_color_t led) {
    if(b_ledOn[(uint8_t)led]) {
        led_off(led);
    } else {
        led_on(led);
    }
}
