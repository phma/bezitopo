/******************************************************/
/*                                                    */
/* crosssection.h - cross sections of roadways        */
/*                                                    */
/******************************************************/

/* A cross section is taken at a station and consists of a list
 * of triples (index,offset,elevation), where index is a dimensionless
 * number and offset and elevation are in meters. By convention,
 * index=0 where offset=0, which is the centerline. index is used to
 * connect corresponding points of one cross section to the next.
 * elevation is relative to the alignment.
 */
#include <map>

struct xsitem
{
  double offset,elevation;
}

class crosssection
{
  map<double,xsitem> data;
}
