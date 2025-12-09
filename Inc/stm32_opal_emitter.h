#ifndef __STM32_OPAL_EMITTER_H__
#define __STM32_OPAL_EMITTER_H__

#include "stm32_opal_utils.h"
#include "stm32_opal_frame.h"
#include "stm32l0xx_hal_tim.h"

/*
*   Encoding OPAL_Frame to PAM4 Symbols (+ Reset buffers and compute CRC16)
*/
OPAL_Status OPAL_Emitter_Encode(OPAL_Frame* frame);

/*
*   Send the frame by enabling the timer
*/
OPAL_Status OPAL_Emitter_Send_Frame(TIM_HandleTypeDef* htim);

/*
*   Drive the DAC to send PAM4 symbol levels
*/
OPAL_Status OPAL_Emitter_Send_Symbol(DAC_HandleTypeDef* hdac, OPAL_PAM4_symbol symbol);

/*
*   Callback to handle interruption from TIMER for frame transmission
*/
void OPAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim, DAC_HandleTypeDef* hdac);

#endif // __STM32_OPAL_EMITTER_H__