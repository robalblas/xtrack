/**************************************************
 * RCSId: $Id: read_norad.c,v 1.6 2018/03/08 10:54:22 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: read_norad.c,v $
 * Revision 1.6  2018/03/08 10:54:22  ralblas
 * _
 *
 * Revision 1.5  2018/02/02 23:14:24  ralblas
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
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "xtrack_basefunc.h"

/*************************************
 * Wild char comparator
 *************************************/
int strcmpwild(char *s,char *v)
{
  while ((*v) && (*s))
  {
    if (*v=='*')
    {
      v++;
      if (!*v) return 0;                   /* "abc"=="abc*" */
      while ((*s) && (strcmpwild(s,v))) s++;
    }
    if ((*v!='?') && (*v!=*s)) return 1;   /* "abc"=="adb" */
    v++; s++;
  }
  if (*s) return 2;                        /* "abc"=="ab" */
  while (*v=='*') v++;
  if (*v) return 3;                        /* "ab"=="abc" */
  return 0;                                /* "abc"=="a*c" enz. */
}    
/* Translate Kepler angles to radians */
void kepler_d2r(KEPLER *kepler)
{
  kepler->inclination=D2R(kepler->inclination);
  kepler->raan=D2R(kepler->raan);
  kepler->perigee=D2R(kepler->perigee);
  kepler->anomaly=D2R(kepler->anomaly);
}

/* Wild characters: * and ?, anywhere in string.
    return=0 if match
    return!=0 if no match
*/

static void get_satfreq(SAT *sat)
{
  if (strstr(sat->satname,"NOAA"))
  {
    if (strstr(sat->satname,"15"))     { sat->hfreq=1702.5; sat->lfreq=137.620;   }
    if (strstr(sat->satname,"16"))     { sat->hfreq=0;      sat->lfreq=0;         }
    if (strstr(sat->satname,"17"))     { sat->hfreq=0;      sat->lfreq=0;         }
    if (strstr(sat->satname,"18"))     { sat->hfreq=1707.0; sat->lfreq=137.9125;  }
    if (strstr(sat->satname,"19"))     { sat->hfreq=1698.0; sat->lfreq=137.100;   }
  }
  if (strstr(sat->satname,"METEOR"))   { sat->hfreq=1700.0; }
  if (strstr(sat->satname,"METOP-A"))  { sat->hfreq=1701.3; }
  if (strstr(sat->satname,"METOP-B"))  { sat->hfreq=1701.3; }
  if (strstr(sat->satname,"METOP-C"))  { sat->hfreq=1701.3; }
  if (strstr(sat->satname,"YUN 3A"))   { sat->hfreq=1704.5; }
  if (strstr(sat->satname,"YUN 3B"))   { sat->hfreq=1704.5; }
  if (strstr(sat->satname,"YUN 3C"))   { sat->hfreq=1701.3; }
}

SAT *read_msat(FILE *fp,char *satnames_toload,SAT *sat)
{
  KEPLER kepler;
  char satname_infile[100];
  char *satname,satnames[100];
  if (!fp) return NULL;
  rewind(fp);
  while ((read_norad_next_keps(fp,satname_infile,&kepler)))
  {
    strncpy(satnames,satnames_toload,90);
    for (satname=strtok(satnames,";"); satname; satname=strtok(NULL,";"))
    {
      if (!strcmpwild(satname_infile,satname))
      {
        sat=Create_Sat(&sat);
        if (!sat) continue;
        sat->kepler=kepler;
        calc_orbitconst(&sat->kepler,&sat->orbit);
        strcpy(sat->satname,satname_infile);
        if (strstr(sat->satname,"NOAA"))
        {
          sat->orbit.max_sens_angle=D2R(55.4); /* for NOAA */
          sat->orbit.width_original=2048;
          if (sat->hfreq==0.) get_satfreq(sat); // if freq. not defined use fixed one
        }
        else if (strstr(sat->satname,"METOP"))
        {
          sat->orbit.max_sens_angle=D2R(55.4); /* for Metop */
          sat->orbit.width_original=2048;
          if (sat->hfreq==0.) get_satfreq(sat);
        }
        else if (strstr(sat->satname,"METEOR"))
        {
          sat->orbit.max_sens_angle=D2R(55.4); /* for Meteor */
          sat->orbit.width_original=1540;
          if (sat->hfreq==0.) get_satfreq(sat);
        }
        else if (strstr(sat->satname,"FENG"))
        {
          sat->orbit.max_sens_angle=D2R(55.4); /* for Fengyun */
          sat->orbit.width_original=2048;
          if (sat->hfreq==0.) get_satfreq(sat);
        }
        else if (strstr(sat->satname,"SEASTAR??")) /* To find out actual name! */
        {
          sat->orbit.max_sens_angle=D2R(58.3); /* for Seastar */
          sat->orbit.width_original=2048;
        }
        else
          sat->orbit.max_sens_angle=0;         /* No sensor calc */

      }
    }
  }
  return sat;
}

