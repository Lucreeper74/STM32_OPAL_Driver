#include "stm32_opal_uart.h"

OPAL_UART_RX_Handle huart_rx;

void OPAL_UART_RX_Init(UART_HandleTypeDef* huart) {
    huart_rx.UART_Handle = huart;
    huart_rx.rx_index    = 0;
    huart_rx.cmd_ready   = 0;

    HAL_UART_Receive_IT(huart_rx.UART_Handle, (uint8_t*) &huart_rx.rx_char, 1);
}

OPAL_UART_Command OPAL_UART_RX_ParseCmd(OPAL_UART_RX_Handle* huart_rx) {
    OPAL_UART_Command cmd = {};

    // Check for space to separate command and parameter
    const char *space = strchr((const char*) huart_rx->rx_buffer, ' ');

    if (space != NULL) {
        // Get Command part before space
        size_t cmd_length = space - (const char*) huart_rx->rx_buffer;
        strncpy(cmd.command, (const char*) huart_rx->rx_buffer, cmd_length);
        cmd.command[cmd_length] = '\0';

        // Get parameter part after space
        strncpy((char*) cmd.param, space + 1, OPAL_UART_CMD_FIELDS_SIZE - 1);
        cmd.param[OPAL_UART_CMD_FIELDS_SIZE - 1] = '\0';
        cmd.has_param = true;
        
    } else {
        // No parameter, copy entire buffer as command
        strncpy(cmd.command, (const char*) huart_rx->rx_buffer, OPAL_UART_CMD_FIELDS_SIZE - 1);
        cmd.command[OPAL_UART_CMD_FIELDS_SIZE - 1] = '\0';
    }

    // Clear command ready flag after processing
    huart_rx->cmd_ready = 0;
    return cmd;
}

void OPAL_UART_RX_Callback(UART_HandleTypeDef* huart) {
    if (huart->Instance != huart_rx.UART_Handle->Instance)
        return;

    if (huart_rx.rx_char == '\n' || huart_rx.rx_char == '\r') {
        // End of command
        huart_rx.rx_buffer[huart_rx.rx_index] = '\0'; // Null-terminate the string
        huart_rx.cmd_ready = 1; // Set command ready flag
        huart_rx.rx_index = 0; // Reset index for next command
    } else {
        // Store received character in buffer if space is available
        if (huart_rx.rx_index < OPAL_UART_RX_BUFFER_SIZE - 1) {
            huart_rx.rx_buffer[huart_rx.rx_index++] = huart_rx.rx_char;
        } else {
            // Buffer overflow, reset index
            huart_rx.rx_index = 0;
        }
    }

    // Restart UART reception
    HAL_UART_Receive_IT(huart_rx.UART_Handle, (uint8_t*) &huart_rx.rx_char, 1);
}
