/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		22-07-2019      AML	    Creacion
***********************************************************************************************/

#ifndef _BLINDS_BUTTON
#define _BLINDS_BUTTON

/* INCLUDES */
/* -------- */
#include "ges_pulse.h"

#include <stdbool.h>
#include <stdint.h>

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define PIN_INPUT_UP                19//10
#define PIN_INPUT_DOWN              15

#define DEFAULT_LOCK_BUTTON         false

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool BUTTON_Init(int iCore);

uint64_t BUTTON_UpPressed(void);
uint64_t BUTTON_DownPressed(void);

void BUTTON_SetLockButton(bool bLock);
bool BUTTON_IsButtonLocked(void);


#endif