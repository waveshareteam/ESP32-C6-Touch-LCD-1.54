#ifndef __BSP_SDCARD_H_
#define __BSP_SDCARD_H_
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include <stdio.h>

#define EXAMPLE_PIN_SD_CS GPIO_NUM_17



#ifdef __cplusplus
extern "C" {
#endif

void bsp_sdcard_init(void);
uint64_t bsp_sdcard_get_size(void);

#ifdef __cplusplus
}
#endif


#endif
