#ifndef _TEST_RF_H
#define _TEST_RF_H

/* INCLUDES */
/* -------- */
#include <stdint.h>
#include <stdbool.h>

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define PIN_RF_TEST_INT         39
#define PIN_RF_TEST_ADC         36

#define MAX_RF_BUFFER           500

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool RfTest_start(uint16_t uiNumTests, uint16_t uiRateMiliseconds);

bool RfTest_IsRunning(void);
uint16_t RfTest_GetNumTxDetections(void);
uint16_t * RfTest_GetArrayMeasures(void);

#endif