/**************************************************
 * RCSId: $Id: predict_meer.c,v 1.7 2018/02/02 22:54:27 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: predict_meer.c,v $
 * Revision 1.7  2018/02/02 22:54:27  ralblas
 * _
 *
 * Revision 1.5  2017/04/11 20:42:15  ralblas
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
#include "xtrack.h"
#include "xtrack_func.h"
#include "defines.h"
#define IS_GEO(h) ((h > 33000.) && (h<39000.))
extern DBASE *db;

void select_this_sat(SAT *sat)
{
  SAT *sat1=sat;
  Rewind(sat1);
  for (; sat1; sat1=sat1->next)
  {
    sat1->selected=FALSE;
  }
  sat->selected=TRUE;
}

/****************************************************************
  Generate list of overkomsten
  sat           predict for this sat.
  genstart_tm   start searching from this time
  genstop_tm    stop searching time 
  Return:       linked list of tracks
 ****************************************************************/
TRACK *gen_track(SAT *sat,struct tm genstart_tm,struct tm genstop_tm)
{
  GtkWidget *progress_bar;
  TRACK *track=NULL;
  TRACK track1;
  struct tm start_tm;
  struct tm stop_tm;
  gint32 predtimemax;
  gint32 predtime;
  gint32 predtime1;
  start_tm=genstart_tm;
  start_tm.tm_min=start_tm.tm_sec=0;
  predtimemax=mktime_ntz(&genstop_tm)-mktime_ntz(&start_tm);
  predtime=predtimemax;
  progress_bar=Create_Progress(NULL,"Predicting...");
  while (predtime > 0)
  {
    predtime1=predtime;
    if (Update_Progress(progress_bar,predtimemax-predtime,predtimemax))
    {
      break;
    }
    if (predict(sat,&start_tm,&stop_tm,&track1,&db->refpos,&db->rotor,db->elev_horiz,db->elev_det,TRUE))
    {
      if (track1.max_elev>=db->elev_det)
      {
        if (mktime_ntz(&genstop_tm) > mktime_ntz(&start_tm))
        {
          if (track1.max_elev > db->elev_horiz)
          {
            track=Create_Track(track);
            track1.prev=track->prev;
            track1.next=track->next;
            *track=track1;
            track->sat=sat;
          }
        }
      }
      predtime=mktime_ntz(&genstop_tm)-mktime_ntz(&start_tm);
      if (predtime1==predtime)
      {
        start_tm.tm_min+=10;
        mktime_ntz(&start_tm);
        predtime=mktime_ntz(&genstop_tm)-mktime_ntz(&start_tm);
      }
    }
  }
  Close_Window(gtk_widget_get_toplevel(progress_bar));
  Rewind(track);
  return track;
}

static void add_to_list(GtkWidget *w,TRACK *tr,gboolean hres)
{
  char *tmp[7];
  char txt[7][100];
  struct tm tm;
  int pday=-1;
  int rownr=0;
  gtk_clist_clear(GTK_CLIST(w));
  for (; tr; tr=tr->next)
  {
    tm=tr->up_time; 
    tm.tm_hour+=db->utc_offset; mktime_ntz(&tm);
    if (tm.tm_mday!=pday)
    {
      tmp[0]="-----------------------------------------"; tmp[1]=tmp[2]=tmp[3]=tmp[4]=tmp[5]=tmp[0];
      tmp[6]="0";
      gtk_clist_append(GTK_CLIST(w), tmp);
    } 
    pday=tm.tm_mday;

    sprintf(txt[0],"%s",tr->sat->satname);
    tm=tr->up_time; tm.tm_hour+=db->utc_offset; mktime_ntz(&tm);
    sprintf(txt[1],"%s",tmstrsh(&tm));
    tm=tr->down_time; tm.tm_hour+=db->utc_offset; mktime_ntz(&tm);
    sprintf(txt[2],"%s",tmstrsh(&tm));
    tm=tr->maxelev_time; tm.tm_hour+=db->utc_offset; mktime_ntz(&tm);
    sprintf(txt[3],"%s",tmstrsh(&tm));
    sprintf(txt[4],"%4.1f",R2D(tr->max_elev));
    if (hres)
      sprintf(txt[5],"%.1f",tr->sat->hfreq);
    else
      sprintf(txt[5],"%.3f",tr->sat->lfreq);
    sprintf(txt[6],"%d",++rownr);
    tmp[0]=txt[0];
    tmp[1]=txt[1];
    tmp[2]=txt[2];
    tmp[3]=txt[3];
    tmp[4]=txt[4];
    tmp[5]=txt[5];
    tmp[6]=txt[6];
    gtk_clist_append(GTK_CLIST(w), tmp);
  }
}


