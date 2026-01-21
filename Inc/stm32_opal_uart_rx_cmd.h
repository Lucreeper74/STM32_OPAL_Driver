#ifndef __OPAL_SERIAL_RX_CMD_H__
#define __OPAL_SERIAL_RX_CMD_H__

#include "stm32_opal_frame.h"
#include "stm32_opal_emitter.h"
#include "stm32_uart_rx.h"

#include <stdlib.h>
#include <stdio.h>

/*
*   Definition of OPAL RX Unit - UART Commands enumeration
*/
typedef enum {
    VERIFY,
    UNKNOWN_COMMAND,
} OPAL_UART_RX_CommandType;

/*
*   Pair mapping command types to their string representations
*/
typedef struct {
    OPAL_UART_RX_CommandType type; /* !< Command type */
    const char* name; /* !< Command name string */
} OPAL_UART_RX_CommandType_Map;

/*
*   Array to get the string representation of a command type
*/
static const OPAL_UART_RX_CommandType_Map cmdType_map[] = {
    { VERIFY, "VERIFY"}
};

/*
*  Process OPAL RX unit commands received via UART
*/
void OPAL_RX_UART_processCommand(const UART_Command* cmd, OPAL_Emitter_Handle* htx);

#endif // __OPAL_SERIAL_RX_CMD_H__