/*
 * SpecialTasks.c
 *
 *  Created on: Sep 5, 2017
 *      Author: Joonatan
 */

#include "SpecialTasks.h"
#include "Fonts/Fonts.h"
#include "display.h"
#include "LcdWriter.h"
#include <stdlib.h>
#include "pot.h"
#include "Bitmaps/Bitmaps.h"
#include "misc.h"
#include "random.h"

/*****************************************************************************************************
 *
 * Private defines
 *
 *****************************************************************************************************/

#define SMALL_SHOT_X 20u
#define SMALL_SHOT_Y 50u
#define SMALL_SHOT_INTERVAL 20u

#define SPECIALTASK_FONT FONT_ARIAL_14_BOLD
#define SPECIALTASK_SMALL_FONT FONT_ARIAL_12


/*****************************************************************************************************
 *
 * Private type definitions
 *
 *****************************************************************************************************/

typedef struct
{
    const char * upper_text;
    const char * middle_text;
    const char * lower_text;

    /* Valid values for these are 0-3*/
    const U8 nude_level;    /* Minimum nudity setting for this task to appear.                      */
    const U8 sexy_level;    /* Minimum sexy setting for this task to appear                         */ /* Can't believe I'm coding this... */
} Task_T;

typedef struct
{
    const Task_T * task_array;
    U8 * counter_array;
    U16 number_of_array_items;
} Task_Array_Descriptor;

typedef enum
{
    DIANA,
    MIRTEL,

    NUMBER_OF_SISTERS
} Migur_T;


/*****************************************************************************************************
 *
 * Private function forward declarations
 *
 *****************************************************************************************************/

//Private Boolean DrinkTwiceTask(U8 sec, const char * headerWord);
Private Boolean DrinkTwiceTask(U8 sec, SpecialTaskType type);
Private Boolean SpecialTaskWithRandomText(U8 sec, SpecialTaskType type);
Private const Task_T * getRandomTaskFromArray(const Task_T * array, U8 array_size, U8 *counter_ptr);
Private Boolean isTaskEnabled(const Task_T * task_ptr, U8 sexy_level, U8 nude_level);


Private void ReplaceStringEscapeChars(const char * str, char * dest);
Private Boolean isStringFitForDisplay(const char * str, U16 * length_dest);

/*****************************************************************************************************
 *
 * Private constant declarations
 *
 *****************************************************************************************************/
Private const SpecialTaskFunc priv_special_tasks_girls_array[] =
{
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &DrinkTwiceTask,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
};


Private const SpecialTaskFunc priv_special_tasks_guys_array[] =
{
  &SpecialTaskWithRandomText,
  &DrinkTwiceTask,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
};

Private const SpecialTaskFunc priv_special_tasks_everybody_array[] =
{
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &SpecialTaskWithRandomText,
  &DrinkTwiceTask,
};


