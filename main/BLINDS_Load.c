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
#define NVM_KEY_CALIB               "calib_1"

/* INTERNAL FUNCTIONS */
/* ------------------ */
void _blinds_Task(void * xParams);
bool _check_end_of_career(void);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
xRELAY_t xReleUp;
xRELAY_t xReleDown;
BLINDS_CLASS xBlind1;
// POLLTIMER xTimerNotify;

bool bStatusUp = false;
bool bStatusDown = false;
uint8_t uiLevelBlind1;
uint64_t uiRiseBlind1;
uint64_t uiFallBlind1;
bool bCalibratedBlind1;
bool bNotifyEndCalib1 = false;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
bool _check_end_of_career(void)
{
static bool bDetectedCareer = false;
static uint8_t uiCareerCycles = 0;

    if ((bStatusUp == false) && (bStatusDown == false)) {
        bDetectedCareer = false; uiCareerCycles = 0;
    } else if (HLW8012_GetMeanPower() > MIN_POWER_LOAD) { 
        if (bDetectedCareer == false) ESP_LOGI(TAG_LOAD, "Start detecction moving");
        bDetectedCareer = true; uiCareerCycles = CAREER_CYCLES;
    } else if (uiCareerCycles) {
        uiCareerCycles -= 1;
    } else if (bDetectedCareer == true) {
        ESP_LOGI(TAG_LOAD, "Detected end of career");
        bDetectedCareer = false;
        return true;
    }

    return false;
}

void _blinds_Task(void * xParams)
{
bool bTempUp, bTempDown;
BLIND_STATES xTempStatus;
uint8_t uiTempLevel;

    while(1)
    {
        BLINDS_Engine(&xBlind1);

        xTempStatus = BLINDS_GetStatus(&xBlind1, &bTempUp, &bTempDown, &uiTempLevel);
        if (bStatusUp != bTempUp) { ESP_LOGI(TAG_LOAD, "Rele UP %d", bTempUp); bStatusUp = bTempUp; if (bStatusUp) RELAY_On(&xReleUp, true); else RELAY_Off(&xReleUp, true); }
        if (bStatusDown != bTempDown) { ESP_LOGI(TAG_LOAD, "Rele DOWN %d", bTempDown); bStatusDown = bTempDown; if (bStatusDown) RELAY_On(&xReleDown, true); else RELAY_Off(&xReleDown, true); }
        if (_check_end_of_career() == true) BLINDS_EndOfCareer(&xBlind1);
        
        // Saving changes
        if (xTempStatus == BLIND_STOPPED) {
            if (uiLevelBlind1 != uiTempLevel) { 
                if (NVS_WriteInt8(NVM_KEY_LEVEL, uiTempLevel) == true) {
                    uiLevelBlind1 = uiTempLevel;
                    MEM_SendInfo(1, PROTOCOL_VARIABLE_LEVEL, (double)uiLevelBlind1);
                    ESP_LOGI(TAG_LOAD, "New blind level %d", uiLevelBlind1);
                } else {
                    ESP_LOGE(TAG_LOAD, "Fail saving new level");
                }
                // TMR_SetPollTimer(&xTimerNotify, TIME_NOTIFY_STATUS);
            }

            if (bNotifyEndCalib1 == true) {
                if (LOAD_IsCalibrated() == true) { MEM_SendInfo(1, PROTOCOL_VARIABLE_CALIBRATION, 0); LOAD_SetCalibrated(true); } 
                else { MEM_SendInfo(1, PROTOCOL_VARIABLE_CALIBRATION, 2); }
                bNotifyEndCalib1 = false;
            }

            LOAD_SetRiseTime(BLINDS_GetRiseTime(&xBlind1));
            LOAD_SetFallTime(BLINDS_GetFallTime(&xBlind1));
        } else if ((LOAD_IsCalibrating() == true) && (bNotifyEndCalib1 == false)) {
            MEM_SendInfo(1, PROTOCOL_VARIABLE_CALIBRATION, 1); bNotifyEndCalib1 = true;
        }
        
        // if (TMR_GetPollTimeElapsed(&xTimerNotify) == true) {
        //     ESP_LOGI(TAG_LOAD, "New load level %d", uiLevelBlind1);
        //     MEM_SendInfo(1, PROTOCOL_VARIABLE_LEVEL, (double)uiLevelBlind1);
        //     if (NVS_WriteInt8(NVM_KEY_LEVEL, uiLevelBlind1) == true) ESP_LOGE(TAG_LOAD, "Fail saving new level");
        // } 

        TMR_delay(50*TIMER_MSEG);
    }
}

