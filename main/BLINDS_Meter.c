/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		29-01-2020      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "BLINDS_Meter.h"

#include <stdio.h>
#include <stdint.h>

#include "esp_log.h"

#include "ges_timer.h"
#include "ges_nvs.h"
#include "ges_HLW8012.h"
#include "MEM_Main.h"
#include "BLINDS_Load.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_METER                   "[METER]"

#define NVM_MAX_POWER_KEY           "max_power"
#define NVM_CTRL_POWER_KEY          "ctrl_power"

#define MAX_POWER_CNT_LIMIT         4

/* INTERNAL FUNCTIONS */
/* ------------------ */
static void _meter_protection_task(void * xParams);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
static bool bOverCurrentDetected = false;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
static void _meter_protection_task(void * xParams)
{
static uint8_t uiCntOverCurrent = 0;
// static uint8_t uiCntOverCustom = 0;

    while (1)
    {
        if (METER_GetCurrent() > (float)MAX_HW_CURRENT) { 
            ESP_LOGW(TAG_METER, "Max current detected"); 
            uiCntOverCurrent++;
            if (uiCntOverCurrent >= MAX_POWER_CNT_LIMIT) {
                uiCntOverCurrent = 0;
                bOverCurrentDetected = true; 
                LOAD_Stop();
            }          
        // } else if ((bCtrlMaxPower) && (fMaxPower) && (METER_GetPower() > fMaxPower)) { 
        //     ESP_LOGW(TAG_METER, "Max power detected");
        //     uiCntOverCustom++;
        //     if (uiCntOverCustom == MAX_POWER_CNT_LIMIT) {
        //         uiCntOverCustom = 0;
        //         bOverConsumptionDetected = true; 
        //         LOAD_Stop();
        //     }            
        } else if (LOAD_IsStopped() == false){
            // if (bOverConsumptionDetected == true) bOverConsumptionDetected = false;
            if (bOverCurrentDetected == true) bOverCurrentDetected = false; 
            uiCntOverCurrent = 0;
            // uiCntOverCustom = 0;
        // } else {
        //     if (bCtrlMaxPower == false) bOverConsumptionDetected = false;
        //     uiCntOverCurrent = uiCntOverCustom = 0;
        }

        TMR_delay(1*TIMER_SEG);
    }
}

bool METER_Init(int iCore)
{
    ESP_LOGI(TAG_METER, "Initializing device...");
    NVS_Init();
    // Config power measures
    HLW8012_Config(HLW8012_SEL, BL0937, HLW8012_CF, HLW8012_CF1, HLW8012_VOLTAGE_PERIOD);
    return xTaskCreatePinnedToCore(_meter_protection_task, "_meter_protection_task", 3*1024, NULL, 5, NULL, iCore);
    return true;
}

float METER_GetPower(void)                  { return ((LOAD_IsStopped() == false) && (HLW8012_GetMeanPower() > MIN_POWER_LOAD)) ? HLW8012_GetMeanPower() : 0; }
float METER_GetCurrent(void)                { return (LOAD_IsStopped() == false) ? (HLW8012_GetMeanCurrent()/1000) : 0; }
float METER_GetVoltage(void)                { return HLW8012_GetMeanVoltage(); }

bool METER_GetMaxCurrentDetected(void)      { return bOverCurrentDetected; }

