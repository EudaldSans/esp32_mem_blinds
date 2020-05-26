/********************************************************************************************
* Historico de Revisiones
*
* Versión	Fecha		    Autor	Comentario
------------------------------------------------------------------------------------------------
* V0.0 		14-06-2019      AML	    Creacion
***********************************************************************************************/

/* INCLUDES */
/* -------- */
#include "BLINDS_Device.h"

#include <stdbool.h>
#include "esp_log.h"

/* TYPES */
/* ----- */

/* DEFINES */
/* ------- */
#define TAG_DEVICE                  "[DEVICE]"

/* INTERNAL FUNCTIONS */
/* ------------------ */

/* EXTERNAL FUNCTIONS */
/* ------------------ */

/* PUBLIC FUNCTIONS */
/* ---------------- */

/* INTERNAL VARIABLES */
/* ------------------ */

/* EXTERNAL VARIABLES */
/* ------------------ */

/* CODE */
/* ---- */
void DEVICE_SetCmd(uint8_t uiChild, PROTOCOL_VARIABLES xVar, double dValue)
{
    // if (uiChild != 1) { ESP_LOGW(TAG_DEVICE, "Wrong child Set"); return; }
    // switch (xVar)
    // {
    //     case PROTOCOL_VARIABLE_STATUS:      ESP_LOGI(TAG_DEVICE, "SET STATUS");
    //                                         if (dValue) DIMMER_On(); else DIMMER_Off();
    //                                         break;

    //     case PROTOCOL_VARIABLE_LEVEL:       ESP_LOGI(TAG_DEVICE, "SET LEVEL");
    //                                         MEMDIM_SetLevel((uint8_t)dValue);
    //                                         break;

    //     case PROTOCOL_VARIABLE_IDENTIFY:    ESP_LOGI(TAG_DEVICE, "Identify signal");
    //                                         break;

    //     default:                            break;
    // }
}

void DEVICE_GetCmd(uint8_t uiChild, PROTOCOL_VARIABLES xVar)
{
    // if (uiChild != 1) { ESP_LOGW(TAG_DEVICE, "Wrong child Get"); return; }
    // switch (xVar)
    // {
    //     case PROTOCOL_VARIABLE_STATUS:      ESP_LOGI(TAG_DEVICE, "GET STATUS REQUEST");
    //                                         MEM_SendInfo(uiChild, PROTOCOL_VARIABLE_STATUS, (double)(DIMMER_GetLevel() ? true : false));
    //                                         break;

    //     case PROTOCOL_VARIABLE_LEVEL:       ESP_LOGI(TAG_DEVICE, "GET LEVEL REQUEST");
    //                                         MEM_SendInfo(uiChild, PROTOCOL_VARIABLE_LEVEL, (double)DIMMER_GetLevel());
    //                                         break;

    //     default:                            break;
    // }
}

// void DEVICE_ResCmd(uint8_t uiChild, PROTOCOL_VARIABLES xVar, double dValue)
// {
//     switch (xVar)
//     {
//         case PROTOCOL_VARIABLE_STATUS:
//         case PROTOCOL_VARIABLE_LEVEL:
//         default:                            break;
//     }
// }

void DEVICE_Reset(void)
{
    ESP_LOGW(TAG_DEVICE, "Reset request");
}

void DEVICE_Apply(float fPercentatge)
{
    ESP_LOGW(TAG_DEVICE, "New dimming value %f", fPercentatge);
}
