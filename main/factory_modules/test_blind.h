#ifndef _TEST_BLIND
#define _TEST_BLIND

/* INCLUDES */
/* -------- */
#include <stdint.h>
#include <stdbool.h>
#include "../BLINDS_Load.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool BlindTest_Init(uint8_t uiCore);
void BlindTest_SetStatus(bool bStatus);
bool BlindTest_GetStatus(void);
void BlindTest_SetSense(bool bUp);
bool BlindTest_GetSense(void);

#endif