/****************************************************************
  Generate list of overkomsten and create list
  sat           predict for this sat.
  genstart_tm   start searching from this time
  genstop_tm    stop searching time 
 ****************************************************************/
static TRACK *predict_range(GtkWidget *widget,SAT *sat,struct tm genstart_tm,struct tm genrange_tm)
{
  TRACK *track;
/* Create track */
  struct tm stop_tm=genstart_tm;
  stop_tm.tm_mday+=genrange_tm.tm_mday;
  stop_tm.tm_mon+=genrange_tm.tm_mon+1;
  stop_tm.tm_year+=genrange_tm.tm_year;
  mktime_ntz(&stop_tm);
  track=gen_track(sat,genstart_tm,stop_tm);
  return track;
}

//     tra -> trb -> trc -> trd
// ==> tra -> trc -> trb -> trd
static TRACK *sort_track(TRACK *track)
{
  TRACK *tr,*tra,*trb,*trc,*trd;
  int done=0;
  while (!done)
  {
    done=1;
    for (tr=track; tr; tr=tr->next)
    {
      if (!tr->next) continue;
      // sort by up-time, not by time-at-max. elev.
      if (mktime_ntz(&tr->up_time)>mktime_ntz(&tr->next->up_time))
      {
        // tra=tr->prev ==> trb=tr ==> trc=tr->next ==> td=tr->next->next
        // tra ==> trc ==> trb ==> trd
        done=0;
        tra=tr->prev;
        trb=tr;
        trc=tr->next;
        trd=tr->next->next;

        if (tra) tra->next=trc; else track=trc;
        trc->prev=tra;
        trc->next=trb;
        trb->prev=trc;
        trb->next=trd;
        if (trd) trd->prev=trb;
        break;
        
      }
    }
  }
  return track;
}

void show_predict(GtkWidget *widget,SAT *sat,gboolean all_visible,struct tm genstart_tm,struct tm genrange_tm)
{
  GtkWidget *w=Find_Widget(widget,"Updown");
  TRACK *track1,*track=NULL,*tr;
  for (; sat; sat=sat->next)
  {
    if (!IS_GEO(sat->orbit.height/1000.))
    {
      if ((sat->selected) || ((all_visible) && (sat->visible)))
      {
        track1=predict_range(widget,sat,genstart_tm,genrange_tm);
        for (tr=track; ((tr) && (tr->next)); tr=tr->next);
        if (!tr) track=track1; else tr->next=track1;
      }
    }
  }
  track=sort_track(track);
  add_to_list(w,track,db->hres);
  Remove_Tracks(db->track);
  db->track=track;
}

void pri_trackstartstop(TRACK *tr)
{
  FILE *fp=stdout;
  for (; tr; tr=tr->next)
  {
    fprintf(fp,"Up  : (%7.2f,%7.2f) @ %s\n",
         R2D(tr->up_pos.lon),R2D(tr->up_pos.lat),timestr(&tr->up_time));
    fprintf(fp,"Down: (%7.2f,%7.2f) @ %s\n\n",
         R2D(tr->down_pos.lon),R2D(tr->down_pos.lat),timestr(&tr->down_time));
  }
}


void plot_startstop(FILE *fp,TRACK *track)
{
  int x,y,dx,dy;
  struct tm up_time=track->up_time;
  struct tm down_time=track->down_time;
  int clr;
  gboolean going_s=track->going_south;
  gboolean fill;
  gboolean in_color=FALSE;
  float max_elev=track->max_elev;
  going_s=(track->up_pos.lat>track->down_pos.lat? TRUE : FALSE);

  x=t2x(dagtijd(up_time));
  y=t2y(mndtijd(up_time));
  dx=t2x(dagtijd(down_time))-t2x(dagtijd(up_time));
  if (dx<0) dx=0;
  dy=3;

  if (in_color)
  {
    clr=(going_s? 0x0f0 : 0xf00);
    fill=TRUE;
  }
  else
  {
    clr=0x000;
    fill=going_s;
  }
  draw_rect(fp,x,y+1,dx,dy,clr,fill);
  draw_text(fp,x+dx,y,"%4.0f",R2D(max_elev));
}

