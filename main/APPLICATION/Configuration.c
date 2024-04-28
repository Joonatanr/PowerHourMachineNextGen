/*
 * Configuration.c
 *
 *  Created on: 27 Dec 2023
 *      Author: Joonatan
 */


#include "Configuration.h"
#include "nvs_flash.h"
#include "nvs.h"

/***************************** Private definitions*************************************************/


#define CONFIG_START 0x0003F000
#define CONFIG_SIZE 32u /* Should be more than enough. */
#define STORAGE_NAMESPACE "powerhour_conf"

typedef struct
{
    U8 task_frequency;
    U8 selected_color_scheme;
    U8 brightness;
    U8 buzzer;

    /* TODO : Add more items, at the very least custom color schemes etc. */
    U8 unused[CONFIG_SIZE - 4u];
} Configuration_T;

typedef union
{
    Configuration_T conf;
    U8 data[CONFIG_SIZE];
} Configuration_Data_T;

/***************************** Private function forward declarations ******************************/

Private esp_err_t loadConfig(void);
Private Boolean verifyConfig(void);
Private esp_err_t storeConfig(void);


/***************************** Private variables **************************************************/

Private Configuration_Data_T priv_conf;

/***************************** Public functions  **************************************************/
Public void configuration_start(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    /* Load the configuration data from flash memory. */
    if(loadConfig() != ESP_OK)
    {
    	printf("Failed to load configuration\n");
    }
    else
    {
    	printf("Config loaded successfully\n");
    }

    if (verifyConfig() == TRUE)
    {
    	storeConfig();
    }
}

Public U32 configuration_getItem(Configuration_Item item)
{
    switch(item)
    {
        case CONFIG_ITEM_TASK_FREQ:
            return priv_conf.conf.task_frequency;
        case CONFIG_ITEM_COLOR_SCHEME:
            return priv_conf.conf.selected_color_scheme;
        case CONFIG_ITEM_BRIGHTNESS:
            return priv_conf.conf.brightness;
        case CONFIG_ITEM_BUZZER:
            return priv_conf.conf.buzzer;
        default:
            return 0u;
    }
}

Public void configuration_setItem(U32 value, Configuration_Item item)
{
    Boolean isDirty = FALSE;

    switch(item)
    {
        case CONFIG_ITEM_TASK_FREQ:
            if (priv_conf.conf.task_frequency != value)
            {
                priv_conf.conf.task_frequency = value;
                isDirty = TRUE;
            }
            break;
        case CONFIG_ITEM_COLOR_SCHEME:
            if (priv_conf.conf.selected_color_scheme != value)
            {
                priv_conf.conf.selected_color_scheme = value;
                isDirty = TRUE;
            }
            break;
        case CONFIG_ITEM_BRIGHTNESS:
            if (priv_conf.conf.brightness != value)
            {
                priv_conf.conf.brightness = value;
                isDirty = TRUE;
            }
            break;
        case CONFIG_ITEM_BUZZER:
            if (priv_conf.conf.buzzer != value)
            {
                priv_conf.conf.buzzer = value;
                isDirty = TRUE;
            }
            break;
        default:
            break;
    }

    if (isDirty)
    {
        storeConfig();
    }
}


/***************************** Private functions  **************************************************/

Private Boolean verifyConfig(void)
{
    /* Returns true if we need to modify the config (because of incorrect values etc. ) */
    Boolean res = FALSE;

    if (priv_conf.conf.task_frequency <= 1u || priv_conf.conf.task_frequency > 6u)
    {
        /* Revert to default. */
        priv_conf.conf.task_frequency = 3u;
        res = TRUE;
    }

    if (priv_conf.conf.selected_color_scheme > 3u)
    {
        priv_conf.conf.selected_color_scheme = 0u;
        res = TRUE;
    }

    if (priv_conf.conf.brightness > 100u || (priv_conf.conf.brightness < 10))
    {
        priv_conf.conf.brightness = 60u;
        res = TRUE;
    }

    if (priv_conf.conf.buzzer > 1u)
    {
        priv_conf.conf.buzzer = 1u;
        res = TRUE;
    }

    return res;
}

Private esp_err_t loadConfig(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
    	printf("nvs open failed\n");
    	return err;
    }

    /* Lets do this in a stupid way for now. Read out each variable separately... */
    err = nvs_get_u8(my_handle, "task_frequency", &priv_conf.conf.task_frequency);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    err = nvs_get_u8(my_handle, "color_scheme", &priv_conf.conf.selected_color_scheme);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    err = nvs_get_u8(my_handle, "brightness", &priv_conf.conf.brightness);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    err = nvs_get_u8(my_handle, "buzzer", &priv_conf.conf.buzzer);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    // Close
    nvs_close(my_handle);

    return ESP_OK;
}


Private esp_err_t storeConfig(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
    	return err;
    }

    /* Lets do this in a stupid way for now. Write out each variable separately... */
    err = nvs_set_u8(my_handle, "task_frequency", priv_conf.conf.task_frequency);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    err = nvs_set_u8(my_handle, "color_scheme", priv_conf.conf.selected_color_scheme);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    err = nvs_set_u8(my_handle, "brightness", priv_conf.conf.brightness);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    err = nvs_set_u8(my_handle, "buzzer", priv_conf.conf.buzzer);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    // Close
    nvs_close(my_handle);

	return ESP_OK;
}

