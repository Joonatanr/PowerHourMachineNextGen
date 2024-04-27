/*
 * display.c
 *
 *  Created on: 7 Mar 2024
 *      Author: Joonatan
 */

/*
**====================================================================================
** Imported definitions
**====================================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "display.h"

/*
**====================================================================================
** Private constant definitions
**====================================================================================
*/

#define LCD_HOST    SPI2_HOST

#define PIN_NUM_DC         5
#define PIN_NUM_RST        3
#define PIN_NUM_DISPLAY_CS 4
#define PIN_NUM_BCKL       2

/*
**====================================================================================
** Private type definitions
**====================================================================================
*/

typedef struct
{
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/*
**====================================================================================
** Private function forward declaration
**====================================================================================
*/

static void lcd_spi_pre_transfer_callback(spi_transaction_t *t);
static void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd, bool keep_cs_active);
static void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len);
static void lcd_init(spi_device_handle_t spi);
static void send_display_data(spi_device_handle_t spi, int xPos, int yPos, int width, int height, uint16_t *linedata, bool isBufferConstant);
static void wait_display_data_finish(spi_device_handle_t spi);


/*
**====================================================================================
** Private variable declarations
**====================================================================================
*/
//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]=
{
    /* Memory Data Access Control, MX=MV=1, MY=ML=MH=0, RGB=0 */
    {0x36, {(1 << 7)| (1<<5) }, 1},
    /* Interface Pixel Format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Porch Setting */
    {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
    /* Gate Control, Vgh=13.65V, Vgl=-10.43V */
    {0xB7, {0x45}, 1},
    /* VCOM Setting, VCOM=1.175V */
    {0xBB, {0x2B}, 1},
    /* LCM Control, XOR: BGR, MX, MH */
    {0xC0, {0x2C}, 1},
    /* VDV and VRH Command Enable, enable=1 */
    {0xC2, {0x01, 0xff}, 2},
    /* VRH Set, Vap=4.4+... */
    {0xC3, {0x11}, 1},
    /* VDV Set, VDV=0 */
    {0xC4, {0x20}, 1},
    /* Frame Rate Control, 60Hz, inversion=0 */
    {0xC6, {0x0f}, 1},
    /* Power Control 1, AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V */
    {0xD0, {0xA4, 0xA1}, 1},
    /* Positive Voltage Gamma Control */
    {0xE0, {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19}, 14},
    /* Negative Voltage Gamma Control */
    {0xE1, {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19}, 14},
    /* Sleep Out */
    {0x11, {0}, 0x80},
    /* Display On */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff}
};

static spi_device_handle_t priv_spi_handle;
static uint16_t *line_data;


/*
**====================================================================================
** Public function definitions
**====================================================================================
*/
/* Sends initial commands and sets up the display. Must be called before writing to the display.*/
void display_init(void)
{
    esp_err_t ret;

    spi_device_interface_config_t devcfg=
    {
        .clock_speed_hz=40*1000*1000,           //Clock out at 40 MHz
        .mode=0,                                //SPI mode 0
        .spics_io_num=PIN_NUM_DISPLAY_CS,       //CS pin
        .queue_size=12,                         //We want to be able to queue 12 transactions at a time
        .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };

    printf("Initializing SPI bus... \n");

    //Attach the LCD to the SPI bus that was initialized in main.c
    ret=spi_bus_add_device(LCD_HOST, &devcfg, &priv_spi_handle);
    ESP_ERROR_CHECK(ret);

    printf("Initializing LCD display... \n");

    //Initialize the LCD
    lcd_init(priv_spi_handle);

    /* This buffer is used by the fill Rectangle function. */
    line_data = heap_caps_malloc(DISPLAY_MAX_TRANSFER_SIZE, MALLOC_CAP_DMA);
}


void display_drawScreenBuffer(uint16_t *buf)
{
    wait_display_data_finish(priv_spi_handle);
    send_display_data(priv_spi_handle, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, buf, false);
}


void display_drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *bmp_buf)
{
    wait_display_data_finish(priv_spi_handle);
    send_display_data(priv_spi_handle, x, y, width, height, bmp_buf, false);
}

/* Draws a rectangle directly on the display at the given coordinates. */
void display_fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
	uint16_t buf_size = MIN(DISPLAY_MAX_TRANSFER_SIZE, height*width*sizeof(uint16_t));

	wait_display_data_finish(priv_spi_handle);
    assert(line_data != NULL);

    for (int x = 0; x < (buf_size / 2); x++)
    {
    	line_data[x] = color;
    }

	send_display_data(priv_spi_handle, x, y, width, height, line_data, true);
}


/*
**====================================================================================
** Private function definitions
**====================================================================================
*/
//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
static void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}


