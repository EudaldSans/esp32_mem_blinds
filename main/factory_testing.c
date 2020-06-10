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

#include "ges_wifi.h"
#include "ges_http_server.h"
#include "ges_nvs.h"

#include "factory_testing.h"
#include "test_button.h"
#include "test_relay.h"
#include "test_meter.h"
#include "test_leds.h"
#include "test_sync.h"

/* TYPES */
/* ----- */
typedef enum {
    TEST_STEP0=0,
    TEST_STEP1,
    TEST_PASSED,
    TEST_NUM_STEPS
} TEST_STEPS;

/* DEFINES */
/* ------- */

//Useful constants
#define FACTORY_TEST_NVS_KEY_STATE      "PROD_TEST_STATE"
#define FACTORY_TEST_NVS_KEY_RESETS     "PROD_TEST_RESET"
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
void __URI_get_reset(httpd_req_t * xReq);

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
void factorytest_updateNVS();
void _reinitStep();
void uiatoa(uint8_t* a, size_t len_a, char* s, size_t len_s);

/* INTERNAL VARIABLES */
/* ------------------ */
static uint16_t resets = 0;

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
        HTTP_RegisterUri(xWebUi, HTTP_GET, "/reset", __URI_get_reset);
        INFO("URIs initiated");
        return true;
    }
    ERROR("httpd was not able to start"); 
    return false;
}

TEST_STEPS factorytest_getState(void)
{
    uint16_t uiState = 0;
    NVS_ReadInt16(FACTORY_TEST_NVS_KEY_STATE, &uiState);
    ESP_LOGI(TAG_FACTORY_TESTING, "getState() : %d", uiState);
    return (TEST_STEPS)uiState;
}

bool factorytest_setState(TEST_STEPS step){
    bool res = false;
    res = NVS_WriteInt16(FACTORY_TEST_NVS_KEY_STATE, (uint16_t)step);
    ESP_LOGI(TAG_FACTORY_TESTING, "setState(%d) : %s", step, res ? "true": "false");
    _reinitStep();
    return res;
}

void _reinitStep(){
    resets = 1;
    NVS_WriteInt16(FACTORY_TEST_NVS_KEY_RESETS, resets);
}

char * _state_to_string(TEST_STEPS state){
    switch(state){
        case TEST_STEP0:
            return "phase0";
        case TEST_STEP1:
            return "phase1";
        case TEST_PASSED:
            return "passed";
        default:
            return "unknown";
    }
}

TEST_STEPS stateFromString(char * state){
    if(strcmp(state, "phase0")==0)
        return TEST_STEP0;
    else if(strcmp(state, "phase1")==0)
        return TEST_STEP1;
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
        vTaskDelay(100);
    //wait for requests, we should not get here
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
    for(int i=0; i<ButtonTest_GetTotalButtons(); i++){
        snprintf(key, sizeof(key), "%d", i);
        cJSON_AddItemToObject(payload, key, obj = cJSON_CreateObject());
        cJSON_AddNumberToObject(obj, "shortPulsations", ButtonTest_GetShortPulsations(i));
        cJSON_AddNumberToObject(obj, "longPulsations", ButtonTest_GetLongPulsations(i));
    }


    payload_str = cJSON_Print(payload);
    ESP_LOGI(TAG_FACTORY_TESTING, "GET /buttons\n%s\n", payload_str);
    HTTP_ResponseSend(xReq, payload_str, HTTP_CONTENT_JSON);
}

void __URI_post_buttons(httpd_req_t * xReq){
    cJSON * payload = cJSON_CreateObject();

    ButtonTest_Reset();

    cJSON_AddBoolToObject(payload, "result", true);
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
}

