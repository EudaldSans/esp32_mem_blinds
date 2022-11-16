/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		22-07-2019      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "test_button.h"
#include "factory_testing.h"

#include "esp_log.h"
#include "ges_wifi.h"
#include <stddef.h>

/* DEFINES */
/* ------- */
#define TAG_BUTTON                  "[TEST BUTTON]"
#define TAG_AUX                     "[AUX IN]"

#define N_BUTTONS 2


/* TYPES */
/* ----- */
typedef struct ButtonPulse_s {
    uint16_t shortPulsations;
    int64_t  iTimestamp;
    uint16_t longPulsations;
} ButtonPulse_t;

/* INTERNAL FUNCTIONS */
/* ------------------ */

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
static PULSE_OBJ_t xButtons[N_BUTTONS];
WIFI_MODES_t mode_managed = WIFI_MODE_MANAGED;
bool mode_ap = false;

/* EXTERNAL VARIABLES */
/* ------------------ */
ButtonPulse_t pulsations[N_BUTTONS];


/* CODE */
/* ---- */
static void start_ap_mode(){
    ESP_LOGI(TAG_BUTTON, "Start "FACTORY_TEST_AP_SSID" AP");
    WIFI_Disable();
    WIFI_Init(WIFI_MODE_APS, FACTORY_TEST_AP_SSID, FACTORY_TEST_AP_PSK, WIFI_CHANNEL_ALL);      
    mode_ap = true;
}

void _test_button_callback(size_t i, bool bCompleted, uint64_t uiTime){
    if(bCompleted && (GPULSE_TimeToPulseType(uiTime)) &&  WIFI_IsConnected(&mode_managed) == false && mode_ap == false){
        start_ap_mode();
    }

    if (bCompleted == false) return;
    switch (GPULSE_TimeToPulseType(uiTime)) {
        case PULSE_NULL:        ESP_LOGW(TAG_BUTTON, "Button %d : Pulse NULL", i);
                                break;

        case PULSE_SHORT:       ESP_LOGI(TAG_BUTTON, "Button %d : short pulse", i);
                                pulsations[i].shortPulsations++;
                                    pulsations[i].iTimestamp = esp_timer_get_time();
                                break;

        default:                ESP_LOGI(TAG_BUTTON, "Button %d : long pulse", i);
                                pulsations[i].longPulsations++;
                                    pulsations[i].iTimestamp = esp_timer_get_time();
			                    break;
    } 	

}

static void _factory_button_up_callback(bool bCompleted, uint64_t uiTime) 
{
    _test_button_callback(0, bCompleted, uiTime);
}

static void _factory_button_down_callback(bool bCompleted, uint64_t uiTime) 
{
    _test_button_callback(1, bCompleted, uiTime);
}

bool ButtonTest_Init(int iCore)
{
    ESP_LOGI(TAG_BUTTON, "Initializing test_button module");
    GPULSE_ConfigPushButton(PIN_INPUT_UP,  false, GPIO_INPUT_PULLUP, iCore, _factory_button_up_callback, &xButtons[0]);
    GPULSE_ConfigPushButton(PIN_INPUT_DOWN,  false, GPIO_INPUT_PULLUP, iCore, _factory_button_down_callback, &xButtons[1]);
    ButtonTest_Reset();
    return true;	
}

void ButtonTest_Reset(void)
{
    for(int i=0; i<N_BUTTONS; i++){
        pulsations[i].shortPulsations = 0;
        pulsations[i].longPulsations = 0;
    }
}

int ButtonTest_GetTotalButtons(void)
{
    return N_BUTTONS;
}

int ButtonTest_GetShortPulsations(int button)
{
    if(button < N_BUTTONS){
        return pulsations[button].shortPulsations;
    }
    else{
        return -1;
    }
}

int ButtonTest_GetLongPulsations(int button)
{
    if(button < N_BUTTONS){
        return pulsations[button].longPulsations;
    }
    else{
        return -1;
    }
}

int64_t ButtonTest_GetTimestamp(int button)
{
    return (button < N_BUTTONS) ? pulsations[button].iTimestamp : -1;
}
