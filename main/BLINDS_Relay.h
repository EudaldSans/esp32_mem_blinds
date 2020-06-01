#ifndef _BLINDS_RELAY
#define _BLINDS_RELAY

/* INCLUDES */
/* -------- */
#include <stdint.h>
#include <stdbool.h>

#include "ges_timer.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define PIN_RELAY                   33

#define PIN_V                       35
#define PIN_I                       32
#define PIN_VREF                    25
#define VREF_LEVEL                  1650

#define DEFAULT_RELAY_TIME          6000
// #define DEFAULT_TIME_ON             0
#define DEFAULT_TIME_OFF            0
#define DEFAULT_TIME_IN_ON          0

#define DEFAULT_RESTORE_LAST        true
// #define DEFAULT_INITIAL_STATUS      true

#define TIME_NOTIFY_STATUS          (2*TIMER_SEG)

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool SRELAY_Init(int iCore);

bool SRELAY_GetStatus(void);
void SRELAY_ON(void);
void SRELAY_OFF(void);
void SRELAY_Toggle(void);

// void SRELAY_SetTimeOn(uint16_t uiTime);
void SRELAY_SetTimeInOn(uint16_t uiTime);
void SRELAY_SetTimeOff(uint16_t uiTime);
// uint16_t SRELAY_GetTimeOn(void);
uint16_t SRELAY_GetTimeInOn(void);
uint16_t SRELAY_GetTimeOff(void);

void SRELAY_SetRestoreLast(bool bStatus);
bool SRELAY_GetRestoreLast(void);
// void SRELAY_SetStatusAfterReset(bool bStatus);
// bool SRELAY_GetStatusAfterReset(void);


#endif