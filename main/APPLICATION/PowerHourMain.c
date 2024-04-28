/*
 * PowerHourMain.c
 *
 *  Created on: 13 Dec 2023
 *      Author: Joonatan
 */

#include "PowerHourMain.h"
#include "display.h"
#include "LcdWriter.h"
#include "Buzzer.h"
#include "Bitmaps/Bitmaps.h"
#include "systimer.h"
#include "BitmapHandler.h"
#include "SpecialTasks.h"
#include "Scheduler.h"
#include "buttons.h"
#include "misc.h"
#include "configuration.h"
#include "MessageBox.h"
#include "pot.h"
#include "random.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ENABLE_BORDERS

//#define DISABLE_BUZZER_FOR_TESTING
//#define PSEUDORANDOM_NUMBER_TEST

#define CLOCK_X_OFFSET 3u
#define CLOCK_Y_OFFSET 15u
#define CLOCK_LETTER_SPACING 23u

#define BEERSHOT_IMAGE_X_OFFSET 3u
#define BEERSHOT_IMAGE_Y_OFFSET 15u

#define TEXT_X_OFFSET 3u
#define TEXT_Y_OFFSET 3u
#define TEXT_LINE_DISTANCE 18u

#define BORDER_WIDTH 2u

/* CLOCK area dimensions */
#define CLOCK_AREA_X_BEGIN  0u
#define CLOCK_AREA_Y_BEGIN  0u
#define CLOCK_AREA_X_END    115u
#define CLOCK_AREA_Y_END    86u

/* BEERSHOT area dimensions */
#define BEERSHOT_AREA_X_BEGIN (CLOCK_AREA_X_END + BORDER_WIDTH)
#define BEERSHOT_AREA_Y_BEGIN 0u
#define BEERSHOT_AREA_X_END DISPLAY_WIDTH - 1u
#define BEERSHOT_AREA_Y_END CLOCK_AREA_Y_END

/* Text area dimensions */
#define TEXT_AREA_X_BEGIN 0u
#define TEXT_AREA_X_END   (DISPLAY_WIDTH - 1u)
#define TEXT_AREA_Y_BEGIN (CLOCK_AREA_Y_END + BORDER_WIDTH)
#define TEXT_AREA_Y_END DISPLAY_HEIGHT - 1u




/*****************************************************************************************************
 *
 * Private type definitions
 *
 *****************************************************************************************************/

typedef enum
{
    CONTROLLER_INIT,
    CONTROLLER_COUNTING,
    CONTROLLER_OVERRIDDEN,
    CONTROLLER_FINAL,
    CONTROLLER_EXITING,
    CONTROLLER_NUMBER_OF_STATES
} controllerState;

typedef enum
{
    BEERSHOT_NO_ACTION,         //No action with bitmap.
    BEERSHOT_EMPTY,             //Draw initial beershot bitmap.
    BEERSHOT_BEGIN_FILLING,     //Initiate filling animation.
    BEERSHOT_FULL,              //Draw full beershot.
    BEERSHOT_BEGIN_EMPTYING,    //Initiate emptying animation.

    OVERRIDE_FUNCTION,          //Overrides ordinary function, displays bitmap instead.
    RESTORE_FUNCTION,           //Restore normal handling.
} beerShotAction;

typedef enum
{
    BEERSHOT_FROZEN,    //Nothing currently to draw.
    BEERSHOT_FILLING,
    BEERSHOT_EMPTYING
} beershotState;


typedef struct
{
    U8 second;                      // When the event should be triggered.
    const char * upperText;         // Text to be written to upper part of display.
    const char * lowerText;         // Text to be written to lower part of display.
    beerShotAction shot_action;     // Action to be performed on bitmap.
    OverrideFunc    func;           // Optional parameter - This function will override all behaviour, used for special actions.
} ControllerEvent;

typedef struct
{
    const ControllerEvent * event_array;
    U8                        event_cnt;

    BitmapHandler_FileCategory_t bitmap_category; /* Specifies which random bitmap should be pre-loaded to be used for the intro sequence. */
    BitmapHandler_FileCategory_t bitmap_spicy;
} SchedulerTaskConf_T;

