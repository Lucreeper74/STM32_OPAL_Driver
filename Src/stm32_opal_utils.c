#include "stm32_opal_utils.h"
 
size_t OPAL_msb_index(uint16_t data) {
    if (data == 0)
        return 0;

    size_t index = 0;
    while(data >>= 1)
        index++;

    return index;
}

uint32_t OPAL_GetAPB1_TimerClockFreq() {
    RCC_ClkInitTypeDef clkconfig;
    uint32_t flashLatency;
    HAL_RCC_GetClockConfig(&clkconfig, &flashLatency);

    return clkconfig.APB1CLKDivider == RCC_HCLK_DIV1 ? HAL_RCC_GetPCLK1Freq() : 2 * HAL_RCC_GetPCLK1Freq();
}