/*
 * sdCard.c
 *
 *  Created on: Mar 13, 2024
 *      Author: JRE
 */
#include <stdio.h>
#include <string.h>

#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_vfs_fat.h"

#include "sdCardHandler.h"
#include "display.h"

#define MOUNT_POINT "/sdcard"
#define PIN_NUM_SDCARD_CS    7


/****************** Private type definitions *******************/

#pragma pack(push)  // save the original data alignment
#pragma pack(1)     // Set data alignment to 1 byte boundary
typedef struct
{
    uint16_t type;              // Magic identifier: 0x4d42
    uint32_t size;              // File size in bytes
    uint16_t reserved1;         // Not used
    uint16_t reserved2;         // Not used
    uint32_t offset;            // Offset to image data in bytes from beginning of file
    uint32_t dib_header_size;   // DIB Header size in bytes
    int32_t  width_px;          // Width of the image
    int32_t  height_px;         // Height of image
    uint16_t num_planes;        // Number of color planes
    uint16_t bits_per_pixel;    // Bits per pixel
    uint32_t compression;       // Compression type
    uint32_t image_size_bytes;  // Image size in bytes
    int32_t  x_resolution_ppm;  // Pixels per meter
    int32_t  y_resolution_ppm;  // Pixels per meter
    uint32_t num_colors;        // Number of colors
    uint32_t important_colors;  // Important colors
} BMPHeader;
#pragma pack(pop)  // restore the previous pack setting


/**************** Private function forward declarations **************/
static esp_err_t read_bmp_file(const char *path, uint16_t * output_buffer);
static const char *TAG = "SD Card Handler";

/**************** Private variable declarations ******************/

uint8_t  bmp_line_buffer[(MAX_BMP_LINE_LENGTH * 3) + 4u];

/**************** Public functions  **************/
void sdCard_init(void)
{
	esp_err_t ret;

	// Options for mounting the filesystem.
    esp_vfs_fat_sdmmc_mount_config_t mount_config =
    {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;

    ESP_LOGI(TAG, "Initializing SD card");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    host.slot = SPI2_HOST;
    host.max_freq_khz = 4000;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_SDCARD_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    ESP_LOGI(TAG, "Filesystem mounted");
}


void sdCard_Read_bmp_file(const char *path, uint16_t * output_buffer)
{
	char str[64] = MOUNT_POINT;
	strcat(str, path);

	read_bmp_file(str, output_buffer);
}

/*********** Private functions ***********/


static esp_err_t read_bmp_file(const char *path, uint16_t * output_buffer)
{
	BMPHeader header;
	FILE *f;
	uint16_t line_stride;
	uint16_t line_px_data_len;
	uint16_t * dest_ptr = output_buffer;

	ESP_LOGI(TAG, "Reading file %s", path);
    f = fopen(path, "r");

    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }

    fread(&header, sizeof(BMPHeader), 1u, f);

    ESP_LOGE(TAG, "Bitmap width : %ld", header.width_px);
    ESP_LOGE(TAG, "Bitmap height : %ld", header.height_px);

    /* Take padding into account... */
    line_px_data_len = header.width_px * 3u;
    line_stride = (line_px_data_len + 3u) & ~0x03;

    for (int y = 0u; y < header.height_px; y++)
    {
    	fseek(f, ((header.height_px - (y + 1)) * line_stride) + header.offset, SEEK_SET);
    	fread(bmp_line_buffer, sizeof(uint8_t), line_stride, f);

      for (int x = 0u; x < line_px_data_len; x+=3u )
      {
        *dest_ptr++ = CONVERT_888RGB_TO_565RGB(bmp_line_buffer[x + 2], bmp_line_buffer[x+1], bmp_line_buffer[x]);
      }
    }

    fclose(f);

    return ESP_OK;
}
