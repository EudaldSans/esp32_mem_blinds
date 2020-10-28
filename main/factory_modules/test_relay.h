#ifndef _TEST_RELAY
#define _TEST_RELAY

/* INCLUDES */
/* -------- */
#include <stdint.h>
#include <stdbool.h>

#define PIN_RELAY_UP                2
#define PIN_RELAY_DOWN              4
#define PIN_V                       35
#define PIN_I                       32
#define PIN_VREF                    25
#define VREF_LEVEL                  1650

/* TYPES */
/* ----- */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool RelayTest_Init(uint8_t uiCore);
int RelayTest_GetStatus(int iRelay);
int RelayTest_SetStatus(int iRelay, bool bStatus);
int RelayTest_GetTotalRelays(void);
int RelayTest_GetCalibrations(int iRelay);
int RelayTest_Calibrate(int iRelay);
int RelayTest_ResistorCalibrate(int iRelay);
int RelayTest_GetOperateTime(int iRelay);
int RelayTest_SetOperateTime(int iRelay, int iTime);
int RelayTest_GetReleaseTime(int iRelay);
int RelayTest_SetReleaseTime(int iRelay, int iTime);

#endif