#ifndef __STM32_OPAL_FRAME_H__
#define __STM32_OPAL_FRAME_H__

#include "stm32_opal_utils.h"
#include <string.h>

// Definition of known frame Sequences
#define OPAL_FRAME_PREAMBLE     0xCCCC  // => 11 00 11 00 11 00 11 00 - 2 Bytes
#define OPAL_FRAME_START_BYTE   0x1B    // => 00 01 10 11 - 1 Byte
#define OPAL_FRAME_PAYLOAD_SIZE 4       // In Bytes

#define OPAL_FRAME_SIZE (2 + 1 + 1 + OPAL_FRAME_PAYLOAD_SIZE + 2) // In Bytes
#define OPAL_FRAME_SAMPLES_SIZE (OPAL_FRAME_SIZE * OPAL_SYMBOLS_PER_BYTE) // In Symbols/Samples

#define OPAL_FRAME_PREAMBLE_SIZE (sizeof(uint16_t) * OPAL_SYMBOLS_PER_BYTE) // Size of the Preamble in symbols (also Index of Start of Frame in the frame buffer)

/*
*   Represent the structure of a Frame transmitted via OPAL protocol
*/
typedef struct {
    uint16_t Preamble;     /* !< Init the frame (Known sequence) */
    uint8_t  StartFrame;   /* !< Define the start of the frame (Known sequence) */ 
    uint8_t  DataType;     /* !< Define the type of the data transmitted */
    uint8_t  Data[OPAL_FRAME_PAYLOAD_SIZE]; /* !< Define the data transmitted */
    uint16_t CRC16;        /* !< Used to verify the frame integrity */
} OPAL_Frame;

/*
*   Represent the interpreted payload data transmitted in a frame
*/
typedef union {
    char    var_str[OPAL_FRAME_PAYLOAD_SIZE / sizeof(char)];
    int     var_int[OPAL_FRAME_PAYLOAD_SIZE / sizeof(int)];
    float   var_float[OPAL_FRAME_PAYLOAD_SIZE / sizeof(float)];
    uint8_t var_bin[OPAL_FRAME_PAYLOAD_SIZE];
} OPAL_Data;

/*
*   Represent the datatype containtend into a frame
*/
typedef enum {
    TYPE_BIN    = 0x00,
    TYPE_STRING = 0x01,
    TYPE_INT    = 0x02,
    TYPE_FLOAT  = 0x03,
} OPAL_DataType;

/*
*   Definition of an OPAL frame for tests & debugging purposes
*/
static const OPAL_Frame OPAL_TestFrame = {
        .Preamble     = OPAL_FRAME_PREAMBLE,                     // 0xCCCC = 52428
        .StartFrame   = OPAL_FRAME_START_BYTE,                   // 0x1B = 27
        .DataType     = TYPE_INT,                                // 0x02 = 2
        .Data         = {0xAC, 0xF7, 0x89, 0x7B} // 172, 247, 137, 123
        // .CRC16 will be computed dynamically
};


/*
*   Compute the CRC16 for an OPAL_Frame
*/
uint16_t OPAL_Frame_Compute_CRC16(const OPAL_Frame* frame);

/*
*   Conversion between OPAL_Frame struct to a bytes array (in Big-endians)
*/
void OPAL_Frame_Bytes_Conversion(const OPAL_Frame* frame, uint8_t* frame_bytes);

/*
*   Conversion between bytes array to OPAL_Frame struct (in Big-endians)
*/
void OPAL_Frame_Symbols_Conversion(const uint8_t* frame_bytes, OPAL_Frame* frame);

/*
*   Compute the Hamming Distance between two OPAL frames
*/
size_t OPAL_Frame_getHammingDistance(const OPAL_Frame* frame1, const OPAL_Frame* frame2);

#endif // __STM32_OPAL_FRAME_H__