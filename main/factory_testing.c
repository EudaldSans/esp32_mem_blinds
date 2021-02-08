/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		08-04-2020      POL	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "stdbool.h"
#include "sdkconfig.h"
#include "cJSON.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "OTA_APP.h"
#include "MEM_Installation.h"
#include "ges_wifi.h"
#include "ges_http_server.h"
#include "ges_nvs.h"
#include "ges_timer.h"
#include "ges_adc.h"

#include "factory_testing.h"
#include "test_button.h"
#include "test_relay.h"
#include "test_meter.h"
#include "test_leds.h"
#include "test_sync.h"

#include "test_rf.h"

/* TYPES */
/* ----- */
typedef enum {
    TEST_PASSED = 0,
    TEST_STEP0,
    TEST_STEP1,
    TEST_STEP2,
    TEST_NUM_STEPS
} TEST_STEPS;

/* DEFINES */
/* ------- */

//Useful constants
#define FACTORY_TEST_NVS_KEY_STATE      "TEST_STATE"
#define FACTORY_TEST_NVS_KEY_RESETS     "TEST_RESET"
#define MAC_BYTES_LEN                   6

//Preprocesor snippets
#define TAG_FACTORY_TESTING             "[FACTORY_TESTING]"
#define INFO(msg)                       ESP_LOGI(TAG_FACTORY_TESTING, msg)
#define ERROR(msg)                      ESP_LOGE(TAG_FACTORY_TESTING, msg)

/* INTERNAL FUNCTIONS */
/* ------------------ */
bool _factorytest_webStart(SERVER_OBJ* xWebUi);
TEST_STEPS factorytest_getState(void);
char * _state_to_string(TEST_STEPS step);
void _factorytest_taskLoop(void * xParams);
void __URI_root(httpd_req_t * xReq);
void __URI_get_state(httpd_req_t * xReq);
void __URI_post_state(httpd_req_t * xReq);
void __URI_get_info(httpd_req_t * xReq);
void __URI_post_info(httpd_req_t * xReq);
void __URI_get_buttons(httpd_req_t * xReq);
void __URI_post_buttons(httpd_req_t * xReq);
void __URI_get_relays(httpd_req_t * xReq);
void __URI_post_relays(httpd_req_t * xReq);
void __URI_get_meter(httpd_req_t * xReq);
void __URI_post_meter(httpd_req_t * xReq);
void __URI_get_ac(httpd_req_t * xReq);
void __URI_get_leds(httpd_req_t * xReq);
void __URI_set_leds(httpd_req_t * xReq);
void __URI_post_reset(httpd_req_t * xReq);
void __URI_post_wifi(httpd_req_t * xReq);
void __URI_get_wifi(httpd_req_t * xReq);
void __URI_get_rssi(httpd_req_t * xReq);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
void factorytest_updateNVS(void);
void _reinitStep(void);
void uiatoa(uint8_t* a, size_t len_a, char* s, size_t len_s);

/* INTERNAL VARIABLES */
/* ------------------ */
static uint16_t resets = 0;
bool bSoftReset = false;

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
bool _factorytest_webStart(SERVER_OBJ* xWebUi)
{
    INFO("Initializing web server");
    if (HTTP_StartWebserver(xWebUi, FACTORY_TEST_HTTP_SERVER_ID, FACTORY_TEST_HTTP_PRIORITY, FACTORY_TEST_HTTP_PORT, 20) == true)
    {
        INFO("Initiated webserver, registring URIs");
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/", __URI_root);
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/state", __URI_get_state);
        HTTP_RegisterUri(xWebUi, HTTP_POST, "/state", __URI_post_state);
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/info", __URI_get_info);
        HTTP_RegisterUri(xWebUi, HTTP_POST, "/info", __URI_post_info);
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/buttons", __URI_get_buttons);
        HTTP_RegisterUri(xWebUi, HTTP_POST, "/buttons", __URI_post_buttons);
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/relays", __URI_get_relays);
        HTTP_RegisterUri(xWebUi, HTTP_POST, "/relays", __URI_post_relays);
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/meter", __URI_get_meter);
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/ac", __URI_get_ac);
        HTTP_RegisterUri(xWebUi, HTTP_POST, "/meter", __URI_post_meter);
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/leds", __URI_get_leds);
        HTTP_RegisterUri(xWebUi, HTTP_POST, "/leds", __URI_set_leds);
        HTTP_RegisterUri(xWebUi, HTTP_POST, "/reset", __URI_post_reset);
        HTTP_RegisterUri(xWebUi, HTTP_POST, "/wifi", __URI_post_wifi);
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/wifi", __URI_get_wifi);
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/rssi", __URI_get_rssi);
        INFO("URIs initiated");
        return true;
    }
    ERROR("httpd was not able to start"); 
    return false;
}

