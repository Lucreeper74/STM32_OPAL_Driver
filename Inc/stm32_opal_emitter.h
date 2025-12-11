#ifndef __STM32_OPAL_EMITTER_H__
#define __STM32_OPAL_EMITTER_H__

#include "stm32_opal_utils.h"
#include "stm32_opal_frame.h"

/*
*   Definition of OPAL Emitter status
*/
typedef enum {
    OPAL_EMITTER_IDLE = 0x0,
    OPAL_EMITTER_BUSY,
} OPAL_Emitter_Status;

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
void OPAL_Emitter_Finished_Callback(TIM_HandleTypeDef* htim, DAC_HandleTypeDef* hdac);

#endif // __STM32_OPAL_EMITTER_H__