/* Easy tasks. -> Really softcore.  */
Private const Task_T priv_TextArrayGirlsLevel1[] =
{
     { "The girl with ",     "the fanciest clothes"  , "drinks 2x"          , .nude_level = 0u, .sexy_level = 0u}, /* 1  */
     { NULL,                 "Only girls drink"      , NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 2  */
     { "Girls drink",        "without "      ,         "using hands"        , .nude_level = 0u, .sexy_level = 1u}, /* 3  */
     { "Choose one girl",    "who drinks 3x ",         NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 4  */
     { "The girl with",      "the longest hair ",      "drinks a shot"      , .nude_level = 0u, .sexy_level = 0u}, /* 5  */
     { "The girl with",      "the highest voice",      "drinks a shot"      , .nude_level = 0u, .sexy_level = 0u}, /* 6  */
     { "All blondes",        "drink 2x ",              NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 7  */
     { "All brunettes",      "drink 2x ",              NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 8  */
     { "The tallest girl",   "drinks 2x ",             NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 9  */
     { "The youngest girl",  "drinks 2x ",             NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 10 */
     { "All girl BESTies",   "drink  2x ",             NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 11 */
     { "The shortest girl",  "drinks 2x ",             NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 12 */
     { "Girls propose",      "the next toast ",        NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 13 */
     { "All redheads",       "drink 2x ",              NULL                 , .nude_level = 0u, .sexy_level = 0u}, /* 14 */
     { "All girls",          "with purple hair ",      "drink 2x"           , .nude_level = 0u, .sexy_level = 0u}, /* 15 */
     { "All girls",          "with glasses ",          "drink 2x"           , .nude_level = 0u, .sexy_level = 0u}, /* 16 */
     { "Choose 1 girl",      "that drinks 2x",        NULL                  , .nude_level = 0u, .sexy_level = 0u}, /* 17 */
     { "All girls",          "wearing black",         "drink 2x"            , .nude_level = 0u, .sexy_level = 0u}, /* 18 */

     /* Strange cases, where somebody is playing on easy mode, but with maximum nudity. */
     {  "All girls take off"        , "one item of",             "clothing"   ,  .nude_level = 3u, .sexy_level = 0u   },
     {  "All girls take off"        , "one item of",             "clothing"   ,  .nude_level = 3u, .sexy_level = 0u   },
     {  "All blondes take off"      , "one item of",             "clothing"   ,  .nude_level = 3u, .sexy_level = 0u   },
     {  "All brunettes take off"    , "one item of",             "clothing"   ,  .nude_level = 3u, .sexy_level = 0u   },
     {  "All redheads"              , "one item of",             "clothing"   ,  .nude_level = 3u, .sexy_level = 0u   },

     /* Strange cases, where somebody is playing on easy mode, but with maximum sexyness */
     {  "Girl closest to"           , "machine spins",         "the bottle"      ,  .nude_level = 0u, .sexy_level = 3u},
     {  "All girls find"            , "somebody to",           "kiss"            ,  .nude_level = 0u, .sexy_level = 3u},
     {  "One girls must"            , "make out with",         "one Migur"       ,  .nude_level = 0u, .sexy_level = 3u},
};
Private U8 priv_TextArrayGirlsLevel1_counter[NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel1)];

/* Easy tasks. */
Private const Task_T priv_TextArrayGuysLevel1[] =
{
     {  NULL                    , "Only guys drink",            NULL          , .nude_level = 0u, .sexy_level = 0u}, /* 1  */
     {  "Guys drink"            , "without",                 "using hands"    , .nude_level = 0u, .sexy_level = 1u}, /* 2  */
     {  "The toughest guy"      , "drinks 3x",                  NULL          , .nude_level = 0u, .sexy_level = 0u}, /* 3  */
     {  "The biggest playboy"   , "drinks 3x",                  NULL          , .nude_level = 0u, .sexy_level = 0u}, /* 4  */
     {  NULL                    , "Guys must sing",         "a song together" , .nude_level = 0u, .sexy_level = 0u}, /* 5  */
     {  "Last guy to put his"   , "finger on his nose",        "drinks 2x"    , .nude_level = 0u, .sexy_level = 0u}, /* 6  */
     {  "Choose one guy"        , "who drinks 3x ",             NULL          , .nude_level = 0u, .sexy_level = 0u}, /* 7  */
     {  "All guys"              , "drop and do 10  ",        "pushups"        , .nude_level = 0u, .sexy_level = 1u}, /* 8  */
     {  "All guys with"         , "a six-pack ",             "drink 3x"       , .nude_level = 0u, .sexy_level = 0u}, /* 9  */
     {  "The most wasted"       , "guy drinks",              "water-shot"     , .nude_level = 0u, .sexy_level = 0u}, /* 10 */
     {  "Guys with"             , "hair gel",                "drink 3x"       , .nude_level = 0u, .sexy_level = 0u}, /* 11 */
     {  "Single Guys "          , "drink vodka",             NULL             , .nude_level = 0u, .sexy_level = 0u}, /* 12 */
     {  "The youngest guy"      , "drinks 2x",               NULL             , .nude_level = 0u, .sexy_level = 0u}, /* 14 */
     {  "All guys that"         , "are in the army",         "drink 3x"       , .nude_level = 0u, .sexy_level = 0u}, /* 15 */
     {  "All guys",               "with glasses ",           "drink 2x"       , .nude_level = 0u, .sexy_level = 0u}, /* 16 */
     {  "One guy",                "must say a toast",        "that rhymes"    , .nude_level = 0u, .sexy_level = 0u}, /* 17 */
     { "All guys whose" ,         "name starts with",     "S drinks 2x"       , .nude_level = 0u, .sexy_level = 0u}, /* 18 */

/* Tasks that need criteria. */
/* Strange cases, where somebody is playing on easy mode, but with maximum nudity. */
     {  "All guys take off"        , "one item of",             "clothing"      ,  .nude_level = 3u, .sexy_level = 0u  },
     {  "All guys take off"        , "one item of",             "clothing"      ,  .nude_level = 3u, .sexy_level = 0u  },
     {  "All guys take off"        , "one item of",             "clothing"      ,  .nude_level = 3u, .sexy_level = 0u  },
     {  "All guys take off"        , "their shirts",            NULL            ,  .nude_level = 3u, .sexy_level = 0u  },
     {  "All guys who have"        , "wanked in past 2 days",   "take off shirt",  .nude_level = 3u, .sexy_level = 1u  },


/* Strange cases, where somebody is playing on easy mode, but with maximum sexyness */
     {  "Guy closest to"           , "machine spins",         "the bottle"      ,  .nude_level = 0u, .sexy_level = 3u  },
     {  "Guys drink vodka" ,       "Girls have to",           "kiss"            ,  .nude_level = 0u, .sexy_level = 3u  },
};
Private U8 priv_TextArrayGuysLevel1_counter[NUMBER_OF_ITEMS(priv_TextArrayGuysLevel1)];


/* Easy Tasks */
Private const Task_T priv_TextArrayAllLevel1[] =
{
/* Casual tasks : These should not contain any extra criteria. */
     {  NULL                    , "Everybody drink 2x",            NULL        ,  .nude_level = 0u, .sexy_level = 0u    }, /* 1  */
     {  "Everybody drinks"      , "without using",              "hands"        ,  .nude_level = 0u, .sexy_level = 0u    }, /* 2  */
     {  "Most sober person"     , "drinks 3x",                     NULL        ,  .nude_level = 0u, .sexy_level = 0u    }, /* 3  */
     {  "Grab your bottles"     , "and do a ",              "waterfall"        ,  .nude_level = 0u, .sexy_level = 0u    }, /* 4  */
     {  "Those in relationship" , "drink  2x",                     NULL        ,  .nude_level = 0u, .sexy_level = 0u    }, /* 5  */
     {  "Those who are single"  , "drink  2x",                     NULL        ,  .nude_level = 0u, .sexy_level = 0u    }, /* 6  */
     {  "Vodka round"           , "for everybody",                 NULL        ,  .nude_level = 0u, .sexy_level = 0u    }, /* 7  */
     {  "Who is closest to "    , "the machine must",    "tell dirty joke"     ,  .nude_level = 0u, .sexy_level = 0u    }, /* 8  */
     {  "Everybody who is "     , "wearing sweaters",    "drink 2x"            ,  .nude_level = 0u, .sexy_level = 0u    }, /* 9  */
     {  "Rock-Paper-Scissors "  , "tournament in 60 sec", "loser drinks vodka" ,  .nude_level = 0u, .sexy_level = 0u    }, /* 10 */
     {  "Last person to"        , "put hand on their", "nose drinks 3x"        ,  .nude_level = 0u, .sexy_level = 0u    }, /* 11 */

/* Tasks that need criteria. */
     {  "Everyone takes off"     , "one sock",                     NULL       ,   .nude_level = 1u, .sexy_level = 0u   },
     {  "Those that have"        , "french-kissed today",    "drink 3x"       ,   .nude_level = 0u, .sexy_level = 1u   },
     {  "Everybody who has"      , "kissed a Migur",         "drinks vodka"   ,   .nude_level = 0u, .sexy_level = 1u   },
     {  "One closest to machine" , "gets slapped on",        "the ass"        ,   .nude_level = 0u, .sexy_level = 2u   },


/* Strange cases, where somebody is playing on easy mode, but with maximum nudity. */
     {  "Everyone takes off"     , "one item of",             "clothing"      ,   .nude_level = 3u, .sexy_level = 0u  },
     {  "Everyone takes off"     , "one item of",             "clothing"      ,   .nude_level = 3u, .sexy_level = 0u  },
     {  "Everyone takes off"     , "one item of",             "clothing"      ,   .nude_level = 3u, .sexy_level = 0u  },
     {  "Everyone takes off"     , "one item of",             "clothing"      ,   .nude_level = 3u, .sexy_level = 0u  },

/* Strange cases, where somebody is playing on easy mode, but with maximum sexyness */
     {  "One closest to"         , "machine spins",         "the bottle"      ,   .nude_level = 0u, .sexy_level = 3u  },
     {  "Everybody finds"        , "somebody to",           "kiss"            ,   .nude_level = 0u, .sexy_level = 3u  },
     {  "Guys drink vodka" ,       "Girls have to",         "kiss"            ,   .nude_level = 0u, .sexy_level = 3u  },
};

Private U8 priv_TextArrayAllLevel1_counter[NUMBER_OF_ITEMS(priv_TextArrayAllLevel1)];


/* Medium tasks */
Private const Task_T priv_TextArrayGirlsLevel2[] =
{
 /* Casual tasks : These should not contain any extra criteria. */
     { "Girls",              "I have never ever"     , NULL                ,   .nude_level = 0u, .sexy_level = 0u }, /* 1  */
     { "All couples  ",      "drink 2x"              , NULL                ,   .nude_level = 0u, .sexy_level = 0u }, /* 2  */
     { "All girls whose" ,   "name starts with",     "S drinks 2x"         ,   .nude_level = 0u, .sexy_level = 0u }, /* 3  */
     { "All bad girls",      "drink 2x ",              NULL                ,   .nude_level = 0u, .sexy_level = 0u }, /* 4  */
     { "All female" ,        "organisers",           "drink 2x"            ,   .nude_level = 0u, .sexy_level = 0u }, /* 5  */
     { "All good girls",     "drink 2x ",              NULL                ,   .nude_level = 0u, .sexy_level = 0u }, /* 6  */
     { "Last girl to" ,      "put finger on",        "nose drinks 2x"      ,   .nude_level = 0u, .sexy_level = 0u }, /* 7  */
     { "All girls that",     "have boyfriends ",     "drink 2x"            ,   .nude_level = 0u, .sexy_level = 0u }, /* 8  */
     { "All blondes",        "drink vodka ",              NULL             ,   .nude_level = 0u, .sexy_level = 0u }, /* 9  */
     { "All brunettes",      "drink vodka ",              NULL             ,   .nude_level = 0u, .sexy_level = 0u }, /* 10 */
     { "All girls whose" ,   "name starts with",     "L drinks 2x"         ,   .nude_level = 0u, .sexy_level = 0u }, /* 11 */
     { "All girls with" ,    "blue eyes",            "drink 2x"            ,   .nude_level = 0u, .sexy_level = 0u }, /* 12 */
     { "All girls with" ,    "a pony tail",          "drink 2x"            ,   .nude_level = 0u, .sexy_level = 0u }, /* 13 */

 /* Tasks that need criteria. */
     { "The girl with ",     "the sexiest voice"     , "drinks 2x "        ,   .nude_level = 0u, .sexy_level = 1u }, /* 14  */
     { "The girl with  ",    "the largest boobs"     , "drinks 2x"         ,   .nude_level = 0u, .sexy_level = 1u }, /* 15  */
     { "The girls with" ,    "the shortest skirt",   "drinks 2x"           ,   .nude_level = 1u, .sexy_level = 0u }, /* 16  */
     { "All girls lose" ,    "1 item of ",   "clothing"                    ,   .nude_level = 2u, .sexy_level = 0u }, /* 17  */
     { "All girls lose" ,    "1 item of ",   "clothing"                    ,   .nude_level = 2u, .sexy_level = 0u }, /* 18  */
     { "All girls lose" ,    "1 item of ",   "clothing"                    ,   .nude_level = 2u, .sexy_level = 0u }, /* 19  */
     { "All girls lose" ,    "1 item of ",   "clothing"                    ,   .nude_level = 2u, .sexy_level = 0u }, /* 20  */
     { "All girls lose" ,    "2 items of ",  "clothing"                    ,   .nude_level = 3u, .sexy_level = 0u }, /* 21  */
     { "All girls lose" ,    "2 items of ",  "clothing"                    ,   .nude_level = 3u, .sexy_level = 0u }, /* 22  */

     { "Girls who are",      "former virgins",       "drink 2x"            ,   .nude_level = 0u, .sexy_level = 1u }, /* 23 */
     { "Girls must purr",    "like a kitten",        "after drinking!"     ,   .nude_level = 0u, .sexy_level = 1u }, /* 24 */
     { "All girls who" ,     "kissed a girl",        "today drink 2x"      ,   .nude_level = 0u, .sexy_level = 0u }, /* 25 */
     { "All girls shall" ,   "slap a guy and",       "then drink 2x"       ,   .nude_level = 0u, .sexy_level = 2u }, /* 26 */
     { "All girls must" ,    "drink while",          "dancing"             ,   .nude_level = 0u, .sexy_level = 2u }, /* 27 */
     { "All girls must" ,    "drink while",          "twerking"            ,   .nude_level = 0u, .sexy_level = 3u }, /* 28 */
     { "At least two girls" ,"must make out",        "before drinking"     ,   .nude_level = 0u, .sexy_level = 3u }, /* 29 */

};

Private U8 priv_TextArrayGirlsLevel2_counter[NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel2)];


/* Medium tasks. */
Private const Task_T priv_TextArrayGuysLevel2[] =
{
 /* Casual tasks : These should not contain any extra criteria. */
     {  "The guy with the"      , "biggest balls",            "drinks vodka"        ,    .nude_level = 0u, .sexy_level = 0u },  /* 1  */
     {  "Guys"                  , "Never have I ever",          NULL                ,    .nude_level = 0u, .sexy_level = 0u },  /* 2  */
     {  "All guys whose"        , "name starts with",         "A drinks 2x"         ,    .nude_level = 0u, .sexy_level = 0u },  /* 3  */
     {  "All couples  "         , "drink 2x",                   NULL                ,    .nude_level = 0u, .sexy_level = 0u },  /* 4  */
     {  "All male"              , "organisers",               "drink 2x"            ,    .nude_level = 0u, .sexy_level = 0u },  /* 5  */
     {  "The guy with"          , "the biggest beer",         "belly drinks 2x"     ,    .nude_level = 0u, .sexy_level = 0u },  /* 6  */
     {  "All guys with"         , "beards",                   "drink 2x"            ,    .nude_level = 0u, .sexy_level = 0u },  /* 7  */
     {  "All former senors",      "drink vodka",               NULL                 ,    .nude_level = 0u, .sexy_level = 0u },  /* 8  */
     {  "Guys must do",           "10 squats",                "before drinking"     ,    .nude_level = 0u, .sexy_level = 0u },  /* 9  */
     {  "Guys that love",         "heavy metal",              "drink vodka"         ,    .nude_level = 0u, .sexy_level = 0u },  /* 10 */
     {  "The guy who resembles",  "Elvis most",               "drinks 3x"           ,    .nude_level = 0u, .sexy_level = 0u },  /* 11 */
     {  "The guy who resembles",  "Kurt Cobain most",         "drinks 3x"           ,    .nude_level = 0u, .sexy_level = 0u },  /* 12 */

     {  "Guys must drink",       "while pretending to",        "be Italian"         ,    .nude_level = 0u, .sexy_level = 0u },  /* 12 */

     {  "The guy with the ",      "least amount of hair",     "drinks 3x"           ,    .nude_level = 0u, .sexy_level = 0u },  /* 13 */
     {  "Guys must drink while ", "beatboxing the ",          "Pornhub theme"       ,    .nude_level = 0u, .sexy_level = 1u },  /* 14 */
     {  "Guys must drink while ", "singing ",                 "Barbie Girl!"        ,    .nude_level = 0u, .sexy_level = 1u },  /* 15 */
     {  "All guys who like "    , "bad girls ",               "drink 3x"            ,    .nude_level = 0u, .sexy_level = 0u },  /* 16 */
     {  "All guys who like "    , "good girls ",              "drink 3x"            ,    .nude_level = 0u, .sexy_level = 0u },  /* 17 */

 /* Tasks that need criteria. */
     {  "All guys must come"    , " up with a pickup ",       "line in 60 seconds"  ,   .nude_level = 0u, .sexy_level = 1u },  /* 18 */
     {  "All guys lose"         , "One Item of Clothing",       NULL                ,   .nude_level = 2u, .sexy_level = 0u },  /* 19 */
     {  "All guys lose"         , "One Item of Clothing",       NULL                ,   .nude_level = 2u, .sexy_level = 0u },  /* 20 */
     {  "All guys lose"         , "two items of Clothing",      NULL                ,   .nude_level = 3u, .sexy_level = 0u },  /* 21 */
     {  "All guys lose"         , "two items of Clothing",      NULL                ,   .nude_level = 3u, .sexy_level = 0u },  /* 22 */
     {  "All guys who are",       "virgins drink 2x",         "and best of luck!"   ,   .nude_level = 0u, .sexy_level = 1u },  /* 23 */
     {  "Guys who have",          "kissed a dude",            "drink vodka!"        ,   .nude_level = 0u, .sexy_level = 0u },  /* 24 */
     {  "Guys must",              "say meow",                 "after drinking!"     ,   .nude_level = 0u, .sexy_level = 1u },  /* 25 */
     {  "Guys say No Homo!",      "Look each other in",       "the eyes and drink"  ,   .nude_level = 0u, .sexy_level = 0u },  /* 26 */
     {  "All guys who have ",     "shaved balls",             "drink vodka"         ,   .nude_level = 0u, .sexy_level = 2u },  /* 27 */
     {  "All guys must" ,         "drink while",             "dancing"             ,    .nude_level = 0u, .sexy_level = 2u },  /* 28 */
     {  "All guys must" ,         "drink while",             "holding hands"       ,    .nude_level = 0u, .sexy_level = 1u },  /* 29 */
     {  "All guys who" ,          "own a vibrator",          "drink 3x"            ,    .nude_level = 0u, .sexy_level = 1u },  /* 30 */

};
Private U8 priv_TextArrayGuysLevel2_counter[NUMBER_OF_ITEMS(priv_TextArrayGuysLevel2)];

/* Medium difficulty tasks */
Private const Task_T priv_TextArrayAllLevel2[] =
{
 /* Casual tasks : These should not contain any extra criteria. */
     {  NULL                    , "Everybody drink 2x",            NULL       ,   .nude_level = 0u, .sexy_level = 0u   }, /* 1  */
     {  NULL                    , "Everybody drink 3x",            NULL       ,   .nude_level = 0u, .sexy_level = 0u   }, /* 2  */
     {  "Everybody drinks"      , "1 shot of vodka",     "and 1 shot of beer" ,   .nude_level = 0u, .sexy_level = 0u   }, /* 3  */
     {  "Everybody drinks"      , "1 shot of vodka",    "or 5x shots of beer" ,   .nude_level = 0u, .sexy_level = 0u   }, /* 4  */
     {  "Everybody who is "     , "already drunk",          "drinks 3x"       ,   .nude_level = 0u, .sexy_level = 0u   }, /* 5  */
     {  "Whoever is the most"   , "drunk (probably Mirtel)","drinks 3x"       ,   .nude_level = 0u, .sexy_level = 0u   }, /* 6  */
     {  "Whoever is closest to" , "the machine must", "distribute 6 beershots",   .nude_level = 0u, .sexy_level = 0u   }, /* 7  */

 /* Tasks that need criteria. */
     {  "Everybody takes off"   , "1 Item of Clothing", "or drinks vodka"    ,    .nude_level = 1u, .sexy_level = 0u },  /* 8  */
     {  "Everybody takes off"   , "1 Item of Clothing",          NULL        ,    .nude_level = 2u, .sexy_level = 0u },  /* 9  */
     {  "Everybody takes off"   , "1 Item of Clothing", "or drinks vodka"    ,    .nude_level = 1u, .sexy_level = 0u },  /* 10  */
     {  "Everybody takes off"   , "1 Item of Clothing",          NULL        ,    .nude_level = 2u, .sexy_level = 0u },  /* 11  */

     {  "Everybody who is"      , "horny drinks",                   "3x"      ,   .nude_level = 0u, .sexy_level = 1u }, /* 12  */
     {  "Everybody who has"     , "ever watched gay",   "porn drinks 3x"      ,   .nude_level = 0u, .sexy_level = 2u }, /* 13  */
     {  "Everybody who has"     , "watched porn",       "today drinks 3x"     ,   .nude_level = 0u, .sexy_level = 1u }, /* 14  */

     {  "Everybody drinks"      , "while slapping",       "their own ass"     ,   .nude_level = 0u, .sexy_level = 2u },  /* 15  */
     {  "Everybody drinks"      , "while sitting on",     "their hands"       ,   .nude_level = 0u, .sexy_level = 1u },  /* 16  */

     {  "Everybody who has"     , "gotten an orgasm",     "today drinks 3x"   ,   .nude_level = 0u, .sexy_level = 2u },  /* 17  */
     {  "Every who has ever"    , "had a penis in their", "mouth drink 3x"    ,   .nude_level = 0u, .sexy_level = 2u },  /* 18  */

     {  "Everybody drinks"      , "while holding hands",  "in a circle"       ,   .nude_level = 0u, .sexy_level = 1u },  /* 19  */

};

Private U8 priv_TextArrayAllLevel2_counter[NUMBER_OF_ITEMS(priv_TextArrayAllLevel2)];


/* Hard tasks.  */
Private const Task_T priv_TextArrayGirlsLevel3[] =
{
 /* Casual tasks : These should not contain any extra criteria. */
     {  "Vodka round!!!",          "for girls!!!"   ,                   NULL   ,  .nude_level = 0u, .sexy_level = 0u }, /* 1  */
     {  "Girl showing the"     ,   "least cleavage"  ,           "drinks 3x"   ,  .nude_level = 0u, .sexy_level = 0u }, /* 2  */
     {  "Girls can"            ,   "slap one of"  ,              "the guys"    ,  .nude_level = 0u, .sexy_level = 0u }, /* 3  */
     {  "All girls must",          "drink under",                "the table"   ,  .nude_level = 0u, .sexy_level = 0u }, /* 4  */
     {  "Girls must drink",       "while pretending",       "to be latinas"    ,  .nude_level = 0u, .sexy_level = 1u }, /* 5 */
     {  "Girls must do a",        "dance-off",         "losers drink vodka"    ,  .nude_level = 0u, .sexy_level = 1u }, /* 6 */

 /* Tasks that need criteria. */
     {  "Girls must"             , "do a ",                      "sexy dance"  ,  .nude_level = 0u, .sexy_level = 2u }, /* 7    */
     {  "Most naked"           ,   "girl drinks ",               "2x"          ,  .nude_level = 1u, .sexy_level = 0u }, /* 8    */
     {  "All girls lose"       ,   "two items of ",              "clothing"    ,  .nude_level = 2u, .sexy_level = 0u }, /* 9    */
     {  "Girls with"           ,   ">5 flags&numbers",           "drink 3x"    ,  .nude_level = 0u, .sexy_level = 1u }, /* 10   */
     {  "Girls who've had"     ,   "sex with 1 of the",  "players drink 2x"    ,  .nude_level = 0u, .sexy_level = 1u }, /* 11   */
     {  "Girls do"             ,   "bodyshots!!!",                     NULL    ,  .nude_level = 0u, .sexy_level = 2u }, /* 12   */
     {  "The last girl"        ,   "to finish shot"  ,    "loses 1 clothing"   ,  .nude_level = 2u, .sexy_level = 0u }, /* 13   */
     {  "The last girl"        ,   "to finish shot"  ,    "loses 1 clothing"   ,  .nude_level = 2u, .sexy_level = 0u }, /* 14   */
     {  "Girl showing the"     ,   "most cleavage"  ,           "drinks 3x"    ,  .nude_level = 1u, .sexy_level = 1u }, /* 15   */
     {  "Girls: 1 shot for"    ,   "each guy you slept"  , "with this year"   ,  .nude_level = 0u, .sexy_level = 2u }, /* 16   */
     {  "All girls with"       ,   "black underwear"      , "drink vodka"      ,  .nude_level = 0u, .sexy_level = 1u }, /* 17   */
     { " Girls must make"      ,   "a naughty"   ,                "toast"      ,  .nude_level = 0u, .sexy_level = 1u }, /* 18   */
     { " Girls must make"      ,   "a kinky"   ,                  "toast"      ,  .nude_level = 0u, .sexy_level = 1u }, /* 19   */
     {  "All girls must"       ,   "drink with hands",    "on their butts"     ,  .nude_level = 0u, .sexy_level = 2u }, /* 20   */
     { "All girls must"        ,   "drink while",          "twerking"          ,  .nude_level = 0u, .sexy_level = 2u }, /* 21   */
};

Private U8 priv_TextArrayGirlsLevel3_counter[NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel3)];


/* Hard tasks.  */
Private const Task_T priv_TextArrayGuysLevel3[] =
{
 /* Casual tasks : These should not contain any extra criteria. */
     {  "Vodka round!!!",        "for guys!!!"   ,                   NULL    ,  .nude_level = 0u, .sexy_level = 0u  }, /* 1 */
     {  "1 guy must",            "drink while"     ,    "upside down"        ,  .nude_level = 0u, .sexy_level = 0u  }, /* 2 */
     {  "All guys must",         "drink under",         "the table"          ,  .nude_level = 0u, .sexy_level = 0u  }, /* 3 */
     {  "Army guys",             "do 20 pushups,", "civilians 10 pushups"    ,  .nude_level = 0u, .sexy_level = 0u  }, /* 4 */
     {  "One guy must",          "make a split,",  "or vodka round for all"  ,  .nude_level = 0u, .sexy_level = 1u  }, /* 5 */
     {  "Guys must drink",       "pretending they",  "are finns in sauna"    ,  .nude_level = 0u, .sexy_level = 1u  }, /* 6 */
     {  "Guys who have",         "large cocks",         "drink vodka"        ,  .nude_level = 0u, .sexy_level = 1u  }, /* 6 */

 /* Tasks that need criteria. */
     {  "Guys must"            , "do a ",                      "sexy dance"  ,      .nude_level = 0u, .sexy_level = 2u  }, /* 7  */
     {  "Most naked"           , "guy drinks ",                "2x"          ,      .nude_level = 1u, .sexy_level = 0u  }, /* 8  */
     {  "All guys lose"        , "two items of ",         "clothing"         ,      .nude_level = 2u, .sexy_level = 0u  }, /* 9  */
     {  "Guys with"            , ">5 flags&numbers",      "drink 3x"         ,      .nude_level = 0u, .sexy_level = 1u  }, /* 10 */
     {  "Guys who want to"     , "sleep with 1 of",       "the players drink",      .nude_level = 0u, .sexy_level = 1u  }, /* 11 */
     {  "Guys do"              , "bodyshots!!!",                     NULL    ,      .nude_level = 0u, .sexy_level = 2u  }, /* 12 */
     {  "All Guys"             , "with a boner",          "drink 3x"         ,      .nude_level = 0u, .sexy_level = 2u  }, /* 13 */
     {  "The last guy"         , "to finish shot"  ,    "loses 2 clothing"   ,      .nude_level = 2u, .sexy_level = 0u  }, /* 14 */
     {  "All guys drink"       , "while doing"     ,    "pushups"            ,      .nude_level = 0u, .sexy_level = 1u  }, /* 15 */
     {  "For each inch",         "of his D length",     "guys drink 1 shot"  ,      .nude_level = 0u, .sexy_level = 1u  }, /* 16 */
     {  "All guys that",         "saw boobs today",     "drink vodka"        ,      .nude_level = 0u, .sexy_level = 1u  }, /* 17 */
     {  "All guys lose",         "their shirts",        NULL                 ,      .nude_level = 2u, .sexy_level = 0u  }, /* 18 */
     {  "All guys must" ,        "drink while",          "twerking"          ,      .nude_level = 0u, .sexy_level = 2u  }, /* 19 */
     {  "Before guys drink they","must demonstrate their","tongue skills on shot" , .nude_level = 0u, .sexy_level = 2u  }, /* 20 */
     {  "Before guys drink they","must toast their",     "favorite pornstar" ,      .nude_level = 0u, .sexy_level = 1u  }, /* 21 */
     {  "All guys who have"     ,"fingered a girl",      "today, drink 3"    ,      .nude_level = 0u, .sexy_level = 2u  }, /* 22 */

};
Private U8 priv_TextArrayGuysLevel3_counter[NUMBER_OF_ITEMS(priv_TextArrayGuysLevel3)];

/* Hard tasks */
Private const Task_T priv_TextArrayAllLevel3[] =
{
 /* Casual tasks : These should not contain any extra criteria. */
     {  NULL                    , "Everybody drink 2x",            NULL       ,     .nude_level = 0u, .sexy_level = 0u   }, /* 1  */
     {  NULL                    , "Everybody drink 2x",            NULL       ,     .nude_level = 0u, .sexy_level = 0u   }, /* 2  */
     {  "Everybody drinks"      , "vodka",                         NULL       ,     .nude_level = 0u, .sexy_level = 0u   }, /* 3  */
     {  "Everybody drinks"      , "on their knees",                NULL       ,     .nude_level = 0u, .sexy_level = 0u   }, /* 4  */
     {  "Increase sexy slider"  , "or drink vodka",                NULL       ,     .nude_level = 0u, .sexy_level = 0u   }, /* 5  */
     {  "Increase nudity slider", "or drink vodka",                NULL       ,     .nude_level = 0u, .sexy_level = 0u   }, /* 6  */

     /* Tasks that need criteria. */
     {  "Everybody takes off"   , "2 items of Clothing", "or drinks vodka"  ,       .nude_level = 1u, .sexy_level = 0u  }, /* 7  */
     {  "Everybody takes off"   , "1 item of Clothing",          NULL       ,       .nude_level = 2u, .sexy_level = 0u  }, /* 8  */
     {  "Everybody takes off"   , "2 Item of Clothing", "or drinks vodka"   ,       .nude_level = 1u, .sexy_level = 0u  }, /* 9  */
     {  "The player wearing"    , "the most clothes, loses", "3 items of clothing", .nude_level = 3u, .sexy_level = 0u  }, /* 10  */
     {  "The player wearing"    , "the most clothes, loses", "2 items of clothing", .nude_level = 2u, .sexy_level = 0u  }, /* 11  */

     {  "Everybody takes off"   , "one items of Clothing",          NULL      ,     .nude_level = 2u, .sexy_level = 0u  }, /* 12  */
     {  "Everybody takes off"   , "their shirts",                   NULL      ,     .nude_level = 2u, .sexy_level = 0u  }, /* 13  */
     {  "Everybody takes off"   , "their pants",                    NULL      ,     .nude_level = 3u, .sexy_level = 0u  }, /* 14  */

     {  "Everybody drinks"      , "with their hands",     "on their chest"    ,     .nude_level = 0u, .sexy_level = 2u  }, /* 15  */
     {  "Everybody drinks"      , "with their hands",     "behind their back" ,     .nude_level = 0u, .sexy_level = 1u  }, /* 16  */
     {  "Everybody drinks"      , "with their hands",     "on their head"     ,     .nude_level = 0u, .sexy_level = 0u  }, /* 17  */
     {  "Everybody who has"     , "had sex today",                "drinks 3x" ,     .nude_level = 0u, .sexy_level = 1u  }, /* 18  */
     {  "Everybody who is"      , "sitting in their phone",       "drinks 3x" ,     .nude_level = 0u, .sexy_level = 0u  }, /* 19  */
     {  "Everybody who is"      , "sitting in their phone",     "loses pants" ,     .nude_level = 2u, .sexy_level = 0u  }, /* 20  */

     {  "Everybody who has"     , "done butt-stuff",     "drinks vodka"       ,     .nude_level = 0u, .sexy_level = 2u  }, /* 21  */

     {  "One round of rock"     , "paper scissors, losers", "lose 1 item of clothing" , .nude_level = 2u, .sexy_level = 0u  }, /* 22  */
};

Private U8 priv_TextArrayAllLevel3_counter[NUMBER_OF_ITEMS(priv_TextArrayAllLevel3)];


/* Hardcore tasks.  */
Private const Task_T priv_TextArrayGirlsLevel4[] =
{
 /* Casual tasks : These should not contain any extra criteria. (Although considering the difficulty level, they might still be pretty hardcore... ) */
     { "Vodka round!!!" ,   "for girls!!!"        , NULL                ,  .nude_level = 0u, .sexy_level = 0u    }, /*  1   */
     { "Guys give a",       "dare for girls"      , "to do next round"  ,  .nude_level = 0u, .sexy_level = 0u    }, /*  2   */
     { "Girls give a",      "dare for guys"       , "to do next round"  ,  .nude_level = 0u, .sexy_level = 0u    }, /*  3   */
     { "Girl wearing the"  ,"most clothes"        , "drinks 3x"         ,  .nude_level = 0u, .sexy_level = 0u    }, /*  4   */
     { "Girls that still"  ,"have shirts on"      , "must drink 3x"     ,  .nude_level = 0u, .sexy_level = 0u    }, /*  5   */
     { "Girls who are "    ,"wearing black lingerie" ,"must drink 3x"   ,  .nude_level = 0u, .sexy_level = 0u    }, /*  6   */
     { "Girls wearing strings " ,"must drink 3x " ,"or remove strings"   , .nude_level = 0u, .sexy_level = 0u    }, /*  7   */
     { "One girl must",     "make a split or,"    , "vodka round for all" ,.nude_level = 0u, .sexy_level = 1u    }, /*  8   */
     { "Girls, if the carpet","does not match the", "drapes, drink vodka" ,.nude_level = 0u, .sexy_level = 2u    }, /*  8   */

 /* Tasks that need criteria. */
     { "1 girl must",       "do a lapdance"       , "to one guy"        , .nude_level = 0u, .sexy_level = 3u     }, /*  9  */
     { "All girls",         "lose 1 item"         , "of clothing"       , .nude_level = 2u, .sexy_level = 0u     }, /*  10  */
     { "2 girls",           "make out"            , "or drink vodka"    , .nude_level = 0u, .sexy_level = 0u     }, /*  11  */
     { "All clean",         "shaven girls"        , "drink 1x"          , .nude_level = 0u, .sexy_level = 1u     }, /*  12  */
     { "All girls",         "who masturbated"     , "today drink 2x"    , .nude_level = 0u, .sexy_level = 1u     }, /*  13  */
     { "Girls must",        "fake an orgasm"      , "or drink 3x"       , .nude_level = 0u, .sexy_level = 2u     }, /*  14 */
     { "Girls must",        "take off shirt"      , "or drink 3x"       , .nude_level = 2u, .sexy_level = 0u     }, /*  15  */
     { "Girls  sit",        "on guy's laps"       , "for next round"    , .nude_level = 0u, .sexy_level = 2u     }, /*  16  */
     { "Everybody who",     "isnt't wearing a"    , "bra drinks vodka"  , .nude_level = 1u, .sexy_level = 1u     }, /*  17 */
     { "2 Girls must",      "French-kiss"         , "or drink vodka"    , .nude_level = 0u, .sexy_level = 1u     }, /*  18 */
     { "Girls who swallow", "drink 1x, others"    , "drink vodka"       , .nude_level = 0u, .sexy_level = 1u     }, /*  19 */
     { "Girls must",        "take off bra or"     , "drink vodka"       , .nude_level = 3u, .sexy_level = 0u     }, /*  20 */
     { "All girls squeeze", "their boobs"         , "while drinking"    , .nude_level = 0u, .sexy_level = 2u     }, /*  21 */ /* God I love this machine :D */
     { "Only the girl"     ,"with the biggest"    , "boobs drinks"      , .nude_level = 0u, .sexy_level = 1u     },  /* 22 */
     { "Girls must make"   ,"a toast with"   ,      "a kinky voice"     , .nude_level = 0u, .sexy_level = 1u     },  /* 23 */
     { "Girls drink 1x for", "each day since they", "last masturbated",   .nude_level = 0u, .sexy_level = 3u     },  /* 24 */
     { "All girls who",      "have been in",        "handcuffs drink 3x", .nude_level = 0u, .sexy_level = 1u     },  /* 25 */
     { "All girls who",      "swallow, drink 1x",   "who spit drink 3x",  .nude_level = 0u, .sexy_level = 1u     },  /* 26 */

     { "All girls must drink", "while squeezing",   "their boobs",           .nude_level = 0u, .sexy_level = 2u  },  /* 27 */
     { "All girls must choose", "a girl and ",      "kiss her",              .nude_level = 0u, .sexy_level = 2u  },  /* 28 */
     { "All girls must drink", "while squeezing",   "each others boobs",     .nude_level = 0u, .sexy_level = 2u  },  /* 29 */
     { "The last girl"      , "to finish shot"  ,  "must do striptease"  ,   .nude_level = 3u, .sexy_level = 2u  },  /* 30 */
     { "Unless one girl" ,    "flashes her boobs", "all girls drink vodka",  .nude_level = 3u, .sexy_level = 0u  },  /* 31  */

     { "Girls must drink " ,  "1x for each ",      "sex toy they own"     ,  .nude_level = 0u, .sexy_level = 1u  },  /* 32 */
     { "Girls must treat " ,  "this beershot ",    "like a blowjob"       ,  .nude_level = 0u, .sexy_level = 2u  },  /* 33 */
     { "Girls must drink " ,  "3x if they own ",   "a vibrator"           ,  .nude_level = 0u, .sexy_level = 1u  },  /* 34 */
     { "Girls must drink " ,  "vodka if they own ","a black dildo"        ,  .nude_level = 0u, .sexy_level = 1u  },  /* 35 */
     { "One girl must"     ,  "perform a lapdance","on Mirtel"            ,  .nude_level = 0u, .sexy_level = 3u  },  /* 36 */

     { "In 60 seconds"     ,  "all girls must kiss ",  "every single girl",   .nude_level = 0u, .sexy_level = 3u },  /* 37   */
     { "In 60 seconds",       "all girls must kiss ",  "every single player", .nude_level = 0u, .sexy_level = 3u },  /* 38   */
     { "In 60 seconds",       "all girls must kiss ",  "every single guy"  ,  .nude_level = 0u, .sexy_level = 3u },  /* 39   */

     { "All girls",           "lose 1 item",  "of clothing"  ,  .nude_level = 1u, .sexy_level = 0u },  /* 39   */
     { "All girls",           "lose 1 item",  "of clothing"  ,  .nude_level = 1u, .sexy_level = 0u },  /* 39   */
     { "All girls",           "lose 1 item",  "of clothing"  ,  .nude_level = 1u, .sexy_level = 0u },  /* 39   */
     { "All girls",           "lose 1 item",  "of clothing"  ,  .nude_level = 1u, .sexy_level = 0u },  /* 39   */
     { "All girls",           "lose 1 item",  "of clothing"  ,  .nude_level = 1u, .sexy_level = 0u },  /* 39   */

     { "All girls",           "lose 2 items",  "of clothing"  ,  .nude_level = 2u, .sexy_level = 0u },  /* 39   */
     { "All girls",           "lose 2 items",  "of clothing"  ,  .nude_level = 2u, .sexy_level = 0u },  /* 39   */
     { "All girls",           "lose 2 items",  "of clothing"  ,  .nude_level = 2u, .sexy_level = 0u },  /* 39   */
     { "All girls",           "lose 2 items",  "of clothing"  ,  .nude_level = 2u, .sexy_level = 0u },  /* 39   */
     { "All girls",           "lose 2 items",  "of clothing"  ,  .nude_level = 2u, .sexy_level = 0u },  /* 39   */

};
Private U8 priv_TextArrayGirlsLevel4_counter[NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel4)];

/* Hardcore tasks */
Private const Task_T priv_TextArrayGuysLevel4[] =
{
 /* Casual tasks : These should not contain any extra criteria. (Although considering the difficulty level, they might still be pretty hardcore... ) */
     { "Vodka round!!!" ,  "for guys!!!"       ,   NULL                ,  .nude_level = 0u, .sexy_level = 0u,        },  /*  1  */
     { "Girls give a",     "dare for guys"     ,   "to do next round"  ,  .nude_level = 0u, .sexy_level = 0u,        },  /*  2  */
     { "Guys",             "drink without"     ,   "using hands"       ,  .nude_level = 0u, .sexy_level = 0u,        },  /*  3 */
     { "Guy wearing the"  ,"least clothes"        ,"drinks 3x"         ,  .nude_level = 0u, .sexy_level = 0u,        },  /*  4 */
     { "Guy who is",       "most wasted"       ,   "does a vodka"      ,  .nude_level = 0u, .sexy_level = 0u,        },  /*  5  */
     { "All guys who ",    "touched boobs"     ,   "today drink 3x"    ,  .nude_level = 0u, .sexy_level = 1u,        },  /*  6  */

 /* Tasks that need criteria. */
     { "2 guys",           "make out"          ,   "or drink vodka"    ,  .nude_level = 0u, .sexy_level = 1u,      },  /*  7  */
     { "All guys",         "who wanked"        ,   "today drink 3x"    ,  .nude_level = 0u, .sexy_level = 2u,      },  /*  8  */
     { "Guys must",        "fake an orgasm"    ,   "or drink vodka"    ,  .nude_level = 0u, .sexy_level = 2u,      },  /*  9  */
     { "Guys must name",   "20 sex positions"  ,   "or drink vodka"    ,  .nude_level = 0u, .sexy_level = 1u,      },  /*  10  */
     { "Next round guys",  "do bodyshots"      ,   "from the girls"    ,  .nude_level = 0u, .sexy_level = 3u,      },  /*  11  */
     { "All guys",         "get a spanking"    ,   "from the girls"    ,  .nude_level = 0u, .sexy_level = 2u,      },  /*  12  */
     { "All guys",         "lose"              ,   "their pants"       ,  .nude_level = 2u, .sexy_level = 0u,      },  /*  13 */
     { "All guys",         "lose one item"     ,   "of clothing"       ,  .nude_level = 2u, .sexy_level = 0u,      },  /*  14 */
     { "All guys",         "lose one item"     ,   "of clothing"       ,  .nude_level = 2u, .sexy_level = 0u,      },  /*  15 */
     { "All guys",         "lose two items"    ,   "of clothing"       ,  .nude_level = 2u, .sexy_level = 0u,      },  /*  16 */
     { "All guys",         "lose two items"    ,   "of clothing"       ,  .nude_level = 2u, .sexy_level = 0u,      },  /*  17 */
     { "All guys",         "lose three items"  ,   "of clothing"       ,  .nude_level = 3u, .sexy_level = 0u,      },  /*  18 */
     { "All guys",         "lose three items"  ,   "of clothing"       ,  .nude_level = 3u, .sexy_level = 0u,      },  /*  19 */
     { "All guys",         "lose three items"  ,   "of clothing"       ,  .nude_level = 3u, .sexy_level = 0u,      },  /*  20 */
     { "Guys drink 1x for","each day since they" , "last masturbated",    .nude_level = 0u, .sexy_level = 1u       },  /*  21  */

     { "The tallest guy" , "and the shortest guy", "must switch shirts",  .nude_level = 1u, .sexy_level = 1u,      }, /*   22 */

     { "Guys that haven't","eaten pussy in 30"      ,"days drink vodka"         , .nude_level = 0u, .sexy_level = 1u,   },  /*  23 */
     { "2 guys must"      ,"make out or"            ,"drink vodka"              , .nude_level = 0u, .sexy_level = 0u,   },  /*  24 */
     { "All guys must say","one girl from the world" ,"they would sleep with"   , .nude_level = 0u, .sexy_level = 1u,   },  /*  25 */
     { "All guys must say","one guy from the room" ,"they would sleep with"     , .nude_level = 0u, .sexy_level = 1u,   },  /*  26 */

     { "Guys drink 1x for",     "each day since they"   ,"last masturbated"  ,    .nude_level = 0u, .sexy_level = 2u    },  /*  27  */
     { "All guys must drink",  "while touching",         "their crotch",          .nude_level = 0u, .sexy_level = 2u    },  /*  28  */
     { "All guys must drink",  "while touching",         "each others' crotch",   .nude_level = 0u, .sexy_level = 2u    },  /*  29  */
     { "The last guy",         "to finish shot"  ,  "must do striptease"  ,       .nude_level = 3u, .sexy_level = 2u    },  /*  30  */

     { "All guys must drink","with their asses ",  "bare"  ,                      .nude_level = 3u, .sexy_level = 2u    },  /* 31   */
     { "The bravest guy",   "must get naked! "   ,  "No clothes!"  ,              .nude_level = 3u, .sexy_level = 1u    },  /* 32   */

     { "In 60 seconds",     "all guys must kiss ",  "every single girl"  ,        .nude_level = 0u, .sexy_level = 3u    },  /* 33   */
     { "In 60 seconds",     "all guys must kiss ",  "every single player"  ,      .nude_level = 0u, .sexy_level = 3u    },  /* 34   */
     { "In 60 seconds",     "all guys must kiss ",  "every single guy"  ,         .nude_level = 0u, .sexy_level = 3u    },  /* 35   */
     { "Guys who have jerked", "off dreaming of a",  "girl here, drink vodka"   , .nude_level = 0u, .sexy_level = 3u    },  /* 36   */
     { "Guy closest to machine", "must mimic closest girl",  "in bed, while drinking"   , .nude_level = 0u, .sexy_level = 3u    },  /* 37   */
};

Private U8 priv_TextArrayGuysLevel4_counter[NUMBER_OF_ITEMS(priv_TextArrayGuysLevel4)];

/* Hardcore tasks */
Private const Task_T priv_TextArrayAllLevel4[] =
{
 /* Casual tasks : These should not contain any extra criteria. (Although considering the difficulty level, they might still be pretty hardcore... ) */

     {  NULL                    , "Everybody drink 2x",            NULL       ,  .nude_level = 0u, .sexy_level = 0u    }, /* 1  */
     {  NULL                    , "Everybody drinks",            "vodka"      ,  .nude_level = 0u, .sexy_level = 0u    }, /* 2  */
     {  NULL                    , "Everybody drinks",            "vodka"      ,  .nude_level = 0u, .sexy_level = 0u    }, /* 3  */
     {  NULL                    , "Everybody drink 3x",            NULL       ,  .nude_level = 0u, .sexy_level = 0u    }, /* 4  */
 /* Tasks that need criteria. */

     {  "Everybody who has "    , "masturbated today",       "drinks vodka"   ,  .nude_level = 0u, .sexy_level = 2u    }, /* 5  */
     {  "All who have touched " , "touched a penis in",     "24h drinks vodka",  .nude_level = 0u, .sexy_level = 2u    }, /* 6  */

     {  "Everybody must name"   , "craziest place they ",   "had sex at"      ,  .nude_level = 0u, .sexy_level = 1u    }, /* 7  */
     {  "Everybody must kiss"   , "a member of the",        "opposite sex"    ,  .nude_level = 0u, .sexy_level = 2u    }, /* 8  */
     {  "Everybody must kiss"   , "a member of the",        "same sex"        ,  .nude_level = 0u, .sexy_level = 2u    }, /* 9  */
     {  "Everybody must spank"  , "the player on the",      "left, and drink" ,  .nude_level = 0u, .sexy_level = 2u    }, /* 10  */

     {  "Everybody takes off"   , "two items of Clothing", "or drinks vodka"  ,  .nude_level = 1u, .sexy_level = 0u  }, /* 11  */
     {  "Everybody takes off"   , "one item of Clothing",          NULL       ,  .nude_level = 1u, .sexy_level = 0u  }, /* 12  */
     {  "Everybody takes off"   , "two Item of Clothing", "or drinks vodka"   ,  .nude_level = 1u, .sexy_level = 0u  }, /* 13 */
     {  "Everybody takes off"   , "one items of Clothing",          NULL      ,  .nude_level = 1u, .sexy_level = 0u  }, /* 14  */
     {  "Everybody takes off"   , "their shirts",                   NULL      ,  .nude_level = 1u, .sexy_level = 0u  }, /* 15  */
     {  "Everybody takes off"   , "their pants",                    NULL      ,  .nude_level = 2u, .sexy_level = 0u  }, /* 16  */

     {  "Everybody drinks"      , "with their hands",     "on their chest"    ,  .nude_level = 0u, .sexy_level = 1u,    }, /* 17  */
     {  "Everybody drinks"      , "with their hands",     "on their butt"     ,  .nude_level = 0u, .sexy_level = 1u,    }, /* 18  */
     {  "Everybody drinks"      , "with their hands",     "in their pants"    ,  .nude_level = 0u, .sexy_level = 3u,    }, /* 19  */

     {  "Everybody must kiss"   , "someone of the ",     "opposite sex"       ,  .nude_level = 0u, .sexy_level = 2u   }, /* 20  */
     {  "Everybody must kiss"   , "someone of the ",     "same sex"           ,  .nude_level = 0u, .sexy_level = 2u   }, /* 21  */
     {  "Everybody must kiss"   , "someone of the ",     "opposite sex"       ,  .nude_level = 0u, .sexy_level = 2u   }, /* 22  */
     {  "Everybody must kiss"   , "someone of the ",     "same sex"           ,  .nude_level = 0u, .sexy_level = 2u   }, /* 23  */

     {  "Everybody drinks"      , "with their hands",     "in their pants"    ,  .nude_level = 0u, .sexy_level = 3u    }, /* 24  */
     {  "Error #0x43F677"       , "Party is getting",     "too fucked up!!!"  ,  .nude_level = 3u, .sexy_level = 3u    }, /* 25  */
};

Private U8 priv_TextArrayAllLevel4_counter[NUMBER_OF_ITEMS(priv_TextArrayAllLevel4)];


Private const Task_Array_Descriptor priv_task_array_desc[NUMBER_OF_TASK_TYPES][4] =
{
     {
        {.task_array = priv_TextArrayGirlsLevel1 , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel1), .counter_array = priv_TextArrayGirlsLevel1_counter},
        {.task_array = priv_TextArrayGirlsLevel2 , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel2), .counter_array = priv_TextArrayGirlsLevel2_counter},
        {.task_array = priv_TextArrayGirlsLevel3 , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel3), .counter_array = priv_TextArrayGirlsLevel3_counter},
        {.task_array = priv_TextArrayGirlsLevel4 , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel4), .counter_array = priv_TextArrayGirlsLevel4_counter},
     },

     {
        {.task_array = priv_TextArrayGuysLevel1  , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel1), .counter_array = priv_TextArrayGuysLevel1_counter},
        {.task_array = priv_TextArrayGuysLevel2  , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel2), .counter_array = priv_TextArrayGuysLevel2_counter},
        {.task_array = priv_TextArrayGuysLevel3  , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel3), .counter_array = priv_TextArrayGuysLevel3_counter},
        {.task_array = priv_TextArrayGuysLevel4  , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayGuysLevel4), .counter_array = priv_TextArrayGuysLevel4_counter},
     },

     {
        {.task_array = priv_TextArrayAllLevel1  , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayAllLevel1), .counter_array = priv_TextArrayAllLevel1_counter},
        {.task_array = priv_TextArrayAllLevel2  , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayAllLevel2), .counter_array = priv_TextArrayAllLevel2_counter},
        {.task_array = priv_TextArrayAllLevel3  , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayAllLevel3), .counter_array = priv_TextArrayAllLevel3_counter},
        {.task_array = priv_TextArrayAllLevel4  , .number_of_array_items = NUMBER_OF_ITEMS(priv_TextArrayAllLevel4), .counter_array = priv_TextArrayAllLevel4_counter},
     },
};


