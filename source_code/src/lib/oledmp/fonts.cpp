
#include "fonts.h"

font_t fontsHQ[] = {
#ifdef FONT_CHECKBOOK_12
    { checkbook_12, checkbook_12_asciimap, CHECKBOOK_12_HEIGHT },
#endif
#ifdef FONT_CHECKBOOK_14
    { checkbook_14, checkbook_14_asciimap, CHECKBOOK_14_HEIGHT },
#endif
#ifdef FONT_PROFONT_10_100DPI
    { profont_10_100, profont_10_100_asciimap, PROFONT_10_100_HEIGHT },
#endif
#ifdef FONT_PROFONT_10_72DPI
    { profont_10_72, profont_10_72_asciimap, PROFONT_10_72_HEIGHT },
#endif
};

