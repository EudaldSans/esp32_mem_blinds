/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		21-04-2020      PDM	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "test_meter.h"

#include "esp_log.h"
#include "ges_nvs.h"
#include "ges_HLW8012.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_TEST_METER                  "[TEST_METER]"
#define CALIBRATION_SAMPLES             10
#define HLW8012_VOLTAGE_TEST_PERIOD     (60 * 1000 * 1000)

#define MIN_POWER_TO_REPORT             2   // In Watts
#define MAX_HW_CURRENT                  10  // In Amperes
#define DEFAULT_MAX_POWER               0   // In Watts ('0' Disable)

/* INTERNAL FUNCTIONS */
/* ------------------ */
bool _calibrate(void);

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */
static uint16_t uiCalibrationsDone = 0;
static float fCalibrationPower = 0.0;
static float fCalibrationVoltage = 0.0;
static float fCalibrationCurrent = 0.0;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */

bool MeterTest_Init(uint8_t uiCore)
{
    // Config power measures
    HLW8012_Config(HLW8012_SEL, BL0937, HLW8012_CF, HLW8012_CF1, HLW8012_VOLTAGE_PERIOD);
    return true;
}

bool MeterTest_Calibrate(void){
    bool ok;
    if(fCalibrationCurrent != 0.0 && fCalibrationVoltage != 0.0 && fCalibrationPower != 0.0){
        ok = _calibrate();
        if(ok){
            uiCalibrationsDone++;
            return true;
        }else{
            return false;
        }
    }
    else{
        ESP_LOGE(TAG_TEST_METER, "calibration parameters not set");
        return false;
    }
}

void MeterTest_SetCalibrationVoltage(float v){
    fCalibrationVoltage = v;
}

float MeterTest_GetCalibrationVoltage(void){
    return fCalibrationVoltage;
}

void MeterTest_SetCalibrationCurrent(float i){
    fCalibrationCurrent = i;
}

float MeterTest_GetCalibrationCurrent(void){
    return fCalibrationCurrent;
}

void MeterTest_SetCalibrationPower(float p){
    fCalibrationPower = p;
}

float MeterTest_GetCalibrationPower(void){
    return fCalibrationPower;
}

int MeterTest_GetCalibrations(void){
    return uiCalibrationsDone;
}

#if defined(CONFIG_BOARD_MEM_BLINDS)
float MeterTest_GetPower(void)                  { return HLW8012_GetMeanPower(); }
float MeterTest_GetKp(void)                     { return HLW8012_GetKp(); }
bool MeterTest_SetKp(float fVal)                { return HLW8012_SetKp(fVal); }
float MeterTest_GetCurrent(void)                { return HLW8012_GetMeanCurrent(); }
float MeterTest_GetKi(void)                     { return HLW8012_GetKi(); }
bool MeterTest_SetKi(float fVal)                { return HLW8012_SetKi(fVal); }
float MeterTest_GetVoltage(void)                { return HLW8012_GetMeanVoltage(); }
float MeterTest_GetKv(void)                     { return HLW8012_GetKv(); }
bool MeterTest_SetKv(float fVal)                { return HLW8012_SetKv(fVal); }

bool _calibrate(void){ 
    ESP_LOGI(TAG_TEST_METER, "MeterTest.calibrate(%f, %f, %f)", fCalibrationPower, fCalibrationVoltage, fCalibrationCurrent); 
    HLW8012_Calibrate(CALIBRATION_SAMPLES, fCalibrationPower, fCalibrationVoltage, fCalibrationCurrent);
    return true; 
}

#elif defined(CONFIG_BOARD_LOLIN)
float MeterTest_GetPower(void)                  { return 55.0; }
float MeterTest_GetKp(void)                     { return kp; }
bool MeterTest_SetKp(float fVal)                { kp = fVal; return true; }
float MeterTest_GetCurrent(void)                { return 0.25; }
float MeterTest_GetKi(void)                     { return ki; }
bool MeterTest_SetKi(float fVal)                { ki = fVal; return true; }
float MeterTest_GetVoltage(void)                { return 220.0; }
float MeterTest_GetKv(void)                     { return kv; }
bool MeterTest_SetKv(float fVal)                { kv = fVal; return true; }
bool _calibrate(void)                  { ESP_LOGI(TAG_TEST_METER, "MeterTest.calibrate(%f, %f, %f)", fCalibrationPower, fCalibrationVoltage, fCalibrationCurrent); return true; }
#endif