/*
 * spi_handler.c
 *
 *  Created on: Apr 27, 2024
 *      Author: JRE
 */



#include "spi_handler.h"
#include "driver/spi_master.h"
#include "display.h"


#define PIN_NUM_CLK   12	/* SPI Clock pin */
#define PIN_NUM_MOSI  11	/* SPI Master Out, Slave in  - The ESP32 transmits data over this pin */
#define PIN_NUM_MISO  13	/* SPI Master In, Slave Out  - The ESP32 receives data over this pin. */


Public uint8_t spi_handler_init(void)
{
    esp_err_t ret;

    printf("Setting up SPI peripheral\n");

    spi_bus_config_t bus_cfg =
    {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = DISPLAY_MAX_TRANSFER_SIZE,
    };

    ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);

    if (ret != ESP_OK)
    {
        printf("Failed to initialize bus.\n");
        return 0u;
    }

    return 1u;
}
