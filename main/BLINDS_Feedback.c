/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		23-01-2020      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "BLINDS_Feedback.h"

#include "esp_log.h"
#include <stddef.h>
#include <stdint.h>

#include "ges_mtx.h"
#include "ges_led.h"
#include "ges_nvs.h"
#include "ges_timer.h"
#include "ges_color.h"

#include "MEM_Main.h"
#include "BLINDS_Button.h"
#include "BLINDS_Load.h"

/* DEFINES */
/* ------- */
#define TAG_FEEDBACK                "[FEEDBACK]"

#define NVM_LED_IDLE_SIGNAL         "idle_signal"
#define NVM_KEY_HIDE_SIGNAL         "hide_signal"

/* TYPES */
/* ----- */

/* INTERNAL FUNCTIONS */
/* ------------------ */
static void _feedback_signal_task(void * xParams);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
static xMTX_t xFeedbackMtx = MTX_INIT_CONFIG_DEFAULT;
static xLED_t xLedUp;
static xLED_t xLedDown;
static FEEDBACK_IDLE_SIGNALS_t xIdleSignal;
static bool bEnableHideSignal = true;
static POLLTIMER_t xTimerSignal;
static bool bMotionSignal = false;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
static void _feedback_signal_task(void * xParams)
{
    while (1)
    {
        MTX_Lock(&xFeedbackMtx);
        if (TMR_GetPollTimeRunning(&xTimerSignal) == true) {       // Wait here while timer is running
            if (bMotionSignal == true) {
                if (LOAD_IsOpening() == true) {
                    LED_Off(&xLedDown, false);
                    LED_Blink(&xLedUp, SIGNAL_BLIND_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_BLIND_ON, SIGNAL_BLIND_OFF, false);
                } else if (LOAD_IsClosing() == true) {
                    LED_Off(&xLedUp, false);
                    LED_Blink(&xLedDown, SIGNAL_BLIND_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_BLIND_ON, SIGNAL_BLIND_OFF, false);
                } else {
                    bMotionSignal = false;
                    LED_On(&xLedDown, SIGNAL_BLIND_STOP_LEVEL/100, false); LED_On(&xLedUp, SIGNAL_BLIND_STOP_LEVEL/100, false);
                    TMR_SetPollTimer(&xTimerSignal, SIGNAL_BLIND_STOP_DURATION*TIMER_MSEG);
                }
            }
            TMR_GetPollTimeElapsed(&xTimerSignal);
        } else if (LOAD_IsCalibrating() == true) {
            LED_Blink(&xLedUp, SIGNAL_CALIBRATE_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_CALIBRATE_ON, SIGNAL_CALIBRATE_OFF, false);
            LED_Blink(&xLedDown, SIGNAL_CALIBRATE_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_CALIBRATE_ON, SIGNAL_CALIBRATE_OFF, false);
        // } else if (LOAD_IsOpening() == true) {
        //     LED_Off(&xLedDown, false);
        //     LED_Blink(&xLedUp, SIGNAL_BLIND_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_BLIND_ON, SIGNAL_BLIND_OFF, false);
        // } else if (LOAD_IsClosing() == true) {
        //     LED_Off(&xLedUp, false);
        //     LED_Blink(&xLedDown, SIGNAL_BLIND_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_BLIND_ON, SIGNAL_BLIND_OFF, false);
        } else if ((MEM_GetStatus() == MEM_STATUS_HIDE) && (bEnableHideSignal == true)) {
            LED_Off(&xLedUp, false);
            LED_Blink(&xLedDown, SIGNAL_OFFLINE_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_OFFLINE_ON, SIGNAL_OFFLINE_OFF, false);
        } else if (LOAD_IsCalibrated() == false) {
            LED_Off(&xLedUp, false);
            LED_Blink(&xLedDown, SIGNAL_NEED_CALIBRATE_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_NEED_CALIBRATE_ON, SIGNAL_NEED_CALIBRATE_OFF, false);
        } else if (xIdleSignal == FEEDBACK_IDLE_ON) {
            LED_On(&xLedUp, 0.2, false); LED_On(&xLedDown, 0.2, false);
        } else if (xIdleSignal == FEEDBACK_IDLE_OFF) {
            LED_Off(&xLedUp, false); LED_Off(&xLedDown, false);
        } else {
            LED_Off(&xLedUp, false);
            LED_Blink(&xLedDown, SIGNAL_ERROR_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_ERROR_ON, SIGNAL_ERROR_OFF, false);
        }
        MTX_Unlock(&xFeedbackMtx);

        TMR_delay(100*TIMER_MSEG);
    }
}

