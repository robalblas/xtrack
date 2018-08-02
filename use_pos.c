/**************************************************
 * RCSId: $Id: use_pos.c,v 1.9 2018/03/08 10:42:32 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: use_pos.c,v $
 * Revision 1.9  2018/03/08 10:42:32  ralblas
 * _
 *
 * Revision 1.8  2018/02/02 23:15:53  ralblas
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
/*
oud: gtk_timeout_add ==> repeated_calc ==> gmtime2adjust ==> Set_Adjust ==> time_callback ==> idle_func ==>do_onepos_all

nw:  gtk_timeout_add ==> repeated_calc
*/
#include "xtrack.h"
#include "xtrack_func.h"
#include "xtrack_basefunc.h"
#include <ctype.h>
#include <stdlib.h>
#include "defines.h"

extern DBASE *db;



static void send_onepos(SAT *sat,GtkWidget *wnd)
{
  DIRECTION satdir=sat->dir;
  if ((sat->max_elev) && (sat->max_elev<db->elev_det)) return;
  static int op;
  if (satdir.elev>=db->elev_horiz) op=1;
  float xazim,yelev;
  if (db->rotor.use_xy)
  {
    xazim=db->rotor.to_rotor.x;
    yelev=db->rotor.to_rotor.y;
  }
  else
  {
    xazim=db->rotor.to_rotor.azim;
    yelev=db->rotor.to_rotor.elev;
  }
  
#if __ADD_RS232__
  if (db->to_serial)
  {
    if (op)
    {
      char cmd2[100];
      int pass_e1_w0=sat->pass_e1_w0;
      if (db->force_pass_e1_w0=='e') pass_e1_w0=1; 
      if (db->force_pass_e1_w0=='w') pass_e1_w0=0; 
#ifndef XXX
      strcpy(cmd2,pos2uart(db->rs232.portnr,db->rs232.command,xazim,yelev,sat->pass_e1_w0));
      if (cmd2[strlen(cmd2)-1]=='\n') strcpy(cmd2+strlen(cmd2)-1,"\\n");
      Set_Entry(Find_Window(wnd,Lab_WNDDebug),LAB_RS232CMD,cmd2);
#else
      char prog[100];
      load_cmd(prog,db->rs232.command,pass_e1_w0,xazim,yelev);
      strcpy(cmd2,prog); if (cmd2[strlen(cmd2)-1]=='\n') strcpy(cmd2+strlen(prog)-1,"\\n");
      Set_Entry(Find_Window(wnd,Lab_WNDDebug),LAB_RS232CMD,cmd2);

      if (RS232_Open_Close(-1,0, 0))
        RS232_SendBuf(db->rs232.portnr, (unsigned char*)prog, strlen(prog));
#endif
    }
  }
#endif

#if __ADD_USB__ == 1
  if (db->to_usb)
  {
    if (op)
      pos2godil(&db->usb,&db->rotor,xazim,yelev,db->decoderdisplinfo);
  }
#endif
  if (satdir.elev<db->elev_horiz)
  {
    if ((db->rotor.to_rotor.x == db->rotor.storm.x) &&
        (db->rotor.to_rotor.y == db->rotor.storm.y))
      op=0;
  }
}

static void do_onepos(struct tm_ms cur_tm,GtkWidget *wnd,SAT *sat)
{
  if (!sat) return;
  if ((!sat->visible) && (!sat->selected) && (sat->type!=sun)) return;
  calc_satrelinfo(cur_tm,sat,&db->rotor,&db->refpos,TRUE);

  if (sat->type==satellite)
  {
    draw_onepos(cur_tm.tm,wnd,sat,sat->pos);
  }
  else if (sat->type==sun)
  {
    if ((sat->selected) || (sat->visible))
    draw_mark(wnd,sat->pos,'c'*0x100,NULL);
//    draw_vis(wnd,sat,&sat->pos,TRUE);
  }
  else // moon
  {
    draw_mark(wnd,sat->pos,'C'*0x100+sat->m_ilum,NULL);
  }
  // sat->max_elev: max_elev is of pass accepted, maxelev_nextpass is max. elev current pass
  if ((sat->selected)) //  && (sat->maxelev_nextpass >= db->elev_det))
  {
    char rgb[3];
    gboolean will_track=(sat->maxelev_nextpass >= db->elev_det);
    if (db->out_on) { rgb[0]=0x00; rgb[1]=0xff; rgb[2]=0x00; } // green if on
    else            { rgb[0]=0xff; rgb[1]=0x00; rgb[2]=0x00; } // red if off
    calc_rotpos(sat->dir,will_track,&db->rotor,db->elev_horiz,db->reptime);

    Set_Entry(wnd,LAB_LON,"%.1f",R2D(sat->pos.lon));
    Set_Entry(wnd,LAB_LAT,"%.1f",R2D(sat->pos.lat));

    if (db->show_torot)
    {
      Set_Entry(wnd,LAB_ELE,"%.1f",db->rotor.to_rotor.elev);
      Set_Entry(wnd,LAB_AZI,"%.1f",db->rotor.to_rotor.azim);
      Set_Entry(wnd,LAB_XXX,"%.1f",db->rotor.to_rotor.x); // rp1);
      Set_Entry(wnd,LAB_YYY,"%.1f",db->rotor.to_rotor.y); // rp2);
    }
    else
    {
      Set_Entry(wnd,LAB_ELE,"%.1f",R2D(sat->dir.elev));
      Set_Entry(wnd,LAB_AZI,"%.1f",R2D(sat->dir.azim));
      Set_Entry(wnd,LAB_XXX,"%.1f",R2D(sat->dir.x));
      Set_Entry(wnd,LAB_YYY,"%.1f",R2D(sat->dir.y));
    }
    Set_Entry(wnd,LAB_DIST,"%d",(int)(sat->dir.dist/1000.));
    if (db->out_on)
    {
      send_onepos(sat,wnd);
      pos2extprog(sat,db->rotor.to_rotor);
    }
    else
    {
     #if __ADD_USB__ == 1
      if (db->to_usb)
      {
        if (db->usb.ftHandle)
        {
        // usb_init_connect(&db->usb); // Te traag, dan gaan tijdknoppen dubbel doen!
          usb_write_reg(&db->usb,A_DSP,0);
        }
      }
     #endif
    }
  }
}


void do_onepos_all(GtkWidget *wnd)
{
  SAT *sat;
  if (db->redraw) return;
  if (db->disable_draw) return;

  // for shadow first calc. pos sun (shadow drawn in clear_wnd)
  if ((sat=Find_Sat(db->sat,"SUN"))) do_onepos(db->glob_tm_ms,wnd,sat);
  clear_wnd(wnd); // achtergrond opnieuw schrijven

  // calc. pos. and draw selected sat's
  for (sat=db->sat; sat; sat=sat->next)
  {
    do_onepos(db->glob_tm_ms,wnd,sat);
  }
  refresh_wnd(wnd);
}
