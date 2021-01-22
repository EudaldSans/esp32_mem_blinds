#ifndef TEST_METER_H
#define TEST_METER_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define GPIO_INA_V              25
#define INA_VOLTAGE             1650

#define HLW8012_SEL             27
#define HLW8012_CF              26 
#define HLW8012_CF1             34
#define HLW8012_VOLTAGE_PERIOD  (60 * 1000 * 1000)

bool MeterTest_Init(uint8_t uiCore);
float MeterTest_GetPower(void);
float MeterTest_GetKp(void);
bool MeterTest_SetKp(float fVal);
float MeterTest_GetCurrent(void);
float MeterTest_GetKi(); 
bool MeterTest_SetKi(float fVal);
float MeterTest_GetVoltage(void);
float MeterTest_GetKv();
bool MeterTest_SetKv(float fVal);
bool MeterTest_Calibrate(void);
int MeterTest_GetCalibrations(void);
void MeterTest_SetCalibrationVoltage(float v);
float MeterTest_GetCalibrationVoltage(void);
void MeterTest_SetCalibrationCurrent(float i);
float MeterTest_GetCalibrationCurrent(void);
void MeterTest_SetCalibrationPower(float p);
float MeterTest_GetCalibrationPower(void);
#endif