typedef struct
{
    U16 counter;
    Boolean is_enabled;
} SchedulerTaskState_T;

typedef struct
{
    const char * text_str;
    U8 text_x;
    U8 text_y;
    FontType_t text_font;
    Boolean isInverted;
} IntroSequence;


/*****************************************************************************************************
 *
 * Private function Prototypes
 *
 *****************************************************************************************************/

Private void drawClock(void);
Private void incrementTimer(void);
Private void drawBorders(void);
Private void drawBeerShot(beerShotAction action);
Private void drawBeerShotLevel(U8 level);
Private void drawBeerShotBackground(void);
Private void redrawBackground(void);
Private void drawTextOnLine(const char * text, int line);

Private U8 getScheduledSpecialTask(const ControllerEvent ** event_ptr);
Private U8 selectRandomTaskIndex(void);

Private Boolean guysSpecialIntro(U8 sec);
Private Boolean girlsSpecialIntro(U8 sec);
Private Boolean EverybodySpecialIntro(U8 sec);
Private Boolean genericIntroFunction(const IntroSequence * intro_ptr, U8 sec);

Private void doFinalAction(void);
Private void setupStartOfMinute(void);
Private void drawEventText(const ControllerEvent* event);
Private void handlePreemptiveBitmapLoad(void);

/* Button handlers. */
Private void subscribeButtonHandlers(void);
Private void HandleUpButton(void);
Private void HandleDownButton(void);
Private void HandleRightButton(void);
Private void HandleLeftButton(void);

Private void handleMessageBoxResponse(MsgBox_Response resp);

/*****************************************************************************************************
 *
 * Private variable declarations.
 *
 *****************************************************************************************************/

//volatile U32 maximum_cyclic_period = 0u;

/* Beer shot variables */
#define NUMBER_OF_BEERSHOT_IMAGES 8U
#if 0
Private const tImage * priv_beershot_images[NUMBER_OF_BEERSHOT_IMAGES] =
{
     &BeerShot_1,
     &BeerShot_2,
     &BeerShot_3,
     &BeerShot_4,
     &BeerShot_5,
     &BeerShot_6,
     &BeerShot_7,
     &BeerShot_8,
};
#else
Private const tImage * priv_beershot_images[NUMBER_OF_BEERSHOT_IMAGES] =
{
     &beer1,
     &beer2,
     &beer3,
     &beer4,
     &beer5,
     &beer6,
     &beer7,
     &beer8,
};
#endif

Private U8 priv_beershot_counter = 0u;
Private beershotState priv_beer_state;
/* End of Beer shot variables. */

Private U16 priv_curr_minute;
Private U16 priv_curr_second;

Private controllerState priv_state;
Private U8 priv_task_frequency = 3u; /* Default value is a task every 2 minutes. */
Private Boolean priv_is_buzzer_enabled = TRUE;

Private const ControllerEvent priv_normal_minute_events[] =
{
     { .second = 7u,  .upperText = "",                      .lowerText = "",        .shot_action = BEERSHOT_EMPTY            , .func = NULL },
     { .second = 21u, .upperText = "Fill shots",            .lowerText = NULL,      .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL },
     { .second = 45u, .upperText = "Ready",                 .lowerText = NULL,      .shot_action = BEERSHOT_FULL             , .func = NULL },
     { .second = 59u, .upperText = "Proosit!",              .lowerText = "Cheers!", .shot_action = BEERSHOT_BEGIN_EMPTYING   , .func = NULL },
};

Private const ControllerEvent priv_guys_drink_events[] =
{
     { .second = 7u,  .upperText = "",                      .lowerText = "",                            .shot_action = OVERRIDE_FUNCTION         , .func = &guysSpecialIntro   },
     { .second = 21u, .upperText = "At the end of round",   .lowerText = "there is a task for guys",    .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL },
     { .second = 45u, .upperText = "Ready guys!",           .lowerText = "Task for guys coming up",     .shot_action = BEERSHOT_FULL             , .func = NULL },
     { .second = 59u, .upperText = "Proosit!",              .lowerText = "Cheers guys!",                .shot_action = OVERRIDE_FUNCTION         , .func = &guysSpecialTask    },
};

