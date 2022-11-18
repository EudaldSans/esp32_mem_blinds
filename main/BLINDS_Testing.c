/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		08-04-2020      POL	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "sdkconfig.h"
#include "FACTORY_Main.h"

#include "BLINDS_Button.h"
#include "BLINDS_Feedback.h"
#include "BLINDS_Load.h"
#include "BLINDS_Meter.h"
#include "ges_HLW8012.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_MAIN            "[FACTORY_TESTING]"

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
bool TEST_Init(void)
{
    // Buttons
    FACTORY_ConfigButton(0, FACTORY_BUTTON_PUSH, PIN_INPUT_UP, false, GPIO_INPUT_PULLUP, 0, 0);
    FACTORY_ConfigButton(1, FACTORY_BUTTON_PUSH, PIN_INPUT_DOWN, false, GPIO_INPUT_PULLUP, 0, 0);

    // Leds
    FACTORY_ConfigLed(0, PIN_LED_UP, true);
    FACTORY_ConfigLed(1, PIN_LED_DOWN, true);
    
    // Sync
    FACTORY_ConfigSync(PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK);

    // Metering
    METER_Init(CONFIG_TEST_METER_CORE);
    FACTORY_ConfigGetPowerCallback(HLW8012_GetMeanPower);
    FACTORY_ConfigGetCurrentCallback(HLW8012_GetMeanCurrent);
    FACTORY_ConfigGetVoltageCallback(HLW8012_GetMeanVoltage);
    FACTORY_ConfigGetKpCallback(HLW8012_GetKp); FACTORY_ConfigSetKpCallback(HLW8012_SetKp);
    FACTORY_ConfigGetKiCallback(HLW8012_GetKi); FACTORY_ConfigSetKiCallback(HLW8012_SetKi);
    FACTORY_ConfigGetKvCallback(HLW8012_GetKv); FACTORY_ConfigSetKvCallback(HLW8012_SetKv);
    FACTORY_ConfigCalibrateCallback(HLW8012_Calibrate);

    TEST_FactoryTestStart();
    return true;
}

bool TEST_IsPassed(void)    { return TEST_IsFactoryTestPassed(); }




