/* Options.hpp */

/* Put the product in full screen mode */
//#define BASIC_DD_FULL_SCREEN

/* If full screen, do fast BMP blitting */
#ifdef BASIC_DD_FULL_SCREEN
#define FAST_BMP_COLORING
#endif

/* Only do this if you want to make sure that bmp is absolutely correct */
//#define BMP_SLOW_DRAWING

#define _NDEBUG

