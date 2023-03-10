/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		22-07-2019      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "BLINDS_Button.h"

#include "esp_log.h"
#include <stddef.h>

#include "ges_nvs.h"
#include "ges_timer.h"
#include "MEM_Main.h"
#include "BLINDS_Device.h"
#include "BLINDS_Feedback.h"
#include "BLINDS_Load.h"

/* DEFINES */
/* ------- */
#define TAG_BUTTON                  "[BUTTON]"
#define TAG_BUTTON_UP               "[BUTTON_UP]"
#define TAG_BUTTON_DOWN             "[BUTTON_DW]"

#define NVM_LOCK_BUTTON             "lock_button"

/* TYPES */
/* ----- */

/* INTERNAL FUNCTIONS */
/* ------------------ */
static void _button_up_callback(bool bComplete, uint64_t uiTime);
static void _button_down_callback(bool bComplete, uint64_t uiTime);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
static PULSE_OBJ_t xButtonUp;
static PULSE_OBJ_t xButtonDown;

static bool bLockButton = DEFAULT_LOCK_BUTTON;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
static void _button_up_callback(bool bCompleted, uint64_t uiTime) 
{
    ESP_LOGI(TAG_BUTTON_UP, "Detectected %d %lld", (int)bCompleted, uiTime);

    switch (GPULSE_TimeToPulseType(uiTime)) 
    {
        case PULSE_NULL:        if (bCompleted) ESP_LOGW(TAG_BUTTON_UP, "PULSE NULL");
                                break;

        case PULSE_SHORT:       if (bLockButton == false) {
                                    if (bCompleted) {
                                        if (LOAD_IsStopped() == true) {
                                            ESP_LOGI(TAG_BUTTON_UP, "PULSE UP");
                                            LOAD_Open();
                                            FEEDBACK_MotionSignal(60000);
                                        } else {
                                            ESP_LOGI(TAG_BUTTON_UP, "PULSE STOP");
                                            LOAD_Stop();
                                            FEEDBACK_StopSignal();
                                        }
                                    }
                                } else {
                                    FEEDBACK_ErrorSignal();
                                    ESP_LOGW(TAG_BUTTON_UP, "warning button is locked");
                                }
                                break;

        case PULSE_LONG10:
        case PULSE_LONG15:      if (bLockButton == false) {
                                    FEEDBACK_CalibratingSignal();
                                    if (bCompleted) {
                                        ESP_LOGI(TAG_BUTTON_UP, "PULSE CALIBRATE");
                                        LOAD_Calibrate();
                                    }
                                } else {
                                    FEEDBACK_ErrorSignal();
                                    ESP_LOGW(TAG_BUTTON_UP, "warning button is locked");
                                }
                                break;

        case PULSE_LONG30:      FEEDBACK_CustomSignal(SIGNAL_RESET_LEVEL, SIGNAL_RESET_ON, SIGNAL_RESET_OFF, SIGNAL_RESET_DURATION);
                                if (bCompleted) {
                                    ESP_LOGI(TAG_BUTTON_UP, "PULSE RESET DEFAULT");
                                    MEM_ResetToDefault();
                                }
                                break;

        case PULSE_LONG50:      FEEDBACK_CustomSignal(SIGNAL_INCLUSION_LEVEL, SIGNAL_INCLUSION_ON, SIGNAL_INCLUSION_OFF, SIGNAL_INCLUSION_DURATION);
                                if (bCompleted == true) {
                                    ESP_LOGI(TAG_BUTTON, "TOGGLE SIGNAL NOT INCLUDED");
                                    FEEDBACK_EnableHideSignal(!FEEDBACK_HideSignalIsEnabled());
                                }
                                break;

        default:                if (bLockButton == false) {
                                    FEEDBACK_CustomSignal(SIGNAL_INCLUSION_LEVEL, SIGNAL_INCLUSION_ON, SIGNAL_INCLUSION_OFF, SIGNAL_INCLUSION_DURATION);
                                    if (bCompleted) {
                                        ESP_LOGI(TAG_BUTTON_UP, "PULSE INCLUSION");
                                        MEM_StartVisibility();
                                    }
                                } else {
                                    FEEDBACK_ErrorSignal();
                                    ESP_LOGW(TAG_BUTTON_UP, "warning button is locked");
                                }
			                    break;
    } 	
}

