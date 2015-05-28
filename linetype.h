/******************************************************/
/*                                                    */
/* linetype.h - dash patterns and line widths         */
/*                                                    */
/******************************************************/

/* Dash patterns are encoded as a string of bytes, where:
 * 0xff 1024 mm
 * 0x41 1.00 mm
 * 0x00 0.093 mm
 * 0x01 0.097 mm
 * 0x02 0.100 mm
 * 0x54 2.0 mm
 * Line widths are encoded as shorts on a logarithmic scale,
 * with the special value 0 meaning 0.
 */
