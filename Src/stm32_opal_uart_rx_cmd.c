#include "stm32_opal_uart_rx_cmd.h"


// To get param number
// int test_num = atoi(cmd->param);

void OPAL_RX_UART_processCommand(const UART_Command* cmd, OPAL_Emitter_Handle* htx) {
    OPAL_UART_RX_CommandType cmdType = UNKNOWN_COMMAND;

    // Find command type from string
    for (int i = 0; i < (sizeof(cmdType_map)/sizeof(OPAL_UART_RX_CommandType_Map)); i++) {
        if (strcmp((const char*) cmd->command, cmdType_map[i].name) == 0) {
            cmdType = cmdType_map[i].type;
            break;
        }
    }
    
    // Process command based on type
    switch (cmdType) {
        case VERIFY: {
            // Handle VERIFY command
            printf("Communication verified successfully!\r\n");
            break;
        }

        default:
            break;
    }
}