#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_freertos_hooks.h"
#include "esp_adc/adc_oneshot.h"

#define LED_PIN GPIO_NUM_2   
#define BUTTON_PIN GPIO_NUM_0  

//#define LED_PIN GPIO_NUM_22     // LED externo
//#define BUTTON_PIN GPIO_NUM_23  // Botón externo

volatile bool g_ledRapido = true;
volatile bool g_botonPres = false;
volatile bool g_sensorActivo = false;

adc_oneshot_unit_handle_t adc1_handle;

TaskHandle_t hLedRapido = NULL;
TaskHandle_t hLedLento = NULL;

void vTaskLedRapido(void *pvParameters) {
    int contadorRapido = 0; 
    
    while(1) {
        if (g_ledRapido) {
            if (contadorRapido % 5 == 0) {
            }
            
            gpio_set_level(LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100)); 
            gpio_set_level(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            
            contadorRapido++;
        } else if (g_botonPres) {
            vTaskSuspend(NULL); 
        } else {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

void vTaskLedLento(void *pvParameters) {
    int contadorTimeout = 0;
    while(1) {
        if (!g_ledRapido) {
            gpio_set_level(LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            
            gpio_set_level(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
            
            contadorTimeout++;
            
            if (contadorTimeout >= 5) {
                printf("5 s. Completados. Regresando a Frecuencia Rapida\n");
                
                g_ledRapido = true;
                g_botonPres = false;
                g_sensorActivo = false;
                contadorTimeout = 0; 
                
                if (hLedRapido != NULL) {
                    vTaskResume(hLedRapido); 
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(50)); 
        }
    }
}

void vTaskSensor(void *pvParameters) {
    while(1) {
        if (g_sensorActivo) {
            int raw = 0;
            
            adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &raw);
            
            float voltaje = (raw * 3.3) / 4095.0;
            
            printf("ADC = %d %.2fV\n", raw, voltaje);
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void vTaskMonitor(void *pvParameters) {
    bool ultimoEstado = true; 
    
    while(1) {
        bool estadoActual = gpio_get_level(BUTTON_PIN);

        if (ultimoEstado == true && estadoActual == false) {
            g_botonPres = true;
            g_ledRapido = false;
            g_sensorActivo = true;
        }
        ultimoEstado = estadoActual;

        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

bool my_idle_hook(void) {
    static TickType_t ultimo_mensaje = 0;
    TickType_t ahora = xTaskGetTickCount();
    
    if ((ahora - ultimo_mensaje) > pdMS_TO_TICKS(1000)) {

        ultimo_mensaje = ahora;
    }
    
    return true; 
}

void app_main(void) {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);

    esp_register_freertos_idle_hook(my_idle_hook);

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12, 
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config);

    xTaskCreate(vTaskLedRapido, "LedRapido", 2048, NULL, 1, &hLedRapido);
    xTaskCreate(vTaskLedLento,  "LedLento",  2048, NULL, 2, &hLedLento);
    xTaskCreate(vTaskSensor,    "Sensor",    2048, NULL, 3, NULL);
    xTaskCreate(vTaskMonitor,   "Monitor",   2048, NULL, 4, NULL);
}