void __URI_get_relays(httpd_req_t * xReq)
{
    cJSON * payload = cJSON_CreateObject();
    cJSON * obj;
    char * payload_str;
    char key[10];
    for(int i=0; i<RelayTest_GetTotalRelays(); i++){
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
}

void __URI_post_relays(httpd_req_t * xReq){
    char cData[500];
    size_t bSize;
    int bLenGetData;
    bool result = true;
    int ret = false;

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
        cJSON_ArrayForEach(relay, reqData){
            if(cJSON_IsObject(relay)){
                index = atoi(relay->string);
                if(cJSON_HasObjectItem(relay, "status")){
                    item = cJSON_GetObjectItem(relay, "status");
                    if(cJSON_IsBool(item)){
                        ret = RelayTest_SetStatus(index, cJSON_IsTrue(item));
                        result = (ret != -1);
                    }
                }
                if(cJSON_HasObjectItem(relay, "operateTime")){
                    item = cJSON_GetObjectItem(relay, "operateTime");
                    if(cJSON_IsNumber(item)){
                        ret = RelayTest_SetOperateTime(index, item->valueint);
                        result = (ret != -1);
                    }
                }
                if(cJSON_HasObjectItem(relay, "releaseTime")){
                    item = cJSON_GetObjectItem(relay, "releaseTime");
                    if(cJSON_IsNumber(item)){
                        ret = RelayTest_SetReleaseTime(index, item->valueint);
                        result = (ret != -1);
                    }
                }
                if(cJSON_HasObjectItem(relay, "runCalibration")){
                    item = cJSON_GetObjectItem(relay, "runCalibration");
                    if(cJSON_IsTrue(item)){
                        RelayTest_Calibrate(index);
                    }
                }
                if(cJSON_HasObjectItem(relay, "runResistiveCalibration")){
                    item = cJSON_GetObjectItem(relay, "runResistiveCalibration");
                    if(cJSON_IsTrue(item)){
                        RelayTest_ResistorCalibrate(index);
                    }
                }
            }
        }
    }

    cJSON_AddBoolToObject(payload, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
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
}

void __URI_post_meter(httpd_req_t * xReq){
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
        if(cJSON_HasObjectItem(reqData, "kp")){
            obj = cJSON_GetObjectItem(reqData, "kp");
            val = atof(cJSON_Print(obj));
            MeterTest_SetKp(val);
        }
        if(cJSON_HasObjectItem(reqData, "kv")){
            obj = cJSON_GetObjectItem(reqData, "kv");
            val = atof(cJSON_Print(obj));
            MeterTest_SetKv(val);
        }
        if(cJSON_HasObjectItem(reqData, "ki")){
            obj = cJSON_GetObjectItem(reqData, "ki");
            val = atof(cJSON_Print(obj));
            MeterTest_SetKi(val);
        }
        if(cJSON_HasObjectItem(reqData, "calibrationParams")){
            obj = cJSON_GetObjectItem(reqData, "calibrationParams");
            if(cJSON_HasObjectItem(obj, "power")){
                tmp = cJSON_GetObjectItem(obj, "power");
                val = atof(cJSON_Print(tmp));
                MeterTest_SetCalibrationPower(val);
            }
            if(cJSON_HasObjectItem(obj, "voltage")){
                tmp = cJSON_GetObjectItem(obj, "voltage");
                val = atof(cJSON_Print(tmp));
                MeterTest_SetCalibrationVoltage(val);
            }
            if(cJSON_HasObjectItem(obj, "current")){
                tmp = cJSON_GetObjectItem(obj, "current");
                val = atof(cJSON_Print(tmp));
                MeterTest_SetCalibrationCurrent(val);
            }
        }
        if(cJSON_HasObjectItem(reqData, "runCalibration")){
            obj = cJSON_GetObjectItem(reqData, "runCalibration");
            if(cJSON_IsTrue(obj)){
                MeterTest_Calibrate();
            }
        }
    }

    cJSON_AddBoolToObject(payload, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
}

void __URI_get_ac(httpd_req_t * xReq) {
    cJSON * xPayload = cJSON_CreateObject();
    cJSON_AddNumberToObject(xPayload, "T", SyncTest_GetPeriod());
    cJSON_AddNumberToObject(xPayload, "Ton", SyncTest_GetPeriod()/2);
    HTTP_ResponseSend(xReq, cJSON_Print(xPayload), HTTP_CONTENT_JSON);
}

void __URI_get_state(httpd_req_t * xReq) {
    uint16_t step = factorytest_getState();
    char * step_description = _state_to_string(step);
    cJSON * xPayload = cJSON_CreateObject();
    cJSON_AddItemToObject(xPayload, "state", cJSON_CreateString(step_description));
    HTTP_ResponseSend(xReq, cJSON_Print(xPayload), HTTP_CONTENT_JSON);
}

void __URI_post_state(httpd_req_t * xReq) {
    char cData[500];
    size_t bSize;
    int bLenGetData;
    bool result = true;
    cJSON * xJsonData;
    cJSON * xJsonObj;
    cJSON * xJsonResp = cJSON_CreateObject();

    char * state_string = NULL;
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
    }
    else{
        result = false;
    }

    cJSON_AddBoolToObject(xJsonResp, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(xJsonResp), HTTP_CONTENT_JSON);
}

