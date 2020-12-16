#ifndef _BLINDS_FEEDBACK
#define _BLINDS_FEEDBACK

/* INCLUDES */
/* -------- */
#include <stdint.h>
#include <stdbool.h>

#include "MEM_Definitions.h"

/* TYPES */
/* ----- */
typedef enum {
    FEEDBACK_IDLE_OFF = 0,
    FEEDBACK_IDLE_ON,
    FEEDBACK_IDLE_STATUS,
    FEEDBACK_NUM_IDLE_SIGNALS
} FEEDBACK_IDLE_SIGNALS;

/* DEFINES */
/* ------- */
#define PIN_LED_UP                  12
#define PIN_LED_DOWN                23

#define FADETIME_LEDS               0     // Time in ms
#define DEFAULT_IDLE_SIGNAL         FEEDBACK_IDLE_OFF

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool FEEDBACK_Init(int iCore);

void FEEDBACK_CustomSignal(float fLum, uint16_t uiTimeOn, uint16_t uiTimeOff, uint16_t uiTimeMs);
void FEEDBACK_OfflineSignal(void);
void FEEDBACK_InclusionSignal(void);
void FEEDBACK_CalibratingSignal(void);
void FEEDBACK_ErrorSignal(void);
void FEEDBACK_MoveEndSignal(void);

void FEEDBACK_SetIdleSignal(FEEDBACK_IDLE_SIGNALS xSignal);
FEEDBACK_IDLE_SIGNALS FEEDBACK_GetIdleSignal(void);

#endif