#include "Fonts.h"

Private const tFont * priv_fonts[NUMBER_OF_FONTS] =
{
     &FontSmall12,   /* FONT_SMALL_FONT_12          */
     &Arial12,       /* FONT_ARIAL_12               */
     &Arial14Bold,   /* FONT_ARIAL_14_BOLD          */
     &FontLargeText, /* FONT_ARIAL_16_BOLD          */
     &ClockFont      /* FONT_TNR_HUGE_NUMBERS       */
};



Public const tFont * font_get_font_ptr(FontType_t font)
{
    if (font < NUMBER_OF_FONTS)
    {
        return priv_fonts[font];
    }
    else
    {
        return NULL;
    }
}

Public U8 font_getCharWidth(char asc, FontType_t font)
{
    const tFont * font_info_ptr = priv_fonts[font];
    char c;

    U8 res = 0u;

    if ((asc >= 0x20u) && (asc <= (0x20u + font_info_ptr->length)))
    {
        c = asc - 0x20u;
        res = font_info_ptr->chars[c].image->width;
    }

    return res;
}

Public U8 font_getFontHeight(FontType_t font)
{
    const tFont * font_info_ptr = priv_fonts[font];
    U8 res = font_info_ptr->chars[0].image->height;

    return res;
}
