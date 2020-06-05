/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		29-01-2020      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "test_relay.h"
#include "esp_log.h"
#include "ges_relay.h"
#include "ges_nvs.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#if defined(CONFIG_BOARD_MEM_BLINDS)
    #define N_RELAYS 2 
#else
    #define N_RELAYS 0
#endif

#define TAG_TEST_RELAY              "[TEST_RELAY]"
#define PIN_SYNC                    0

/* INTERNAL FUNCTIONS */
/* ------------------ */
void _status_test_relay_up_changed(bool bOn);
void _status_test_relay_down_changed(bool bOn);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
static uint16_t uiCalibrationsDone[N_RELAYS];
xRELAY_t xRelays[N_RELAYS];

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
void _status_test_relay_up_changed(bool bOn)
{
    ESP_LOGI(TAG_TEST_RELAY, "callback relay up changed %d", (int)bOn);
}

void _status_test_relay_down_changed(bool bOn)
{
    ESP_LOGI(TAG_TEST_RELAY, "callback relay down changed %d", (int)bOn);
}


bool RelayTest_Init(uint8_t uiCore)
{
    ESP_LOGI(TAG_TEST_RELAY, "Initializing relay...");
    // Config relays
    #if defined(CONFIG_BOARD_MEM_BLINDS)
    if (RELAY_Config(PIN_RELAY_UP, true, PIN_V, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, PIN_I, PIN_VREF, VREF_LEVEL, _status_test_relay_up_changed, &xRelays[0]) == false) ESP_LOGE(TAG_TEST_RELAY, "Relay 0 unconfigured");
    if (RELAY_Config(PIN_RELAY_DOWN, true, PIN_V, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, PIN_I, PIN_VREF, VREF_LEVEL, _status_test_relay_down_changed, &xRelays[1]) == false) ESP_LOGE(TAG_TEST_RELAY, "Relay 1 unconfigured");
    #endif

    return true;
}

int RelayTest_GetStatus(int iRelay)
{
    if(iRelay < N_RELAYS){
        return xRelays[iRelay].bState;
    }
    else{
        return -1;
    }
}

int RelayTest_SetStatus(int iRelay, bool bStatus)
{
    if(iRelay < N_RELAYS){
        ESP_LOGI(TAG_TEST_RELAY, "relay[%d].status : %d", iRelay, bStatus);
        if(bStatus)
            RELAY_On(&xRelays[iRelay], false);
        else
            RELAY_Off(&xRelays[iRelay], false);
        return (int)true;
    }
    else{
        return -1;
    }
}

int RelayTest_GetTotalRelays(void)
{
    return N_RELAYS;
}

int RelayTest_GetCalibrations(int iRelay)
{
    if(iRelay < N_RELAYS){
        return uiCalibrationsDone[iRelay];
    }
    else{
        return -1;
    }
}

int RelayTest_Calibrate(int iRelay)
{
    if(iRelay < N_RELAYS){
        xRELAY_t * relay = &xRelays[iRelay];
        ESP_LOGI(TAG_TEST_RELAY, "relay[%d].calibrate()", iRelay);
        RELAY_FactoryCalibrate(PIN_SYNC, GPIO_INPUT_PULLUP, GPIO_INPUT_INTERRUPT_LOW, relay);
        ESP_LOGI(TAG_TEST_RELAY, "\nxRelays[%d].uiReleaseTime : %d\nxRelays[%d].uiOperateTime : %d",
            iRelay, relay->uiOperateTime,
            iRelay, relay->uiReleaseTime
        );
        uiCalibrationsDone[iRelay]++;
        return (int)true;
    }
    else{
        return -1;
    }
}

int RelayTest_ResistorCalibrate(int iRelay)
{
    if(iRelay < N_RELAYS){
        xRELAY_t * relay = &xRelays[iRelay];
        ESP_LOGI(TAG_TEST_RELAY, "relay[%d].resistorCalibrate()", iRelay);
        RELAY_Calibrate(relay);
        ESP_LOGI(TAG_TEST_RELAY, "\nxRelays[%d].uiReleaseTime : %d\nxRelays[%d].uiOperateTime : %d",
            iRelay, relay->uiOperateTime,
            iRelay, relay->uiReleaseTime
        );
        uiCalibrationsDone[iRelay]++;
        return (int)true;
    }
    else{
        return -1;
    }
}

int RelayTest_GetOperateTime(int iRelay)
{
    if(iRelay < N_RELAYS){
        return xRelays[iRelay].uiOperateTime;
    }
    else{
        return -1;
    }
}

int RelayTest_SetOperateTime(int iRelay, int iTime){
    int ret = (int)false;
    if(iRelay < N_RELAYS){
        ESP_LOGI(TAG_TEST_RELAY, "relay[%d].operateTime : %d", iRelay, iTime);
        RELAY_SetOperateTime(&xRelays[iRelay], iTime);
        return ret;
    }
    else{
        return -1;
    }
}

int RelayTest_GetReleaseTime(int iRelay)
{
    if(iRelay < N_RELAYS){
        return xRelays[iRelay].uiReleaseTime;
    }
    else{
        return -1;
    }
}

int RelayTest_SetReleaseTime(int iRelay, int iTime)
{
    int ret = (int)false;
    if(iRelay < N_RELAYS){
        ESP_LOGI(TAG_TEST_RELAY, "relay[%d].releaseTime : %d", iRelay, iTime);
        RELAY_SetReleaseTime(&xRelays[iRelay], iTime);
        return ret;
    }
    else{
        return -1;
    }
}
