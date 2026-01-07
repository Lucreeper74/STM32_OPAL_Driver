#ifndef __STM32_OPAL_USART_H__
#define __STM32_OPAL_USART_H__

#include "stm32_opal_utils.h"

#include <string.h>
#include <stdlib.h>
#include <sys/_intsup.h>

#define OPAL_UART_RX_BUFFER_SIZE 64  /* !< Size of the UART RX buffer */
#define OPAL_UART_CMD_FIELDS_SIZE (OPAL_UART_RX_BUFFER_SIZE / 2)  /* !< Size of the UART parameter buffer */

/*
*   Definition of OPAL UART Command structure
*/
typedef struct {
    char    command[OPAL_UART_CMD_FIELDS_SIZE]; /* !< Command type */
    char    param[OPAL_UART_CMD_FIELDS_SIZE];   /* !< Command parameter */
    bool    has_param;     /* !< Flag indicating if parameter is present */
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