/*****************************************************************************************************
 *
 * Private variable declarations
 *
 *****************************************************************************************************/
Private const Task_T * priv_task_str_ptr;
Private char priv_str_buf[128];
Private SpecialTaskFunc priv_selected_task_ptr;


/*****************************************************************/

/*****************************************************************************************************
 *
 * Public function definitions
 *
 *****************************************************************************************************/

Public void SpecialTasks_init(void)
{
    memset(priv_TextArrayGirlsLevel1_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel1));
    memset(priv_TextArrayGirlsLevel2_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel2));
    memset(priv_TextArrayGirlsLevel3_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel3));
    memset(priv_TextArrayGirlsLevel4_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayGirlsLevel4));

    memset(priv_TextArrayGuysLevel1_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayGuysLevel1));
    memset(priv_TextArrayGuysLevel2_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayGuysLevel2));
    memset(priv_TextArrayGuysLevel3_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayGuysLevel3));
    memset(priv_TextArrayGuysLevel4_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayGuysLevel4));

    memset(priv_TextArrayAllLevel1_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayAllLevel1));
    memset(priv_TextArrayAllLevel2_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayAllLevel2));
    memset(priv_TextArrayAllLevel3_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayAllLevel3));
    memset(priv_TextArrayAllLevel4_counter, 0u, NUMBER_OF_ITEMS(priv_TextArrayAllLevel4));

}


