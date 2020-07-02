/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		23-01-2020      PDM	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "test_leds.h"

#include "esp_log.h"
#include <stddef.h>
#include "esp_log.h"

#include "ges_led.h"
#include "ges_nvs.h"
#include "ges_timer.h"
#include "ges_color.h"


/* DEFINES */
/* ------- */
#define TAG_TEST_LEDS               "[LEDS_TEST]"
#ifdef CONFIG_BOARD_MEM_BLINDS
    #define N_LEDS                      2
#else
    #define N_LEDS                      0 
#endif

/* INTERNAL VARIABLES */
/* ------------------ */
static xLED_t xLeds[N_LEDS];

/* CODE */
/* ---- */
bool LedsTest_Init(uint8_t uiCore)
{
    #if defined(CONFIG_BOARD_MEM_BLINDS)
        LED_ConfigSTD(&xLeds[0], PIN_LED_UP, true, FADETIME_LEDS);
        LED_ConfigSTD(&xLeds[1], PIN_LED_DOWN, true, FADETIME_LEDS);
        for(int i=0; i<N_LEDS; i++){
            LedsTest_SetVal(i, 0);
        }
    #endif
    return true;	
}

int LedsTest_GetMax(int iLed)
{
    float max;

    if(iLed<N_LEDS){
        LED_GetMaxBrightnessSTD(&(xLeds[iLed]), &max);
        return (int)(max*100.0);
    }
    else{
        return 0;
    }
}

bool LedsTest_SetMax(int iLed, int iMaxVal)
{
    ESP_LOGI(TAG_TEST_LEDS, "LedsTest.setMax(%d, %d)", iLed, iMaxVal);
    if(iLed < N_LEDS){
        LED_SetMaxBrightnessSTD(&(xLeds[iLed]), (double)(iMaxVal/100.0));
    }
    else{
        return false;
    }
    return true;
}

int LedsTest_GetVal(int iLed)
{
    if(iLed<N_LEDS){
        return (int)(xLeds[iLed].fBrightness * 100.0);
    }
    else{
        return 0;
    }
}

bool LedsTest_SetVal(int iLed, int iVal)
{
    ESP_LOGI(TAG_TEST_LEDS, "LedsTest.setVal(%d, %d)", iLed, iVal);
    bool ret = true;

    if(iLed < N_LEDS){
        LED_On(&xLeds[iLed], (double)(iVal/100.0), false);
    }
    else{
        ret = false;
    }
    return ret;
}

int LedsTest_getTotalLeds(void)
{
    return N_LEDS;
}