void __URI_get_info(httpd_req_t * xReq) {
    bool res = false;
    uint16_t currentResets = resets - 1;
    char hwId[33];
    size_t len;
    size_t* len_ptr;
    uint8_t* macBytes;
    char mac[MAC_BYTES_LEN*2 + 1];
    char secret[65];
    char serialId[32];
    uint16_t date;
    char error_code[] = "undefined";
    cJSON * xPayload = cJSON_CreateObject();

    len = sizeof(hwId);
    len_ptr = &len;
    res = NVS_ReadStr(KEY_SERIAL_CODE, hwId, len_ptr);
    if(!res)
        strcpy(hwId, error_code);

    len = sizeof(secret);
    len_ptr = &len;
    res = NVS_ReadStr(KEY_CLIENT_SECRET, secret, len_ptr);
    if(!res)
        strcpy(secret, error_code);

    res = NVS_ReadInt16(KEY_DATE, &date);
    if(!res)
        date = 0;

    len = sizeof(serialId);
    len_ptr = &len;
    res = NVS_ReadStr(KEY_SERIAL_ID, serialId, len_ptr);
    if(!res)
        strcpy(serialId, error_code);
    
    macBytes = WIFI_GetMacAddress();
    uiatoa(macBytes, MAC_BYTES_LEN, mac, sizeof(mac));
    ESP_LOGI(TAG_FACTORY_TESTING, "GET /info\nsecret : %s\nhwId : %s", secret, hwId);

    cJSON_AddItemToObject(xPayload, "hwId", cJSON_CreateString(hwId));
    cJSON_AddItemToObject(xPayload, "secret", cJSON_CreateString(secret));
    cJSON_AddNumberToObject(xPayload, "resetsDone", currentResets);
    cJSON_AddItemToObject(xPayload, "mac", cJSON_CreateString(mac));
    cJSON_AddNumberToObject(xPayload, "date", date);
    cJSON_AddStringToObject(xPayload, "serialId", serialId);
    HTTP_ResponseSend(xReq, cJSON_Print(xPayload), HTTP_CONTENT_JSON);
}

void uiatoa(uint8_t* a, size_t len_a, char* s, size_t len_s){
    char buffer[3];
    memset(s, 0, len_s);
    for(int i=0; i<len_a; i++){
        snprintf(buffer, sizeof(buffer), "%x", a[i]);
        strncat(s, buffer, len_s);
    }
}