static void _button_down_callback(bool bCompleted, uint64_t uiTime) 
{
    ESP_LOGI(TAG_BUTTON_DOWN, "Detectected %d %lld", (int)bCompleted, uiTime);

    switch (GPULSE_TimeToPulseType(uiTime)) 
    {
        case PULSE_NULL:        if (bCompleted) ESP_LOGW(TAG_BUTTON_DOWN, "PULSE NULL");
                                break;

        case PULSE_SHORT:       if (bLockButton == false) {
                                    if (bCompleted) {
                                        if (LOAD_IsStopped() == true) {
                                            ESP_LOGI(TAG_BUTTON_DOWN, "PULSE DOWN");
                                            LOAD_Close();
                                            FEEDBACK_MotionSignal(60000);
                                        } else {
                                            ESP_LOGI(TAG_BUTTON_DOWN, "PULSE STOP");
                                            LOAD_Stop();
                                            FEEDBACK_StopSignal();
                                        }
                                    }
                                } else {
                                    FEEDBACK_ErrorSignal();
                                    ESP_LOGW(TAG_BUTTON_DOWN, "warning button is locked");
                                }
                                break;
        
        case PULSE_LONG10:
        case PULSE_LONG15:      if (bLockButton == false) {
                                    FEEDBACK_CalibratingSignal();
                                    if (bCompleted) {
                                        ESP_LOGI(TAG_BUTTON_UP, "PULSE CALIBRATE");
                                        LOAD_Calibrate();
                                    }
                                } else {
                                    FEEDBACK_ErrorSignal();
                                    ESP_LOGW(TAG_BUTTON_UP, "warning button is locked");
                                }
                                break;

        case PULSE_LONG30:      FEEDBACK_CustomSignal(SIGNAL_RESET_LEVEL, SIGNAL_RESET_ON, SIGNAL_RESET_OFF, SIGNAL_RESET_DURATION);
                                if (bCompleted) {
                                    ESP_LOGI(TAG_BUTTON_DOWN, "PULSE RESET DEFAULT");
                                    MEM_ResetToDefault();
                                }
                                break;

        case PULSE_LONG50:      FEEDBACK_CustomSignal(SIGNAL_INCLUSION_LEVEL, SIGNAL_INCLUSION_ON, SIGNAL_INCLUSION_OFF, SIGNAL_INCLUSION_DURATION);
                                if (bCompleted == true) {
                                    ESP_LOGI(TAG_BUTTON, "TOGGLE SIGNAL NOT INCLUDED");
                                    FEEDBACK_EnableHideSignal(!FEEDBACK_HideSignalIsEnabled());
                                }
                                break;

        default:                if (bLockButton == false) {
                                    FEEDBACK_CustomSignal(SIGNAL_INCLUSION_LEVEL, SIGNAL_INCLUSION_ON, SIGNAL_INCLUSION_OFF, SIGNAL_INCLUSION_DURATION);
                                    if (bCompleted) {
                                        ESP_LOGI(TAG_BUTTON_DOWN, "PULSE INCLUSION");
                                        MEM_StartVisibility();
                                    }
                                } else {
                                    FEEDBACK_ErrorSignal();
                                    ESP_LOGW(TAG_BUTTON_DOWN, "warning button is locked");
                                }
			                    break;
    } 	
}

bool BUTTON_Init(int iCore)
{
    if (NVS_ReadBoolean(NVM_LOCK_BUTTON, &bLockButton) == false) bLockButton = DEFAULT_LOCK_BUTTON;
    ESP_LOGI(TAG_BUTTON, "Config buttons");
	if (GPULSE_ConfigPushButton(PIN_INPUT_UP,  false, GPIO_INPUT_PULLUP, iCore, _button_up_callback, &xButtonUp) == false) ESP_LOGE(TAG_BUTTON_UP, "Fail config button");
	if (GPULSE_ConfigPushButton(PIN_INPUT_DOWN, false, GPIO_INPUT_PULLUP, iCore, _button_down_callback, &xButtonDown) == false) ESP_LOGE(TAG_BUTTON_DOWN, "Fail config button");
    return true;	
}

uint64_t BUTTON_UpPressed(void)         
{ 
    return GPULSE_GetHoldTime(&xButtonUp);
}

uint64_t BUTTON_DownPressed(void)
{
    return GPULSE_GetHoldTime(&xButtonDown);
}

void BUTTON_SetLockButton(bool bLock)
{
    if (bLockButton != bLock) {
        if (NVS_WriteBoolean(NVM_LOCK_BUTTON, bLock) == false) { ESP_LOGE(TAG_BUTTON, "Fail saving LOCK"); return; }
        bLockButton = bLock;
    }
}

bool BUTTON_IsButtonLocked(void)            { return bLockButton; }
