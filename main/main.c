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
#include "factory_testing.h"

#include "ges_nvs.h"
#include "ges_debug.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_MAIN            "[MAIN]"

/* INTERNAL FUNCTIONS */
/* ------------------ */
// void _dropout_callback(uint64_t uiElapsedTime);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
// void _dropout_callback(uint64_t uiElapsedTime)
// {
//     ESP_LOGW(TAG_MAIN, "Dropout done...");
// }

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

    NVS_Init(); NVS_WriteStrToModule("OTA", "FW", "21000333_B_1.1.5");

    if(TEST_IsFactoryTestPassed() == true) {
        if (MEM_StartComs(0, DEVICE_SetCmd, DEVICE_GetCmd, NULL, DEVICE_Reset) == false) ESP_LOGE(TAG_MAIN, "Error creating MEM Wireless coms\n");
        if (LOAD_Init(1) == false) ESP_LOGE(TAG_MAIN, "Error starting relays management");
        if (BUTTON_Init(0) == false) ESP_LOGE(TAG_MAIN, "Error starting buttons management");
        if (FEEDBACK_Init(0) == false) ESP_LOGE(TAG_MAIN, "Error starting feedback management");
    } else {
        if (TEST_FactoryTestStart() == false) ESP_LOGE(TAG_MAIN, "Error starting test task");
    }

    DEBUG_Start();
}


