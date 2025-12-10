#include "stm32_opal_emitter.h"

static volatile bool tx_active = false;   // Default state inactive (Volatile for ITR access)
static uint16_t dac_buffer[OPAL_FRAME_BUFFER_SIZE] = {};

OPAL_Status OPAL_Emitter_Encode(OPAL_Frame* frame) {
    if (frame == NULL)
        return OPAL_ERROR_NULL_PTR;

    frame->CRC16 = OPAL_Compute_CRC16(frame);

    uint8_t frame_bytes[OPAL_FRAME_SIZE] = {};
    OPAL_Frame_Byte_Conversion(frame, frame_bytes);

    // Convert all frame bytes to symbols
    OPAL_PAM4_symbol frame_symbols[OPAL_FRAME_BUFFER_SIZE] = {};
    size_t symbol_index = 0;
    for (size_t i = 0; i < OPAL_FRAME_SIZE; i++) {
        OPAL_byte_to_pam4(frame_bytes[i], &frame_symbols[symbol_index]);
        symbol_index += OPAL_SYMBOLS_PER_BYTE;
    }

    // Load DAC buffer with voltage levels
    for (size_t i = 0; i < OPAL_FRAME_BUFFER_SIZE-2; i++)
        dac_buffer[i] = OPAL_symbol_to_voltage(frame_symbols[i]);

    // Reset TX flag
    tx_active = false;
    
    return OPAL_SUCCESS;
}

OPAL_Status OPAL_Emitter_Send_Frame(TIM_HandleTypeDef* htim, DAC_HandleTypeDef* hdac) {
    tx_active = true;
    
    // Load first value to handle undefined state at start
    HAL_DAC_SetValue(hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0x000);

    HAL_TIM_Base_Start(htim);
    HAL_DAC_Start_DMA(hdac, DAC_CHANNEL_1, (uint32_t*)dac_buffer, OPAL_FRAME_BUFFER_SIZE, DAC_ALIGN_12B_R);

    return OPAL_SUCCESS;
}

void OPAL_Transmission_Finished_Callback(TIM_HandleTypeDef* htim, DAC_HandleTypeDef* hdac) {
    if (tx_active) {
        tx_active = false; // End of transmission
        HAL_TIM_Base_Stop(htim);
        HAL_DAC_Stop_DMA(hdac, DAC_CHANNEL_1);
        //HAL_DAC_SetValue(hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0x000); // Reset DAC to 0V
    }
}