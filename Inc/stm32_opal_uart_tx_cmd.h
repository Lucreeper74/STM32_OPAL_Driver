#ifndef __OPAL_SERIAL_TX_CMD_H__
#define __OPAL_SERIAL_TX_CMD_H__

#include "stm32_opal_frame.h"
#include "stm32_opal_emitter.h"
#include "stm32_uart_rx.h"

#include <stdlib.h>
#include <stdio.h>

/*
*   Definition of OPAL TX Unit - UART Commands enumeration
*/
typedef enum {
    SEND_TEST_FRAME = 0x0,
    VERIFY,
    UNKNOWN_COMMAND,
} OPAL_UART_TX_CommandType;

/*
*   Pair mapping command types to their string representations
*/
typedef struct {
    OPAL_UART_TX_CommandType type; /* !< Command type */
    const char* name; /* !< Command name string */
} OPAL_UART_TX_CommandType_Map;

/*
*   Array to get the string representation of a command type
*/
static const OPAL_UART_TX_CommandType_Map cmdType_map[] = {
    { SEND_TEST_FRAME, "SEND_TEST_FRAME" },
    { VERIFY, "VERIFY"}
};

/*
*  Process OPAL TX unit commands received via UART
*/
void OPAL_TX_UART_processCommand(const UART_Command* cmd, OPAL_Emitter_Handle* htx);

#endif // __OPAL_SERIAL_TX_CMD_H__