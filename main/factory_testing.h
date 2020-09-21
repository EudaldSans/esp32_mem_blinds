#ifndef _FACTORY_TESTING
#define _FACTORY_TESTING

/* INCLUDES */
/* -------- */

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define FACTORY_TEST_PRODUCT_SERIES     "blinds"
#define FACTORY_TEST_AP_SSID            FACTORY_TEST_PRODUCT_SERIES"_mem_ap"
#define FACTORY_TEST_AP_PSK             "1234567890"
#define FACTORY_TEST_STEP0_SSID         FACTORY_TEST_PRODUCT_SERIES"_mem_fase_0"
#define FACTORY_TEST_STEP0_PSK          "1234567890"
#define FACTORY_TEST_STEP1_SSID         FACTORY_TEST_PRODUCT_SERIES"_mem_fase_1"
#define FACTORY_TEST_STEP1_PSK          "1234567890"
#define FACTORY_TEST_STEP2_SSID         FACTORY_TEST_PRODUCT_SERIES"_mem_fase_2"
#define FACTORY_TEST_STEP2_PSK          "1234567890"
#define FACTORY_TEST_HTTP_SERVER_ID     4 
#define FACTORY_TEST_HTTP_PORT          8000
#define FACTORY_TEST_HTTP_PRIORITY      3

#define WIFI_TEST_TIMEOUT               10000
#define KEY_DATE                        "date"
#define KEY_SERIAL_ID                   "serial_id"

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
void TEST_WifiInit();
bool TEST_FactoryTestStart(void);
bool TEST_IsFactoryTestPassed(void);

#endif