bool LOAD_Init(int iCore)
{
uint8_t uiData8;

    ESP_LOGI(TAG_LOAD, "Initializing relay...");
    NVS_Init();
    
    if (RELAY_Config(PIN_RELAY_UP, true, PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, PIN_SRS, PIN_VREF, VREF_LEVEL, NULL, &xReleUp) == false) ESP_LOGE(TAG_LOAD, "Relay up unconfigured");
    if (RELAY_Config(PIN_RELAY_DOWN, true, PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, PIN_SRS, PIN_VREF, VREF_LEVEL, NULL, &xReleDown) == false) ESP_LOGE(TAG_LOAD, "Relay down unconfigured");
    if (HLW8012_Config(PIN_SEL, BL0937, PIN_CF, PIN_CF1, VOLTAGE_PERIOD) == false) ESP_LOGE(TAG_LOAD, "HLW8012 unconfigured");

    if (NVS_ReadInt8(NVM_KEY_LEVEL, &uiLevelBlind1) == false) uiLevelBlind1 = 0;
    if (NVS_ReadBoolean(NVM_KEY_CALIB, &bCalibratedBlind1) == false) bCalibratedBlind1 = false;
    BLINDS_Start(uiLevelBlind1, bCalibratedBlind1, &xBlind1);
    if (NVS_ReadInt8(NVM_KEY_MODE, &uiData8) == true) BLINDS_SetMode(&xBlind1, (BLIND_MODES)uiData8);
    if (NVS_ReadInt64(NVM_KEY_RISE, &uiRiseBlind1) == true) BLINDS_SetRiseTime(&xBlind1, uiRiseBlind1); else uiRiseBlind1 = BLINDS_GetRiseTime(&xBlind1);
    if (NVS_ReadInt64(NVM_KEY_FALL, &uiFallBlind1) == true) BLINDS_SetFallTime(&xBlind1, uiFallBlind1); else uiFallBlind1 = BLINDS_GetFallTime(&xBlind1);

    ESP_LOGI(TAG_LOAD, "BLIND Initial level %d", uiLevelBlind1);
    ESP_LOGI(TAG_LOAD, "BLIND initial calibration status %d", (int)bCalibratedBlind1);
    ESP_LOGI(TAG_LOAD, "BLIND Initial mode %d", uiData8);
    ESP_LOGI(TAG_LOAD, "BLIND Initial rise %lld fall %lld", uiRiseBlind1, uiFallBlind1);

    return xTaskCreatePinnedToCore(_blinds_Task, "blinds_task", 3*1024, NULL, 5, NULL, iCore);
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
        if (NVS_WriteInt8(NVM_KEY_MODE, (uint8_t)xMode) == false) { ESP_LOGE(TAG_LOAD, "Fail saving mode"); return false; }
        BLINDS_SetMode(&xBlind1, xMode);
        ESP_LOGI(TAG_LOAD, "MODE %d", (int)xMode);
    }
    return true;
}

bool LOAD_SetCalibrated(bool bCalibrated)
{
    if (bCalibratedBlind1 != bCalibrated) {
        if (NVS_WriteBoolean(NVM_KEY_CALIB, bCalibrated) == false) { ESP_LOGE(TAG_LOAD, "Fail saving calibration status"); return false; }
        BLINDS_SetCalibrated(&xBlind1, bCalibrated); bCalibratedBlind1 = bCalibrated;
        ESP_LOGI(TAG_LOAD, "CALIBRATION STATUS %d", bCalibratedBlind1);
    }
    return true;
}

bool LOAD_SetRiseTime(uint64_t uiMicroSeconds)
{
    if (uiRiseBlind1 != uiMicroSeconds) {
        if (NVS_WriteInt64(NVM_KEY_RISE, uiMicroSeconds) == false) { ESP_LOGE(TAG_LOAD, "Fail saving rise time"); return false; }
        uiRiseBlind1 = uiMicroSeconds; BLINDS_SetRiseTime(&xBlind1, uiRiseBlind1);
        ESP_LOGI(TAG_LOAD, "RISE TIME %lld", uiRiseBlind1);
    }
    return true;
}

bool LOAD_SetFallTime(uint64_t uiMicroSeconds)
{
    if (uiFallBlind1 != uiMicroSeconds) {
        if (NVS_WriteInt64(NVM_KEY_FALL, uiMicroSeconds) == false) { ESP_LOGE(TAG_LOAD, "Fail saving fall time"); return false; }
        uiFallBlind1 = uiMicroSeconds; BLINDS_SetFallTime(&xBlind1, uiFallBlind1);
        ESP_LOGI(TAG_LOAD, "FALL TIME %lld", uiFallBlind1);
    }
    return true;
}


