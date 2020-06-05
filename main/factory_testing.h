#ifndef _FACTORY_TESTING
#define _FACTORY_TESTING

/* INCLUDES */
/* -------- */

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define FACTORY_TEST_STEP0_SSID         "blinds_mem_fase_0"
#define FACTORY_TEST_STEP0_PSK          "1234567890"
#define FACTORY_TEST_STEP1_SSID         "blinds_mem_fase_1"
#define FACTORY_TEST_STEP1_PSK          "1234567890"
#define FACTORY_TEST_HTTP_SERVER_ID     4 
#define FACTORY_TEST_HTTP_PORT          8000
#define FACTORY_TEST_HTTP_PRIORITY      3

#define KEY_SERIAL_CODE                 "serial_code"
#define KEY_CLIENT_SECRET               "client_id"
#define KEY_DATE                        "date"
#define KEY_SERIAL_ID                   "serial_id"

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */
bool TEST_FactoryTestStart(void);
bool TEST_IsFactoryTestPassed(void);

#endif