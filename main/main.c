/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		14-06-2019      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include <stdbool.h>
#include "esp_log.h"
#include "esp_spi_flash.h"

#include "MEM_Main.h"
#include "BLINDS_Device.h"
#include "BLINDS_Button.h"
#include "BLINDS_Feedback.h"
#include "BLINDS_Load.h"
#include "BLINDS_Meter.h"
#include "factory_testing.h"

#include "ges_nvs.h"
#include "ges_debug.h"
#include "ges_dropout.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_MAIN            "[MAIN]"

/* INTERNAL FUNCTIONS */
/* ------------------ */
void _dropout_before_callback(void);
void _dropout_callback(uint64_t uiElapsedTime);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
bool bTriacStatus = false;
bool bRelayStatus = false;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
void _dropout_before_callback(void)
{
    ESP_LOGW(TAG_MAIN, "Dropout detected...");
    bTriacStatus = GPIO_GetOutput(PIN_TRIAC_ON);
    bRelayStatus = GPIO_GetOutput(PIN_RELAY_UPDOWN);
    GPIO_SetOutput(PIN_TRIAC_ON, true);
    vTaskDelay(pdMS_TO_TICKS(10));
    GPIO_SetOutput(PIN_RELAY_UPDOWN, false);
}

void _dropout_callback(uint64_t uiElapsedTime)
{
    ESP_LOGW(TAG_MAIN, "Dropout done...");
    GPIO_SetOutput(PIN_RELAY_UPDOWN, bRelayStatus);
    vTaskDelay(pdMS_TO_TICKS(10));
    GPIO_SetOutput(PIN_TRIAC_ON, bTriacStatus);
}

void app_main(void)
{
esp_chip_info_t chip_info;
    
    printf("New project in branch aml...\n");
    esp_chip_info(&chip_info);

    printf("********************************\n");

    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("********************************\n");
    fflush(stdout);

    NVS_Init(); NVS_WriteStrToModule("OTA", "FW", "21000333_D_2.1.0");

    if(TEST_IsFactoryTestPassed() == true) {
        if (MEM_StartComs(0, DEVICE_SetCmd, DEVICE_GetCmd, NULL, DEVICE_Reset) == false) ESP_LOGE(TAG_MAIN, "Error creating MEM Wireless coms\n");
        if (LOAD_Init(1) == false) ESP_LOGE(TAG_MAIN, "Error starting relays management");
        if (METER_Init(0) == false) ESP_LOGE(TAG_MAIN, "Error initiating power measures & protecctions");
        if (BUTTON_Init(0) == false) ESP_LOGE(TAG_MAIN, "Error starting buttons management");
        if (FEEDBACK_Init(0) == false) ESP_LOGE(TAG_MAIN, "Error starting feedback management");
        if (DROPOUT_Config(PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE, 200, 0, _dropout_before_callback, _dropout_callback) == false) ESP_LOGE(TAG_MAIN, "Error starting dropout protection");
    } else {
        if (TEST_FactoryTestStart() == false) ESP_LOGE(TAG_MAIN, "Error starting test task");
    }

    DEBUG_Start();
}


