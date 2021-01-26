/* USB Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "led.h"
#include "sdkconfig.h"
#include "tinyusb.h"
#include "vrfree_connection.h"


static const char *TAG = "vrfree_com";

uint8_t USB_rx_buffer[64];

// USB Device Driver task
// This top level thread processes all usb events and invokes callbacks
static void usb_device_task(void *param) {
    (void)param;
    ESP_LOGI(TAG, "USB task started");
    while (1) {
        tud_task(); // RTOS forever loop
    }
}

// void cdc_task(void* params)
// {
//   (void) params;

//   // RTOS forever loop
//   while ( 1 )
//   {
//     if ( tud_cdc_connected() )
//     {
//       ESP_LOGI(TAG,"Connected");
//       // connected and there are data available
//       if ( tud_cdc_available() )
//       {

//       	led_toggle();

//         uint8_t buf[64];

//         // read and echo back
//         uint32_t count = tud_cdc_read(buf, sizeof(buf));

//         for(uint32_t i=0; i<count; i++)
//         {
//           tud_cdc_write_char(buf[i]);

//           if ( buf[i] == '\r' ) {
//               tud_cdc_write_str("\n > ");
//           }
//         }

//         tud_cdc_write_flush();
//       }
//     } 

//     // For ESP32-S2 this delay is essential to allow idle how to run and reset wdt
//     vTaskDelay(pdMS_TO_TICKS(10));
//   }
// }



void app_main(void) {

    ESP_LOGI(TAG, "USB initialization");

    tinyusb_config_t tusb_cfg = {
        .descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false // In the most cases you need to use a `false` value
    };

    led_init();

    vrfree_connection_init();

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGI(TAG, "USB initialization DONE");
    
    // Create a task for tinyusb device stack:
    xTaskCreate(usb_device_task, "usbd", 4096, NULL, 24, NULL);
    //xTaskCreate( cdc_task, "cdc", 4096, NULL, 23, NULL);

    return;
}

void tud_cdc_rx_cb(uint8_t itf) {
  ESP_LOGI(TAG,"Data received on interface %d.",itf);
  led_toggle(LED_CLR_G);

  uint32_t num_bytes = tud_cdc_available();

  uint32_t bytes_read = tud_cdc_read(USB_rx_buffer,num_bytes);

  vrfree_connection_onNewData(USB_rx_buffer,num_bytes);

  ESP_LOGI(TAG,"num bytes: %d - bytes read: %d",num_bytes, bytes_read);
}

 void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding) {
    ESP_LOGI(TAG,"Line coding changed");
 }

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
	led_on(LED_CLR_B);
	ESP_LOGI(TAG,"SET_CONTROL_LINE_STATE");
  if (dtr && rts)
  {
    tud_cdc_write_str("Welcome to tinyUSB CDC example!!!\r\n");
  }
}
