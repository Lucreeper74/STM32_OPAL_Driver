#include "stm32_opal_emitter.h"

static volatile uint8_t symbol_index = 0; // Volatile because can be modified by an ITR
static volatile bool tx_active = false;   // Default state inactive
static OPAL_PAM4_symbol frame_buffer[OPAL_FRAME_SIZE * OPAL_SYMBOLS_PER_BYTE];

OPAL_Status OPAL_Emitter_Encode(OPAL_Frame* frame) {
    if (frame == NULL)
        return OPAL_ERROR_NULL_PTR;

    frame->CRC16 = OPAL_Compute_CRC16(frame);

    uint8_t frame_bytes[OPAL_FRAME_SIZE] = {};
    OPAL_Frame_Byte_Conversion(frame, frame_bytes);

    size_t symbol_index = 0;
    for (size_t i = 0; i < OPAL_FRAME_SIZE; i++) {
        OPAL_byte_to_pam4(frame_bytes[i], &frame_buffer[symbol_index]);
        symbol_index += OPAL_SYMBOLS_PER_BYTE;
    }

    tx_active = false;
    symbol_index = 0;
    
    return OPAL_SUCCESS;
}

OPAL_Status OPAL_Emitter_Send_Frame(TIM_HandleTypeDef* htim) {
    tx_active = true;
    symbol_index = 0;
    HAL_TIM_Base_Start_IT(htim);

    return OPAL_SUCCESS;
}

OPAL_Status OPAL_Emitter_Send_Symbol(DAC_HandleTypeDef* hdac, OPAL_PAM4_symbol symbol) {
    HAL_StatusTypeDef dac_status = HAL_DAC_SetValue(hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, OPAL_symbol_to_voltage(symbol));

    if (dac_status == HAL_ERROR)
        return OPAL_DAC_ERROR;

    return OPAL_SUCCESS;
}

void OPAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim, DAC_HandleTypeDef* hdac) {
    if (tx_active) {
        if (symbol_index < (OPAL_FRAME_SIZE * OPAL_SYMBOLS_PER_BYTE)) {
            OPAL_Emitter_Send_Symbol(hdac, frame_buffer[symbol_index++]);
        } else {
            tx_active = false; // End of transmission
            HAL_TIM_Base_Stop_IT(htim);
            OPAL_Emitter_Send_Symbol(hdac, 0x00); // Reset DAC value
        }
    }
}