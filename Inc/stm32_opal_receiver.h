#ifndef __STM32_OPAL_RECEIVER_H__
#define __STM32_OPAL_RECEIVER_H__

#include "stm32_opal_utils.h"
#include "stm32_opal_frame.h"

#define OPAL_ADC_BUFFER_SIZE (2 * OPAL_FRAME_BUFFER_SIZE) // Size (in Symbol/Sample) of ADC buffer for DMA (Almost 2x the preamble size to ensure detection)
_Static_assert(OPAL_ADC_BUFFER_SIZE % 2 == 0, "OPAL_ADC_BUFFER_SIZE must be even!");
#define OPAL_ADC_BUFFER_HALF_SIZE (OPAL_ADC_BUFFER_SIZE / 2)

/*
*   Definition of OPAL Receiver status
*/
typedef enum {
    OPAL_RECEIVER_IDLE = 0x0,
    OPAL_RECEIVER_SNIFFING,
    OPAL_RECEIVER_WAITING_DECODE,
    OPAL_RECEIVER_BUSY,
} OPAL_Receiver_Status;

/*
*   Definition of OPAL Receiver handle structure
*/
typedef struct {
    ADC_HandleTypeDef*              ADC_Handle;     /* !< Associated ADC Handle */
    TIM_HandleTypeDef*              TIM_Handle;     /* !< Associated Timer Handle */
    volatile OPAL_Receiver_Status   Status;         /* !< Receiver Status (Volatile for ITR access) */
    volatile bool                   HLF_CPLT_flag;  /* !< Indicates which half of the DMA buffer is filled (Volatile for ITR access) */
    volatile bool                   SKIP_NEXT_flag; /* !< Indicates if the next half of the DMA buffer callback is skipped (Volatile for ITR access) */
    volatile uint16_t               DTC_idx;        /* !< Index of detected preamble in current buffer (Volatile for ITR access) */
    volatile uint16_t               DMA_ADC_buffer[OPAL_ADC_BUFFER_SIZE];   /* !< ADC DMA buffer (Volatile for DMA access) */
    uint16_t                        Frame_buffer[OPAL_FRAME_BUFFER_SIZE];   /* !< Buffer to store the extracted frame samples */
} OPAL_Receiver_Handle;

extern OPAL_Receiver_Handle hrx; // Global OPAL Receiver handle

/*
*   Function to initialize the OPAL Receiver handle
*/
void OPAL_Receiver_Init(ADC_HandleTypeDef* hadc, TIM_HandleTypeDef* htim);

/*
*   Function to start sniffing mode
*/
OPAL_Status OPAL_Receiver_Start_Sniffing(OPAL_Receiver_Handle* hrx);

/*
*   Function to stop sniffing mode
*/
OPAL_Status OPAL_Receiver_Stop_Sniffing(OPAL_Receiver_Handle* hrx);

/*
*   Function to decode a received frame from the frame buffer to OPAL_Frame structure (Return to Sniffing mode after decoding)
*/
OPAL_Status OPAL_Receiver_Decode(OPAL_Receiver_Handle* hrx, OPAL_Frame* frame);

/*
*   Function to detect preamble in the ADC buffer
*/
bool OPAL_Receiver_Detect_Preamble(OPAL_Receiver_Handle* hrx, uint16_t offset);

/*
*   Callback to be called when half or full ADC DMA buffer is filled
*/
void OPAL_Receiver_Buffer_Callback(OPAL_Receiver_Handle* hrx);

#endif // __STM32_OPAL_RECEIVER_H__