void generate_ps(TRACK *track,SAT *sat)
{
  FILE *fp=stdout;
  char fn[100],*p;
  TRACK *track_mom;
  int min_hour=0,max_hour=24;
  int maxmon;
  int n=0;

/* Determine filename */
  sprintf(fn,"%s_%d",db->sat_sel->satname,db->genstart_tm.tm_mon+1);
  while ((p=strchr(fn,'/'))) *p='_';
  while ((p=strchr(fn,'-'))) *p='_';
  while ((p=strchr(fn,' '))) *p='_';
  strcat(fn,".ps");

  fp=fopen(fn,"w");
  if (!fp) { printf("??? Can't open %s!!!\n",fn); return; }

  db->ps_x_offset=min_hour;
  db->ps_y_offset=480;
  db->ps_max_h=200;
  db->ps_max_t=(max_hour-min_hour)*60;

  for (track_mom=track; track_mom; track_mom=track_mom->next)
  {
  }

  ps_hdr(fp);

  for (track_mom=track; track_mom; track_mom=track_mom->next)
  {
    if (strcmp(track_mom->sat->satname,db->sat_sel->satname)) continue;
    if ((!track_mom->prev) ||
        (track_mom->prev->up_time.tm_mon != track_mom->up_time.tm_mon))
    {
      db->ps_y_offset-=(db->ps_max_h+40);
      maxmon=titel(fp,db,n++);
      ps_rast(fp,min_hour,max_hour,maxmon);
      setfont(fp,7);
    }
    plot_startstop(fp,track_mom);
  }
  ps_ldr(fp);

  if ((fp) && (fp!=stdout)) fclose(fp);
  Create_Message("Generated %s",fn);
}

static struct tm *gm2local(struct tm *time)
{
  static struct tm tm;

  tm = *time;
  tm.tm_hour+=db->utc_offset; mktime_ntz(&tm);
  return &tm;
}

void generate_txt(FILE *fp,TRACK *track,SAT *sat)
{
  TRACK *tr;
//  fprintf(fp,"%s_%d\n",db->sat_sel->satname,db->genstart_tm.tm_mon+1);
  for (tr=track; tr; tr=tr->next)
  {
    fprintf(fp,"%-15s ",tr->sat->satname);
    fprintf(fp,"%-10s",tmstrsh(gm2local(&tr->up_time)));
    fprintf(fp,"%-10s",tmstrsh(gm2local(&tr->down_time)));
    fprintf(fp,"%-10s",tmstrsh(gm2local(&tr->maxelev_time)));
    fprintf(fp,"%4.1f",R2D(tr->max_elev));
    fprintf(fp,"\n");
  }
}

void pri_track(FILE *fp,SAT *sat,struct tm tma,struct tm tmb)
{
  EPOINT pos_subsat;
  DIRECTION satdir;
  struct tm tm;
  int nrsec=mktime_ntz(&tmb)-mktime_ntz(&tma);
  if ((nrsec<=0) || (nrsec>24*3600))
  {
    Create_Message("???"," nrsec= %d\n",nrsec);
    return;
  }
  fprintf(fp,"%s_%d\n",db->sat_sel->satname,db->genstart_tm.tm_mon+1);
  for (tm=tma; mktime_ntz(&tm) < mktime_ntz(&tmb); tm.tm_sec++)
  {
    calc_satrelpos(tm,&pos_subsat,&satdir,&db->refpos,sat,&db->rotor);
    fprintf(fp,"lon= %f  lat= %f  elev= %f  azim= %f  x= %f  y= %f\n",
       R2D(pos_subsat.lon), R2D(pos_subsat.lat),R2D(satdir.elev),R2D(satdir.azim),90-R2D(satdir.x),90-R2D(satdir.y));
  }
}
