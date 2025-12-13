#include "stm32_opal_receiver.h"

OPAL_Receiver_Handle hrx;

void OPAL_Receiver_Init(ADC_HandleTypeDef* hadc, TIM_HandleTypeDef* htim) {
    hrx.ADC_Handle      = hadc;
    hrx.TIM_Handle      = htim;
    hrx.Status          = OPAL_RECEIVER_IDLE;
    hrx.HLF_CPLT_flag   = false;
    hrx.SKIP_NEXT_flag  = false;
    hrx.DTC_idx         = 0;

    // Setup TIM Frequency
    hrx.TIM_Handle->Instance->ARR = 1;
    hrx.TIM_Handle->Instance->PSC = (uint32_t) ((OPAL_GetAPB1_TimerClockFreq() / OPAL_SAMPLING_FREQ) - 1);
}

OPAL_Status OPAL_Receiver_Start_Sniffing(OPAL_Receiver_Handle* hrx) {
    if (hrx->Status != OPAL_RECEIVER_IDLE)
        return OPAL_ERROR; // Cannot start sniffing if not idle

    if (HAL_ADC_Start_DMA(hrx->ADC_Handle, (uint32_t*) hrx->DMA_ADC_buffer, OPAL_ADC_BUFFER_SIZE) != HAL_OK)
        return OPAL_ADC_ERROR;

    if (HAL_TIM_Base_Start(hrx->TIM_Handle) != HAL_OK)
        return OPAL_TIM_ERROR;

    hrx->Status = OPAL_RECEIVER_SNIFFING;
    hrx->HLF_CPLT_flag = false;
    hrx->DTC_idx = 0;
    return OPAL_SUCCESS;
}

OPAL_Status OPAL_Receiver_Stop_Sniffing(OPAL_Receiver_Handle* hrx) {
    if (hrx->Status != OPAL_RECEIVER_SNIFFING)
        return OPAL_ERROR; // Cannot stop sniffing if not sniffing

    if (HAL_ADC_Stop_DMA(hrx->ADC_Handle) != HAL_OK)
        return OPAL_ADC_ERROR;

    if (HAL_TIM_Base_Stop(hrx->TIM_Handle) != HAL_OK)
        return OPAL_TIM_ERROR;

    hrx->Status = OPAL_RECEIVER_IDLE;
    return OPAL_SUCCESS;
}

OPAL_Status OPAL_Receiver_Decode(OPAL_Receiver_Handle* hrx, OPAL_Frame* frame) {
    if (frame == NULL)
        return OPAL_ERROR_NULL_PTR;

    OPAL_Utils_Symbol_Thresholds thresholds = {
        .lvl0_threshold = (hrx->Frame_buffer[OPAL_FRAME_SOF_INDEX] + hrx->Frame_buffer[OPAL_FRAME_SOF_INDEX + 1]) / 2,
        .lvl1_threshold = (hrx->Frame_buffer[OPAL_FRAME_SOF_INDEX + 1] + hrx->Frame_buffer[OPAL_FRAME_SOF_INDEX + 2]) / 2,
        .lvl2_threshold = (hrx->Frame_buffer[OPAL_FRAME_SOF_INDEX + 2] + hrx->Frame_buffer[OPAL_FRAME_SOF_INDEX + 3]) / 2
    };
    
    // Convert frame buffer voltages to bytes array
    uint8_t frame_bytes[OPAL_FRAME_SIZE] = {};
    size_t symbol_index = 0;
    for (size_t i = 0; i < OPAL_FRAME_SIZE; i++) {
        OPAL_PAM4_symbol s0 = OPAL_voltage_to_symbol(hrx->Frame_buffer[symbol_index + 0], &thresholds);
        OPAL_PAM4_symbol s1 = OPAL_voltage_to_symbol(hrx->Frame_buffer[symbol_index + 1], &thresholds);
        OPAL_PAM4_symbol s2 = OPAL_voltage_to_symbol(hrx->Frame_buffer[symbol_index + 2], &thresholds);
        OPAL_PAM4_symbol s3 = OPAL_voltage_to_symbol(hrx->Frame_buffer[symbol_index + 3], &thresholds);

        frame_bytes[i] = OPAL_pam4_to_byte((OPAL_PAM4_symbol[]){s0, s1, s2, s3}, 0, OPAL_SYMBOLS_PER_BYTE);
        symbol_index += OPAL_SYMBOLS_PER_BYTE;
    }

    // Extract frame fields from bytes array
    OPAL_Frame_Symbols_Conversion(frame_bytes, frame);

    hrx->Status = OPAL_RECEIVER_IDLE; // Return to idle state before restarting sniffing

    // CRC16 verification
    if (frame->CRC16 != OPAL_Frame_Compute_CRC16(frame))
        return OPAL_ERROR_CRC_MISMATCH;

    //OPAL_Receiver_Start_Sniffing(hrx); // Return to sniffing mode
    return OPAL_SUCCESS;
}

bool OPAL_Receiver_Detect_Preamble(OPAL_Receiver_Handle* hrx, uint16_t offset) {
    static OPAL_PAM4_symbol approx_symbols[OPAL_ADC_BUFFER_SIZE] = {}; // Static to avoid reallocation on each call

    // Approximate convertion ADC values to PAM4 symbols (LVL0=00 to LVL3=11)
    for (uint32_t i = 0; i < OPAL_ADC_BUFFER_SIZE; i++)
      approx_symbols[i] = OPAL_voltage_large_threshold(hrx->DMA_ADC_buffer[i]);

    // Search for preamble in the DMA buffer (Sliding Window or Circulary browsing)
    for (size_t i = 0; i < OPAL_ADC_BUFFER_SIZE; i++) {
        uint16_t data = OPAL_pam4_to_byte(approx_symbols, (offset + i + OPAL_SYMBOLS_PER_BYTE), OPAL_ADC_BUFFER_SIZE) << 8 | // MSB (Inverted due to Little Endian)
                        OPAL_pam4_to_byte(approx_symbols, (offset + i), OPAL_ADC_BUFFER_SIZE);                                 // LSB

        if (data == OPAL_FRAME_PREAMBLE) {
            // Save the preamble index for data extraction
            hrx->DTC_idx = i;
            return true;
        }
    }
    return false;
}

void OPAL_Receiver_Buffer_Callback(OPAL_Receiver_Handle* hrx) {
    if (hrx->Status != OPAL_RECEIVER_SNIFFING)
        return; // Ignore callback if not in sniffing mode

    if (hrx->SKIP_NEXT_flag) {
        hrx->SKIP_NEXT_flag = false;
        return; // Ignore next half to avoid re detecting same frame
    }

    hrx->HLF_CPLT_flag = !hrx->HLF_CPLT_flag; // Toggle half-complete flag
    size_t offset = hrx->HLF_CPLT_flag ? (OPAL_ADC_BUFFER_SIZE / 2) : 0; // Offset of the filled half

    if (OPAL_Receiver_Detect_Preamble(hrx, offset)) {
        // Preamble detected
        // Save the current buffer data starting from the preamble
        for (size_t i = 0; i < OPAL_FRAME_BUFFER_SIZE; i++)
            hrx->Frame_buffer[i] = hrx->DMA_ADC_buffer[(offset + hrx->DTC_idx + i) % OPAL_ADC_BUFFER_SIZE];

        hrx->SKIP_NEXT_flag = true;
        OPAL_Receiver_Stop_Sniffing(hrx); // Stop sniffing to decode the frame
        hrx->Status = OPAL_RECEIVER_WAITING_DECODE; // Update status to waiting for decoding
    }
}