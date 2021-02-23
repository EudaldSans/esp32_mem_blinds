/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		14-06-2019      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "BLINDS_Device.h"

#include <stdbool.h>
#include "esp_log.h"

#include "BLINDS_Load.h"
#include "BLINDS_Button.h"
#include "BLINDS_Feedback.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_DEVICE                  "[DEVICE]"

/* INTERNAL FUNCTIONS */
/* ------------------ */

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
void DEVICE_SetCmd(uint8_t uiChild, PROTOCOL_VARIABLES xVar, double dValue)
{
    switch (xVar)
    {
        // case PROTOCOL_VARIABLE_STATUS:              if (uiChild == 1) {
        //                                                 ESP_LOGI(TAG_DEVICE, "SET STATUS");
        //                                                 if (dValue) LOAD_Open(); else LOAD_Close();
        //                                             }
        //                                             break;

        case PROTOCOL_VARIABLE_LEVEL:               if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET LEVEL");
                                                        LOAD_Regulate((uint8_t)dValue);
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_WINDOW_UP:           if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET WINDOW UP");
                                                        LOAD_Open();
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_WINDOW_DOWN:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET WINDOW DOWN");
                                                        LOAD_Close();
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_WINDOW_STOP:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET WINDOW STOP");
                                                        LOAD_Stop();
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_CALIBRATION:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET CALIBRATION");
                                                        LOAD_Calibrate();
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_BLIND_MODE:          if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET BLIND MODE");
                                                        LOAD_SetMode((dValue == 1) ? BLIND_MODE_SUNBLIND : BLIND_MODE_STDBLIND);
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_STATUS_LOCK:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET STATUS LOCK");
                                                        BUTTON_SetLockButton((bool)dValue);
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_IDLE_SIGNAL:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET IDLE SIGNAL");
                                                        if (dValue == 2) FEEDBACK_SetIdleSignal(FEEDBACK_IDLE_ON);
                                                        else FEEDBACK_SetIdleSignal(FEEDBACK_IDLE_OFF);
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_BLINDS_RISE_TIME:    if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET RISE TIME");
                                                        LOAD_SetRiseTime(((uint64_t)dValue)*1000000);
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_BLINDS_FALL_TIME:    if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "SET FALL TIME");
                                                        LOAD_SetFallTime(((uint64_t)dValue)*1000000);
                                                    }
                                                    break;
        
        case PROTOCOL_VARIABLE_RESET_CONF:          if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "RESET CONFIGURATION");
                                                        BUTTON_SetLockButton(DEFAULT_LOCK_BUTTON);
                                                        FEEDBACK_SetIdleSignal(DEFAULT_IDLE_SIGNAL);
                                                        LOAD_SetMode(DEFAULT_BLIND_MODE);
                                                        LOAD_SetRiseTime(DEFAULT_RISE_TIME);
                                                        LOAD_SetFallTime(DEFAULT_FALL_TIME);
                                                        LOAD_SetCalibrated(false);
                                                    } 
                                                    break;

        case PROTOCOL_VARIABLE_RESET_FACTORY:       ESP_LOGI(TAG_DEVICE, "SET RESET FACTORY");
                                                    BUTTON_SetLockButton(DEFAULT_LOCK_BUTTON);
                                                    FEEDBACK_SetIdleSignal(DEFAULT_IDLE_SIGNAL);
                                                    LOAD_SetMode(DEFAULT_BLIND_MODE);
                                                    LOAD_SetRiseTime(DEFAULT_RISE_TIME);
                                                    LOAD_SetFallTime(DEFAULT_FALL_TIME);
                                                    LOAD_SetCalibrated(false);
                                                    break;

        case PROTOCOL_VARIABLE_IDENTIFY:            ESP_LOGI(TAG_DEVICE, "SET IDENTIFY");
                                                    FEEDBACK_CustomSignal(SIGNAL_IDENTIFY_LEVEL, SIGNAL_IDENTIFY_ON, SIGNAL_IDENTIFY_OFF, SIGNAL_IDENTIFY_DURATION);
                                                    break;

        default:                                    break;
    }
}

