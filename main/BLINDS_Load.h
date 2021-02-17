#ifndef _BLINDS_LOAD
#define _BLINDS_LOAD

/* INCLUDES */
/* -------- */
#include <stdint.h>
#include <stdbool.h>

#include "ges_timer.h"
#include "ges_blinds.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define PIN_TRIAC_ON                2
#define PIN_RELAY_UPDOWN            4

#define PIN_VREF                    25
#define PIN_CF                      26
#define PIN_CF1                     34
#define PIN_SEL                     27

#define PIN_SINCRO                  35
// #define PIN_SRS                     32
// #define VREF_LEVEL                  1650
#define VOLTAGE_PERIOD              (60 * 1000 * 1000)

#define MIN_POWER_LOAD              10  // In watts
#define CAREER_CYCLES               5

// #define TIME_NOTIFY_STATUS          (2*TIMER_SEG)
#define DEFAULT_BLIND_MODE          BLIND_MODE_STDBLIND
#define DEFAULT_RISE_TIME           (60 * 1000 * 1000)
#define DEFAULT_FALL_TIME           (60 * 1000 * 1000)

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool LOAD_Init(int iCore);

bool LOAD_Open(void);
bool LOAD_Close(void);
bool LOAD_Stop(void);

bool LOAD_Calibrate(void);
bool LOAD_Regulate(uint8_t uiPercentatge);
uint8_t LOAD_GetPercentatge(void);

bool LOAD_IsOpening(void);
bool LOAD_IsClosing(void);
bool LOAD_IsCalibrating(void);
bool LOAD_IsCalibrated(void);
bool LOAD_IsStopped(void);

bool LOAD_SetMode(BLIND_MODES xMode);
BLIND_MODES LOAD_GetMode(void);

bool LOAD_SetCalibrated(bool bCalibrated);
bool LOAD_SetRiseTime(uint64_t uiMicroSeconds);
uint64_t LOAD_GetRiseTime(void);
bool LOAD_SetFallTime(uint64_t uiMicroSeconds);
uint64_t LOAD_GetFallTime(void);

#endif