Public void SpecialTask_StringLengthSanityTest(void)
{
    U8 ix;
    U8 task_ix;
    U8 category;
    U16 length;

    for(category = 0u; category < NUMBER_OF_TASK_TYPES; category++)
    {
        for (ix = 0u; ix < 4u; ix++)
        {
            for(task_ix = 0u; task_ix < priv_task_array_desc[category][ix].number_of_array_items; task_ix++)
            {
                if (!isStringFitForDisplay(priv_task_array_desc[category][ix].task_array[task_ix].lower_text, &length))
                {
                    printf( "WARNING! String {%s} at index %d, difficulty %d for type %d is too large for display (%d pixels)\n", priv_task_array_desc[category][ix].task_array[task_ix].lower_text, task_ix, ix, category, length);
                }

                if (!isStringFitForDisplay(priv_task_array_desc[category][ix].task_array[task_ix].middle_text, &length))
                {
                    printf( "WARNING! String {%s} at index %d, difficulty %d for type %d is too large for display (%d pixels)\n", priv_task_array_desc[category][ix].task_array[task_ix].middle_text, task_ix, ix, category, length);
                }

                if (!isStringFitForDisplay(priv_task_array_desc[category][ix].task_array[task_ix].upper_text, &length))
                {
                    printf( "WARNING! String {%s} at index %d, difficulty %d for type %d is too large for display (%d pixels)\n", priv_task_array_desc[category][ix].task_array[task_ix].upper_text, task_ix, ix, category, length);
                }

            }
        }
    }
}

