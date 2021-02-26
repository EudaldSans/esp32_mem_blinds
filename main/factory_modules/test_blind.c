/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		29-01-2020      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "test_blind.h"
#include "esp_log.h"

#include "ges_gpio.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_TEST_BLIND          "[TEST_BLIND]"

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
bool BlindTest_Init(uint8_t uiCore)
{
    ESP_LOGI(TAG_TEST_BLIND, "Initializing Blind...");
    GPIO_ConfigOutput(PIN_TRIAC_ON, true);
    GPIO_ConfigOutput(PIN_RELAY_UPDOWN, false);
    return true;
}

void BlindTest_SetStatus(bool bStatus)      
{ 
    GPIO_SetOutput(PIN_TRIAC_ON, !bStatus); 
    ESP_LOGI(TAG_TEST_BLIND, "Set new STATUS %d with DIRECTION %d", BlindTest_GetStatus(), BlindTest_GetDirection());
}

bool BlindTest_GetStatus(void)              { return !GPIO_GetOutput(PIN_TRIAC_ON); }

void BlindTest_SetDirection(bool bUp)
{
bool bLastStatus = BlindTest_GetStatus();

    if (bLastStatus == true) BlindTest_SetStatus(false);
    GPIO_SetOutput(PIN_RELAY_UPDOWN, bUp);
    TMR_delay(50*TIMER_MSEG);
    if (bLastStatus) BlindTest_SetStatus(true);
    ESP_LOGI(TAG_TEST_BLIND, "Set new DIRECTION %d with STATUS %d", BlindTest_GetDirection(), BlindTest_GetStatus());
}

bool BlindTest_GetDirection(void)               { return GPIO_GetOutput(PIN_RELAY_UPDOWN); }