bool FEEDBACK_Init(int iCore)
{
    // ESP_LOGI(TAG_FEEDBACK, "Starting feedback task");
    MTX_Config(&xFeedbackMtx, MTX_DEF);
    bMotionSignal = false;
    NVS_Init();
    if (NVS_ReadInt8(NVM_LED_IDLE_SIGNAL, (uint8_t*)&xIdleSignal) == false) xIdleSignal = DEFAULT_IDLE_SIGNAL;
    if (NVS_ReadBoolean(NVM_KEY_HIDE_SIGNAL, &bEnableHideSignal) == false) bEnableHideSignal = DEFAULT_FEEDBACK_HIDE;
    if (LED_ConfigSTD(&xLedUp, PIN_LED_UP, true, FADETIME_LEDS) == false) return false;
    if (LED_ConfigSTD(&xLedDown, PIN_LED_DOWN, true, FADETIME_LEDS) == false) return false;
    return xTaskCreatePinnedToCore(_feedback_signal_task, "feedback_task", 2048, NULL, 5, NULL, iCore);
    return true;	
}

void FEEDBACK_CustomSignal(float fLum, uint16_t uiTimeOn, uint16_t uiTimeOff, uint16_t uiTimeMs)
{
    if (!uiTimeMs) return;              // If the time to maintain the signal is '0' don't apply the change

    MTX_Lock(&xFeedbackMtx);
    bMotionSignal = false;
    fLum = (fLum <= 0) ? 0 : (fLum > 100) ? 1 : fLum/100;
    LED_Off(&xLedUp, false);
    if (!fLum) {
        LED_Off(&xLedDown, false);
    } else if ((uiTimeOn) && (uiTimeOff) && ((uiTimeOn+uiTimeOff)<=uiTimeMs)) { 
        LED_Blink(&xLedDown, fLum, uiTimeMs/(uiTimeOn+uiTimeOff), uiTimeOn, uiTimeOff, false);
    } else { 
        LED_On(&xLedDown, fLum, false);
    }

    TMR_SetPollTimer(&xTimerSignal, uiTimeMs*TIMER_MSEG);
    MTX_Unlock(&xFeedbackMtx);
}

void FEEDBACK_OfflineSignal(void)
{
    FEEDBACK_CustomSignal(SIGNAL_OFFLINE_LEVEL, SIGNAL_OFFLINE_ON, SIGNAL_OFFLINE_OFF, SIGNAL_OFFLINE_DURATION);
}

void FEEDBACK_InclusionSignal(void)
{
    FEEDBACK_CustomSignal(SIGNAL_INCLUSION_LEVEL, SIGNAL_INCLUSION_ON, SIGNAL_INCLUSION_OFF, SIGNAL_INCLUSION_DURATION);
}

void FEEDBACK_CalibratingSignal(void)
{
    MTX_Lock(&xFeedbackMtx);
    bMotionSignal = false;
    LED_Blink(&xLedUp, SIGNAL_CALIBRATE_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_CALIBRATE_ON, SIGNAL_CALIBRATE_OFF, false);
    LED_Blink(&xLedDown, SIGNAL_CALIBRATE_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_CALIBRATE_ON, SIGNAL_CALIBRATE_OFF, false);
    TMR_SetPollTimer(&xTimerSignal, SIGNAL_CALIBRATE_DURATION*TIMER_MSEG);
    MTX_Unlock(&xFeedbackMtx);
}

void FEEDBACK_ErrorSignal(void)
{
    FEEDBACK_CustomSignal(SIGNAL_ERROR_LEVEL, SIGNAL_ERROR_ON, SIGNAL_ERROR_OFF, SIGNAL_ERROR_DURATION);
}

void FEEDBACK_MotionSignal(uint16_t uiTimeMs)
{
    MTX_Lock(&xFeedbackMtx);
    bMotionSignal = true;
    TMR_SetPollTimer(&xTimerSignal, ((uint64_t)uiTimeMs)*TIMER_SEG);
    MTX_Unlock(&xFeedbackMtx);
}

void FEEDBACK_StopSignal(void)
{
    MTX_Lock(&xFeedbackMtx);
    bMotionSignal = false;
    LED_On(&xLedDown, SIGNAL_BLIND_STOP_LEVEL/100, false); LED_On(&xLedUp, SIGNAL_BLIND_STOP_LEVEL/100, false);
    TMR_SetPollTimer(&xTimerSignal, SIGNAL_BLIND_STOP_DURATION*TIMER_MSEG);
    MTX_Unlock(&xFeedbackMtx);
}

void FEEDBACK_SetIdleSignal(FEEDBACK_IDLE_SIGNALS_t xSignal)
{
    if (xSignal != xIdleSignal) {
        if (NVS_WriteInt8(NVM_LED_IDLE_SIGNAL, (int8_t)xSignal) == false) { ESP_LOGE(TAG_FEEDBACK, "Fail saving IDLE SIGNAL"); return; }
        xIdleSignal = xSignal;
    }
}

FEEDBACK_IDLE_SIGNALS_t FEEDBACK_GetIdleSignal(void)      { return xIdleSignal; }

void FEEDBACK_EnableHideSignal(bool bEnable)
{
    if (bEnable != bEnableHideSignal) {
        if (NVS_WriteBoolean(NVM_KEY_HIDE_SIGNAL, bEnable) == false) { ESP_LOGE(TAG_FEEDBACK, "Fail saving HIDE SIGNAL"); return; }
        bEnableHideSignal = bEnable;
    }
}

bool FEEDBACK_HideSignalIsEnabled(void)                       { return bEnableHideSignal; }
