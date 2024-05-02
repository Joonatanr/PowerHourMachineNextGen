/*
 * BitmapHandler.c
 *
 *  Created on: 6. dets 2023
 *      Author: JRE
 */

#include "BitmapHandler.h"
#include "SdCardHandler.h"
#include "display.h"

#include "esp_vfs_fat.h"

#include "random.h"
#include "systimer.h"

Private const char *TAG = "BitmapHandler";

/* TODO : This is currently doubled. */
#define MOUNT_POINT "/sdcard"

typedef enum
{
    BITMAP_HANDLER_IDLE,
    BITMAP_HANDLER_CYCLIC_LOADING,
    BITMAP_HANDLER_COMPLETE,

    NUMBER_OF_BITMAP_HANDLER_STATES
} BitmapHandlerState;

#pragma pack(push)  // save the original data alignment
#pragma pack(1)     // Set data alignment to 1 byte boundary
typedef struct {
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

//#define BMP_DEBUG_PRINT
Private U8 priv_bmp_line_buffer[(MAX_BMP_LINE_LENGTH * 3) + 4u];

volatile U32 debug_begin_timestamp = 0u;
volatile U32 debug_end_timestamp = 0u;
volatile U32 debug_period = 0u;


Private BitmapHandlerState priv_state = BITMAP_HANDLER_IDLE;
Private FIL priv_f_obj;
Private BMPHeader priv_bmp_header;
Private U16 priv_current_line;
Private U16 * priv_dest_ptr;
Private BitmapLoaderCallback priv_load_complete_callback;

#define BITMAP_LOADER_TIME_SLOT 50u /* We need to finish our operations in 50 milliseconds time. */

/***************************** Private function forward declarations *******************************/

Private Boolean setupBitmapLoad(const char * path);
Private Boolean resumeBitmapRead(void);

#define FILE_NAME_PTR_CHUNK_SIZE 32u

/* Storage variable for bitmap paths... */


typedef struct
{
    const char * directoryName;
    char ** fileNames;
    U32 number_of_files;
} BitmapFilesCategory;


BitmapFilesCategory priv_file_list[NUMBER_OF_FILE_CATEGORIES] =
{
     {.directoryName = "/Men",          .fileNames = NULL, .number_of_files = 0u },
     {.directoryName = "/Women",        .fileNames = NULL, .number_of_files = 0u },
     {.directoryName = "/Everybody",    .fileNames = NULL, .number_of_files = 0u },
     {.directoryName = "/Men_Spicy",    .fileNames = NULL, .number_of_files = 0u },
     {.directoryName = "/Women_Spicy",  .fileNames = NULL, .number_of_files = 0u },
     {.directoryName = "/All_Spicy",    .fileNames = NULL, .number_of_files = 0u },
};


/***************************** Public functions  **************************************************/

Public void BitmapHandler_init(void)
{
    U8 ix;
    priv_state = BITMAP_HANDLER_IDLE;

    for(ix = 0u; ix < NUMBER_OF_FILE_CATEGORIES; ix++)
    {
        priv_file_list[ix].fileNames = malloc(FILE_NAME_PTR_CHUNK_SIZE * sizeof(char*));
        assert(priv_file_list[ix].fileNames);
    }
}

Public void BitmapHandler_start(void)
{
    FRESULT r;
    FF_DIR DI;
    FILINFO FI;
    U16 ix = 0u;
    U8 category;
    BitmapFilesCategory * filelist_ptr;

    /* Basically we want to get a list of BMP files that are in the specific folders on the SD card. */
    for (category = 0u; category < NUMBER_OF_FILE_CATEGORIES; category++)
    {
        filelist_ptr = &priv_file_list[category];
        ix = 0u;

        //printf( "Opening directory : %s \r\n", filelist_ptr->directoryName);
        r = f_opendir(&DI, filelist_ptr->directoryName);

        if(r != FR_OK)
        {
        	printf("Could not open directory : %s  error : %d \r\n", filelist_ptr->directoryName, r);
            continue;
        }
        else
        {
        	printf("Successfully opened directory : %s \r\n", filelist_ptr->directoryName);
        }

        /*Read everything inside the directory*/
        do
        {
            /*Read a directory/file*/
            r = f_readdir(&DI, &FI);
            /* Check for errors. */
            if (r != FR_OK)
            {
                printf( "Error reading file directory %s \n", filelist_ptr->directoryName);
                continue;
            }
            else
            {
            	//printf("f_readdir is successful for directory %s file %s\n", filelist_ptr->directoryName, FI.fname);
            }

            if(strlen(FI.fname) == 0)
            {
            	continue;
            }

           // printf("Trying to allocate for %s fname : %s strlen : %d\n", filelist_ptr->directoryName, FI.fname, strlen(FI.fname));
            /* Add to the list of filenames. */
            filelist_ptr->fileNames[ix] = heap_caps_malloc(64 * sizeof(char), MALLOC_CAP_32BIT);
            assert(filelist_ptr->fileNames[ix]);

            strcpy(filelist_ptr->fileNames[ix], FI.fname);
            ix++;

            if ((ix % FILE_NAME_PTR_CHUNK_SIZE) == 0u)
            {
                realloc(filelist_ptr->fileNames, (FILE_NAME_PTR_CHUNK_SIZE * sizeof(char*)) * ((ix / FILE_NAME_PTR_CHUNK_SIZE) + 1u));
            }

        } while(FI.fname[0]);

        filelist_ptr->number_of_files = ix;

        /** Debug */
        for (ix = 0u; ix < filelist_ptr->number_of_files; ix++)
        {
            printf("File: %s\r\n", filelist_ptr->fileNames[ix]);
        }

        printf("Finished reading directory : %s \r\n", filelist_ptr->directoryName);
        /* End of debug */
    }

    r = f_closedir(&DI);

    int file_index;

    for(ix = 0u; ix < NUMBER_OF_FILE_CATEGORIES; ix++)
    {
    	printf("Category : %d has %d files : \n", ix, priv_file_list[ix].number_of_files);

    	for (file_index = 0u; file_index < priv_file_list[ix].number_of_files; file_index++)
    	{
    		printf("----File : %s\n", priv_file_list[ix].fileNames[file_index]);
    	}
    }
}


/* This functions loads a bitmap, but it locks up the whole CPU. This can be a problem for realtime application. */
Public Boolean BitmapHandler_LoadBitmap(const char * path, U16 * dest)
{
    return sdCard_Read_bmp_file(path, dest);
}


Public Boolean BitmapHandler_StartCyclicLoad(const char * path, U16 * dest, BitmapLoaderCallback cb)
{
    Boolean res = FALSE;

    if (priv_state != BITMAP_HANDLER_IDLE)
    {
        return FALSE;
    }

    if (setupBitmapLoad(path) == TRUE)
    {
        priv_current_line = 0u;
        priv_dest_ptr = dest;
        priv_load_complete_callback = cb;
        priv_state = BITMAP_HANDLER_CYCLIC_LOADING;
        res = TRUE;
    }

    return res;
}


Public void BitmapLoaderCyclic100ms(void)
{
    switch (priv_state)
    {
        case (BITMAP_HANDLER_IDLE):
            /* Nothing to do here... */
            break;
        case (BITMAP_HANDLER_CYCLIC_LOADING):
            if (resumeBitmapRead() == TRUE)
            {
                priv_state = BITMAP_HANDLER_COMPLETE;
            }
            break;
        case (BITMAP_HANDLER_COMPLETE):
            if (priv_load_complete_callback != NULL)
            {
                priv_load_complete_callback();
            }
            priv_state = BITMAP_HANDLER_IDLE;
            break;
        default:
            break;
    }
}

/* We return the full path.. */
Public void BitmapHandler_getRandomBitmapForCategory(BitmapHandler_FileCategory_t type, char *dest)
{
    U16 ix;
    char * ps = dest;

    if ((type < NUMBER_OF_FILE_CATEGORIES) && (priv_file_list[type].number_of_files > 0))
    {
    	ix = generate_random_number(priv_file_list[type].number_of_files - 1u);

        strcpy(ps, priv_file_list[type].directoryName);
        ps += strlen(priv_file_list[type].directoryName);
        *ps = '/';
        ps++;
        strcpy(ps, priv_file_list[type].fileNames[ix]);
    }
}

/***************************** Private function definitions *******************************/
Private Boolean setupBitmapLoad(const char * path)
{
    FRESULT file_res;
    Boolean res = FALSE;
    UINT bytes_read;

	//char str[64] = MOUNT_POINT;
	//strcat(str, path);

    file_res = f_open(&priv_f_obj, path, FA_READ);

    if (file_res == FR_OK)
    {
        /* Lets first try reading out the bitmap header. */
        file_res = f_read(&priv_f_obj, &priv_bmp_header, sizeof(BMPHeader), &bytes_read);

        if (bytes_read == sizeof(BMPHeader))
        {
            /* TODO : Consider, if we should always just ignore a larger bitmap?? Initially lets just get the BMP reading to work properly and then we can start looking at special cases like this. */
            if (priv_bmp_header.width_px <= MAX_BMP_LINE_LENGTH)
            {
                printf("Successfully started loading %s, H : %ld, W : %ld\n", path, priv_bmp_header.height_px, priv_bmp_header.width_px);
            	res = TRUE;
            }
            else
            {
            	printf("File %s does not fit in memory!!!  H : %ld, W : %ld \n", path, priv_bmp_header.height_px, priv_bmp_header.width_px);
            }
        }
    }
    else
    {
    	printf("Setup bitmap load failed  for : %s\n", path);
    }

    return res;
}


Private Boolean resumeBitmapRead(void)
{
    U16 line_stride;
    U16 line_px_data_len;
    U32 begin_time;
    UINT bytes_read;
    FRESULT file_res;
    U32 x;

    Boolean res = FALSE; /* Return true if finished reading. */

    begin_time = systimer_getTimestamp();

    do
    {
        /* Now lets try and read RGB data... */
        /* Take padding into account... */
        line_px_data_len = priv_bmp_header.width_px * 3u;
        line_stride = (line_px_data_len + 3u) & ~0x03;

        file_res = f_lseek(&priv_f_obj, ((priv_bmp_header.height_px - (priv_current_line + 1)) * line_stride) + priv_bmp_header.offset);
        file_res = f_read(&priv_f_obj, priv_bmp_line_buffer, line_stride, &bytes_read);
        priv_current_line++;

        if (file_res == FR_OK)
        {
            for (x = 0u; x < line_px_data_len; x+=3u )
            {
                *priv_dest_ptr++ = CONVERT_888RGB_TO_565RGB(priv_bmp_line_buffer[x+2], priv_bmp_line_buffer[x+1], priv_bmp_line_buffer[x]);
            }
        }

        if (priv_current_line >= priv_bmp_header.height_px)
        {
            res = TRUE;
            break;
        }
    }
    while(systimer_getPeriod(begin_time) < BITMAP_LOADER_TIME_SLOT);

    return res;
}
