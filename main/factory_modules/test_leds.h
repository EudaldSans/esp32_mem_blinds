#ifndef _TEST_LEDS_H
#define _TEST_LEDS_H

/* INCLUDES */
/* -------- */
#include "MEM_Definitions.h"

#include <stdint.h>
#include <stdbool.h>

#define PIN_LED_UP                  23
#define PIN_LED_DOWN                12
#define FADETIME_LEDS               0     // Time in ms

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool LedsTest_Init(uint8_t uiCore);
int LedsTest_GetMax(int iLed);
bool LedsTest_SetMax(int iLed, int iMaxVal);
int LedsTest_GetVal(int iLed);
bool LedsTest_SetVal(int iLed, int iVal);
int LedsTest_getTotalLeds(void);
#endif