Private Boolean isStringFitForDisplay(const char * str, U16 * length_dest)
{
    Boolean res = TRUE;
    ReplaceStringEscapeChars(str, priv_str_buf);
    *length_dest = LcdWriter_getStringWidth(priv_str_buf, SPECIALTASK_SMALL_FONT);
    if (*length_dest > DISPLAY_WIDTH)
    {
        res = FALSE;
    }

    return res;
}

Public Boolean girlsSpecialTask(U8 sec)
{
    Boolean res = FALSE;
    static U8 test_counter = 0u;

    /* If sec is 0, then we have just begun. */
    if (sec == 0u)
    {
        priv_selected_task_ptr = priv_special_tasks_girls_array[test_counter];
        test_counter++;
        if(test_counter >= NUMBER_OF_ITEMS(priv_special_tasks_girls_array))
        {
            test_counter = 0u;
        }
    }

    res = priv_selected_task_ptr(sec, TASK_FOR_GIRLS);
    return res;
}


Public Boolean guysSpecialTask(U8 sec)
{
    Boolean res = FALSE;
    static U8 test_counter = 0u;

    if (sec == 0u)
    {
        priv_selected_task_ptr = priv_special_tasks_guys_array[test_counter];
        test_counter++;
        if(test_counter >= NUMBER_OF_ITEMS(priv_special_tasks_guys_array))
        {
            test_counter = 0u;
        }
    }

    res = priv_selected_task_ptr(sec, TASK_FOR_GUYS);
    return res;
}

