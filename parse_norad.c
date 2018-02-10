/**************************************************
 * RCSId: $Id: parse_norad.c,v 1.2 2016/03/25 18:43:14 ralblas Exp $
 *
 * NORAD 2 lines format routine.
 * Parse NORAD file 
 * Add extracted kepler-info into Kepler-struct.
 * 
 * Project: SSI
 * Author: R. Alblas
 *
 * History: 
 * $Log: parse_norad.c,v $
 * Revision 1.2  2016/03/25 18:43:14  ralblas
 * _
 *
 * Revision 1.1  2015/11/18 18:33:49  ralblas
 * Initial revision
 *
 **************************************************/
/*******************************************************************
 * Copyright (C) 2000 R. Alblas. 
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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sattrack.h"

/*************************************
 * Extract number defined between positions 's' and 'e' from  l
 * and return as float
 *************************************/
static float get_strpart(char *l,int s,int e)
{
  char tmp[100];
  strncpy(tmp,l+s,e-s+1);
  tmp[e-s+1]=0;
  return atof(tmp);
}

/*************************************
 * Read next Kepler-info from file
 * File should be in NORAD 2 lines format.
 * Actually, there are 3 lines per sat; 
 * first line contains satellite name.
 *
 * Return 0 if failed (EOF)
 *************************************/
int read_norad_next_keps(FILE *fp,char *sat_name,KEPLER *kepler)
{
  char *l0,*l1,*l2,*p,tmp[100];

/* Allocate space for 3 lines */
  if (!(l0=calloc(101,1))) return 0;
  if (!(l1=calloc(101,1))) return 0;
  if (!(l2=calloc(101,1))) return 0;

/* Search until a valid NOAA entry is found */
  while (fgets(l2,100,fp))
  {
    if (strlen(l2)>=99) continue;

    if ((*l1=='1') && (*l2=='2')) break;
    p=l0; l0=l1; l1=l2; l2=p;
  }
  if (feof(fp)) return 0;

/* Now, l0 contains sat-name, l1 and l2 the kepler-data */

/* Remove trailing non-alphanum characters from line 0 */
  strncpy(sat_name,l0,100);
  p=sat_name+strlen(sat_name)-1;

//  while ((p > sat_name) && (!isalnum(*p)))  p--; *(p+1)=0;
  while ((p > sat_name) && (strchr(" \n\r	",*p)))  p--; *(p+1)=0;

/* Parse line 1 */
  kepler->epoch_year=(int)get_strpart(l1,18,19);
  if (kepler->epoch_year < 70) kepler->epoch_year+=100;

  kepler->epoch_day=get_strpart(l1,20,31);
  kepler->decay_rate=get_strpart(l1,33,42);

/* Parse line 2 */
  kepler->inclination=D2R(get_strpart(l2,8,15));
  kepler->raan=D2R(get_strpart(l2,17,24));

  sprintf(tmp,"0.%s",l2+26);
  kepler->eccentricity=get_strpart(tmp,0,32-24);

  kepler->perigee=D2R(get_strpart(l2,34,41));
  kepler->anomaly=D2R(get_strpart(l2,43,50));
  kepler->motion=get_strpart(l2,52,62);
  kepler->epoch_rev=get_strpart(l2,63,68);
  free(l0);
  free(l1);
  free(l2);

  return 1;
}
