/******************************************************/
/*                                                    */
/* document.cpp - main document class                 */
/*                                                    */
/******************************************************/
#include "document.h"

void document::copytopopoints(criteria crit)
{
  ptlist::iterator i;
  if (pl.size()<2)
    pl.resize(2);
  pl[1].clear();
  int j;
  bool include;
  for (i=pl[0].points.begin();i!=pl[0].points.end();i++)
  {
    include=false;
    for (j=0;j<crit.size();j++)
      if (i->second.note.find(crit[j].str)!=string::npos)
	include=crit[j].istopo;
    if (include)
      pl[1].addpoint(i->first,i->second);
  }
}
