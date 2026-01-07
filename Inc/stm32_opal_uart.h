#ifndef __STM32_OPAL_USART_H__
#define __STM32_OPAL_USART_H__

#include "stm32_opal_utils.h"

#include <string.h>
#include <stdlib.h>

#define OPAL_UART_RX_BUFFER_SIZE 64  /* !< Size of the UART RX buffer */
#define OPAL_UART_PARAM_SIZE 32  /* !< Size of the UART parameter buffer */

/*
*   Definition of OPAL UART Commands enumeration
*/
typedef enum {
    OPAL_TEST1_COMMAND = 0x0,
    OPAL_TEST2_COMMAND,
    OPAL_TEST_SETCOMMAND,
    OPAL_UNKNOWN_COMMAND,
} OPAL_UART_CommandType;

/*
*   Pair mapping command types to their string representations
*/
typedef struct {
    OPAL_UART_CommandType type; /* !< Command type */
    const char*           name; /* !< Command name string */
} OPAL_CommandType_Map;

/*
*   Array to get the string representation of a command type
*/
static const OPAL_CommandType_Map cmdType_map[] = {
    { OPAL_TEST1_COMMAND,    "TEST1" },
    { OPAL_TEST2_COMMAND,    "TEST2" },
    { OPAL_TEST_SETCOMMAND,  "SETCOMMAND" },
    { OPAL_UNKNOWN_COMMAND,  "UNKNOWN" },
};

/*
*   Definition of OPAL UART Command structure
*/
typedef struct {
    OPAL_UART_CommandType commandType;   /* !< Command type */
    uint8_t               param[OPAL_UART_PARAM_SIZE]; /* !< Command parameter */
    bool                  has_param;     /* !< Flag indicating if parameter is present */
} OPAL_UART_Command;




/*
*   Definition of OPAL UART Receiver handle structure
*/
typedef struct {
    UART_HandleTypeDef* UART_Handle;    /* !< Associated UART Handle */
    volatile uint8_t    rx_char;        /* !< Received character (volatile for ITR access)  */
    volatile uint8_t    rx_buffer[OPAL_UART_RX_BUFFER_SIZE];  /* !< Reception buffer (volatile for ITR access) */
    volatile uint8_t    rx_index;       /* !< Reception buffer index (volatile for ITR access) */
    volatile uint8_t    cmd_ready;      /* !< Command ready flag (volatile for ITR access) */
} OPAL_UART_RX_Handle;

extern OPAL_UART_RX_Handle huart_rx; // Global OPAL UART Receiver handle

/*
*   Function to initialize the OPAL UART Receiver handle
*/
void OPAL_UART_RX_Init(UART_HandleTypeDef* huart);

/*
*   Function to parse the UART RX command
*/
OPAL_UART_Command OPAL_UART_RX_ParseCmd(OPAL_UART_RX_Handle* huart_rx);

/*
*   Function to handle the UART RX callback
*/
void OPAL_UART_RX_Callback(UART_HandleTypeDef* huart);

#endif // __STM32_OPAL_USART_H__