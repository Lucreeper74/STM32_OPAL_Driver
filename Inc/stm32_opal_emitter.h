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

typedef struct {
    DAC_HandleTypeDef*              DAC_Handle;     /* !< Associated DAC Handle */
    TIM_HandleTypeDef*              TIM_Handle;     /* !< Associated Timer Handle */
    volatile OPAL_Emitter_Status    Status;         /* !< Emitter Status (Volatile for ITR access) */
    volatile uint16_t               DAC_buffer[OPAL_FRAME_SAMPLES_SIZE+2];  /* !< DAC DMA Buffer (+2 to cycle the TIM for the last symbol) (Volatile for ITR access) */
} OPAL_Emitter_Handle;

extern OPAL_Emitter_Handle htx; // Global OPAL Emitter handle

/*
*   Function to initialize the OPAL Emitter handle
*/
void OPAL_Emitter_Init(DAC_HandleTypeDef* hdac, TIM_HandleTypeDef* htim);

/*
*   Encoding OPAL_Frame to PAM4 Symbols (+ Reset buffers and compute CRC16)
*/
OPAL_Status OPAL_Emitter_Encode(OPAL_Emitter_Handle* htx, OPAL_Frame* frame);

/*
*   Send the frame by enabling the timer and DAC with DMA
*/
OPAL_Status OPAL_Emitter_Send_Frame(OPAL_Emitter_Handle* htx);

/*
*   Callback to handle interruption from DMA at the end of the frame transmission
*/
void OPAL_Emitter_Finished_Callback(DAC_HandleTypeDef* hdac, OPAL_Emitter_Handle* htx);

#endif // __STM32_OPAL_EMITTER_H__