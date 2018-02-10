/**************************************************
 * RCSId: $Id: dbase_prims.c,v 1.6 2018/02/02 23:03:54 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: dbase_prims.c,v $
 * Revision 1.6  2018/02/02 23:03:54  ralblas
 * _
 *
 *
 **************************************************/
/*******************************************************************
 * Copyright (C) 2000 R. Alblas 
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 * 02111-1307, USA.
 ********************************************************************/
#include "sattrack.h"
#include "xtrack_basefunc.h"
#include <string.h>

SAT *Create_Sat(SAT **sat)
{
  SAT *satnw;
  satnw=calloc(1,sizeof(SAT));
  if ((sat) && (*sat))
  {
    Forward(*sat);
    (*sat)->next=satnw;
    satnw->prev=*sat;
  }
  else if (sat)
  {
    *sat=satnw;
  }
  return satnw;
}

SAT *Remove_Sat(SAT *sat)
{
  SAT *sat_first=sat;
  if (!sat) return NULL;
  Rewind(sat_first);
  if (sat_first==sat) sat_first=sat_first->next;
  if (sat->prev) sat->prev->next=sat->next;
  if (sat->next) sat->next->prev=sat->prev;
  if (sat->track) Remove_Tracks(sat->track);
  free(sat);
  return sat_first;
}
SAT *Find_Sat(SAT *sat,char *satname)
{
  for (; sat; sat=sat->next)
    if (!strcmp(sat->satname,satname))
      return sat;
  return NULL;
}

SAT *find_sat(SAT *sat,char *name)
{
  for (; sat; sat=sat->next)
  {
    if ((sat->satname) && (!strcmp(sat->satname,name))) break;
  }
  return sat;
}

struct tm mom_tm(float hrs_offset,TM_OFFSET *tm_off)
{
  time_t t;
  struct tm tm;
  time(&t);
  tm=*gmtime(&t);
  if (tm_off)
  {
    tm.tm_hour+=(int)(tm_off->tz_offset);
    tm.tm_min+=(tm_off->tz_offset - (int)(tm_off->tz_offset))*60;
    mktime_ntz(&tm);
    tm.tm_sec+=(int)tm_off->sec_offset+(hrs_offset*3600);
  }
  mktime_ntz(&tm);

  return tm; // offset_tm(tm,hrs_offset+(float)(test_min/60.));
}

// !!! Use with tm_off just 1x!!! (statics!)
struct tm_ms mom_tmms(float hrs_offset,TM_OFFSET *tm_off)
{
  static struct tm_ms tmp;
  static int ms;
  struct tm_ms tm_ms;
  tm_ms.tm=mom_tm(hrs_offset,tm_off);
  if (tm_off)
  {
    if (tm_ms.tm.tm_sec==tmp.tm.tm_sec)
    {
      int ds=(tm_off->reptime-(int)tm_off->reptime)*10;
      ms+=ds*100;
    }
    else
    {
      ms=(tm_off->sec_offset-(int)tm_off->sec_offset)*1000;
    }
    tm_ms.ms=ms;
    tmp=tm_ms;
  }
  else
  {
    tm_ms.ms=0;
  }
  return tm_ms;
}
