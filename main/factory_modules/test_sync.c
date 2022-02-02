#include "esp_log.h"
#include "test_sync.h"

#include "ges_signal.h"

static xSIGNAL_t xSignal;
static uint64_t uPeriod = 20000;

static void IRAM_ATTR _callback(bool bValid, uint64_t uiCurrElapsedTime, uint64_t uiAdjElapsedTime, void *pArgs){
    uPeriod = uiAdjElapsedTime;
    SIGNAL_SetVoltageCallback(&xSignal, 19, SIGNALCallbackOnZeroCrossing, _callback, NULL);	
}

bool SyncTest_Init(int iCore){
	SIGNAL_VoltageConfig(PIN_SINCRO, GPIO_INPUT_PULLOFF, GPIO_INPUT_INTERRUPT_RISE_CHECK, 1, &xSignal);
    SIGNAL_SetVoltageCallback(&xSignal, 19, SIGNALCallbackOnZeroCrossing, _callback, NULL);	
    return true;
}

float SyncTest_GetPeriod(){
    return (uPeriod/1e6);
}

float SyncTest_GetTon(){
    return SIGNAL_GetAverageON(SIGNAL_GetByPin(PIN_SINCRO));
}
