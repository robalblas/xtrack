/**************************************************
 * RCSId: $Id: sattrack_funcs.h,v 1.2 2018/02/04 13:06:27 ralblas Exp $
 *
 *  
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: sattrack_funcs.h,v $
 * Revision 1.2  2018/02/04 13:06:27  ralblas
 * _
 *
 * Revision 1.1  2017/04/11 21:00:54  ralblas
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
#include "sattrack.h"

int calc_orbitconst(KEPLER *kepler,ORBIT *orbit);
int calcpossat(struct tm *cur_tm, int cu_ms,KEPLER *,ORBIT *,EPOINT *);
int calcpossatms(struct tm *cur_tm, int, int cu_ms,KEPLER *,ORBIT *,EPOINT *);
void yday2mday_mon(struct tm *tmref);
long mktime_ntz(struct tm *);
struct tm mom_tm0(float);
struct tm mom_tm(float,TM_OFFSET *);
struct tm_ms mom_tmms(float,TM_OFFSET *);
float x2earthangle(float sat_height,SCANDIR scan_hor,int x,int xh,float alpha_max);
EPOINT calc_sun(struct tm tm);
EPOINT calc_moon(struct tm tm,int *,EPOINT *);
SAT *add_sun(SAT **sat);
gboolean predict(SAT *,struct tm *,struct tm *,TRACK *,EPOINT *,ROTOR *,float,float,gboolean);
int read_norad_next_keps(FILE *fp,char *sat_name,KEPLER *kepler);
gboolean calc_rotpos(DIRECTION satdir,ROTOR *rotor,float elev_horiz,float reptime);

void calc_satposms(struct tm_ms ,EPOINT *,DIRECTION *,EPOINT *,SAT *sat,ROTOR *rot);
