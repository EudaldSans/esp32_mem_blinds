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

#include "esp_log.h"
#include <stddef.h>

/* DEFINES */
/* ------- */
#define TAG_BUTTON                  "[TEST BUTTON]"
#define TAG_AUX                     "[AUX IN]"
#if defined(CONFIG_BOARD_MEM_BLINDS) || defined(CONFIG_BOARD_LOLIN)
    #define N_BUTTONS 2
#else
    #define N_BUTTONS 0
#endif


/* TYPES */
/* ----- */
typedef struct ButtonPulse_s {
    uint16_t shortPulsations;
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
static PULSE_OBJ xButtons[N_BUTTONS];

/* EXTERNAL VARIABLES */
/* ------------------ */
ButtonPulse_t pulsations[N_BUTTONS];


/* CODE */
/* ---- */
void _test_button_callback(size_t i, bool bCompleted, uint64_t uiTime){
#if defined(CONFIG_BOARD_MEM_BLINDS) || defined(CONFIG_BOARD_LOLIN)
    if (bCompleted == false) return;
    switch (GPULSE_TimeToPulseType(uiTime)) {
        case PULSE_NULL:        ESP_LOGW(TAG_BUTTON, "Button %d : Pulse NULL", i);
                                break;

        case PULSE_SHORT:       ESP_LOGI(TAG_BUTTON, "Button %d : short pulse", i);
                                pulsations[i].shortPulsations++;
                                break;

        default:                ESP_LOGI(TAG_BUTTON, "Button %d : long pulse", i);
                                pulsations[i].longPulsations++;
			                    break;
    } 	
#endif
}

#ifdef CONFIG_BOARD_MEM_BLINDS
static void _factory_button_up_callback(bool bCompleted, uint64_t uiTime) 
{
    _test_button_callback(0, bCompleted, uiTime);
}

static void _factory_button_down_callback(bool bCompleted, uint64_t uiTime) 
{
    _test_button_callback(1, bCompleted, uiTime);
}

#endif

bool ButtonTest_Init(uint8_t uiCore)
{
    // GPULSE_ConfigPulseButton(PIN_INPUT_AUXILIAR, false, 50, 60, GPIO_INPUT_PULLUP, uiCore, _auxiliar_input_callback, &xInputAux);
    #if defined(CONFIG_BOARD_MEM_BLINDS)
        ESP_LOGI(TAG_BUTTON, "Initializing test_button module");
        GPULSE_ConfigPushButton(PIN_INPUT_UP,  false, GPIO_INPUT_PULLUP, uiCore, _factory_button_up_callback, &xButtons[0]);
        GPULSE_ConfigPushButton(PIN_INPUT_DOWN,  false, GPIO_INPUT_PULLUP, uiCore, _factory_button_down_callback, &xButtons[1]);
    #endif
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