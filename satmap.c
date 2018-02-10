/**************************************************
 * RCSId: $Id: satmap.c,v 1.1 2017/04/11 21:03:54 ralblas Exp $
 *
 * mapping sat
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: satmap.c,v $
 * Revision 1.1  2017/04/11 21:03:54  ralblas
 * Initial revision
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
#include "xtrack.h"

/*************************************
 * Translate sensor angle to angle position on earth.
 * (angle between line "sat <--> middle point earth" and "pos. earth <--> mp"
 * Needed: sat_height: height satellite
 *         x         : x-pos. (for HRPT: 0 ... 1023)
 *         xh        : half value of x (1023 for HRPT)
 *         alpha_max : max. angle sensor
 * If x=0  ==> alpha=alpha_max ==> beta=beta_max
 * if x=xh ==> alpha=0 ==> beta=0        
 *************************************/
float x2earthangle(float sat_height,            /* height sat */
                   SCANDIR scan_hor,            /* scan direction sensor */
                   int x,int xh,                 
                   float alpha_max)
{
  float alpha,beta,tmp1,tmp2;

/* angle sensor in satellite */
  alpha=(xh-x)*alpha_max/xh;
  if (scan_hor==W_E) alpha*=-1;

/* translate into radius angle */
  tmp1=(sat_height+Rearth)/Rearth;
  tmp2=(tmp1*tmp1*sin(alpha)*sin(alpha));
  beta=acos(tmp1*sin(alpha)*sin(alpha)+
                 cos(alpha)*sqrt(1-tmp2));
  if (alpha<0) beta*=-1;
  return beta;
}
