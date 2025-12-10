#ifndef __STM32_OPAL_EMITTER_H__
#define __STM32_OPAL_EMITTER_H__

#include "stm32_opal_utils.h"
#include "stm32_opal_frame.h"

/*
*   Encoding OPAL_Frame to PAM4 Symbols (+ Reset buffers and compute CRC16)
*/
OPAL_Status OPAL_Emitter_Encode(OPAL_Frame* frame);

/*
*   Send the frame by enabling the timer and DAC with DMA
*/
OPAL_Status OPAL_Emitter_Send_Frame(TIM_HandleTypeDef* htim, DAC_HandleTypeDef* hdac);

/*
*   Callback to handle interruption from DMA at the end of the frame transmission
*/
void OPAL_Transmission_Finished_Callback(TIM_HandleTypeDef* htim, DAC_HandleTypeDef* hdac);

#endif // __STM32_OPAL_EMITTER_H__