/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		29-01-2020      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "BLINDS_Relay.h"

#include "esp_log.h"

#include "MEM_Main.h"
#include "ges_relay.h"
#include "ges_nvs.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_RELAY                   "[RELAY]"

#define NVM_OPERATE_TIME_KEY        "operate"
#define NVM_RELEASE_TIME_KEY        "release"
// #define NVM_TIME_ON                 "switchon_time"
#define NVM_TIME_OFF                "delayoff"
#define NVM_TIME_IN_ON              "timedon"
#define NVM_RESTORE_LAST            "restore"
// #define NVM_AFTER_RESET             "after_reset_status"
#define NVM_LAST_STATUS             "relay_std"

/* INTERNAL FUNCTIONS */
/* ------------------ */
void _status_relay_changed(bool bOn);
void _config_relay_changed(uint16_t uiOperateTime, uint16_t uiReleaseTime);
void _set_relay(bool bStatus);
void _relay_timed_action(TimerHandle_t xTimer);
void _relay_notify(TimerHandle_t xTimer);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
xRELAY_t xRele;
uint16_t uiTimeInOn, uiTimeOff;//, uiTimeOn;
bool bRestoreLast, bAfterReset, bLastValue;
bool bStatusRele = false;
bool bNextStatus = false;

TimerHandle_t xTimerAction;
TimerHandle_t xTimerNotify;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */

void _relay_timed_action(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG_RELAY, "Set timed action %d", (int)bNextStatus);
    _set_relay(bNextStatus);
    if ((bNextStatus == true) && (uiTimeInOn)) {
        bNextStatus = false;
        xTimerChangePeriod(xTimerAction, pdMS_TO_TICKS(uiTimeInOn*1000), 10);
        xTimerStart(xTimerAction, 10);
    }
}

void _relay_notify(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG_RELAY, "Relay status notify & save %d", (int)bStatusRele);
    MEM_SendInfo(1, PROTOCOL_VARIABLE_STATUS, (double)bStatusRele);
    if (bLastValue != bStatusRele) {
        bLastValue = bStatusRele;
        NVS_WriteBoolean(NVM_LAST_STATUS, bLastValue);
    }
}

void _status_relay_changed(bool bOn)
{
    ESP_LOGI(TAG_RELAY, "callback relay changed %d", (int)bOn);
    bStatusRele = bOn;
    xTimerReset(xTimerNotify, 10);
}

void _config_relay_changed(uint16_t uiOperateTime, uint16_t uiReleaseTime)
{
    if (RELAY_GetOperateTime(&xRele) != uiOperateTime) NVS_WriteInt16(NVM_OPERATE_TIME_KEY, uiOperateTime);
	if (RELAY_GetReleaseTime(&xRele) != uiReleaseTime) NVS_WriteInt16(NVM_RELEASE_TIME_KEY, uiReleaseTime);
}

void _set_relay(bool bStatus)
{
    if (bStatus == true) RELAY_On(&xRele, false); else RELAY_Off(&xRele, false);
}

bool SRELAY_Init(int iCore)
{
uint16_t uiOperate, uiRelease;

    ESP_LOGI(TAG_RELAY, "Initializing relay...");
    NVS_Init();
    // Load NVM
    // if (NVS_ReadInt(NVM_TIME_ON, &uiTimeOn) == false) uiTimeOn = DEFAULT_TIME_ON;
    if (NVS_ReadInt16(NVM_TIME_OFF, &uiTimeOff) == false) uiTimeOff = DEFAULT_TIME_OFF;
    if (NVS_ReadInt16(NVM_TIME_IN_ON, &uiTimeInOn) == false) uiTimeInOn = DEFAULT_TIME_IN_ON;
    if (NVS_ReadBoolean(NVM_RESTORE_LAST, &bRestoreLast) == false) bRestoreLast = DEFAULT_RESTORE_LAST;
    // if (NVS_ReadBoolean(NVM_AFTER_RESET, &bAfterReset) == false) bAfterReset = DEFAULT_INITIAL_STATUS;
    if (NVS_ReadBoolean(NVM_LAST_STATUS, &bLastValue) == false) bLastValue = false;
    // Config relays
    if (NVS_ReadInt16(NVM_OPERATE_TIME_KEY, &uiOperate) == false) uiOperate = DEFAULT_RELAY_TIME; 
    if (NVS_ReadInt16(NVM_RELEASE_TIME_KEY, &uiRelease) == false) uiRelease = DEFAULT_RELAY_TIME;
    if (RELAY_Config(PIN_RELAY, true, PIN_V, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, PIN_I, PIN_VREF, VREF_LEVEL, uiOperate, uiRelease, _status_relay_changed, _config_relay_changed, &xRele) == false) ESP_LOGE(TAG_RELAY, "Relay unconfigured");

    xTimerAction = xTimerCreate("timer_relay_action", pdMS_TO_TICKS(1000), pdFALSE, (void *)1, _relay_timed_action);
    xTimerNotify = xTimerCreate("timer_relay_notify", pdMS_TO_TICKS(2000), pdFALSE, (void *)2, _relay_notify);

    _set_relay((bRestoreLast) ? bLastValue : false);
    return true;
}