void __URI_post_info(httpd_req_t * xReq) {
    char cData[500];
    size_t bSize;
    int bLenGetData;
    bool result = false;
    bool ret = false;
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

    if(secret != NULL ){
        ESP_LOGI(TAG_FACTORY_TESTING, "set secret : %s", secret);
        ret = NVS_WriteStr(KEY_CLIENT_SECRET, secret);
        result = ret || result;
    }
    if(hwId != NULL ){
        ESP_LOGI(TAG_FACTORY_TESTING, "set hwId : %s", hwId);
        ret = NVS_WriteStr(KEY_SERIAL_CODE , hwId);
        result = ret || result;
    }

    if(serialId != NULL ){
        ESP_LOGI(TAG_FACTORY_TESTING, "set serialId : %s", serialId);
        ret = NVS_WriteStr(KEY_SERIAL_ID , serialId);
        result = ret || result;
    }

    if (cJSON_IsNumber(dateObj)){
        date = dateObj->valueint;
        ESP_LOGI(TAG_FACTORY_TESTING, "set date : %d", date);
        ret = NVS_WriteInt16(KEY_DATE, date);
        result = ret || result;
    }

    cJSON_AddBoolToObject(xJsonResp, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(xJsonResp), HTTP_CONTENT_JSON);
}

void __URI_get_leds(httpd_req_t * xReq)
{
    cJSON* payload = cJSON_CreateObject();
    cJSON* obj;
    char key[10];
    for(int i=0; i < LedsTest_getTotalLeds(); i++){
        obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "value", LedsTest_GetVal(i));
        cJSON_AddNumberToObject(obj, "maxValue", LedsTest_GetMax(i));
        snprintf(key, sizeof(key), "%d", i);
        cJSON_AddItemToObject(payload, key, obj);
    }
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
}

void __URI_set_leds(httpd_req_t * xReq)
{
    size_t bSize;
    int bLenGetData;
    char cData[500];
    bool result = true;
    bool ret;

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
        cJSON_ArrayForEach(led, reqData){
            i = atoi(led->string);
            obj = cJSON_GetObjectItem(led, "maxValue");
            if(cJSON_IsNumber(obj)){
                ret = LedsTest_SetMax(i, obj->valueint);
                result = result && ret;
            }
            obj = cJSON_GetObjectItem(led, "value");
            if(cJSON_IsNumber(obj)){
                ret = LedsTest_SetVal(i, obj->valueint);
                result = result && ret;
            }
        }
    }

    cJSON_AddBoolToObject(payload, "result", result);
    HTTP_ResponseSend(xReq, cJSON_Print(payload), HTTP_CONTENT_JSON);
}

void __URI_get_reset(httpd_req_t * xReq)
{
cJSON * xPayload = cJSON_CreateObject();

    cJSON_AddBoolToObject(xPayload, "result", true);
    HTTP_ResponseSend(xReq, cJSON_Print(xPayload), HTTP_CONTENT_JSON);
    esp_restart();
}

bool TEST_FactoryTestStart(void)
{
    TEST_STEPS state = factorytest_getState();
    NVS_Init();
    factorytest_updateNVS();
    ESP_LOGI(TAG_FACTORY_TESTING, "TEST_FactoryTestStart(%s)", _state_to_string(state));
    if(state == TEST_STEP0)
        WIFI_Init(WIFI_MODE_MANAGED, FACTORY_TEST_STEP0_SSID, FACTORY_TEST_STEP0_PSK);
    else if(state == TEST_STEP1)
        WIFI_Init(WIFI_MODE_MANAGED, FACTORY_TEST_STEP1_SSID, FACTORY_TEST_STEP1_PSK);
    else
        WIFI_Init(WIFI_MODE_MANAGED, FACTORY_TEST_STEP0_SSID, FACTORY_TEST_STEP0_PSK);
    ButtonTest_Init(1);
    RelayTest_Init(1);
    MeterTest_Init(1);
    LedsTest_Init(1);
    SyncTest_Init(1);
    return (xTaskCreatePinnedToCore(_factorytest_taskLoop, "FACTORY_TESTING", CONFIG_STACK_FACTORY_TESTING, NULL, 5, NULL, tskNO_AFFINITY) == ESP_OK);
}

void factorytest_updateNVS(){
    NVS_ReadInt16(FACTORY_TEST_NVS_KEY_RESETS, &resets);
    resets++;
    NVS_WriteInt16(FACTORY_TEST_NVS_KEY_RESETS, resets);
}

bool TEST_IsFactoryTestPassed(void)             { return (factorytest_getState() == TEST_PASSED); }



