#include "stm32_opal_utils.h"
 
size_t OPAL_msb_index(uint16_t data) {
    if (data == 0)
        return 0;

    size_t index = 0;
    while(data >>= 1)
        index++;

    return index;
}