bool SRELAY_GetStatus(void)                      { return bStatusRele; }

void SRELAY_ON(void)
{
    ESP_LOGI(TAG_RELAY, "Switching on");
    // if (uiTimeOn) {
    //     bNextStatus = true;
    //     xTimerStop(xTimerAction, 10); xTimerChangePeriod(xTimerAction, pdMS_TO_TICKS(uiTimeOn*1000), 10); xTimerStart(xTimerAction, 10);
    // } else {
        _set_relay(true);
    // }
}

void SRELAY_OFF(void)
{
    ESP_LOGI(TAG_RELAY, "Switching off");
    if (uiTimeOff) {
        bNextStatus = false;
        xTimerStop(xTimerAction, 10); xTimerChangePeriod(xTimerAction, pdMS_TO_TICKS(uiTimeOff*1000), 10); xTimerStart(xTimerAction, 10);
    } else {
        _set_relay(false);
    }
}

void SRELAY_Toggle(void)
{
    ESP_LOGI(TAG_RELAY, "Reverse state");
    if (bStatusRele == true) SRELAY_OFF(); else SRELAY_ON();
}

// void SRELAY_SetTimeOn(uint16_t uiTime)
// {
//     if (uiTimeOn != uiTime) {
//         NVS_WriteInt(NVM_TIME_ON, uiTime);
//         NVS_ReadInt(NVM_TIME_ON, &uiTimeOn);
//     }
// }

void SRELAY_SetTimeInOn(uint16_t uiTime)
{
    if (uiTimeInOn != uiTime) {
        if (NVS_WriteInt16(NVM_TIME_IN_ON, uiTime) == false) { ESP_LOGE(TAG_RELAY, "Fail saving TIMED ON"); return; }
        uiTimeInOn = uiTime;
    }
}

void SRELAY_SetTimeOff(uint16_t uiTime)
{
    if (uiTimeOff != uiTime) {
        if (NVS_WriteInt16(NVM_TIME_OFF, uiTime) == false) { ESP_LOGE(TAG_RELAY, "Fail saving DELAY OFF"); return; }
        uiTimeOff = uiTime;
    }
}

void SRELAY_SetRestoreLast(bool bStatus)
{
    if (bRestoreLast != bStatus) {
        if (NVS_WriteBoolean(NVM_RESTORE_LAST, bStatus) == false) { ESP_LOGE(TAG_RELAY, "Fail saving RESTORE LAST"); return; }
        bRestoreLast = bStatus;
    }
}

// void SRELAY_SetStatusAfterReset(bool bStatus)
// {
//     if (bAfterReset != bStatus) {
//         if (NVS_WriteBoolean(NVM_LAST_STATUS, bStatus) == false) { ESP_LOGE(TAG_RELAY, "Fail saving DEFAULT STATUS"); return; }
//         bAfterReset = bStatus;
//     }
// }

// uint16_t SRELAY_GetTimeOn(void)                 { return uiTimeOn; }
uint16_t SRELAY_GetTimeInOn(void)               { return uiTimeInOn; }
uint16_t SRELAY_GetTimeOff(void)                { return uiTimeOff; }
bool SRELAY_GetRestoreLast(void)                { return bRestoreLast; }
// bool SRELAY_GetStatusAfterReset(void)           { return bAfterReset; }