Private const ControllerEvent priv_girls_drink_events[] =
{
     { .second = 7u,  .upperText = "",                      .lowerText = "",                            .shot_action = OVERRIDE_FUNCTION         , .func = &girlsSpecialIntro   },
     { .second = 21u, .upperText = "At the end of round",   .lowerText = "there is a task for girls",   .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL },
     { .second = 45u, .upperText = "Ready girls!",          .lowerText = "Task for girls coming up",    .shot_action = BEERSHOT_FULL             , .func = NULL },
     { .second = 59u, .upperText = "Proosit!",              .lowerText = "Cheers girls!",               .shot_action = OVERRIDE_FUNCTION         , .func = &girlsSpecialTask    },
};

Private const ControllerEvent priv_everybody_drink_events[] =
{
     { .second = 7u,  .upperText = "",                      .lowerText = "",                        .shot_action = OVERRIDE_FUNCTION         , .func = &EverybodySpecialIntro   },
     { .second = 21u, .upperText = "At the end of round",   .lowerText = "a task for everybody",    .shot_action = BEERSHOT_BEGIN_FILLING    , .func = NULL },
     { .second = 45u, .upperText = "Ready!",                .lowerText = "Task for all coming up",  .shot_action = BEERSHOT_FULL             , .func = NULL },
     { .second = 59u, .upperText = "Proosit!",              .lowerText = "Cheers!",             .shot_action = OVERRIDE_FUNCTION         , .func = &everybodySpecialTask    },
};


/* This is a scheduler for special minutes.
 * It contains data about the frequency and offset of special minutes as well
 * as links to their respective actions. */
Private const SchedulerTaskConf_T priv_scheduler_conf[NUMBER_OF_TASK_TYPES] =
{
     {  .event_array = priv_girls_drink_events,     .event_cnt = NUMBER_OF_ITEMS(priv_girls_drink_events),      .bitmap_category = FILES_WOMEN     ,.bitmap_spicy = FILES_WOMEN_HOT },  /*   TASK_FOR_GIRLS        */
     {  .event_array = priv_guys_drink_events,      .event_cnt = NUMBER_OF_ITEMS(priv_guys_drink_events),       .bitmap_category = FILES_MEN       ,.bitmap_spicy = FILES_MEN_HOT   },  /*   TASK_FOR_GUYS         */
     {  .event_array = priv_everybody_drink_events, .event_cnt = NUMBER_OF_ITEMS(priv_everybody_drink_events),  .bitmap_category = FILES_BESTIES ,.bitmap_spicy = FILES_ALL_HOT   },  /*   TASK_FOR_EVERYONE     */
};


Private const ControllerEvent priv_initial_event =
{
 .second = 1u,
 .upperText = "Begin!",
 .lowerText = NULL,
 .shot_action = BEERSHOT_EMPTY
};


Private SchedulerTaskState_T priv_scheduler_state[NUMBER_OF_TASK_TYPES];
Private Boolean priv_isFirstRun = TRUE;
Private U8 priv_override_counter;
Private OverrideFunc priv_override_ptr;

Private const ControllerEvent * priv_currentMinuteEvents_ptr = priv_normal_minute_events;
Private U8 priv_currentControllerEvents_cnt = NUMBER_OF_ITEMS(priv_normal_minute_events);

Private const ControllerEvent * priv_nextMinuteEvents_ptr = priv_normal_minute_events;
Private U8 priv_nextControllerEvents_cnt = NUMBER_OF_ITEMS(priv_normal_minute_events);
Private BitmapHandler_FileCategory_t priv_file_type_to_load = FILES_NONE;
Private char priv_bitmap_name_buf[64u];
Private Boolean priv_is_bitmap_loaded = FALSE;

Private uint16_t * priv_sd_card_image_buffer = NULL;

/*****************************************************************************************************
 *
 * Public function definitions
 *
 *****************************************************************************************************/

Public void powerHour_init(void)
{
    priv_curr_minute = 0u;
    priv_curr_second = 0u;
    priv_beershot_counter = 0u;

    priv_state = CONTROLLER_INIT;
    priv_is_buzzer_enabled = configuration_getItem(CONFIG_ITEM_BUZZER);

    if (priv_sd_card_image_buffer == NULL)
    {
    	priv_sd_card_image_buffer = heap_caps_malloc(DISPLAY_HEIGHT*DISPLAY_WIDTH*sizeof(uint16_t), MALLOC_CAP_32BIT);
    	assert(priv_sd_card_image_buffer);
    }

    SpecialTasks_init();
}