Public Boolean everybodySpecialTask(U8 sec)
{
    Boolean res = FALSE;
    static U8 test_counter = 0u;

    if (sec == 0u)
    {
        priv_selected_task_ptr = priv_special_tasks_everybody_array[test_counter];
        test_counter++;
        if (test_counter >= NUMBER_OF_ITEMS(priv_special_tasks_everybody_array))
        {
            test_counter = 0u;
        }
    }

    res = priv_selected_task_ptr(sec, TASK_FOR_EVERYONE);
    return res;
}



/*****************************************************************************************************
 *
 * Private function definitions
 *
 *****************************************************************************************************/
Private Boolean DrinkTwiceTask(U8 sec, SpecialTaskType type)
{
    Boolean res = FALSE;

    switch(sec)
    {
    case(1u):
       display_clear();
       switch(type)
       {
           case TASK_FOR_GIRLS:
               strcpy(priv_str_buf, "Girls");
               strcat(priv_str_buf, " drink");
               break;
           case TASK_FOR_GUYS:
               strcpy(priv_str_buf, "Guys");
               strcat(priv_str_buf, " drink");
               break;
           case TASK_FOR_EVERYONE:
               strcpy(priv_str_buf, "Everyone");
               strcat(priv_str_buf, " drinks");
               break;
           default:
               break;
       }

       display_drawStringCenter(priv_str_buf, DISPLAY_CENTER, 2u, SPECIALTASK_FONT, FALSE);
       display_drawStringCenter("2x", DISPLAY_CENTER ,20u, SPECIALTASK_FONT, FALSE);
       break;
    case (2u):
        display_drawTimageCenter(&laudur, DISPLAY_CENTER - SMALL_SHOT_INTERVAL, SMALL_SHOT_Y);
       break;
    case (3u):
        display_drawTimageCenter(&laudur, DISPLAY_CENTER + SMALL_SHOT_INTERVAL, SMALL_SHOT_Y);
       break;
    case(10u):
       res = TRUE;
       break;
    default:
        break;
    }

    return res;
}


