/**************************************************
 * RCSId: $Id: predict.c,v 1.3 2017/04/11 20:40:25 ralblas Exp $
 *
 * predict 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: predict.c,v $
 * Revision 1.3  2017/04/11 20:40:25  ralblas
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
#include <string.h>
#include <stdlib.h>
#include "sattrack.h"
#include "xtrack_basefunc.h"

TRACK *Create_Track(TRACK *track)
{
  TRACK *track_nw;

  track_nw=calloc(1,sizeof(TRACK));
  if (!track_nw) { printf("OUT OF MEM!\n"); exit(1); } 

  if (track)
  {
    while (track->next) track=track->next;

    track->next=track_nw;
    track_nw->prev=track;
  }
  return track_nw;
}

void Remove_Tracks(TRACK *track)
{
  while (track)
  {
    if (track->next)
    {
      track=track->next;
      free(track->prev);
    }
    else
    {
      free(track);
      break;
    }
  }
}
typedef enum {mode_unk=0,search_up,search_down,search_maxelev} SEARCH_MODE;


/* Search for satellite passing horizon */
static gboolean search_horizon(SAT *sat,int sec_incr,
                        SEARCH_MODE search_mode,
                        struct tm *start_tm,  /* start time -> calc. time */
                        EPOINT *pos_subsat,ROTOR *rot,float *mom_elev,EPOINT *refpos,float elev_hor)
{
  DIRECTION satdir;
  float prev_elev=-10;
  int i;
  long stopsec=24*60*60; // sec_incr;
  for (i=0; i<stopsec; i+=ABS(sec_incr))
  {
    mktime_ntz(start_tm);
/* calc. mom. position */
    calc_satrelpos(*start_tm,pos_subsat,&satdir,refpos,sat,rot);
    if (mom_elev) *mom_elev=satdir.elev;

    switch(search_mode)
    {
      case search_up:
        if (satdir.elev>=elev_hor) return TRUE;
      break;

      case search_maxelev:
        if (mom_elev)
        {
          if (*mom_elev < prev_elev) return TRUE;
          prev_elev=*mom_elev;
        }
      case search_down: /* Also case search_maxelev! */
        if (satdir.elev<elev_hor) return TRUE;
      break;
      default: break;
    }
    start_tm->tm_sec+=sec_incr;
  }
  return FALSE;
}

/****************************************************************
  Predict sat. pass
  sat         predict for this sat.
  start_tm    start searching from this time; returns up-time
  stop_tm     if !=NULL this will return down-time 
  tr          gets info about sat. track while above horizon
  refpos      reference postion
  elev_hor    sat visible if elevation > this number
  elev_det    ignore if max. elevation < this number
  forward     FALSE: skip current pass if so; TRUE: include current pass
 ****************************************************************/
gboolean predict(SAT *sat,struct tm *start_tm,struct tm *stop_tm,TRACK *tr,EPOINT *refpos,ROTOR *rot,
                 float elev_hor,float elev_det,gboolean forward)
{
  int trials=10;
/* Search 'down'; find last sat. pass */
  do
  {
    int jump_out=(forward? 30 : -30);
    if (!search_horizon(sat,jump_out,search_down,start_tm,&tr->up_pos,rot,NULL,refpos,elev_hor))
      return FALSE; // can't find 'sat=down'

    /* ========== Search satellite Up ========== */
    /* Seach 'Up' fast (! uptime < 30 secs: may skip) */
    if (!search_horizon(sat,30,search_up,start_tm,&tr->up_pos,rot,NULL,refpos,elev_hor))
      return FALSE; // can't find 'sat=up'

    /* Go back 1 min and search 'Up' precise */
    start_tm->tm_min--;
    start_tm->tm_sec++;
    if (!search_horizon(sat,1,search_up,start_tm,&tr->up_pos,rot,NULL,refpos,elev_hor))
      return FALSE; // can't find 'sat=up'

    /* Found up-time */
    tr->up_time=*start_tm;
    sat->up_time=*start_tm;

    /* Stop if no 'stop_time' requested */
    if (!stop_tm) return TRUE;

    /* ========== Search satellite max. elevation ========== */
    /* Asume 'Up', search 'Max. elev' precise, and determine max. elevation */
    *stop_tm=*start_tm;
    if (!search_horizon(sat,1,search_maxelev,stop_tm,&tr->maxelev_pos,rot,&tr->max_elev,refpos,elev_hor))
      return FALSE; // can't find max. elev

    /* Found max. elev */
    tr->maxelev_time=*stop_tm;
    tr->pass_e1_w0=(tr->maxelev_pos.lon > refpos->lon? 1 : 0);
    sat->pass_e1_w0=tr->pass_e1_w0;

    /* ========== Search satellite Down ========== */
    /* Asume 'Up', search 'Down' fast */
    if (!search_horizon(sat,30,search_down,stop_tm,&tr->down_pos,rot,NULL,refpos,elev_hor))
      return FALSE; // can't find 'sat=down'

    /* Go back 1 min and search 'Down' precise */
    stop_tm->tm_min--;
    stop_tm->tm_sec++;
    if (!search_horizon(sat,1,search_down,stop_tm,&tr->down_pos,rot,NULL,refpos,elev_hor))
      return FALSE; // can't find 'sat=down'

    /* Found down-time */
    tr->down_time=*stop_tm;

    sat->max_elev=tr->max_elev;
    if (tr->max_elev>=elev_det) break;
    *start_tm=*stop_tm;
    start_tm->tm_min++;
  } while ((trials--) >0);
if (trials==0) return FALSE;
  /* Ready; both start and stop time determined. */
  return TRUE;
}


