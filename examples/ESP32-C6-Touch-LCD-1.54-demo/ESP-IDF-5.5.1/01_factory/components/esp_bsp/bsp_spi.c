#include "bsp_spi.h"
#include "esp_log.h"

static const char *TAG = "bsp_spi";

void bsp_spi_init(void)
{
    ESP_LOGI(TAG, "SPI BUS init");
    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = EXAMPLE_PIN_SCLK;
    buscfg.mosi_io_num = EXAMPLE_PIN_MOSI;
    buscfg.miso_io_num = EXAMPLE_PIN_MISO;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;

    ESP_ERROR_CHECK(spi_bus_initialize(EXAMPLE_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
}