//Initialize the display
static void lcd_init(spi_device_handle_t spi)
{
    int cmd=0;
    const lcd_init_cmd_t* lcd_init_cmds;

    //Initialize non-SPI GPIOs
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = ((1ULL<<PIN_NUM_DC) | (1ULL<<PIN_NUM_RST) | (1ULL<<PIN_NUM_BCKL));
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = true;
    gpio_config(&io_conf);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    lcd_init_cmds = st_init_cmds;

    //Send all the commands
    while (lcd_init_cmds[cmd].databytes!=0xff)
    {
        lcd_cmd(spi, lcd_init_cmds[cmd].cmd, false);
        lcd_data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);

        if (lcd_init_cmds[cmd].databytes&0x80)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        cmd++;
    }

    ///Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 1);
}


/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
static void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd, bool keep_cs_active)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    if (keep_cs_active)
    {
      t.flags = SPI_TRANS_CS_KEEP_ACTIVE;   //Keep CS active after data transfer
    }
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}


/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
static void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

uint8_t priv_number_of_transfers = 0u;

/* Updates the whole screen */
static void send_display_data(spi_device_handle_t spi, int xPos, int yPos, int width, int height, uint16_t *linedata, bool isBufferConstant)
{
    esp_err_t ret;
    int total_size_bytes = width * height * 2;
    int chunk_ix = 5;
    uint16_t * line_ptr;
    int curr_transfer_size;

    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
    static spi_transaction_t trans[12];

	uint16_t end_column = (xPos + width) - 1u;
    uint16_t end_row = (yPos + height) - 1u;

    end_column = MIN(end_column, DISPLAY_WIDTH);
    end_row = MIN(end_row, DISPLAY_HEIGHT);

    //In theory, it's better to initialize trans and data only once and hang on to the initialized
    //variables. We allocate them on the stack, so we need to re-init them each call.
    for (int ix = 0; ix < 12; ix++)
    {
        memset(&trans[ix], 0, sizeof(spi_transaction_t));
        trans[ix].flags=SPI_TRANS_USE_TXDATA;
    }

    trans[0].tx_data[0]=0x2A;           	//Column Address Set
    trans[0].length = 8;
    trans[0].user=(void*)0;

    trans[1].tx_data[0]=xPos >> 8;      	//Start Col High
    trans[1].tx_data[1]=xPos & 0xffu;   	//Start Col Low
    trans[1].tx_data[2]=end_column >> 8;	//End Col High
    trans[1].tx_data[3]=end_column & 0xff;	//End Col Low
    trans[1].length = 8*4;
    trans[1].user=(void*)1;

    trans[2].tx_data[0]=0x2B;           	//Page address set
    trans[2].length = 8;
    trans[2].user=(void*)0;

    trans[3].tx_data[0]=yPos >> 8;        	//Start page high
    trans[3].tx_data[1]=yPos & 0xff;      	//start page low
    trans[3].tx_data[2]=end_row >> 8;    	//end page high
    trans[3].tx_data[3]=end_row & 0xff;  	//end page low
    trans[3].length = 8*4;
    trans[3].user=(void*)1;

    trans[4].tx_data[0]=0x2C;           	//memory write
    trans[4].length = 8;
    trans[4].user=(void*)0;

    line_ptr = linedata;

    while(total_size_bytes > 0)
    {
    	curr_transfer_size = MIN(total_size_bytes, DISPLAY_MAX_TRANSFER_SIZE);
    	trans[chunk_ix].tx_buffer = line_ptr;        			//finally send the line data
    	trans[chunk_ix].length=curr_transfer_size * 8;  	//Data length, in bits
    	trans[chunk_ix].flags = 0; 							//undo SPI_TRANS_USE_TXDATA flag
    	trans[chunk_ix].user =(void*)1;

    	chunk_ix++;
    	total_size_bytes -= curr_transfer_size;

    	if(!isBufferConstant)
    	{
    		line_ptr += (curr_transfer_size / 2); //Not ideal... We have a U16 ptr, but transfer size is in bytes...
    	}
    }

    trans[chunk_ix - 1].flags = 0;
    priv_number_of_transfers = chunk_ix;

    //Queue all transactions.
    for (int ix=0; ix < chunk_ix; ix++)
    {
        ret=spi_device_queue_trans(spi, &trans[ix], portMAX_DELAY);
        assert(ret==ESP_OK);
    }
}


static void wait_display_data_finish(spi_device_handle_t spi)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    //Wait for all transactions to be done and get back the results.
    for (int x=0; x < priv_number_of_transfers; x++)
    {
        ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    }
    priv_number_of_transfers = 0u;
}