/****************************************************************
  Predict sat. outage
  sat         predict for this sat.
  start_tm    start searching from this time; returns outage-time
  tr          gets info about sat. track while above horizon
 ****************************************************************/
#define FIX_SAT
gboolean predict_colin(SAT *sat,struct tm *start_tm1,TRACK *tr,EPOINT *refpos)
{
  DIRECTION satdir;
  DIRECTION zondir;
  EPOINT pos_subsat,pos_subsun;
  struct tm reftm_sat;
  struct tm start_tm=*start_tm1;
  int rise,i;
  SAT *zon=NULL;
  int dir=1;
/* ========== Search satellite elev=sun elev ========== */
//  start_tm=*gmtime(&sat->orbit.ref_time);
#if __ADD_SUNMOON__ == 1
  zon=add_sun(NULL);
#endif
  reftm_sat=*gmtime(&sat->orbit.ref_time);
  calc_satrelpos(reftm_sat,&pos_subsat,&satdir,refpos,sat,NULL); // current pos. sat
  sat->pass_e1_w0=(pos_subsat.lon > refpos->lon? 1 : 0);
  if (zon) calc_satrelpos(start_tm,&pos_subsun,&zondir,refpos,zon,NULL);

  // search azim sat = azim sun during 1 day
  if (zondir.azim<satdir.azim) rise=1; else rise=-1;
  for (i=0; i<60*24; i+=1)
  {
    if (zon) calc_satrelpos(start_tm,&pos_subsun,&zondir,refpos,zon,NULL);
#ifndef FIX_SAT
    calc_satrelpos(start_tm,&pos_subsat,&satdir,refpos,sat,NULL);
#endif

//printf("%d  %.2f  %.2f  %.2f\n",i,R2D(zondir.azim),R2D(satdir.azim),ABS(R2D(zondir.azim)-R2D(satdir.azim)));
    if (ABS(R2D(zondir.azim)-R2D(satdir.azim)) < 2) break;
    start_tm.tm_min++;
    mktime_ntz(&start_tm);
  }
  if (i>=60*24) return FALSE;

  // search elev sat = elev sun
  if (zondir.elev<satdir.elev) rise=1; else rise=-1;
  for (i=0; i<365; i+=1)
  {
    if (zon) calc_satrelpos(start_tm,&pos_subsun,&zondir,refpos,zon,NULL);
#ifndef FIX_SAT
    calc_satrelpos(start_tm,&pos_subsat,&satdir,refpos,sat,NULL);
#endif
    if (rise>0)
    {
      if (zondir.elev>satdir.elev) break;
    }
    else
    {
      if (zondir.elev<satdir.elev) break;
    }

    start_tm.tm_mday+=1;
    mktime_ntz(&start_tm);
  }
  if (i==365) return FALSE;

  // search elev sat = elev sun
  for (i=0; i<3600*24; i++)
  {
    float diff1,diff2,dazim;
    if (zon) calc_satrelpos(start_tm,&pos_subsun,&zondir,refpos,zon,NULL);
#ifndef FIX_SAT
    calc_satrelpos(start_tm,&pos_subsat,&satdir,refpos,sat,NULL);
#endif
    dazim=zondir.azim-satdir.azim;
    if (i==0) diff1=dazim;
    else if (i==1) diff2=dazim;
    else dir=(diff1>diff2? -1 : 1);
    if ((int)(R2D(zondir.azim)*100)==(int)(R2D(satdir.azim))*100) break;
    start_tm.tm_sec+=dir;
    mktime_ntz(&start_tm);
  }
  if (i==3600*24) return FALSE;
  *start_tm1=start_tm;
  return TRUE;
}
