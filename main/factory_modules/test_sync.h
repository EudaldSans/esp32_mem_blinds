#ifndef TEST_SYNC_H
#define TEST_SYNC_H

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
bool SyncTest_Init(int iCore);
float SyncTest_GetPeriod();
float SyncTest_GetTon();

#endif