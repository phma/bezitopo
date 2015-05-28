/******************************************************/
/*                                                    */
/* color.h - drawing colors                           */
/*                                                    */
/******************************************************/
/* Colors are stored in two formats: short and int.
 * A short denotes one of 64000 colors (40³) or any of up to 1536 special values
 * (like same as the layer). An int denotes any of 16777216 colors.
 * Colors are as shown on screen, with a black background.
 * When printing, one can do any of the following:
 * • Print as is, wasting lots of black ink.
 * • Exchange white and black, leaving everything else the same.
 * • Invert all colors.
 * • Invert light and dark, leaving hue alone. This is difficult.
 * This operation is done on the 24-bit color.
 */

#define SAMECOLOR 65535

int colorint(unsigned short colorshort);
unsigned short colorshort(int colorint);
