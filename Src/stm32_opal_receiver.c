#include "stm32_opal_receiver.h"

OPAL_Receiver_Handle hrx;

void OPAL_Receiver_Init(ADC_HandleTypeDef* hadc, TIM_HandleTypeDef* htim) {
    hrx.ADC_Handle      = hadc;
    hrx.TIM_Handle      = htim;
    hrx.Status          = OPAL_RECEIVER_IDLE;
    hrx.Buffer_offset   = 0;
    hrx.DTC_idx         = 0;
    hrx.HLF_CPLT_flag   = false;
    hrx.SKIP_NEXT_flag  = false;

    // Setup TIM Frequency
    hrx.TIM_Handle->Instance->PSC = 0; // Cannot be ARR = 0, it won't trigger the ADC
    hrx.TIM_Handle->Instance->ARR = (uint32_t) ((OPAL_GetAPB1_TimerClockFreq() / OPAL_SAMPLING_FREQ) - 1);
}

OPAL_Status OPAL_Receiver_Start_Sniffing(OPAL_Receiver_Handle* hrx) {
    if (hrx->Status != OPAL_RECEIVER_IDLE)
        return OPAL_ERROR; // Cannot start sniffing if not idle

    if (HAL_ADC_Start_DMA(hrx->ADC_Handle, (uint32_t*) hrx->DMA_ADC_buffer, OPAL_ADC_BUFFER_SIZE) != HAL_OK)
        return OPAL_ADC_ERROR;

    if (HAL_TIM_Base_Start(hrx->TIM_Handle) != HAL_OK)
        return OPAL_TIM_ERROR;
    
    hrx->Status = OPAL_RECEIVER_SNIFFING;
    hrx->DTC_idx = 0;
    return OPAL_SUCCESS;
}

OPAL_Status OPAL_Receiver_Stop_Sniffing(OPAL_Receiver_Handle* hrx) {
    if (HAL_ADC_Stop_DMA(hrx->ADC_Handle) != HAL_OK)
        return OPAL_ADC_ERROR;

    if (HAL_TIM_Base_Stop(hrx->TIM_Handle) != HAL_OK)
        return OPAL_TIM_ERROR;

    if (hrx->Status == OPAL_RECEIVER_SNIFFING)
        hrx->Status = OPAL_RECEIVER_IDLE; // Set to idle state if sniffing

    return OPAL_SUCCESS;
}

OPAL_Status OPAL_Receiver_Decode(OPAL_Receiver_Handle* hrx, OPAL_Frame* frame) {
    if (frame == NULL)
        return OPAL_ERROR_NULL_PTR;

    OPAL_Utils_Symbol_Thresholds thresholds = {
        .lvl0_threshold = (hrx->Frame_buffer[OPAL_FRAME_PREAMBLE_SIZE] + hrx->Frame_buffer[OPAL_FRAME_PREAMBLE_SIZE + 1]) / 2,
        .lvl1_threshold = (hrx->Frame_buffer[OPAL_FRAME_PREAMBLE_SIZE + 1] + hrx->Frame_buffer[OPAL_FRAME_PREAMBLE_SIZE + 2]) / 2,
        .lvl2_threshold = (hrx->Frame_buffer[OPAL_FRAME_PREAMBLE_SIZE + 2] + hrx->Frame_buffer[OPAL_FRAME_PREAMBLE_SIZE + 3]) / 2
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
    static const uint16_t preamble_pattern[OPAL_FRAME_PREAMBLE_SIZE] = {
        OPAL_PAM4_VOLTAGE_LEVEL_3, OPAL_PAM4_VOLTAGE_LEVEL_0, OPAL_PAM4_VOLTAGE_LEVEL_3, OPAL_PAM4_VOLTAGE_LEVEL_0,
        OPAL_PAM4_VOLTAGE_LEVEL_3, OPAL_PAM4_VOLTAGE_LEVEL_0, OPAL_PAM4_VOLTAGE_LEVEL_3, OPAL_PAM4_VOLTAGE_LEVEL_0
    }; // Static to avoid reallocation on each call

    int32_t best_correlation = INT32_MIN;
    size_t best_index = 0;

    // Search for preamble position in the DMA buffer (Sliding Window or Circulary browsing)
    for (size_t i = 0; i < (OPAL_ADC_BUFFER_SIZE - (OPAL_FRAME_PREAMBLE_SIZE * OPAL_OVERSAMPLING_FACTOR)); i++) {
        int32_t correlation = 0;

        // Compute correlation for each buffer position (to find the preamble + optimal phase)
        for (size_t j = 0; j < OPAL_FRAME_PREAMBLE_SIZE; j++) {
            int32_t diff = (int32_t) hrx->DMA_ADC_buffer[(offset + i + j*OPAL_OVERSAMPLING_FACTOR) % OPAL_ADC_BUFFER_SIZE] - preamble_pattern[j];
            correlation -= diff * diff; // Quadratique error

            if (correlation < best_correlation)
                break; // This index is too bad, go to the next one
        }

        if (correlation > best_correlation) {
            best_correlation = correlation;
            best_index = i;
        }
    }

    #define CORRELATION_THRESHOLD -5000000

    if (best_correlation > CORRELATION_THRESHOLD) {
        hrx->DTC_idx = best_index;
        return true;
    }

    return false;
}

OPAL_Status OPAL_Receiver_Process(OPAL_Receiver_Handle* hrx) {
    switch (hrx->Status) {
        case OPAL_RECEIVER_SNIFFING: {
            if (hrx->HLF_CPLT_flag) {
                hrx->HLF_CPLT_flag = false; // Reset the half-complete flag

                if (OPAL_Receiver_Detect_Preamble(hrx, hrx->Buffer_offset)) {
                    // Preamble found in the current buffer

                    if (hrx->DTC_idx >= OPAL_ADC_BUFFER_HALF_SIZE) // If the frame isn't fully contained in the current buffer
                        break; // Wait for the next buffer half ITR to catch the full frame

                    hrx->Status = OPAL_RECEIVER_RECEIVING; // Update status to receiving
                }
            } 
            break;
        }
        case OPAL_RECEIVER_RECEIVING: {
                // Save the current buffer data starting from the preamble
                for (size_t i = 0; i < OPAL_FRAME_BUFFER_SIZE; i++)
                    hrx->Frame_buffer[i] = hrx->DMA_ADC_buffer[(hrx->Buffer_offset + hrx->DTC_idx + i * OPAL_OVERSAMPLING_FACTOR) % OPAL_ADC_BUFFER_SIZE];

                OPAL_Receiver_Stop_Sniffing(hrx); // Stop receiving data to decode the frame

                hrx->SKIP_NEXT_flag = true;
                hrx->Status = OPAL_RECEIVER_WAITING_DECODE; // Update status to waiting for decoding
            break;
        }

        default:
            break;
    }

    return OPAL_SUCCESS;
}

void OPAL_Receiver_Buffer_Callback(OPAL_Receiver_Handle* hrx) {
    if (hrx->SKIP_NEXT_flag)
        hrx->SKIP_NEXT_flag = false; // Skip the current half-complete DMA buffer
    else
        hrx->HLF_CPLT_flag = true; // Set half-complete flag

    hrx->Buffer_offset = hrx->HLF_CPLT_flag ? (OPAL_ADC_BUFFER_SIZE / 2) : 0; // Offset of the filled half
}