void DEVICE_GetCmd(uint8_t uiChild, PROTOCOL_VARIABLES xVar)
{
    switch (xVar)
    {
        case PROTOCOL_VARIABLE_LEVEL:               if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET LEVEL");
                                                        MEM_SendInfo(1, xVar, (double)LOAD_GetPercentatge());
                                                    }
                                                    break;
        
        case PROTOCOL_VARIABLE_WINDOW_UP:           if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET WINDOW UP");
                                                        MEM_SendInfo(1, xVar, (double)LOAD_IsOpening());
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_WINDOW_DOWN:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET WINDOW DOWN");
                                                        MEM_SendInfo(1, xVar, (double)LOAD_IsClosing());
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_WINDOW_STOP:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET WINDOW STOP");
                                                        MEM_SendInfo(1, xVar, (double)LOAD_IsStopped());
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_CALIBRATION:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET CALIBRATION");
                                                        MEM_SendInfo(1, xVar, (LOAD_IsCalibrating() == true) ? 1 : (LOAD_IsCalibrated() == false) ? 2 : 0);
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_BLIND_MODE:          if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET BLIND MODE");
                                                        MEM_SendInfo(1, xVar, (double)((LOAD_GetMode() == BLIND_MODE_SUNBLIND) ? 1 : 0));
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_STATUS_LOCK:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET STATUS LOCK");
                                                        MEM_SendInfo(1, xVar, (double)BUTTON_IsButtonLocked());
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_IDLE_SIGNAL:         if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET IDLE SIGNAL");
                                                        if (FEEDBACK_GetIdleSignal() == FEEDBACK_IDLE_ON) MEM_SendInfo(uiChild, PROTOCOL_VARIABLE_IDLE_SIGNAL, 2);
                                                        else MEM_SendInfo(uiChild, PROTOCOL_VARIABLE_IDLE_SIGNAL, 0);
                                                        break;
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_BLINDS_RISE_TIME:    if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET RISE TIME");
                                                        MEM_SendInfo(uiChild, xVar, LOAD_GetRiseTime()/1000000);
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_BLINDS_FALL_TIME:    if (uiChild == 1) {
                                                        ESP_LOGI(TAG_DEVICE, "GET FALL TIME");
                                                        MEM_SendInfo(uiChild, xVar, LOAD_GetFallTime()/1000000);
                                                    }
                                                    break;

        case PROTOCOL_VARIABLE_STATUS_INFO:         ESP_LOGI(TAG_DEVICE, "GET GLOBAL INFO DEVICE");
                                                    MEM_SendInfo(1, PROTOCOL_VARIABLE_LEVEL, (double)LOAD_GetPercentatge());
                                                    MEM_SendInfo(1, PROTOCOL_VARIABLE_STATUS_LOCK, (double)BUTTON_IsButtonLocked());
                                                    MEM_SendInfo(1, PROTOCOL_VARIABLE_IDLE_SIGNAL, (FEEDBACK_GetIdleSignal() == FEEDBACK_IDLE_ON) ? 2 : 0);
                                                    MEM_SendInfo(1, PROTOCOL_VARIABLE_BLIND_MODE, (double)((LOAD_GetMode() == BLIND_MODE_SUNBLIND) ? 1 : 0));
                                                    MEM_SendInfo(1, PROTOCOL_VARIABLE_BLINDS_RISE_TIME, LOAD_GetRiseTime()/1000000);
                                                    MEM_SendInfo(1, PROTOCOL_VARIABLE_BLINDS_FALL_TIME, LOAD_GetFallTime()/1000000);
                                                    MEM_SendInfo(1, PROTOCOL_VARIABLE_CALIBRATION, (LOAD_IsCalibrating() == true) ? 1 : (LOAD_IsCalibrated() == false) ? 2 : 0);
                                                    break;

        default:                                    break;
    }
}

// void DEVICE_ResCmd(uint8_t uiChild, PROTOCOL_VARIABLES xVar, double dValue)
// {
//     switch (xVar)
//     {
//         case PROTOCOL_VARIABLE_STATUS:
//         case PROTOCOL_VARIABLE_LEVEL:
//         default:                            break;
//     }
// }

void DEVICE_Reset(void)
{
    ESP_LOGW(TAG_DEVICE, "Reset request");
}

