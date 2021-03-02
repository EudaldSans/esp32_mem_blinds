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
#include "ges_signal.h"
#include "ges_relay.h"
#include "ges_HLW8012.h"
#include "ges_nvs.h"

/* TYPES */
/* ----- */
typedef enum {
    BLIND_MOTION_STOP = 0,
    BLIND_MOTION_UP,
    BLIND_MOTION_DOWN,
    BLIND_MOTION_NUM_SENSES
} BLIND_MOTION_SENSES;

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
void _isr_triac(bool bValid, uint64_t uiPeriod, uint64_t uiMeanPeriod, void *pArgs);
void _turn_on_triac(void);
bool _check_end_of_career(void);
bool _save_level_blinds(uint8_t uiLevel);
void _motion_sense(BLIND_MOTION_SENSES xSense);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
// xRELAY_t xReleUp;
// xRELAY_t xReleDown;
BLINDS_CLASS xBlind1;
xSIGNAL_t xSignalTriac;
// POLLTIMER xTimerNotify;

bool bStatusUp = false;
bool bStatusDown = false;
uint8_t uiLevelBlind1;
uint64_t uiRiseBlind1;
uint64_t uiFallBlind1;
bool bCalibratedBlind1;
bool bNotifyEndCalib1 = false;
uint8_t uiLastNotificatedLevel;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
void IRAM_ATTR _isr_triac(bool bValid, uint64_t uiPeriod, uint64_t uiMeanPeriod, void *pArgs)
{
    if (bValid == true) { 
        GPIO_SetOutput(PIN_TRIAC_ON, false);
    }
}

void _turn_on_triac(void)
{
    if (GPIO_GetOutput(PIN_TRIAC_ON) == false) return;
    SIGNAL_SetVoltageCallback(SIGNAL_GetByPin(PIN_SINCRO), SIGNAL_CALLBACK_HIGH_PRIORITY_5, SIGNALCallbackOnZeroCrossing, _isr_triac, NULL); TMR_delay(50*TIMER_MSEG); 
    if (GPIO_GetOutput(PIN_TRIAC_ON) == true) { ESP_LOGW(TAG_LOAD, "Triac turn on without sincro"); GPIO_SetOutput(PIN_TRIAC_ON, false); }
}

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

bool _save_level_blinds(uint8_t uiLevel)
{
    if (uiLevelBlind1 != uiLevel) {
        if (NVS_WriteInt8(NVM_KEY_LEVEL, uiLevel) == true) {
            uiLevelBlind1 = uiLevel;
            ESP_LOGI(TAG_LOAD, "New blind level %d", uiLevelBlind1);
        } else {
            ESP_LOGE(TAG_LOAD, "Fail saving new level");
            return false;
        }
    }

    return true;
}

void _motion_sense(BLIND_MOTION_SENSES xSense)
{
    switch (xSense)
    {
        case BLIND_MOTION_UP:       ESP_LOGI(TAG_LOAD, "Motion blinds UP");
                                    GPIO_SetOutput(PIN_RELAY_UPDOWN, true); TMR_delay(50*TIMER_MSEG);
                                    _turn_on_triac();
                                    break;

        case BLIND_MOTION_DOWN:     ESP_LOGI(TAG_LOAD, "Motion blinds DOWN");
                                    GPIO_SetOutput(PIN_RELAY_UPDOWN, false); TMR_delay(50*TIMER_MSEG);
                                    _turn_on_triac();
                                    break;

        default:                    ESP_LOGI(TAG_LOAD, "Motion blinds STOP");
                                    GPIO_SetOutput(PIN_TRIAC_ON, true); TMR_delay(50*TIMER_MSEG);
                                    break;
    }
}