/*****************************************************************************************************
 *
 * Private function definitions
 *
 *****************************************************************************************************/


Private void ReplaceStringEscapeChars(const char * str, char * dest)
{
    const char * ps = str;

    if (ps == NULL)
    {
        return;
    }

    while(*ps)
    {
        if(*ps == '{')
        {
            if (*(ps+1) == '$' && *(ps+2) == '}' )
            {
                ps+=3;
                dest += addstr(dest, "UNDEFINED");
            }
            else
            {
                *dest = *ps;
                ps++;
                dest++;
                *dest = 0;
            }
        }
        else
        {
            *dest = *ps;
            ps++;
            dest++;
            *dest = 0;
        }
    }
}

/* The sec parameter specifies the current second from the beginning of the task.
 * This function is called cyclically after every second. */
Private Boolean SpecialTaskWithRandomText(U8 sec, SpecialTaskType type)
{
    Boolean res = FALSE;

    int taskLevel;


    switch(type)
    {
        case TASK_FOR_GIRLS:
            taskLevel = pot_getSelectedRange(POTENTIOMETER_GIRLS);
            break;
        case TASK_FOR_GUYS:
            taskLevel = pot_getSelectedRange(POTENTIOMETER_GUYS);
            break;
        case TASK_FOR_EVERYONE:
            taskLevel = (pot_getSelectedRange(POTENTIOMETER_GIRLS) + pot_getSelectedRange(POTENTIOMETER_GUYS)) / 2; /* We just take the average here... */
            break;
        default:
            /* Should not happen. */
            taskLevel = -1;
            break;
    }

    if (taskLevel < 0)
    {
        taskLevel = 0;
    }

    const Task_T * SelectedTaskArray;
    U16 number_of_items;
    U8 * counter_array_ptr;

    SelectedTaskArray = priv_task_array_desc[type][taskLevel].task_array;
    number_of_items = priv_task_array_desc[type][taskLevel].number_of_array_items;
    counter_array_ptr = priv_task_array_desc[type][taskLevel].counter_array;

    switch(sec)
    {
        case(1u):
            {
                priv_task_str_ptr = getRandomTaskFromArray(SelectedTaskArray, number_of_items, counter_array_ptr);
                break;
            }
        case (2u):
            {
                display_clear();
                if (priv_task_str_ptr->upper_text != NULL)
                {
                    ReplaceStringEscapeChars(priv_task_str_ptr->upper_text, priv_str_buf);
                    if (LcdWriter_getStringWidth(priv_str_buf, SPECIALTASK_FONT) <= DISPLAY_WIDTH)
                    {
                        display_drawStringCenter(priv_str_buf, DISPLAY_CENTER, 30u, SPECIALTASK_FONT, FALSE);
                    }
                    else
                    {
                        display_drawStringCenter(priv_str_buf, DISPLAY_CENTER, 30u, SPECIALTASK_SMALL_FONT, FALSE);
                    }
                }
            }
            break;
        case (3u):
            {
                if (priv_task_str_ptr->middle_text != NULL)
                {
                    ReplaceStringEscapeChars(priv_task_str_ptr->middle_text, priv_str_buf);
                    if (LcdWriter_getStringWidth(priv_str_buf, SPECIALTASK_FONT) <= DISPLAY_WIDTH)
                    {
                        display_drawStringCenter(priv_str_buf, DISPLAY_CENTER, 60u, SPECIALTASK_FONT, FALSE);
                    }
                    else
                    {
                        display_drawStringCenter(priv_str_buf, DISPLAY_CENTER, 60u, SPECIALTASK_SMALL_FONT, FALSE);
                    }
                }
            }
            break;
        case(4u):
            {
                if (priv_task_str_ptr->lower_text != NULL)
                {
                    ReplaceStringEscapeChars(priv_task_str_ptr->lower_text, priv_str_buf);
                    if (LcdWriter_getStringWidth(priv_str_buf, SPECIALTASK_FONT) <= DISPLAY_WIDTH)
                    {
                        display_drawStringCenter(priv_str_buf, DISPLAY_CENTER, 90u, SPECIALTASK_FONT, FALSE);
                    }
                    else
                    {
                        display_drawStringCenter(priv_str_buf, DISPLAY_CENTER, 90u, SPECIALTASK_SMALL_FONT, FALSE);
                    }
                }
            }
            break;
        case (8u):
            {
                if ((pot_getSelectedRange(POTENTIOMETER_SEXY_LEVEL) == 0u) && (pot_getSelectedRange(POTENTIOMETER_NUDE_LEVEL) == 0u))
                {
                    display_clear();
                    display_drawStringCenter("Come on guys!", DISPLAY_CENTER, 4u, SPECIALTASK_FONT, FALSE);
                    display_drawStringCenter("don't be pussies!", DISPLAY_CENTER, 23u, SPECIALTASK_FONT, FALSE);
                    display_drawStringCenter("Turn up some sexy!", DISPLAY_CENTER, 43u, SPECIALTASK_FONT, FALSE);
                }
            }
            break;
        case(12u):
            {
                res = TRUE;
            }
            break;
        default:
            break;
    }

    return res;
}

