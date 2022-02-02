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
bool BlindTest_Init(int iCore);
void BlindTest_SetStatus(bool bStatus);
bool BlindTest_GetStatus(void);
void BlindTest_SetDirection(bool bUp);
bool BlindTest_GetDirection(void);

#endif