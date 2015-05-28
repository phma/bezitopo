/******************************************************/
/*                                                    */
/* layer.h - drawing layers                           */
/*                                                    */
/******************************************************/

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

extern std::vector<layer> layers;
