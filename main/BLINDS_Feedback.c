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

/* DEFINES */
/* ------- */
#define TAG_FEEDBACK                "[FEEDBACK]"

#define NVM_LED_IDLE_SIGNAL         "idle_signal"

/* TYPES */
/* ----- */

/* INTERNAL FUNCTIONS */
/* ------------------ */
void _feedback_signal_Task(void * xParams);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
xMTX_t xFeedbackMtx = MTX_INIT_CONFIG_DEFAULT;
static xLED_t xLedUp;
static xLED_t xLedDown;
FEEDBACK_IDLE_SIGNALS xIdleSignal;
POLLTIMER xTimerSignal;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
void _feedback_signal_Task(void * xParams)
{
    while (1)
    {
        if (TMR_GetPollTimeRunning(&xTimerSignal) == true) {       // Wait here while timer is running
            TMR_GetPollTimeElapsed(&xTimerSignal);
        } else if ((SRELAY_GetStatus() == false) && (METER_GetMaxCurrentDetected() == true)) {
            LED_Blink(&xLedFront, SIGNAL_OVERCURRENT_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_OVERCURRENT_ON, SIGNAL_OVERCURRENT_OFF, false);
        // } else if ((SRELAY_GetStatus() == false) && (METER_GetOverPowerDetected() == true)) {
        //     LED_Blink(&xLedFront, 1, LED_BLINK_ALWAYS, 500, 500, false);
        } else if (MEM_GetStatus() == MEM_STATUS_HIDE) {
            LED_Blink(&xLedFront, SIGNAL_OFFLINE_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_OFFLINE_ON, SIGNAL_OFFLINE_OFF, false);
        } else if (xIdleSignal == FEEDBACK_IDLE_OFF) {
            LED_Off(&xLedFront);
        } else if (xIdleSignal == FEEDBACK_IDLE_ON) {
            LED_On(&xLedFront, (SRELAY_GetStatus() == true) ? 1 : 0.5);
        } else if (xIdleSignal == FEEDBACK_IDLE_STATUS) {
            if (SRELAY_GetStatus() == true) LED_On(&xLedFront, 1);
        } else {
            LED_Blink(&xLedFront, SIGNAL_ERROR_LEVEL/100, LED_BLINK_ALWAYS, SIGNAL_ERROR_ON, SIGNAL_ERROR_OFF, false);
        }

        TMR_delay(100*TIMER_MSEG);
    }
}

bool FEEDBACK_Init(int iCore)
{
float fMax;

    // ESP_LOGI(TAG_FEEDBACK, "Starting feedback task");
    MTX_Config(&xFeedbackMtx, MTX_DEF);
    NVS_Init();
    if (NVS_ReadInt8(NVM_LED_IDLE_SIGNAL, (uint8_t*)&xIdleSignal) == false) xIdleSignal = DEFAULT_IDLE_SIGNAL;
    if (LED_ConfigSTD(&xLedUp, PIN_LED, false, fMax, FADETIME_LEDS) == false) return false;
    if (LED_ConfigSTD(&xLedDown, PIN_LED, false, fMax, FADETIME_LEDS) == false) return false;
    return xTaskCreatePinnedToCore(_feedback_signal_Task, "feedback_task", 2048, NULL, 5, NULL, iCore);
    return true;	
}

void FEEDBACK_CustomSignal(float fLum, uint16_t uiTimeOn, uint16_t uiTimeOff, uint16_t uiTimeMs)
{
    if (!uiTimeMs) return;              // If the time to maintain the signal is '0' don't apply the change

    MTX_Lock(&xFeedbackMtx);
    fLum = (fLum <= 0) ? 0 : (fLum > 1) ? 1 : fLum/100;
    LED_Off(&xLedUp);
    if (!fLum) {
        LED_Off(&xLedDown);
    } else if ((uiTimeOn) && (uiTimeOff) && ((uiTimeOn+uiTimeOff) < uiTimeMs)) { 
        LED_Blink(&xLedDown, fLum, uiTimeMs/(uiTimeOn+uiTimeOff), uiTimeOn, uiTimeOff, false);
    } else { 
        LED_On(&xLedDown, fLum);
    }
        
    TMR_SetPollTimer(&xTimerSignal, uiTimeMs*TIMER_MSEG);
    MTX_Lock(&xFeedbackMtx);
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

}

void FEEDBACK_ErrorSignal(void)
{
    
}

void FEEDBACK_SetIdleSignal(FEEDBACK_IDLE_SIGNALS xSignal)
{
    if (xSignal != xIdleSignal) {
        if (NVS_WriteInt8(NVM_LED_IDLE_SIGNAL, (int8_t)xSignal) == false) { ESP_LOGE(TAG_FEEDBACK, "Fail saving IDLE SIGNAL"); return; }
        xIdleSignal = xSignal;
    }
}

FEEDBACK_IDLE_SIGNALS FEEDBACK_GetIdleSignal(void)      { return xIdleSignal; }