TEST_STEPS factorytest_getState(void)
{
bool res;
uint16_t uiState = (uint16_t)TEST_STEP0;

    NVS_Init();
    res = NVS_ReadInt16(FACTORY_TEST_NVS_KEY_STATE, &uiState);
    ESP_LOGI(TAG_FACTORY_TESTING, "getState() : %d (%s)", (int)uiState, res ? "OK" : "ERROR");
    return (TEST_STEPS)uiState;
}

bool factorytest_setState(TEST_STEPS step)
{
bool res = false;

    NVS_Init();
    res = NVS_WriteInt16(FACTORY_TEST_NVS_KEY_STATE, (uint16_t)step);
    ESP_LOGI(TAG_FACTORY_TESTING, "setState(%d) : %s", step, res ? "true": "false");
    _reinitStep();
    return res;
}

void _reinitStep(void)
{
    NVS_Init();
    resets = 1;
    NVS_WriteInt16(FACTORY_TEST_NVS_KEY_RESETS, resets);
}

char * _state_to_string(TEST_STEPS state)
{
    switch(state)
    {
        case TEST_STEP0:        return "phase0"; break;
        case TEST_STEP1:        return "phase1"; break;
        case TEST_STEP2:        return "phase2"; break;
        case TEST_PASSED:       return "passed"; break;
        default:                return "unknown"; break;
    }
}

TEST_STEPS stateFromString(char * state)
{
    if(strcmp(state, "phase0")==0)
        return TEST_STEP0;
    else if(strcmp(state, "phase1")==0)
        return TEST_STEP1;
    else if(strcmp(state, "phase2")==0)
        return TEST_STEP2;
    else if (strcmp(state, "passed")==0)
        return TEST_PASSED;
    else
        return TEST_STEP0;
}

void _factorytest_taskLoop(void * xParams)
{
SERVER_OBJ xWebUi;

    while(HTTP_IsInitiated(&xWebUi) != true){
        _factorytest_webStart(&xWebUi);
        vTaskDelay(100);
    }
    
    while(true) 
    {
        if (bSoftReset) { ADC_WaitAndLockAllDMA(); esp_restart(); }
        vTaskDelay(100);
        //wait for requests, we should not get here
    }
}

void __URI_root(httpd_req_t * xReq) 
{
    HTTP_ResponseSend(xReq, "test state brief to be implemented", HTTP_CONTENT_TEXT);
}

void __URI_get_buttons(httpd_req_t * xReq)
{
cJSON * payload = cJSON_CreateObject();
cJSON * obj;
char * payload_str;
char key[10];

    for(int i=0; i<ButtonTest_GetTotalButtons(); i++)
    {
        snprintf(key, sizeof(key), "%d", i);
        cJSON_AddItemToObject(payload, key, obj = cJSON_CreateObject());
        cJSON_AddNumberToObject(obj, "shortPulsations", ButtonTest_GetShortPulsations(i));
        cJSON_AddNumberToObject(obj, "longPulsations", ButtonTest_GetLongPulsations(i));
        cJSON_AddNumberToObject(obj, "timestamp", (double)ButtonTest_GetTimestamp(i));
    }

    payload_str = cJSON_Print(payload);
    ESP_LOGI(TAG_FACTORY_TESTING, "GET /buttons\n%s\n", payload_str);
    HTTP_ResponseSend(xReq, payload_str, HTTP_CONTENT_JSON);
    cJSON_Delete(payload);
}

void __URI_post_buttons(httpd_req_t * xReq)
{
cJSON * payload = cJSON_CreateObject();

    ButtonTest_Reset();
    cJSON_AddBoolToObject(payload, "result", true);
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
    cJSON_Delete(payload);
}

