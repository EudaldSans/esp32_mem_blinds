#include "esp_log.h"

#include "ges_signal.h"

#define PIN_V                       25
#define PIN_I                       33
#define PIN_VREF                    26
#define VREF_LEVEL                  1650

static xSIGNAL_t xSignal;
static uint64_t uPeriod = 20000;

static void IRAM_ATTR _callback(bool bValid, uint64_t uiCurrElapsedTime, uint64_t uiAdjElapsedTime, void *pArgs){
    uPeriod = uiAdjElapsedTime;
    SIGNAL_SetVoltageCallback(&xSignal, 19, SIGNALCallbackOnZeroCrossing, _callback, NULL);	
}

bool SyncTest_Init(uint8_t uiCore){
	SIGNAL_VoltageConfig(PIN_V, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, 1, &xSignal);
    SIGNAL_SetVoltageCallback(&xSignal, 19, SIGNALCallbackOnZeroCrossing, _callback, NULL);	
    return true;
}

float SyncTest_GetPeriod(){
    return (uPeriod/1e6);
}