Public void powerHour_start(void)
{
    U8 ix;
#ifdef PSEUDORANDOM_NUMBER_TEST
    char buf[10];
    U8 xCoord = 5u;
    U8 yCoord = 5u;
#endif

    /* Set up critical variables, as we might want to restart the game. */
    priv_curr_minute = 0u;
    priv_curr_second = 0u;
    priv_state = CONTROLLER_INIT;
    priv_task_frequency = configuration_getItem(CONFIG_ITEM_TASK_FREQ);

    regenerate_random_number_seed();

#ifdef PSEUDORANDOM_NUMBER_TEST
    int number = generate_random_number(30u);

    for (ix = 0u; ix < 40u; ix++)
    {
        long2string(number, buf);
        display_drawString(buf, xCoord, yCoord, FONT_SMALL_FONT_12, FALSE);
        yCoord += 10u;

        if (yCoord > 120u)
        {
            yCoord = 5u;
            xCoord += 20u;
        }

        number = generate_random_number(30u);
    }

    timer_delay_msec(10000);
#endif

    redrawBackground();


    drawClock();
    drawBeerShot(priv_beershot_counter);

    /* Set up buttons */
    subscribeButtonHandlers();

    //We start counting.
    priv_isFirstRun = TRUE;
    priv_state = CONTROLLER_COUNTING;

    priv_currentMinuteEvents_ptr = priv_normal_minute_events;
    priv_currentControllerEvents_cnt = NUMBER_OF_ITEMS(priv_normal_minute_events);

    priv_nextMinuteEvents_ptr = priv_normal_minute_events;
    priv_nextControllerEvents_cnt = NUMBER_OF_ITEMS(priv_normal_minute_events);

    for (ix = 0u; ix < NUMBER_OF_TASK_TYPES; ix++)
    {
        priv_scheduler_state[ix].counter = 0u;
        priv_scheduler_state[ix].is_enabled = TRUE; /* In this version all tasks are always enabled. */
    }
}



Public void powerHour_cyclic1000msec(void)
{
    U8 ix;
    const ControllerEvent * event_ptr = NULL;
    beerShotAction action = BEERSHOT_NO_ACTION;

#ifndef DISABLE_BUZZER_FOR_TESTING
    if (priv_is_buzzer_enabled && (priv_curr_second == 59u))
    {
        buzzer_playBeeps(3u);
    }
#endif

    //Game ends and we enter final state.
    if (priv_curr_minute == 60u)
    {
        doFinalAction();
        /* Increment so that we do not get stuck here. */
        priv_curr_minute++;
        priv_state = CONTROLLER_FINAL;
    }

    switch(priv_state)
    {
        case CONTROLLER_INIT:
            //We do not do anything here.
            break;
        case CONTROLLER_COUNTING:
            setupStartOfMinute();
            incrementTimer();

            if (priv_isFirstRun)
            {
                event_ptr = &priv_initial_event;
                priv_isFirstRun = FALSE;
            }
            else
            {
                for (ix = 0u; ix < priv_currentControllerEvents_cnt; ix++)
                {
                    event_ptr = &priv_currentMinuteEvents_ptr[ix];
                    if (event_ptr->second == priv_curr_second)
                    {
                        break;
                    }
                    event_ptr = NULL;
                }
            }

            if (event_ptr != NULL)
            {
                drawEventText(event_ptr);

                action = event_ptr->shot_action;
            }

            //Currently we still finish this cycle and special handling begins on the next.
            if (action == OVERRIDE_FUNCTION)
            {
                priv_state = CONTROLLER_OVERRIDDEN;
                priv_override_ptr = event_ptr->func;
                priv_override_counter = 0u;
            }

            drawBeerShot(action);
            drawClock();

            /* Here is where we start to pre-emptively load the bitmap for the next minute if necessary. Best not to do it too early when the display buffer might still be used. */
            if (priv_curr_second == 30u)
            {
                if (priv_file_type_to_load != FILES_NONE)
                {
                    handlePreemptiveBitmapLoad();
                }
            }

            break;
        case CONTROLLER_OVERRIDDEN:
            setupStartOfMinute();

            //We still increment timer.
            incrementTimer();

            if (priv_override_ptr(priv_override_counter))
            {
                redrawBackground();
                drawClock();
                drawBeerShot(BEERSHOT_EMPTY);
                priv_state = CONTROLLER_COUNTING;
            }
            priv_override_counter++;
            break;
        case CONTROLLER_FINAL:
            break;
        case CONTROLLER_EXITING:
            returnToMain();
            break;
        default:
            break;
    }
}



