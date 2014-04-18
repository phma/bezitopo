/******************************************************/
/*                                                    */
/* halton.h - Halton subrandom point generator        */
/*                                                    */
/******************************************************/

/* This class generates Halton sequences. The object has a 64-bit
 * number counting from 0 to 2^32*3^20-1. The number's bits and trits
 * are reversed, and it produces one of two kinds of output:
 * * x and y coordinates, one made from the bits and the other from the trits; or
 * * a single number.
 * The x and y coordinates, if the sequence is counted to its full length,
 * are dense enough to cover Egypt with 15 points per square millimeter.
 * They are used for numerical integration to find volumes.
 * The single number is used to select which subsquare when picking
 * a random point within a boundary.
 */

#include <vector>

extern unsigned short btreversetable[62208];
void initbtreverse();
std::vector<unsigned short> splithalton(unsigned long long n);
unsigned long long btreverselong(unsigned long long n);
