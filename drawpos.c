/**************************************************
 * RCSId: $Id: drawpos.c,v 1.4 2018/02/02 23:08:33 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: drawpos.c,v $
 * Revision 1.4  2018/02/02 23:08:33  ralblas
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
#include <math.h>
#include <stdlib.h>
#include "xtrack.h"
#include "xtrack_func.h"
#include "defines.h"

extern DBASE *db;

// calc and draw scanline
void calcdraw_sensor(struct tm cur_tm,GtkWidget *wnd,SAT *sat)
{
  EPOINT pos_subsat;
  EPOINT pos_sat;
  EPOINT pos_earth;
  calc_onepos(sat,cur_tm,0,&pos_earth,&pos_sat,&pos_subsat,NULL);
//  calceleazim(&pos_subsat,&sat->orbit,&db->refpos,&satdir,db->x_at_disc);
  draw_sensor(wnd,sat,&pos_sat,&pos_earth,TRUE);
}

// calc and draw piece of track between tma and tmb
void draw_track(GtkWidget *wnd,SAT *sat,struct tm tma,struct tm tmb)
{
  EPOINT pos_subsat1,pos_subsat2;
  EPOINT pos_sat;
  EPOINT pos_earth;
  int i=0;
  struct tm tm;
  for (tm=tma; mktime_ntz(&tm) < mktime_ntz(&tmb); tm.tm_min++)
  {
    calc_onepos(sat,tm,0,&pos_earth,&pos_sat,&pos_subsat1,NULL);
    if (i) draw_sattrack(wnd,pos_subsat1,pos_subsat2,TRUE,(i==1));
    pos_subsat2=pos_subsat1;
    i++;
  }
  calc_onepos(sat,tmb,0,&pos_earth,&pos_sat,&pos_subsat1,NULL);
  draw_sattrack(wnd,pos_subsat1,pos_subsat2,TRUE,TRUE);
}

// calc and draw piece of track
static void draw_trackpiece(GtkWidget *wnd,SAT *sat,struct tm cur_tm,gboolean rgbm)
{
  struct tm tma;
  struct tm tmb;
  int strtmin=-5;
  int stopmin=+40;

  tma=cur_tm; tma.tm_min+=strtmin;
  tmb=cur_tm; tmb.tm_min+=stopmin;
  draw_track(wnd,sat,tma,tmb);
}

// draw one sat on it's position
void draw_onepos(struct tm cur_tm,GtkWidget *wnd,SAT *sat,EPOINT pos_sat,EPOINT pos_earth)
{
//  static SAT *prev_selected;
  gboolean rgb_flag=TRUE;
  EPOINT pos_subsat=sat->pos;
//  DIRECTION satdir=sat->dir;
  GdkColor clr_s,clr_u,clr1;
  if (db->disable_draw) return;
  draw_sat(wnd,sat,pos_subsat.lon,pos_subsat.lat,rgb_flag);
  calcdraw_sensor(cur_tm,wnd,sat);

  clr_s.red=0xff; clr_s.green=0; clr_s.blue=0xff;
  clr_u.red=0xff; clr_u.green=0; clr_u.blue=0;
  if (sat->selected) clr1=db->clrs.ssat_vis; else clr1=db->clrs.usat_vis;
  draw_vis(wnd,sat,&pos_subsat,rgb_flag,clr1);

  if (sat->selected)
    draw_trackpiece(wnd,sat,cur_tm,TRUE);


  if (sat->visible)
  {
    if (db->next_sat) db->sat_sel=db->next_sat; //next_sat=next passing sat 
// altijd 'predict' doen, zodat oost/west bij start wordt berekend ook als sat al op is
//  if (db->sat_sel!=prev_selected)  
    {
      select_this_sat(db->sat_sel);  // deselect all sats except sat_sel
    }
  }
  if (sat->selected)
  {
    report_nextpass(wnd,sat);
  }

}
