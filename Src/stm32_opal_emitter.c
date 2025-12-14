#include "stm32_opal_emitter.h"

OPAL_Emitter_Handle htx;

void OPAL_Emitter_Init(DAC_HandleTypeDef* hdac, TIM_HandleTypeDef* htim) {
    htx.DAC_Handle      = hdac;
    htx.TIM_Handle      = htim;
    htx.Status          = OPAL_EMITTER_IDLE;

    // Setup TIM Frequency
    htx.TIM_Handle->Instance->PSC = 0; // Cannot be ARR = 0, it won't trigger the ADC
    htx.TIM_Handle->Instance->ARR = (uint32_t) ((OPAL_GetAPB1_TimerClockFreq() / OPAL_TRANSMISSION_FREQ) - 1);
}

OPAL_Status OPAL_Emitter_Encode(OPAL_Emitter_Handle* htx, OPAL_Frame* frame) {
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
        htx->DAC_buffer[i] = OPAL_symbol_to_voltage(frame_symbols[i]);

    // Update emitter status
    htx->Status = OPAL_EMITTER_IDLE;
    
    return OPAL_SUCCESS;
}

OPAL_Status OPAL_Emitter_Send_Frame(OPAL_Emitter_Handle* htx) {
    // Load first value to handle undefined state at start
    HAL_DAC_SetValue(htx->DAC_Handle, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0x000);

    if (HAL_TIM_Base_Start(htx->TIM_Handle) != HAL_OK)
        return OPAL_TIM_ERROR;

    if (HAL_DAC_Start_DMA(htx->DAC_Handle, DAC_CHANNEL_1, (uint32_t*)htx->DAC_buffer, OPAL_FRAME_BUFFER_SIZE+2, DAC_ALIGN_12B_R) != HAL_OK)
        return OPAL_DAC_ERROR;

    htx->Status = OPAL_EMITTER_BUSY;

    return OPAL_SUCCESS;
}

void OPAL_Emitter_Finished_Callback(OPAL_Emitter_Handle* htx) {
    if (htx->Status == OPAL_EMITTER_BUSY) {
        HAL_TIM_Base_Stop(htx->TIM_Handle);
        HAL_DAC_Stop_DMA(htx->DAC_Handle, DAC_CHANNEL_1);
        htx->Status = OPAL_EMITTER_IDLE;
        //HAL_DAC_SetValue(hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0x000); // Reset DAC to 0V
    }
}