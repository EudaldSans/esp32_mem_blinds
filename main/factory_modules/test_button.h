/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		22-07-2019      AML	    Creacion
***********************************************************************************************/

#ifndef _TEST_BUTTON
#define _TEST_BUTTON

/* INCLUDES */
/* -------- */
#include "ges_pulse.h"
#include <stdbool.h>
#include <stdint.h>

#define PIN_INPUT_UP    19
#define PIN_INPUT_DOWN  15

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool ButtonTest_Init(uint8_t uiCore);
void ButtonTest_Reset(void);
int ButtonTest_GetTotalButtons(void);
int ButtonTest_GetShortPulsations(int button);
int ButtonTest_GetLongPulsations(int button);
int64_t ButtonTest_GetTimestamp(int button);

#endif