/* Should return all variables to their initial states. */
Public void powerHour_stop(void)
{
    priv_state = CONTROLLER_INIT;
    priv_curr_minute = 0u;
    priv_curr_second = 0u;
    SpecialTasks_init();
}


Public void powerHour_setTaskFrequency(U16 freq)
{
    priv_task_frequency = freq;
}

Public U16 powerHour_getTaskFrequency(void)
{
    return priv_task_frequency;
}

Public void setBuzzerEnable(U16 enable)
{
    if (enable == 0u)
    {
        priv_is_buzzer_enabled = FALSE;
    }
    else
    {
        priv_is_buzzer_enabled = TRUE;
    }

    configuration_setItem(priv_is_buzzer_enabled, CONFIG_ITEM_BUZZER);
}


Public U16 isBuzzerEnabled(void)
{
    return priv_is_buzzer_enabled;
}

/*****************************************************************************************************
 *
 * Private function definitions
 *
 *****************************************************************************************************/

Private void bitmapLoadedCallback(void)
{
    priv_is_bitmap_loaded = TRUE;
    printf("Bitmap loading complete\n");
}

Public volatile Boolean debug_is_bmp_error = FALSE;

Private void handlePreemptiveBitmapLoad(void)
{
    U16 * buffer_ptr = priv_sd_card_image_buffer;

    priv_is_bitmap_loaded = FALSE;
    printf("Call from PowerHourMachine : %d\n", priv_file_type_to_load);
    BitmapHandler_getRandomBitmapForCategory(priv_file_type_to_load, priv_bitmap_name_buf);
    printf("Starting to load bitmap %s\n", priv_bitmap_name_buf);

    /* TODO : Remove*/
    vTaskDelay(200 / portTICK_PERIOD_MS);

    if (BitmapHandler_StartCyclicLoad(priv_bitmap_name_buf, buffer_ptr, bitmapLoadedCallback) == FALSE)
    {
        debug_is_bmp_error = TRUE;
    }
}


Private void setupStartOfMinute(void)
{
    if ((priv_curr_second == 0u) && (priv_curr_minute > 0u))
    {
        priv_currentControllerEvents_cnt = priv_nextControllerEvents_cnt;
        priv_currentMinuteEvents_ptr = priv_nextMinuteEvents_ptr;
        /* We have to determine what the next minute is going to be like, so we can do some pre-emptive bitmap loading if necessary. */

        priv_nextControllerEvents_cnt = getScheduledSpecialTask(&priv_nextMinuteEvents_ptr);
    }
}

Private void doFinalAction(void)
{
    display_clear();
    display_drawStringCenter("Game Over!", DISPLAY_CENTER, 15u, FONT_LARGE_FONT, FALSE);
    display_drawStringCenter("Congratulations! ",DISPLAY_CENTER, 37u, FONT_LARGE_FONT, FALSE);
    display_drawStringCenter("You are now drunk ",DISPLAY_CENTER, 57u, FONT_LARGE_FONT, FALSE);
}


Private Boolean guysSpecialIntro(U8 sec)
{
    IntroSequence sequence;

    sequence.isInverted = FALSE;
    sequence.text_font = FONT_LARGE_FONT;
    sequence.text_str = "Round for Guys!";
    sequence.text_x = DISPLAY_CENTER;
    sequence.text_y = 7u;

    return genericIntroFunction(&sequence, sec);
}


