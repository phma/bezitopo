/******************************************************/
/*                                                    */
/* layer.h - drawing layers                           */
/*                                                    */
/******************************************************/

#ifndef LAYER_H
#define LAYER_H
#include <string>
#include <vector>
#include "color.h"
#include "linetype.h"

class layer
{
public:
  unsigned short ltype,colr,thik;
  bool visible;
  std::string name;
};
#endif