Private const Task_T priv_default_task = {  "Error selecting task!" , "Everybody drink 2x", "just in case" , .nude_level = 0u, .sexy_level = 0u };


/* NOTE : Each array must contain at least one task with a 0,0,0 for extra requirements. Otherwise there will be no tasks here. */
Private const Task_T * getRandomTaskFromArray(const Task_T * array, U8 array_size, U8 *counter_ptr)
{
    U8 ix;
    U8 min_count = 0xffu;

    U8 * index_array;
    U8 available = 0u;
    U16 result_index;

    U8 sexy_level = pot_getSelectedRange(POTENTIOMETER_SEXY_LEVEL);
    U8 nude_level = pot_getSelectedRange(POTENTIOMETER_NUDE_LEVEL);

    index_array = (U8 *)malloc(sizeof(U8) * array_size);

    if (index_array == NULL)
    {
        /* TODO : Review error handling. Currently will be stuck in infinite loop. */
        while(1);
    }

    for (ix = 0u; ix < array_size; ix++)
    {
        if (isTaskEnabled(&array[ix], sexy_level, nude_level))
        {
            if (counter_ptr[ix] < min_count)
            {
                min_count = counter_ptr[ix];
            }
        }
    }

    for (ix = 0u; ix < array_size; ix++)
    {
        if (isTaskEnabled(&array[ix], sexy_level, nude_level))
        {
            if (counter_ptr[ix] <= min_count)
            {
                /* We can use this item. */
                index_array[available] = ix;
                available++;
            }
        }
    }

    if (available > 0u)
    {
        /* So now index_array should contain all the unused indexes. */
        result_index = index_array[generate_random_number(available - 1u)];
    }
    else
    {
        /* Something has gone wrong. */
        free (index_array);
        return &priv_default_task;
    }

    free(index_array);
    counter_ptr[result_index]++;
    return &array[result_index];
}


Private Boolean isTaskEnabled(const Task_T * task_ptr, U8 sexy_level, U8 nude_level)
{
    Boolean res = TRUE;

    if (task_ptr->sexy_level > sexy_level)
    {
        res = FALSE;
    }

    if (task_ptr->nude_level > nude_level)
    {
        res = FALSE;
    }

    return res;
}
