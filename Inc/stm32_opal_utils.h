#ifndef __STM32_OPAL_UTILS_H__
#define __STM32_OPAL_UTILS_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(STM32F3xx)
#include <stm32f3xx_hal.h>
#elif defined(STM32L0xx)
#include <stm32l0xx_hal.h>
#endif

/*
*   PAM4 voltage levels definition (For 12 bits DAC)
*/
#define OPAL_PAM4_VOLTAGE_LEVEL_0 0x447 // => 1095
#define OPAL_PAM4_VOLTAGE_LEVEL_1 0x82F // => 2095
#define OPAL_PAM4_VOLTAGE_LEVEL_2 0xC17 // => 3095
#define OPAL_PAM4_VOLTAGE_LEVEL_3 0xFFF // => 4095

#define OPAL_PAM4_VOLTAGE_DEADZONE 500 // Dead zone for voltage level detection

#define OPAL_SYMBOLS_PER_BYTE 4 // Each symbol is 2 bits so 4 symbols/byte

#define OPAL_CRC16_DEFAULT   0xFFFF // Default value of the CRC16
#define OPAL_CRC16_GENERATOR 0x8005 // The polynomial generator used for CRC16

#define OPAL_TRANSMISSION_FREQ 1000 // In Hz

/*
*   PAM4 Binary levels definition
*/
typedef enum {
    OPAL_PAM4_LEVEL_0 = 0b00,
    OPAL_PAM4_LEVEL_1 = 0b01,
    OPAL_PAM4_LEVEL_2 = 0b10,
    OPAL_PAM4_LEVEL_3 = 0b11
} OPAL_PAM4_symbol;

/*
*   Definition of OPAL error codes
*/
typedef enum {
    OPAL_SUCCESS = 0x0,
    OPAL_ERROR_MEMORY,
    OPAL_ERROR_NULL_PTR,
    OPAL_ERROR_INVALID_FRAME,
    OPAL_ERROR_UNKNOWN_DATA_TYPE,
    OPAL_ERROR_CRC_MISMATCH,
    OPAL_DAC_ERROR,
    OPAL_ADC_ERROR,
    OPAL_TIM_ERROR,
    OPAL_ERROR
} OPAL_Status;

/*
*   Definition of OPAL Utils symbol thresholds
*/
typedef struct {
    uint16_t lvl0_threshold; /* !< Threshold between LV0 and LV1 */
    uint16_t lvl1_threshold; /* !< Threshold between LV1 and LV2 */
    uint16_t lvl2_threshold; /* !< Threshold between LV3 and LV4 */
} OPAL_Utils_Symbol_Thresholds;

/*
*   Conversion functions between PAM4 symbols and bits/bytes
*/
static inline uint8_t OPAL_pam4_to_bits(OPAL_PAM4_symbol symbol) {
    return (uint8_t) (symbol & 0x03);
}

/*
*   Convert 4 PAM4 symbols to a byte (Big-endian)
*/
static inline OPAL_PAM4_symbol OPAL_bits_to_pam4(uint8_t bits) {
    return (OPAL_PAM4_symbol) (bits & 0x03);
}

/*
*   Convert 4 PAM4 symbols to a byte (Big-endian)
*/
static inline uint8_t OPAL_pam4_to_byte(const OPAL_PAM4_symbol symbols[4], size_t base, size_t size) {
    // To prevent modulo uses in critical loops (which is costly)
    size_t i0 = base + 0; if (i0 >= size) i0 -= size;
    size_t i1 = base + 1; if (i1 >= size) i1 -= size;
    size_t i2 = base + 2; if (i2 >= size) i2 -= size;
    size_t i3 = base + 3; if (i3 >= size) i3 -= size;

    return OPAL_pam4_to_bits(symbols[i0]) << 6 |
           OPAL_pam4_to_bits(symbols[i1]) << 4 |
           OPAL_pam4_to_bits(symbols[i2]) << 2 |
           OPAL_pam4_to_bits(symbols[i3]);
}

/*
*   Convert a byte to 4 PAM4 symbols (Big-endian)
*/
static inline void OPAL_byte_to_pam4(uint8_t byte, OPAL_PAM4_symbol symbols[4]) {
    symbols[0] = OPAL_bits_to_pam4(byte >> 6);
    symbols[1] = OPAL_bits_to_pam4(byte >> 4);
    symbols[2] = OPAL_bits_to_pam4(byte >> 2);
    symbols[3] = OPAL_bits_to_pam4(byte);
}

/*
*   Convert a PAM4 symbol to its corresponding voltage level
*/
static inline uint16_t OPAL_symbol_to_voltage(OPAL_PAM4_symbol symbol) {
    switch (symbol) {
        case OPAL_PAM4_LEVEL_0: return OPAL_PAM4_VOLTAGE_LEVEL_0;
        case OPAL_PAM4_LEVEL_1: return OPAL_PAM4_VOLTAGE_LEVEL_1;
        case OPAL_PAM4_LEVEL_2: return OPAL_PAM4_VOLTAGE_LEVEL_2;
        case OPAL_PAM4_LEVEL_3: return OPAL_PAM4_VOLTAGE_LEVEL_3;
        default: return OPAL_PAM4_LEVEL_0;
    }
}

static inline OPAL_PAM4_symbol OPAL_voltage_to_symbol(uint16_t voltage, const OPAL_Utils_Symbol_Thresholds* thresholds) {
    if (voltage < thresholds->lvl0_threshold)
        return OPAL_PAM4_LEVEL_0;
    else if (voltage < thresholds->lvl1_threshold)
        return OPAL_PAM4_LEVEL_1;
    else if (voltage < thresholds->lvl2_threshold)
        return OPAL_PAM4_LEVEL_2;
    else
        return OPAL_PAM4_LEVEL_3;
}

/*
*   Approximate roughly from a voltage value the corresponding PAM4 symbol between LEVEL_0 and LEVEL_3
*/
static inline OPAL_PAM4_symbol OPAL_voltage_large_threshold(uint16_t voltage) {
    if (voltage > (OPAL_PAM4_VOLTAGE_LEVEL_2 - OPAL_PAM4_VOLTAGE_DEADZONE))
        return OPAL_PAM4_LEVEL_3;
    else
        return OPAL_PAM4_LEVEL_0;
}

/*
*   Get the index of the Most Significant Bit in a 16 bits variable
*/
size_t OPAL_msb_index(uint16_t data);

uint32_t OPAL_GetAPB1_TimerClockFreq();

#endif // __STM32_OPAL_UTILS_H__