Private Boolean girlsSpecialIntro(U8 sec)
{
    IntroSequence sequence;

    sequence.isInverted = FALSE;
    sequence.text_font = FONT_LARGE_FONT;
    sequence.text_str = "Round for Girls!";
    sequence.text_x = DISPLAY_CENTER;
    sequence.text_y = 7u;

    return genericIntroFunction(&sequence, sec);
}


Private Boolean EverybodySpecialIntro(U8 sec)
{
    IntroSequence sequence;

    sequence.isInverted = FALSE;
    sequence.text_font = FONT_LARGE_FONT;
    sequence.text_str = "Round for All!";
    sequence.text_x = DISPLAY_CENTER;
    sequence.text_y = 7u;

    return genericIntroFunction(&sequence, sec);
}


Private Boolean genericIntroFunction(const IntroSequence * intro_ptr, U8 sec)
{
    Boolean res = FALSE;

    switch(sec)
    {
    case(1u):
        display_clear();
        if (priv_is_bitmap_loaded)
        {
            /* TODO : Take into account that an image may be slightly smaller!!!*/
        	display_drawImage(0u, 0u, DISPLAY_WIDTH, DISPLAY_HEIGHT, priv_sd_card_image_buffer);
        	//display_flushBuffer(0u, 0u, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        }
        else
        {
            /* A literal blue screen of death.. */
            display_fill(COLOR_BLUE);
        }
        break;
    case(3u):
        LcdWriter_drawStringCenter(intro_ptr->text_str, intro_ptr->text_x, intro_ptr->text_y, intro_ptr->text_font, disp_ph_prompt_text_color, disp_highlight_color);
        break;
    case(12u):
        res = TRUE;
        break;
    default:
        break;
    }

    return res;
}


//Return default normal cycle if no special events are scheduled.
Private U8 getScheduledSpecialTask(const ControllerEvent ** event_ptr)
{
    U8 ix;
    U8 res;

    if (((priv_curr_minute + 1u) % priv_task_frequency) == 0u)
    {
        ix = selectRandomTaskIndex();
        *event_ptr = priv_scheduler_conf[ix].event_array;
        res = priv_scheduler_conf[ix].event_cnt;
        if ((pot_getSelectedRange(POTENTIOMETER_SEXY_LEVEL) == 3u) || (pot_getSelectedRange(POTENTIOMETER_NUDE_LEVEL) == 3u))
        {
            priv_file_type_to_load = priv_scheduler_conf[ix].bitmap_spicy;
        }
        else
        {
            priv_file_type_to_load = priv_scheduler_conf[ix].bitmap_category;
        }
    }
    else
    {
        *event_ptr = priv_normal_minute_events;
        res = NUMBER_OF_ITEMS(priv_normal_minute_events);
        priv_file_type_to_load = FILES_NONE;
    }

    return res;
}

/* It returns a random enabled task. However the function should prefer tasks that have not yet been displayed or have been displayed less than others. */
Private U8 selectRandomTaskIndex(void)
{
    U8 max_count = 0u;
    U8 min_count = 0xffu;
    U8 ix;
    U8 res = 0u;

    U8 index_array[NUMBER_OF_TASK_TYPES];
    U8 index_length = 0u;

    /* Lets first establish the MAX and MIN count that we have. */
    for (ix = 0u; ix < NUMBER_OF_TASK_TYPES; ix++)
    {
        if (priv_scheduler_state[ix].is_enabled)
        {
            max_count = MAX(priv_scheduler_state[ix].counter, max_count);
            min_count = MIN(priv_scheduler_state[ix].counter, min_count);
        }
    }

    if (max_count == min_count)
    {
        /* We have no preference in this case. We add all indexes that are enabled to the array. */
        for (ix = 0u; ix < NUMBER_OF_TASK_TYPES; ix++)
        {
            if (priv_scheduler_state[ix].is_enabled)
            {
                index_array[index_length] = ix;
                index_length++;
            }
        }

        max_count++;
    }
    else
    {
        /* We prefer tasks that have not been selected yet. */
        for (ix = 0u; ix < NUMBER_OF_TASK_TYPES; ix++)
        {
            if (priv_scheduler_state[ix].is_enabled && (priv_scheduler_state[ix].counter < max_count))
            {
                index_array[index_length] = ix;
                index_length++;
            }
        }
    }

    res = index_array[generate_random_number(index_length - 1u)];


    /* We set the selected count to the max. */
    priv_scheduler_state[res].counter = max_count;

    return res;
}



Private void incrementTimer(void)
{
    priv_curr_second++;
    if (priv_curr_second >= 60u)
    {
        priv_curr_second = 0u;
        priv_curr_minute++;
    }
}


Private void drawEventText(const ControllerEvent* event)
{
    if (event->upperText != NULL)
    {
        drawTextOnLine(event->upperText, 0u);
    }

    if (event->lowerText != NULL)
    {
        drawTextOnLine(event->lowerText, 1u);
    }
}


Private void drawBorders(void)
{
    display_fillRectangle(CLOCK_AREA_X_BEGIN, CLOCK_AREA_Y_END, DISPLAY_WIDTH, BORDER_WIDTH, disp_text_color);
    display_fillRectangle(CLOCK_AREA_X_END, CLOCK_AREA_Y_BEGIN, BORDER_WIDTH, CLOCK_AREA_Y_END, disp_text_color);
}


Private void drawBeerShot(beerShotAction action)
{

   switch(action)
   {
       case BEERSHOT_EMPTY:
           priv_beershot_counter = 0u;
           priv_beer_state = BEERSHOT_FROZEN;
           drawBeerShotLevel(priv_beershot_counter);
           break;
       case BEERSHOT_FULL:
           priv_beershot_counter = NUMBER_OF_BEERSHOT_IMAGES - 1u;
           drawBeerShotLevel(priv_beershot_counter);
           priv_beer_state = BEERSHOT_FROZEN;
           break;
       case BEERSHOT_BEGIN_FILLING:
           priv_beershot_counter = 0u;
           priv_beer_state = BEERSHOT_FILLING;
           break;
       case BEERSHOT_BEGIN_EMPTYING:
           priv_beershot_counter = NUMBER_OF_BEERSHOT_IMAGES - 1u;
           priv_beer_state = BEERSHOT_EMPTYING;
           break;
       case OVERRIDE_FUNCTION:
           priv_beer_state = BEERSHOT_FROZEN;
           //priv_beershot_counter = 0u;
           //drawBeerShotLevel(priv_beershot_counter);
       case BEERSHOT_NO_ACTION:
       default:
           break;
   }

   switch(priv_beer_state)
   {
       case(BEERSHOT_FROZEN):
           //Nothing new to draw.
           break;
       case(BEERSHOT_EMPTYING):
           drawBeerShotLevel(priv_beershot_counter);
           if (priv_beershot_counter > 0u)
           {
               priv_beershot_counter--;
           }
           else
           {
               priv_beershot_counter = NUMBER_OF_BEERSHOT_IMAGES - 1u;
           }
           break;
       case(BEERSHOT_FILLING):
           drawBeerShotLevel(priv_beershot_counter);
           priv_beershot_counter++;
           if (priv_beershot_counter == NUMBER_OF_BEERSHOT_IMAGES)
           {
               priv_beershot_counter = 0u;
           }
           break;
       default:
           break;
   }

}


Private void drawBeerShotLevel(U8 level)
{
    /* Lets begin with some very initial testing. */
    if(level < NUMBER_OF_BEERSHOT_IMAGES)
    {
        drawBitmap(BEERSHOT_AREA_X_BEGIN + BEERSHOT_IMAGE_X_OFFSET, BEERSHOT_AREA_Y_BEGIN + BEERSHOT_IMAGE_Y_OFFSET, priv_beershot_images[level], false);
    }
}

Private void drawBeerShotBackground(void)
{
    display_fillRectangle(BEERSHOT_AREA_X_BEGIN, BEERSHOT_AREA_Y_BEGIN, BEERSHOT_AREA_X_END - BEERSHOT_AREA_X_BEGIN, BEERSHOT_AREA_Y_END - BEERSHOT_AREA_Y_BEGIN, COLOR_BLACK);
}


Private void redrawBackground(void)
{
    display_clear();
#ifdef  ENABLE_BORDERS
    drawBorders();
#endif
    drawBeerShotBackground();
}


Private void drawClock(void)
{
    U8 x_offset = CLOCK_X_OFFSET;

    LcdWriter_drawCharColored('0' + (priv_curr_minute / 10u), x_offset, CLOCK_Y_OFFSET, FONT_TNR_HUGE_NUMBERS, disp_text_color, disp_background_color);
    x_offset += CLOCK_LETTER_SPACING;
    LcdWriter_drawCharColored('0' + (priv_curr_minute % 10u), x_offset, CLOCK_Y_OFFSET, FONT_TNR_HUGE_NUMBERS, disp_text_color, disp_background_color);
    x_offset += CLOCK_LETTER_SPACING - 1u;
    LcdWriter_drawCharColored(':' , x_offset, CLOCK_Y_OFFSET, FONT_TNR_HUGE_NUMBERS, disp_text_color, disp_background_color);
    x_offset += 14u;
    LcdWriter_drawCharColored('0' + (priv_curr_second / 10u), x_offset, CLOCK_Y_OFFSET, FONT_TNR_HUGE_NUMBERS, disp_text_color, disp_background_color);
    x_offset += CLOCK_LETTER_SPACING;
    LcdWriter_drawCharColored('0' + (priv_curr_second % 10u), x_offset, CLOCK_Y_OFFSET, FONT_TNR_HUGE_NUMBERS, disp_text_color, disp_background_color);
}


Private void drawTextOnLine(const char * text, int line)
{
    /* We should have enough space for two lines of text...*/
    if (line <= 1u)
    {
        /* Clear any previous text. */
        display_fillRectangle(TEXT_AREA_X_BEGIN, TEXT_AREA_Y_BEGIN + (line * TEXT_LINE_DISTANCE), TEXT_AREA_X_END - TEXT_AREA_X_BEGIN,TEXT_LINE_DISTANCE, disp_background_color);

        LcdWriter_drawColoredString(text, TEXT_X_OFFSET, TEXT_AREA_Y_BEGIN + TEXT_Y_OFFSET + (line * TEXT_LINE_DISTANCE), FONT_SMALL_FONT_12, disp_ph_prompt_text_color, disp_background_color);
    }
}



/********** Button Handlers  ********/

Private void subscribeButtonHandlers(void)
{
    buttons_subscribeListener(UP_BUTTON,    HandleUpButton);
    buttons_subscribeListener(DOWN_BUTTON,  HandleDownButton);
    buttons_subscribeListener(RIGHT_BUTTON, HandleRightButton);
    buttons_subscribeListener(LEFT_BUTTON,  HandleLeftButton);
}


Private void HandleUpButton(void)
{
    if (priv_state == CONTROLLER_FINAL)
    {
        /* We quit the game. */
        priv_state = CONTROLLER_EXITING;
    }
}

Private void HandleDownButton(void)
{
    if (priv_state == CONTROLLER_FINAL)
    {
        /* We quit the game. */
        priv_state = CONTROLLER_EXITING;
    }
}


Private void HandleRightButton(void)
{
    if (priv_state == CONTROLLER_FINAL)
    {
        /* We quit the game. */
        priv_state = CONTROLLER_EXITING;
    }
    else
    {
        /* This will open up a message box that will allow the user to cancel the game. */
        MessageBox_SetResponseHandler(handleMessageBoxResponse);
        MessageBox_ShowWithOkCancel("Quit game?");
    }
}

Private void HandleLeftButton(void)
{
    if (priv_state == CONTROLLER_FINAL)
    {
        /* We quit the game. */
        priv_state = CONTROLLER_EXITING;
    }
}


/* MessageBox Handler. */
Private void handleMessageBoxResponse(MsgBox_Response resp)
{
    /* In reality this is the only type of response that we can get. */
    /* This is really just for testing right now... */
    if (resp == RESPONSE_OK)
    {
        /* We quit the game. */
        priv_state = CONTROLLER_EXITING;
    }
    else if (resp == RESPONSE_CANCEL)
    {
        /* We resume the game. */
        /* Re subscribe the button handlers. */
        subscribeButtonHandlers();
        redrawBackground();
        drawBeerShotLevel(priv_beershot_counter);
    }
    else
    {
        /* Should not really happen. */
    }
}
