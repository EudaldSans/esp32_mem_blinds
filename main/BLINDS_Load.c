/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		29-01-2020      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "BLINDS_Load.h"

#include "esp_log.h"

#include "MEM_Main.h"
#include "ges_relay.h"
#include "ges_HLW8012.h"
#include "ges_nvs.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_LOAD                    "[LOAD]"

#define NVM_KEY_MODE                "mode_1"
#define NVM_KEY_RISE                "rise_1"
#define NVM_KEY_FALL                "fall_1"
#define NVM_KEY_LEVEL               "level_1"

/* INTERNAL FUNCTIONS */
/* ------------------ */
void _blinds_Task(void * xParams);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
xRELAY_t xReleUp;
xRELAY_t xReleDown;
BLINDS_CLASS xBlind1;
POLLTIMER xTimerNotify;

uint8_t uiBlind1_level;
uint64_t uiBlind1_Rise;
uint64_t uiBlind1_Fall;

uint8_t uiCareer = 0;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
void _blinds_Task(void * xParams)
{
    while(1)
    {
        BLINDS_Engine(&xBlind1);

        // Saving changes
        if (BLINDS_GetLevel(&xBlind1) == BLIND_STOPPED) {
            if (uiBlind1_level != BLINDS_GetLevel(&xBlind1)) { 
                uiBlind1_level = BLINDS_GetLevel(&xBlind1);
                TMR_SetPollTimer(&xTimerNotify, TIME_NOTIFY_STATUS);
            }
            LOAD_SetRiseTime(BLINDS_GetRiseTime(&xBlind1));
            LOAD_SetFallTime(BLINDS_GetFallTime(&xBlind1));
        } 
        
        if (TMR_GetPollTimeElapsed(&xTimerNotify) == true) {
            ESP_LOGI(TAG_LOAD, "New load level %d", uiBlind1_level);
            MEM_SendInfo(1, PROTOCOL_VARIABLE_LEVEL, (double)uiBlind1_level);
            if (NVS_WriteInt8(NVM_KEY_LEVEL, uiBlind1_level) == false) ESP_LOGE(TAG_LOAD, "Fail saving new level");
        } 

        // Detectinf end of career
        // uiCareer = (HLW8012_GetMeanCurrent() > 50) ? 0 : (uiCareer+1); 
        // if (HLW8012_GetMeanCurrent() < )

        TMR_delay(50*TIMER_MSEG);
    }
}

bool LOAD_Init(int iCore)
{
uint8_t uiData8;

    ESP_LOGI(TAG_LOAD, "Initializing relay...");
    NVS_Init();
    
    if (RELAY_Config(PIN_RELAY_UP, true, PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, PIN_SRS, PIN_VREF, VREF_LEVEL, NULL, &xReleUp) == false) ESP_LOGE(TAG_LOAD, "Relay up unconfigured");
    if (RELAY_Config(PIN_RELAY_DOWN, true, PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, PIN_SRS, PIN_VREF, VREF_LEVEL, NULL, &xReleUp) == false) ESP_LOGE(TAG_LOAD, "Relay up unconfigured");
    if (HLW8012_Config(PIN_SEL, BL0937, PIN_CF, PIN_CF1, VOLTAGE_PERIOD) == false) ESP_LOGE(TAG_LOAD, "HLW8012 unconfigured");

    if (NVS_ReadInt8(NVM_KEY_LEVEL, &uiBlind1_level) == false) uiBlind1_level = 0;
    BLINDS_Start(uiBlind1_level, &xBlind1);
    if (NVS_ReadInt8(NVM_KEY_MODE, &uiData8) == true) BLINDS_SetMode(&xBlind1, (BLIND_MODES)uiData8);
    if (NVS_ReadInt64(NVM_KEY_RISE, &uiBlind1_Rise) == true) BLINDS_SetRiseTime(&xBlind1, uiBlind1_Rise); else uiBlind1_Rise = BLINDS_GetRiseTime(&xBlind1);
    if (NVS_ReadInt64(NVM_KEY_FALL, &uiBlind1_Fall) == true) BLINDS_SetFallTime(&xBlind1, uiBlind1_Fall); else uiBlind1_Fall = BLINDS_GetFallTime(&xBlind1);

    return xTaskCreatePinnedToCore(_blinds_Task, "blinds_task", 2048, NULL, 5, NULL, iCore);
    return true;
}

bool LOAD_Open(void)                                    { BLINDS_Open(&xBlind1); return true; }
bool LOAD_Close(void)                                   { BLINDS_Close(&xBlind1); return true; }
bool LOAD_Stop(void)                                    { BLINDS_Stop(&xBlind1); return true; }

bool LOAD_Calibrate(void)                               { BLINDS_StartCalibration(&xBlind1); return true; }
bool LOAD_Regulate(uint8_t uiPercentatge)               { BLINDS_Specific(&xBlind1, uiPercentatge); return true; }
uint8_t LOAD_GetPercentatge(void)                       { return BLINDS_GetLevel(&xBlind1); }

BLIND_MODES LOAD_GetMode(void)                          { return BLINDS_GetMode(&xBlind1); }

uint64_t LOAD_GetRiseTime(void)                         { return BLINDS_GetRiseTime(&xBlind1); }
uint64_t LOAD_GetFallTime(void)                         { return BLINDS_GetFallTime(&xBlind1); }

bool LOAD_IsGoingUp(void)                               
{ 
bool bData;

    BLINDS_GetStatus(&xBlind1, &bData, NULL, NULL);
    return bData;
}

bool LOAD_IsGoingDown(void)
{
bool bData;

    BLINDS_GetStatus(&xBlind1, NULL, &bData, NULL);
    return bData;
}

bool LOAD_IsCalibrating(void)
{
    return (BLINDS_GetStatus(&xBlind1, NULL, NULL, NULL) == BLIND_CALIBRATING);
}

bool LOAD_IsCalibrated(void)
{
    return BLINDS_IsCalibrated(&xBlind1);
}

bool LOAD_IsStopped(void)
{
bool bUp, bDown;

    BLINDS_GetStatus(&xBlind1, &bUp, &bDown, NULL);
    return ((bUp == false) && (bDown == false));
}

bool LOAD_SetMode(BLIND_MODES xMode)
{
    if (BLINDS_GetMode(&xBlind1) != xMode) {
        if (NVS_WriteInt8(NVM_KEY_RISE, (uint8_t)xMode) == false) { ESP_LOGE(TAG_LOAD, "Fail saving mode"); return false; }
        BLINDS_SetMode(&xBlind1, xMode);
    }
    return true;
}


bool LOAD_SetRiseTime(uint64_t uiMicroSeconds)
{
    if (uiBlind1_Rise != uiMicroSeconds) {
        if (NVS_WriteInt64(NVM_KEY_RISE, uiMicroSeconds) == false) { ESP_LOGE(TAG_LOAD, "Fail saving rise time"); return false; }
        uiBlind1_Rise = uiMicroSeconds;
    }
    return true;
}

bool LOAD_SetFallTime(uint64_t uiMicroSeconds)
{
    if (uiBlind1_Fall != uiMicroSeconds) {
        if (NVS_WriteInt64(NVM_KEY_FALL, uiMicroSeconds) == false) { ESP_LOGE(TAG_LOAD, "Fail saving fall time"); return false; }
        uiBlind1_Fall = uiMicroSeconds;
    }
    return true;
}