void __URI_get_relays(httpd_req_t * xReq)
{
cJSON * payload = cJSON_CreateObject();
cJSON * obj;
char * payload_str;
char key[10];

    for(int i=0; i<RelayTest_GetTotalRelays(); i++)
    {
        snprintf(key, sizeof(key), "%d", i);
        cJSON_AddItemToObject(payload, key, obj = cJSON_CreateObject());
        cJSON_AddBoolToObject(obj, "status", (bool)RelayTest_GetStatus(i));
        cJSON_AddNumberToObject(obj, "calibrationsDone", RelayTest_GetCalibrations(i));
        cJSON_AddNumberToObject(obj, "operateTime", RelayTest_GetOperateTime(i));
        cJSON_AddNumberToObject(obj, "releaseTime", RelayTest_GetReleaseTime(i));
    }

    payload_str = cJSON_Print(payload);
    ESP_LOGI(TAG_FACTORY_TESTING, "GET /relays\n%s\n", payload_str);
    HTTP_ResponseSend(xReq, payload_str, HTTP_CONTENT_JSON);
    cJSON_Delete(payload);
}

void __URI_post_relays(httpd_req_t * xReq)
{
char cData[500];
size_t bSize;
int bLenGetData;
bool result = true;

cJSON * reqData;
cJSON * payload = cJSON_CreateObject();
cJSON * relay;
cJSON * item;

int index;

    bSize = MIN(xReq->content_len, sizeof(cData));              // Detect what is the MIN lenght betwwen data and buffer
    bLenGetData = httpd_req_recv(xReq, cData, bSize);           // Getting data info without exceeding the buffer
    ESP_LOGI(TAG_FACTORY_TESTING, "POST /relays\n%s", cData);
    if (bLenGetData) {
        reqData= cJSON_Parse(cData);
        cJSON_ArrayForEach(relay, reqData) {
            if(cJSON_IsObject(relay)) {
                index = atoi(relay->string);
                if(cJSON_HasObjectItem(relay, "status")) {
                    item = cJSON_GetObjectItem(relay, "status");
                    if(cJSON_IsBool(item)){
                        if (RelayTest_SetStatus(index, cJSON_IsTrue(item)) == -1) result = false;
                    }
                }
                if(cJSON_HasObjectItem(relay, "operateTime")) {
                    item = cJSON_GetObjectItem(relay, "operateTime");
                    if(cJSON_IsNumber(item)) {
                        if (RelayTest_SetOperateTime(index, item->valueint) == -1) result = false;
                    }
                }
                if(cJSON_HasObjectItem(relay, "releaseTime")) {
                    item = cJSON_GetObjectItem(relay, "releaseTime");
                    if(cJSON_IsNumber(item)) {
                        if (RelayTest_SetReleaseTime(index, item->valueint) == -1) result = false;
                    }
                }
                if(cJSON_HasObjectItem(relay, "runCalibration")) {
                    item = cJSON_GetObjectItem(relay, "runCalibration");
                    if(cJSON_IsTrue(item)){
                        if (RelayTest_Calibrate(index) == -1) result = false;
                    }
                }
                if(cJSON_HasObjectItem(relay, "runResistiveCalibration")) {
                    item = cJSON_GetObjectItem(relay, "runResistiveCalibration");
                    if(cJSON_IsTrue(item)){
                        if (RelayTest_ResistorCalibrate(index) == -1) result = false;
                    }
                }
            }
        }
    }

    cJSON_AddBoolToObject(payload, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
    cJSON_Delete(payload);
}
void __URI_get_meter(httpd_req_t * xReq)
{
cJSON* calibrationParams = cJSON_CreateObject();
cJSON * payload = cJSON_CreateObject();
char * payload_str;

    cJSON_AddNumberToObject(payload, "power", MeterTest_GetPower());
    cJSON_AddNumberToObject(payload, "voltage", MeterTest_GetVoltage());
    cJSON_AddNumberToObject(payload, "current", MeterTest_GetCurrent());
    cJSON_AddNumberToObject(payload, "kp", MeterTest_GetKp());
    cJSON_AddNumberToObject(payload, "kv", MeterTest_GetKv());
    cJSON_AddNumberToObject(payload, "ki", MeterTest_GetKi());
    cJSON_AddNumberToObject(calibrationParams, "power", MeterTest_GetCalibrationPower());
    cJSON_AddNumberToObject(calibrationParams, "voltage", MeterTest_GetCalibrationVoltage());
    cJSON_AddNumberToObject(calibrationParams, "current", MeterTest_GetCalibrationCurrent());
    cJSON_AddItemToObject(payload, "calibrationParams", calibrationParams);
    cJSON_AddNumberToObject(payload, "calibrationsDone", MeterTest_GetCalibrations());

    payload_str = cJSON_Print(payload);
    ESP_LOGI(TAG_FACTORY_TESTING, "GET /meter\n%s\n", payload_str);
    HTTP_ResponseSend(xReq, payload_str, HTTP_CONTENT_JSON);
    cJSON_Delete(payload);
}

void __URI_post_meter(httpd_req_t * xReq)
{
char cData[500];
size_t bSize;
int bLenGetData;
bool result = true;

cJSON* payload = cJSON_CreateObject();
cJSON* reqData;
cJSON* obj;
cJSON* tmp;
float val;

    bSize = MIN(xReq->content_len, sizeof(cData));              // Detect what is the MIN lenght betwwen data and buffer
    bLenGetData = httpd_req_recv(xReq, cData, bSize);           // Getting data info without exceeding the buffer
    ESP_LOGI(TAG_FACTORY_TESTING, "POST /meter\n%s", cData);
    if (bLenGetData) {
        reqData= cJSON_Parse(cData);
        if(cJSON_HasObjectItem(reqData, "kp")) {
            obj = cJSON_GetObjectItem(reqData, "kp");
            val = atof(cJSON_Print(obj));
            if (MeterTest_SetKp(val) == false) result = false;
        }
        if(cJSON_HasObjectItem(reqData, "kv")) {
            obj = cJSON_GetObjectItem(reqData, "kv");
            val = atof(cJSON_Print(obj));
            if (MeterTest_SetKv(val) == false) result = false;
        }
        if(cJSON_HasObjectItem(reqData, "ki")) {
            obj = cJSON_GetObjectItem(reqData, "ki");
            val = atof(cJSON_Print(obj));
            if (MeterTest_SetKi(val) == false) result = false;
        }
        if(cJSON_HasObjectItem(reqData, "calibrationParams")) {
            obj = cJSON_GetObjectItem(reqData, "calibrationParams");
            if(cJSON_HasObjectItem(obj, "power")) {
                tmp = cJSON_GetObjectItem(obj, "power");
                val = atof(cJSON_Print(tmp));
                MeterTest_SetCalibrationPower(val);
            }
            if(cJSON_HasObjectItem(obj, "voltage")) {
                tmp = cJSON_GetObjectItem(obj, "voltage");
                val = atof(cJSON_Print(tmp));
                MeterTest_SetCalibrationVoltage(val);
            }
            if(cJSON_HasObjectItem(obj, "current")) {
                tmp = cJSON_GetObjectItem(obj, "current");
                val = atof(cJSON_Print(tmp));
                MeterTest_SetCalibrationCurrent(val);
            }
        }
        if(cJSON_HasObjectItem(reqData, "runCalibration")) {
            obj = cJSON_GetObjectItem(reqData, "runCalibration");
            if(cJSON_IsTrue(obj)){
                if (MeterTest_Calibrate() == false) result = false;
            }
        }
    }

    cJSON_AddBoolToObject(payload, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
    cJSON_Delete(payload);
}

void __URI_get_ac(httpd_req_t * xReq) 
{
cJSON * xPayload = cJSON_CreateObject();

    cJSON_AddNumberToObject(xPayload, "T", SyncTest_GetPeriod());
    cJSON_AddNumberToObject(xPayload, "Ton", SyncTest_GetPeriod()/2);
    HTTP_ResponseSend(xReq, cJSON_Print(xPayload), HTTP_CONTENT_JSON);
    cJSON_Delete(xPayload);
}

void __URI_get_state(httpd_req_t * xReq) 
{
uint16_t step = factorytest_getState();
char * step_description = _state_to_string(step);
cJSON * xPayload = cJSON_CreateObject();

    cJSON_AddItemToObject(xPayload, "state", cJSON_CreateString(step_description));
    HTTP_ResponseSend(xReq, cJSON_Print(xPayload), HTTP_CONTENT_JSON);
    cJSON_Delete(xPayload);
}

void __URI_post_state(httpd_req_t * xReq) 
{
char cData[500];
size_t bSize;
int bLenGetData;
bool result = true;
cJSON * xJsonData;
cJSON * xJsonObj;
cJSON * xJsonResp = cJSON_CreateObject();

char * state_string = _state_to_string(factorytest_getState());
TEST_STEPS state;

    bSize = MIN(xReq->content_len, sizeof(cData));              // Detect what is the MIN lenght betwwen data and buffer
    bLenGetData = httpd_req_recv(xReq, cData, bSize);           // Getting data info without exceeding the buffer
    if (bLenGetData) {
        xJsonData = cJSON_Parse(cData);
        if (xJsonData != NULL) {
            xJsonObj = cJSON_GetObjectItem(xJsonData, "state"); if (xJsonObj != NULL) state_string = cJSON_GetStringValue(xJsonObj);
        } else {
            ERROR("unable to parse feedback request");
        }
    } else {
        ERROR("Empty data in set feedback");
    }
    ESP_LOGI(TAG_FACTORY_TESTING, "set test state to %s", state_string);
    if(state_string != NULL ){
        state = stateFromString(state_string);
        result = factorytest_setState(state);
    } else {
        result = false;
    }

    cJSON_AddBoolToObject(xJsonResp, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(xJsonResp), HTTP_CONTENT_JSON);
    cJSON_Delete(xJsonResp);
}

void __URI_get_info(httpd_req_t * xReq) 
{
bool res = false;
uint16_t currentResets = resets - 1;
char hwId[33];
size_t len;
size_t* len_ptr;
uint8_t* macBytes;
char mac[MAC_BYTES_LEN*2 + 1];
char secret[65];
char serialId[32];
char fwVersion[128] = "undefined";
size_t fwVersion_len = sizeof(fwVersion);
uint16_t date;
char error_code[] = "undefined";
cJSON * xPayload = cJSON_CreateObject();

    res = NVS_ReadInt16(KEY_DATE, &date);
    if(!res) date = 0;

    len = sizeof(serialId);
    len_ptr = &len;
    res = NVS_ReadStr(KEY_SERIAL_ID, serialId, len_ptr);
    if(!res) strcpy(serialId, error_code);
    
    NVS_ReadStrFromModule("OTA", "FW", fwVersion, &fwVersion_len);
    macBytes = WIFI_GetMacAddress();
    uiatoa(macBytes, MAC_BYTES_LEN, mac, sizeof(mac));
    ESP_LOGI(TAG_FACTORY_TESTING, "GET /info\nsecret : %s\nhwId : %s", secret, hwId);

    cJSON_AddItemToObject(xPayload, "hwId", cJSON_CreateString(INST_GetClientId()));
    cJSON_AddItemToObject(xPayload, "secret", cJSON_CreateString(INST_GetClientSecret()));
    cJSON_AddNumberToObject(xPayload, "resetsDone", currentResets);
    cJSON_AddItemToObject(xPayload, "mac", cJSON_CreateString(mac));
    cJSON_AddNumberToObject(xPayload, "date", date);
    cJSON_AddStringToObject(xPayload, "serialId", serialId);
    cJSON_AddStringToObject(xPayload, "fwVersion", fwVersion);
    HTTP_ResponseSend(xReq, cJSON_Print(xPayload), HTTP_CONTENT_JSON);
    cJSON_Delete(xPayload);
}

void uiatoa(uint8_t* a, size_t len_a, char* s, size_t len_s)
{
char buffer[3];

    memset(s, 0, len_s);
    for(int i=0; i<len_a; i++){
        snprintf(buffer, sizeof(buffer), "%02x", a[i]);
        strncat(s, buffer, len_s);
    }
}

void __URI_post_info(httpd_req_t * xReq) 
{
char cData[500];
size_t bSize;
size_t len;
int bLenGetData;
bool result = true;
cJSON * xJsonData;
cJSON * xJsonObj;
cJSON * xJsonResp = cJSON_CreateObject();

char * hwId = NULL;
char * secret = NULL;
char * serialId = NULL;
cJSON * dateObj = NULL;
uint16_t date = 0;

    bSize = MIN(xReq->content_len, sizeof(cData));              // Detect what is the MIN lenght betwwen data and buffer
    bLenGetData = httpd_req_recv(xReq, cData, bSize);           // Getting data info without exceeding the buffer
    if (bLenGetData) {
        xJsonData = cJSON_Parse(cData);
        if (xJsonData != NULL) {
            xJsonObj = cJSON_GetObjectItem(xJsonData, "hwId"); if (xJsonObj != NULL)  hwId= cJSON_GetStringValue(xJsonObj);
            xJsonObj = cJSON_GetObjectItem(xJsonData, "secret"); if (xJsonObj != NULL)  secret= cJSON_GetStringValue(xJsonObj);
            xJsonObj = cJSON_GetObjectItem(xJsonData, "serialId"); if (xJsonObj != NULL)  serialId= cJSON_GetStringValue(xJsonObj);
            dateObj = cJSON_GetObjectItem(xJsonData, "date");
        } else {
            ERROR("unable to parse info request");
        }
    } else {
        ERROR("Empty data in set info");
    }

    if(secret != NULL) {
        ESP_LOGI(TAG_FACTORY_TESTING, "set secret : %s", secret);
        len = strlen(secret);
        if (INST_SetClientSecret(secret, len) == false) result = false;
    }
    if(hwId != NULL) {
        ESP_LOGI(TAG_FACTORY_TESTING, "set hwId : %s", hwId);
        len = strlen(hwId);
        if (INST_SetClientId(hwId, len) == false) result = false;
    }

    if(serialId != NULL) {
        ESP_LOGI(TAG_FACTORY_TESTING, "set serialId : %s", serialId);
        if (NVS_WriteStr(KEY_SERIAL_ID , serialId) == false) result = false;
    }

    if (cJSON_IsNumber(dateObj)) {
        date = dateObj->valueint;
        ESP_LOGI(TAG_FACTORY_TESTING, "set date : %d", date);
        if (NVS_WriteInt16(KEY_DATE, date) == false) result = false;
    }

    cJSON_AddBoolToObject(xJsonResp, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(xJsonResp), HTTP_CONTENT_JSON);
    cJSON_Delete(xJsonResp);
}

void __URI_get_leds(httpd_req_t * xReq)
{
cJSON* payload = cJSON_CreateObject();
cJSON* obj;
char key[10];

    for(int i=0; i < LedsTest_getTotalLeds(); i++)
    {
        obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "value", LedsTest_GetVal(i));
        cJSON_AddNumberToObject(obj, "maxValue", LedsTest_GetMax(i));
        snprintf(key, sizeof(key), "%d", i);
        cJSON_AddItemToObject(payload, key, obj);
    }
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
    cJSON_Delete(payload);
}

void __URI_set_leds(httpd_req_t * xReq)
{
size_t bSize;
int bLenGetData;
char cData[500];
bool result = true;

cJSON* payload = cJSON_CreateObject();
cJSON* reqData;
cJSON* led;
cJSON* obj;

int i;

    bSize = MIN(xReq->content_len, sizeof(cData));              // Detect what is the MIN lenght betwwen data and buffer
    bLenGetData = httpd_req_recv(xReq, cData, bSize);           // Getting data info without exceeding the buffer
    ESP_LOGI(TAG_FACTORY_TESTING, "POST /leds\n%s", cData);
    if (bLenGetData) {
        reqData= cJSON_Parse(cData);
        cJSON_ArrayForEach(led, reqData) {
            i = atoi(led->string);
            obj = cJSON_GetObjectItem(led, "maxValue");
            if(cJSON_IsNumber(obj)) {
                if (LedsTest_SetMax(i, obj->valueint) == false) result = false;
            }
            obj = cJSON_GetObjectItem(led, "value");
            if(cJSON_IsNumber(obj)) {
                if (LedsTest_SetVal(i, obj->valueint) == false) result = false;
            }
        }
    }

    cJSON_AddBoolToObject(payload, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
    cJSON_Delete(payload);
}

// void _disable_wifi_test(){
//     ESP_LOGI(TAG_FACTORY_TESTING, "stopping wifi carrier test");
//     RFTEST_Cancel();
//     esp_restart();
//     // RFTEST_Cancel();
//     // WIFI_Disable();
//     // TEST_WifiInit();
//     // while(!WIFI_IsConnected(NULL))
//     // {
//     //     TEST_WifiInit();
//     //     vTaskDelay(10000/portTICK_PERIOD_MS);
//     // }
// }

void __URI_post_wifi(httpd_req_t * xReq) 
{
char cData[500];
size_t bSize;
int bLenGetData;

cJSON * xJsonData;
cJSON * xJsonObj;
cJSON * xJsonResp = cJSON_CreateObject();

uint16_t uiRate = 50;
uint16_t uiLoops = 1;
int8_t iPower = 40;

    ESP_LOGI(TAG_FACTORY_TESTING, "POST /wifi");
    if (RfTest_IsRunning() == true) {
        cJSON_AddBoolToObject(xJsonResp, "result", false);
        HTTP_ResponseSend(xReq, cJSON_Print(xJsonResp), HTTP_CONTENT_JSON);
        cJSON_Delete(xJsonResp);
        return;
    }

    bSize = MIN(xReq->content_len, sizeof(cData));              // Detect what is the MIN lenght betwwen data and buffer
    bLenGetData = httpd_req_recv(xReq, cData, bSize);           // Getting data info without exceeding the buffer
    ESP_LOGI(TAG_FACTORY_TESTING, "Data: %s", cData);
    if (bLenGetData)
    {
        xJsonData = cJSON_Parse(cData);
        if (xJsonData != NULL)
        {
            xJsonObj = cJSON_GetObjectItem(xJsonData, "rfLoops"); if (xJsonObj != NULL) uiLoops = xJsonObj->valueint;
            xJsonObj = cJSON_GetObjectItem(xJsonData, "rfRate"); if (xJsonObj != NULL) uiRate = xJsonObj->valueint;
            xJsonObj = cJSON_GetObjectItem(xJsonData, "rfPower"); if (xJsonObj != NULL) iPower = xJsonObj->valueint;

            iPower = (iPower<40) ? 40 : (iPower>82) ? 82 : iPower;
            ESP_LOGI(TAG_FACTORY_TESTING, "Getted params rf test %d %d power %d", uiLoops, uiRate, iPower);
            esp_wifi_set_max_tx_power(iPower);
            if (RfTest_start(uiLoops, uiRate) == false) {
                HTTP_ResponseErrorCustom(xReq, 500, HTTP_CONTENT_TEXT, "Error starting test");
            } else {
                cJSON_AddBoolToObject(xJsonResp, "result", true);
                HTTP_ResponseSend(xReq, cJSON_Print(xJsonResp), HTTP_CONTENT_JSON);
                ESP_LOGI(TAG_FACTORY_TESTING, "TEST Loops(%d) Rate(%d)", uiLoops, uiRate);
            }
            
        } else {
            HTTP_ResponseErrorCustom(xReq, 500, HTTP_CONTENT_TEXT, "Error parsing config wireless JSON object");
        }
    } else {
        HTTP_ResponseErrorCustom(xReq, 500, HTTP_CONTENT_TEXT, "Error parsing config wireless JSON object");
    }

    cJSON_Delete(xJsonResp);
}

void __URI_get_wifi(httpd_req_t * xReq)
{
cJSON * xJsonInfo = cJSON_CreateObject();
char cStrTemp[1024*3] = "";
char cValue[6];
uint16_t uiIdx = 0;
uint16_t uiTotal;
uint32_t uiAverageRf = 0;

    ESP_LOGI(TAG_FACTORY_TESTING, "GET /wifi");

    uiTotal = RfTest_GetNumTxDetections();
    cJSON_AddStringToObject(xJsonInfo, "rfRunning", (RfTest_IsRunning() == true) ? "true" : "false");
    cJSON_AddNumberToObject(xJsonInfo, "rfDetections", uiTotal);

    for (uiIdx=0; ((uiIdx<uiTotal)&&(uiIdx<MAX_RF_BUFFER)); uiIdx++) {
        uiAverageRf += RfTest_GetArrayMeasures()[uiIdx];
        strcat(cStrTemp, itoa(RfTest_GetArrayMeasures()[uiIdx], cValue, 10)); strcat(cStrTemp, ","); 
    } 
    if (uiIdx) uiAverageRf /= uiIdx;
    cJSON_AddNumberToObject(xJsonInfo, "rfAverage", uiAverageRf);
    cJSON_AddStringToObject(xJsonInfo, "rfMeasures", cStrTemp);

    HTTP_ResponseSend(xReq, cJSON_Print(xJsonInfo), HTTP_CONTENT_JSON);
    ESP_LOGI(TAG_FACTORY_TESTING, "%s", cJSON_Print(xJsonInfo));
    cJSON_Delete(xJsonInfo);
}

void __URI_get_rssi(httpd_req_t *xReq)
{
cJSON * xJsonInfo = cJSON_CreateObject();
cJSON * xJsonArray = cJSON_CreateArray();
cJSON * xJsonAp;
char cStr[32];
int8_t iRssi;
uint16_t uiIdx;

    ESP_LOGI(TAG_FACTORY_TESTING, "GET /rssi");
    WIFI_StartScan(false, WIFI_CHANNEL_ALL);
    for (uiIdx=0; uiIdx<WIFI_GetNumScannedSsids(); uiIdx++)
    {
        WIFI_GetScannedSsid(uiIdx, cStr, &iRssi, NULL, NULL);
        cJSON_AddItemToArray(xJsonArray, xJsonAp = cJSON_CreateObject());
        cJSON_AddStringToObject(xJsonAp, "ssid", cStr);
        cJSON_AddNumberToObject(xJsonAp, "rssi", (double)iRssi);
    }
    cJSON_AddItemToObject(xJsonInfo, "nets", xJsonArray);

    HTTP_ResponseSend(xReq, cJSON_Print(xJsonInfo), HTTP_CONTENT_JSON);
    cJSON_Delete(xJsonInfo);
}

void __URI_post_reset(httpd_req_t * xReq)
{
cJSON * xPayload = cJSON_CreateObject();

    ESP_LOGI(TAG_FACTORY_TESTING, "POST /reset");
    cJSON_AddBoolToObject(xPayload, "result", true);
    HTTP_ResponseSend(xReq, cJSON_Print(xPayload), HTTP_CONTENT_JSON);
    cJSON_Delete(xPayload);
    TMR_delay(100e3);
    bSoftReset = true;
    // esp_restart();
}

void TEST_WifiInit(void)
{
    //Init NVS
    NVS_Init();
    TEST_STEPS state = factorytest_getState();
    factorytest_updateNVS();
    //Init Wi-Fi
    ESP_LOGI(TAG_FACTORY_TESTING, "TEST_FactoryTestStart(%s)", _state_to_string(state));
    if(state == TEST_STEP0){
        WIFI_Init(WIFI_MODE_MANAGED, FACTORY_TEST_STEP0_SSID, FACTORY_TEST_STEP0_PSK);
        ESP_LOGI(TAG_FACTORY_TESTING, "Connect to %s", FACTORY_TEST_STEP0_SSID);
    }
    else if(state == TEST_STEP1){
        WIFI_Init(WIFI_MODE_MANAGED, FACTORY_TEST_STEP1_SSID, FACTORY_TEST_STEP1_PSK);
        ESP_LOGI(TAG_FACTORY_TESTING, "Connect to %s", FACTORY_TEST_STEP1_SSID);
    }
    else if(state == TEST_STEP2){
        WIFI_Init(WIFI_MODE_MANAGED, FACTORY_TEST_STEP2_SSID, FACTORY_TEST_STEP2_PSK);
        ESP_LOGI(TAG_FACTORY_TESTING, "Connect to %s", FACTORY_TEST_STEP2_SSID);
    }
    else{
        WIFI_Init(WIFI_MODE_MANAGED, FACTORY_TEST_STEP0_SSID, FACTORY_TEST_STEP0_PSK);
        ESP_LOGI(TAG_FACTORY_TESTING, "Connect to %s", FACTORY_TEST_STEP0_SSID);
    }
}

bool TEST_FactoryTestStart(void)
{
    TEST_WifiInit();
    ButtonTest_Init(1);
    RelayTest_Init(1);
    MeterTest_Init(1);
    LedsTest_Init(1);
    SyncTest_Init(1);
    RfTest_Init(0);
    return (xTaskCreatePinnedToCore(_factorytest_taskLoop, "FACTORY_TESTING", CONFIG_STACK_FACTORY_TESTING, NULL, 5, NULL, tskNO_AFFINITY) == pdPASS);
}

void factorytest_updateNVS(void)
{
    NVS_ReadInt16(FACTORY_TEST_NVS_KEY_RESETS, &resets);
    resets++;
    NVS_WriteInt16(FACTORY_TEST_NVS_KEY_RESETS, resets);
}

bool TEST_IsFactoryTestPassed(void)
{ 
    NVS_Init();
    bool testsPassed = (factorytest_getState() == TEST_PASSED);
    return testsPassed;
}



