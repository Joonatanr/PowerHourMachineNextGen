/*
 * hardware.c
 *
 *  Created on: Apr 25, 2024
 *      Author: JRE
 */

/*****************************************************************************************************
 *
 * Imported definitions
 *
 *****************************************************************************************************/
#include "hardware.h"
#include "buttons.h"

/* TODO : Move ADC includes to ADC handler. */
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "esp_log.h"
#include "esp_timer.h"

/*****************************************************************************************************
 *
 * Private constant definitions
 *
 *****************************************************************************************************/


/*****************************************************************************************************
 *
 * Private variable declarations
 *
 *****************************************************************************************************/
Private adc_oneshot_unit_handle_t adc2_handle;
Private int adc_raw[2][10];

Private const char *TAG = "PH Hardware";
Private int priv_adc_result = 0;


/*****************************************************************************************************
 *
 * Private function forward declarations
 *
 *****************************************************************************************************/

Private bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
Private void adc_calibration_deinit(adc_cali_handle_t handle);

Private void configure_timer(void);
void timer_callback_10msec(void *param);


/*****************************************************************************************************
 *
 * Public function definitions
 *
 *****************************************************************************************************/
Public void hardware_init(void)
{
	configure_timer();

	buttons_init();

	/* TODO : Move this to ADC handler. */
	//-------------ADC2 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config2 =
    {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    adc_oneshot_chan_cfg_t config =
    {
    		.atten = ADC_ATTEN_DB_11,
			.bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc2_handle));

    //-------------ADC2 Calibration Init---------------//
    adc_cali_handle_t adc2_cali_handle = NULL;
    bool do_calibration2 = adc_calibration_init(ADC_UNIT_2, ADC_CHANNEL_4, ADC_ATTEN_DB_11, &adc2_cali_handle);

    //-------------ADC2 Config---------------//
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_4, &config));
}

/* Runs every 100 ms. */
Public void hardware_main(void)
{
    /* TODO : Move to ADC module. */
	ESP_ERROR_CHECK(adc_oneshot_read(adc2_handle, ADC_CHANNEL_4, &adc_raw[1][0]));
    priv_adc_result = adc_raw[1][0];

    /* TODO : Should the buttons 100msec cyclic run from here?? Might want to run that from same thread as the display driver. */
}

/** TODO */
Public uint16_t hardware_get_pot_value(void)
{
	return priv_adc_result;
}


/*****************************************************************************************************
 *
 * Private function definitions
 *
 *****************************************************************************************************/
Private void configure_timer(void)
{
	const esp_timer_create_args_t my_timer_args =
	{
	      .callback = &timer_callback_10msec,
	      .name = "My Timer"
	};

	esp_timer_handle_t timer_handler;

	printf("Configuring timer...\n");

	ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
	ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 10000)); /* Set timer to 10000 microseconds, (10ms) */
}


/* TODO : Here we are going to run the realtime hi prio thread. */
void timer_callback_10msec(void *param)
{
	buttons_cyclic10msec();
}


/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
/* TODO : Do we really need such hardcore ADC handling??? */
Private bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Calibration Success");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}


Private void adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}


