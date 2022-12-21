#ifndef _BLINDS_DEVICE
#define _BLINDS_DEVICE

/* INCLUDES */
/* -------- */
#include "MEM_Main.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
void DEVICE_SetCmd(uint8_t uiChild, PROTOCOL_VARIABLES_t xVar, double dValue, char * cExtraData);
void DEVICE_GetCmd(uint8_t uiChild, PROTOCOL_VARIABLES_t xVar, double dValue, char * cExtraData);
// void DEVICE_ResCmd(uint8_t uiChild, PROTOCOL_VARIABLES xVar, double dValue);
void DEVICE_Reset(void);
void DEVICE_Apply(float fPercentatge);

#endif