#ifndef _SWITCH_METER
#define _SWITCH_METER

/* INCLUDES */
/* -------- */
#include <stdint.h>
#include <stdbool.h>

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define HLW8012_SEL             27
#define HLW8012_CF              26
#define HLW8012_CF1             34
#define HLW8012_VOLTAGE_PERIOD  (60 * 1000 * 1000)

#define MIN_POWER_LOAD          10  // In watts
#define MAX_HW_CURRENT          3.4 // In Amperes

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool METER_Init(int iCore);

float METER_GetPower(void);
float METER_GetCurrent(void);
float METER_GetVoltage(void);

bool METER_GetMaxCurrentDetected(void);


#endif