void _blinds_Task(void * xParams)
{
bool bTempUp, bTempDown;
BLIND_STATES xTempStatus;
static BLIND_STATES xLastStatus = BLIND_STOPPED;
uint8_t uiTempLevel;

    while(1)
    {
        BLINDS_Engine(&xBlind1);

        xTempStatus = BLINDS_GetStatus(&xBlind1, &bTempUp, &bTempDown, &uiTempLevel);
        if ((bStatusUp != bTempUp) || (bStatusDown != bTempDown)) {
            bStatusUp = bTempUp; bStatusDown = bTempDown;
            _save_level_blinds(uiTempLevel);
            _motion_sense((bStatusUp) ? BLIND_MOTION_UP : (bStatusDown) ? BLIND_MOTION_DOWN : BLIND_MOTION_STOP);
        }
        // if (bStatusUp != bTempUp) { ESP_LOGI(TAG_LOAD, "Rele UP %d", bTempUp); bStatusUp = bTempUp; if (bStatusUp) RELAY_On(&xReleUp, true); else { _save_level_blinds(uiTempLevel); RELAY_Off(&xReleUp, true); } }
        // if (bStatusDown != bTempDown) { ESP_LOGI(TAG_LOAD, "Rele DOWN %d", bTempDown); bStatusDown = bTempDown; if (bStatusDown) RELAY_On(&xReleDown, true); else { _save_level_blinds(uiTempLevel); RELAY_Off(&xReleDown, true); } }
        if (_check_end_of_career() == true) BLINDS_EndOfCareer(&xBlind1);
        
        // Saving changes
        if (xTempStatus == BLIND_STOPPED) {

            if (xLastStatus != BLIND_STOPPED) {
                MEM_SendInfo(1, PROTOCOL_VARIABLE_WINDOW_STOP, 1);
                xLastStatus = BLIND_STOPPED;
            }

            _save_level_blinds(uiTempLevel);
            if (uiLevelBlind1 != uiLastNotificatedLevel) { 
                uiLastNotificatedLevel = uiLevelBlind1;
                MEM_SendInfo(1, PROTOCOL_VARIABLE_LEVEL, (double)uiLevelBlind1);
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
        } else if ((xLastStatus != BLIND_OPENING) && (LOAD_IsOpening() == true)) {
            MEM_SendInfo(1, PROTOCOL_VARIABLE_WINDOW_UP, 1);
            xLastStatus = BLIND_OPENING;
        } else if ((xLastStatus != BLIND_CLOSING) && (LOAD_IsClosing() == true)) {
            MEM_SendInfo(1, PROTOCOL_VARIABLE_WINDOW_DOWN, 1);
            xLastStatus = BLIND_CLOSING;
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
    
    GPIO_ConfigOutput(PIN_TRIAC_ON, true);
    GPIO_ConfigOutput(PIN_RELAY_UPDOWN, false);

    if (SIGNAL_GetByPin(PIN_SINCRO) == NULL) { if (SIGNAL_VoltageConfig(PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, iCore, &xSignalTriac) == false) return false; }                                                                                                         
    // if (RELAY_Config(PIN_RELAY_UP, true, PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, PIN_SRS, PIN_VREF, VREF_LEVEL, NULL, &xReleUp) == false) ESP_LOGE(TAG_LOAD, "Relay up unconfigured");
    // if (RELAY_Config(PIN_RELAY_DOWN, true, PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, PIN_SRS, PIN_VREF, VREF_LEVEL, NULL, &xReleDown) == false) ESP_LOGE(TAG_LOAD, "Relay down unconfigured");
    if (HLW8012_Config(PIN_SEL, BL0937, PIN_CF, PIN_CF1, VOLTAGE_PERIOD) == false) ESP_LOGE(TAG_LOAD, "HLW8012 unconfigured");

    if (NVS_ReadInt8(NVM_KEY_LEVEL, &uiLevelBlind1) == false) uiLevelBlind1 = 0; 
    if (NVS_ReadBoolean(NVM_KEY_CALIB, &bCalibratedBlind1) == false) bCalibratedBlind1 = false;
    if (NVS_ReadInt64(NVM_KEY_RISE, &uiRiseBlind1) == false) uiRiseBlind1 = DEFAULT_RISE_TIME;
    if (NVS_ReadInt64(NVM_KEY_FALL, &uiFallBlind1) == false) uiFallBlind1 = DEFAULT_FALL_TIME;
    if (NVS_ReadInt8(NVM_KEY_MODE, &uiData8) == false) uiData8 = (uint8_t)DEFAULT_BLIND_MODE;
    BLINDS_Start(uiLevelBlind1, (BLIND_MODES)uiData8, uiRiseBlind1, uiFallBlind1, bCalibratedBlind1, &xBlind1);
    
    uiLastNotificatedLevel = uiLevelBlind1;
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

bool LOAD_IsOpening(void)                               
{ 
    return BLINDS_IsOpening(&xBlind1);
}

bool LOAD_IsClosing(void)
{
    return BLINDS_IsClosing(&xBlind1);
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
    return BLINDS_IsStopped(&xBlind1);
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
        
        if(!LOAD_IsCalibrated()){
            LOAD_SetCalibrated(true);
            MEM_SendInfo(1, PROTOCOL_VARIABLE_CALIBRATION, 0);
        }
    }
    return true;
}

bool LOAD_SetFallTime(uint64_t uiMicroSeconds)
{
    if (uiFallBlind1 != uiMicroSeconds) {
        if (NVS_WriteInt64(NVM_KEY_FALL, uiMicroSeconds) == false) { ESP_LOGE(TAG_LOAD, "Fail saving fall time"); return false; }
        uiFallBlind1 = uiMicroSeconds; BLINDS_SetFallTime(&xBlind1, uiFallBlind1);
        ESP_LOGI(TAG_LOAD, "FALL TIME %lld", uiFallBlind1);

        if(!LOAD_IsCalibrated()){
            LOAD_SetCalibrated(true);
            MEM_SendInfo(1, PROTOCOL_VARIABLE_CALIBRATION, 0);
        }
    }
    return true;
}


