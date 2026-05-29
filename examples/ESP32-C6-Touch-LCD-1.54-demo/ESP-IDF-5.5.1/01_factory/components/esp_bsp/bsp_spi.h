#ifndef __BSP_SPI_H__
#define __BSP_SPI_H__

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"

#define EXAMPLE_SPI_HOST SPI2_HOST

#define EXAMPLE_PIN_MISO GPIO_NUM_16
#define EXAMPLE_PIN_MOSI GPIO_NUM_2
#define EXAMPLE_PIN_SCLK GPIO_NUM_1



#ifdef __cplusplus
extern "C" {
#endif

void bsp_spi_init(void);


#ifdef __cplusplus
}
#endif

#endif