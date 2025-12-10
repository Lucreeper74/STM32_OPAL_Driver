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

#define OPAL_SYMBOLS_PER_BYTE 4 // Each symbol is 2 bits so 4 symbols/byte

#define OPAL_CRC16_DEFAULT   0xFFFF // Default value of the CRC16
#define OPAL_CRC16_GENERATOR 0x8005 // The polynomial generator used for CRC16

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
    OPAL_DAC_ERROR
} OPAL_Status;

static inline uint8_t OPAL_pam4_to_bits(OPAL_PAM4_symbol symbol) {
    return (uint8_t) (symbol & 0x03);
}

static inline OPAL_PAM4_symbol OPAL_bits_to_pam4(uint8_t bits) {
    return (OPAL_PAM4_symbol) (bits & 0x03);
}

static inline uint8_t OPAL_pam4_to_byte(const OPAL_PAM4_symbol symbols[4]) {
    return OPAL_pam4_to_bits(symbols[0]) << 6 |
           OPAL_pam4_to_bits(symbols[1]) << 4 |
           OPAL_pam4_to_bits(symbols[2]) << 2 |
           OPAL_pam4_to_bits(symbols[3]);
}

static inline void OPAL_byte_to_pam4(uint8_t byte, OPAL_PAM4_symbol symbols[4]) {
    symbols[0] = OPAL_bits_to_pam4(byte >> 6);
    symbols[1] = OPAL_bits_to_pam4(byte >> 4);
    symbols[2] = OPAL_bits_to_pam4(byte >> 2);
    symbols[3] = OPAL_bits_to_pam4(byte);
}

static inline uint16_t OPAL_symbol_to_voltage(OPAL_PAM4_symbol symbol) {
    switch (symbol) {
        case OPAL_PAM4_LEVEL_0: return OPAL_PAM4_VOLTAGE_LEVEL_0;
        case OPAL_PAM4_LEVEL_1: return OPAL_PAM4_VOLTAGE_LEVEL_1;
        case OPAL_PAM4_LEVEL_2: return OPAL_PAM4_VOLTAGE_LEVEL_2;
        case OPAL_PAM4_LEVEL_3: return OPAL_PAM4_VOLTAGE_LEVEL_3;
        default: return OPAL_PAM4_LEVEL_0;
    }
}

/*
*   Get the index of the Most Significant Bit in a 16 bits variable
*/
size_t OPAL_msb_index(uint16_t data);

#endif // __STM32_OPAL_UTILS_H__