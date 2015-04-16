/******************************************************/
/*                                                    */
/* ldecimal.h - lossless decimal representation       */
/*                                                    */
/******************************************************/

#include <string>

std::string ldecimal(double x);
/* Returns the shortest decimal representation necessary for
 * the double read back in to be equal to the double written.
 */