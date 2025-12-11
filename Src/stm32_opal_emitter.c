#include "stm32_opal_emitter.h"

static volatile OPAL_Emitter_Status emitter_status = OPAL_EMITTER_IDLE; // Default state idling (Volatile for ITR access)

static uint16_t dac_buffer[OPAL_FRAME_BUFFER_SIZE+2] = {}; // +2 to cycle the TIM for the last symbol

OPAL_Status OPAL_Emitter_Encode(OPAL_Frame* frame) {
    if (frame == NULL)
        return OPAL_ERROR_NULL_PTR;

    frame->CRC16 = OPAL_Frame_Compute_CRC16(frame);

    uint8_t frame_bytes[OPAL_FRAME_SIZE] = {};
    OPAL_Frame_Bytes_Conversion(frame, frame_bytes);

    // Convert all frame bytes to symbols
    OPAL_PAM4_symbol frame_symbols[OPAL_FRAME_BUFFER_SIZE] = {};
    size_t symbol_index = 0;
    for (size_t i = 0; i < OPAL_FRAME_SIZE; i++) {
        OPAL_byte_to_pam4(frame_bytes[i], &frame_symbols[symbol_index]);
        symbol_index += OPAL_SYMBOLS_PER_BYTE;
    }

    // Load DAC buffer with voltage levels
    for (size_t i = 0; i < OPAL_FRAME_BUFFER_SIZE; i++)
        dac_buffer[i] = OPAL_symbol_to_voltage(frame_symbols[i]);

    // Update emitter status
    emitter_status = OPAL_EMITTER_IDLE;
    
    return OPAL_SUCCESS;
}

OPAL_Status OPAL_Emitter_Send_Frame(TIM_HandleTypeDef* htim, DAC_HandleTypeDef* hdac) {
    // Load first value to handle undefined state at start
    HAL_DAC_SetValue(hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0x000);

    if (HAL_TIM_Base_Start(htim) != HAL_OK)
        return OPAL_TIM_ERROR;

    if (HAL_DAC_Start_DMA(hdac, DAC_CHANNEL_1, (uint32_t*)dac_buffer, OPAL_FRAME_BUFFER_SIZE, DAC_ALIGN_12B_R) != HAL_OK)
        return OPAL_DAC_ERROR;

    emitter_status = OPAL_EMITTER_BUSY;

    return OPAL_SUCCESS;
}

void OPAL_Emitter_Finished_Callback(TIM_HandleTypeDef* htim, DAC_HandleTypeDef* hdac) {
    if (emitter_status == OPAL_EMITTER_BUSY) {
        HAL_TIM_Base_Stop(htim);
        HAL_DAC_Stop_DMA(hdac, DAC_CHANNEL_1);
        emitter_status = OPAL_EMITTER_IDLE;
        //HAL_DAC_SetValue(hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0x000); // Reset DAC to 0V
    }
}