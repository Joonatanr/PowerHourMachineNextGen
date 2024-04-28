/*
 * pot.c
 *
 *  Created on: 16. dets 2019
 *      Author: JRE
 */

#include "pot.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "esp_log.h"

/* We control the potentiometer and indicator leds in this module. */

/* Private type definitions */
typedef struct
{
    U32 upper_range;
    U32 lower_range;
} pot_range_T;

/* Private const definitions */

#define HYSTERESIS_VALUE 50u
#define CALLBACK_HYSTERESIS 100u
#define NUMBER_OF_POT_RANGES 4

/* ADC value is 14 bit, so between 0 and 16384 , 4096 per quadrant*/
#define RANGE_1_VALUE 3072u
#define RANGE_2_VALUE 2048u
#define RANGE_3_VALUE 1024u
#define RANGE_4_VALUE 0u

Private const char *TAG = "ADC potentiometers";

Private const pot_range_T priv_pot_ranges[NUMBER_OF_POT_RANGES] =
{
     { .lower_range = RANGE_1_VALUE  + HYSTERESIS_VALUE,  .upper_range = 0x4000u                     },
     { .lower_range = RANGE_2_VALUE  + HYSTERESIS_VALUE,  .upper_range = RANGE_1_VALUE - 1u - HYSTERESIS_VALUE   },
     { .lower_range = RANGE_3_VALUE  + HYSTERESIS_VALUE,  .upper_range = RANGE_2_VALUE - 1u - HYSTERESIS_VALUE   },
     { .lower_range = RANGE_4_VALUE,                      .upper_range = RANGE_3_VALUE - 1u  - HYSTERESIS_VALUE   }
};

Private const adc_channel_t priv_channels[NUMBER_OF_DEFINED_POTENTIOMETERS] =
{
		ADC_CHANNEL_7,
		ADC_CHANNEL_5,
		ADC_CHANNEL_6,
		ADC_CHANNEL_4
};


/* Private variable declarations.*/
Private int curADCResult[NUMBER_OF_DEFINED_POTENTIOMETERS] = { 0u };
Private int currentRange[NUMBER_OF_DEFINED_POTENTIOMETERS] = { 0 };
Private adc_oneshot_unit_handle_t adc2_handle;

Private pot_value_changed_func priv_callback_func = NULL;
Private int priv_pot_callback_values[NUMBER_OF_DEFINED_POTENTIOMETERS];

/* Private function forward declarations. */
Private bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
Private void adc_calibration_deinit(adc_cali_handle_t handle);

Private void storeCallbackPotValues(void);
Private void checkForCallbackRequest(void);

/* Public function definitions. */
Public void pot_init(void)
{
	U8 ix;
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

    for(ix = 0u; ix < NUMBER_OF_DEFINED_POTENTIOMETERS; ix++)
    {
    	adc_calibration_init(ADC_UNIT_2, priv_channels[ix], ADC_ATTEN_DB_11, &adc2_cali_handle);
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, priv_channels[ix], &config));

    }
}


Public void pot_cyclic_100ms(void)
{
    U8 ix;
    U8 pot_ix;
    U16 adc_value;
    int measured_range;

    /* TODO : Maybe spread out the measurements over several cycles? */
    for(ix = 0u; ix < NUMBER_OF_DEFINED_POTENTIOMETERS; ix++)
    {
    	ESP_ERROR_CHECK(adc_oneshot_read(adc2_handle, priv_channels[ix], &curADCResult[ix]));
    }

    for (pot_ix = 0u; pot_ix < NUMBER_OF_DEFINED_POTENTIOMETERS; pot_ix++)
    {
        measured_range = -1;
        adc_value = curADCResult[pot_ix];

        for (ix = 0u; ix < NUMBER_OF_POT_RANGES; ix++)
        {
            if (adc_value >= priv_pot_ranges[ix].lower_range && adc_value <= priv_pot_ranges[ix].upper_range)
            {
                measured_range = (NUMBER_OF_POT_RANGES -1u) - ix;
                break;
            }
        }

        if (measured_range != -1)
        {
            currentRange[pot_ix] = measured_range;
        }
    }

    checkForCallbackRequest();
}


Public int pot_getSelectedRange(potentiometer_T pot)
{
    if(pot < NUMBER_OF_DEFINED_POTENTIOMETERS)
    {
        /* Currently the potentiometers are inverted. Not really a problem to fix it here... */
        return 3 - currentRange[pot];
    }
    else
    {
        return -1;
    }
}


Public int pot_getCurrentMeasurement(potentiometer_T pot)
{
    if(pot < NUMBER_OF_DEFINED_POTENTIOMETERS)
    {
        /* Currently the potentiometers are inverted. Not really a problem to fix it here... */
        return curADCResult[pot];
    }
    else
    {
    	return -1;
    }
}


Public void pot_register_callback(pot_value_changed_func func)
{
	storeCallbackPotValues();
	priv_callback_func = func;
}

Public void pot_unregister_callback(void)
{
	priv_callback_func = NULL;
}


/*---------------------------------------------------------------
        Private functions
---------------------------------------------------------------*/

Private void storeCallbackPotValues(void)
{
	U8 ix;

	for(ix = 0u; ix < NUMBER_OF_POT_RANGES; ix++)
	{
		priv_pot_callback_values[ix] = curADCResult[ix];
	}
}

Private void checkForCallbackRequest(void)
{
	U8 ix;

	for (ix = 0u; ix < NUMBER_OF_POT_RANGES; ix++)
	{
		if (((priv_pot_callback_values[ix] + CALLBACK_HYSTERESIS) < curADCResult[ix])
	      || (priv_pot_callback_values[ix] - CALLBACK_HYSTERESIS) > curADCResult[ix])
		{
			if (priv_callback_func != NULL)
			{
				storeCallbackPotValues();
				priv_callback_func();
				break;
			}
		}
	}
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
