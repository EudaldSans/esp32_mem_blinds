/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		23-01-2020      PDM	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "test_rf.h"

#include "esp_log.h"
#include <stddef.h>

#include "ges_gpio.h"
#include "ges_adc.h"
#include "ges_timer.h"
#include "wifi_iperf.h"

/* DEFINES */
/* ------- */
#define TAG_TEST_RF                 "[RF_TEST]"

/* TYPES */
/* ----- */

/* INTERNAL FUNCTIONS */
/* ------------------ */
void _rf_test_task(void * xParams);

/* INTERNAL VARIABLES */
/* ------------------ */
bool bRfRunning = false;
uint16_t uiNumRfTestRequest;
uint16_t uiNumRfTestRate;
uint16_t uiNumRfIsrDetected;
uint16_t * ptrRfMeasures;

/* CODE */
/* ---- */
void _rf_test_task(void * xParams)
{
uint16_t uiSizeBuff = 0;
uint16_t * uiBuffer = 0;
uint16_t uiIdxBuffer;
uint16_t uiMax = 0; 
    
    if (ADC_ConfigDMA(1, CONFIG_ROE_ADC_PIN, GPIO_NC, 6, 0, 40000, 20*1000) == false) { 
        ESP_LOGW(TAG_TEST_RF, "Imposible config ADC for RF measures"); 
        vTaskDelete(NULL);
        return; 
    }

    if (uiNumRfTestRate<10) uiNumRfTestRate = 10;
    ESP_LOGI(TAG_TEST_RF, "RF task started %d %d", uiNumRfTestRequest, uiNumRfTestRate);
    uiNumRfIsrDetected = 0;
    while(uiNumRfTestRequest>0) {
        TMR_delay(uiNumRfTestRate*TIMER_MSEG);
        ESP_LOGI(TAG_TEST_RF, "Wait interrupt");
        uiNumRfTestRequest--;
        ADC_FreeBufferDMA(1); 
        if (ADC_StartDMA(1) == false) { ADC_FreeBufferDMA(1); ESP_LOGW(TAG_TEST_RF, "ADC not available"); continue; }
        IPERF_Start(1, 0);
        uiSizeBuff = ADC_GetBufferDMA(1, &uiBuffer); 
        ADC_FreeBufferDMA(1);
        if (!uiSizeBuff) { ESP_LOGW(TAG_TEST_RF, "Empty buffer adc rf measures"); continue; }
        uiMax = 0;
        for (uiIdxBuffer = 0; uiIdxBuffer < (uiSizeBuff<<0); uiIdxBuffer++) {
            if ((uiBuffer[uiIdxBuffer] < 160) || (uiBuffer[uiIdxBuffer] > 3400)) continue;
            if (uiMax < uiBuffer[uiIdxBuffer]) uiMax = uiBuffer[uiIdxBuffer];
        } 

        if (uiNumRfIsrDetected<MAX_RF_BUFFER) ptrRfMeasures[uiNumRfIsrDetected] = uiMax;
        uiNumRfIsrDetected++;
        ESP_LOGI(TAG_TEST_RF, "New rf measure added %d", uiMax);
    }
    ESP_LOGI(TAG_TEST_RF, "Ended rf task with %d measures", uiNumRfIsrDetected);
    bRfRunning = false; 

    vTaskDelete(NULL);
}

bool RfTest_start(uint16_t uiNumTests, uint16_t uiRateMiliseconds)
{
    if (bRfRunning == true) return false;
    bRfRunning = true; 

    if (ptrRfMeasures != NULL) { free(ptrRfMeasures); ptrRfMeasures = NULL; }
    uiNumRfTestRequest = uiNumTests;
    if (uiNumTests>MAX_RF_BUFFER) uiNumTests = MAX_RF_BUFFER;
    ptrRfMeasures = (uint16_t *)malloc(sizeof(uint16_t)*uiNumTests);
    uiNumRfTestRate = uiRateMiliseconds;
    uiNumRfIsrDetected = 0;
    if (ptrRfMeasures == NULL) return false;

    if (xTaskCreatePinnedToCore(_rf_test_task, "Rf test task", 5*1024, NULL, 5, NULL, 0) != pdPASS) {
        ESP_LOGE(TAG_TEST_RF, "create rf task failed");
        free(ptrRfMeasures); ptrRfMeasures = NULL;
        bRfRunning = false;
        return false;
    }

    ESP_LOGI(TAG_TEST_RF, "rf test started [%d]", uiNumRfTestRequest);
    return true;
}

bool RfTest_IsRunning(void)                 { return bRfRunning; }
uint16_t RfTest_GetNumTxDetections(void)    { return uiNumRfIsrDetected; }
uint16_t * RfTest_GetArrayMeasures(void)    { return ptrRfMeasures; }


