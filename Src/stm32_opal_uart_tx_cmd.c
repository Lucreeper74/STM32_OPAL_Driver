#include "stm32_opal_uart_tx_cmd.h"


// To get param number
// int test_num = atoi(cmd->param);

void OPAL_TX_UART_processCommand(const UART_Command* cmd, OPAL_Emitter_Handle* htx) {
    OPAL_UART_TX_CommandType cmdType = UNKNOWN_COMMAND;

    // Find command type from string
    for (int i = 0; i < (sizeof(cmdType_map)/sizeof(OPAL_UART_TX_CommandType_Map)); i++) {
        if (strcmp((const char*) cmd->command, cmdType_map[i].name) == 0) {
            cmdType = cmdType_map[i].type;
            break;
        }
    }
    
    // Process command based on type
    switch (cmdType) {
        case SEND_TEST_FRAME: {
            // Handle SEND_TEST_FRAME command
            printf("Sending test frame...\r\n");
            OPAL_Frame testFrame = OPAL_TestFrame;
            if (OPAL_Emitter_Encode(htx, &testFrame) == OPAL_SUCCESS) {
                if (OPAL_Emitter_Send_Frame(htx) == OPAL_SUCCESS) {
                printf("Test frame sent successfully!\r\n");
                } else
                    printf("[!] Failed to send the test frame...\r\n");
            } else
                printf("[!] Failed to encode the test frame...\r\n");
            break;
        }

        case VERIFY: {
            // Handle VERIFY command
            printf("Communication verified successfully!\r\n");
            break;
        }

        default:
            break;
    }
}