/**************************************************
 * RCSId: $Id: xtrack_basefunc.h,v 1.3 2018/02/04 22:13:51 ralblas Exp $
 *
 * func defs, also for other programs using tracking code
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: xtrack_basefunc.h,v $
 * Revision 1.3  2018/02/04 22:13:51  ralblas
 * _
 *
 * Revision 1.2  2017/04/11 20:58:13  ralblas
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
#ifndef XTRACK_BASEFUNC
#define XTRACK_BASEFUNC
#include "sattrack.h"

SAT *Create_Sat(SAT **sat);
SAT *Remove_Sat(SAT *sat);
TRACK *Create_Track(TRACK *track);
void Remove_Tracks(TRACK *track);
void yday2mday_mon(struct tm *tmref);
long mktime_ntz(struct tm *tm);
float x2earthangle(float sat_height,SCANDIR scan_hor,int x,int xh,float alpha_max);
void calc_satrelpos(struct tm cur_tm,EPOINT *pos_subsat,DIRECTION *satdir,EPOINT *refpos,SAT *sat,ROTOR *rot);
void calcposearth(struct tm *cur_tm, int ms, EPOINT *pos_earth);
void calc_satrelposms(struct tm_ms cur_tm,EPOINT *pos_subsat,DIRECTION *satdir,EPOINT *refpos,SAT *sat,ROTOR *rot);
void calc_x2satobs(KEPLER *,ORBIT *,EPOINT *,EPOINT *,int,float,int);
gboolean calc_rotpos(DIRECTION satdir,gboolean will_track,ROTOR *rotor,float elev_horiz,float reptime);
void calc_satrelinfo(struct tm_ms cur_tmms,SAT *sat,ROTOR *rotor,EPOINT *refpos,gboolean calc_velo);

int read_norad_next_keps(FILE *fp,char *sat_name,KEPLER *kepler);
SAT *Find_Sat(SAT *sat,char *satname);
SAT *read_msat(FILE *fp,char *satnames_toload,SAT *sat);
int calcpossat(struct tm *cur_tm, int cu_ms,KEPLER *,ORBIT *,EPOINT *);
int calc_orbitconst(KEPLER *kepler,ORBIT *orbit);
gboolean predict(SAT *,struct tm *,struct tm *,TRACK *,EPOINT *,ROTOR *,float,float,gboolean);
struct tm mom_tm(float,TM_OFFSET *);
struct tm_ms mom_tmms(float,TM_OFFSET *);
EPOINT calc_sun(struct tm tm);
EPOINT calc_moon(struct tm tm,int *,EPOINT *);
SAT *add_sun(SAT **sat);

#endif
