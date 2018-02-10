/**************************************************
 * RCSId: $Id: use_pos.c,v 1.8 2018/02/02 23:15:53 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: use_pos.c,v $
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
#include <ctype.h>
#include <stdlib.h>
#include "defines.h"

extern DBASE *db;

#if __ADD_RS232__

#include "rs232.h"

int RS232_Open_Close(int doe_open,int portnr,int speed)
{
  static int is_open;
  if (doe_open>0)
  {
    if (!is_open)
    {
      if (!(RS232_OpenComport(portnr, speed)))
      {
        is_open=1;
      }
    }
    else
    {
      // al open, doe niets
    }
  }
  else if (doe_open<0)
  {
    return is_open;
  }
  else
  {
    if (is_open)
    {
      RS232_CloseComport(portnr);
      is_open=0;
    }
    else
    {
      // was al dicht, doe niets
    }
  }
  return is_open;
}
#endif

// __linux__ is al gedef. op linux-bak
//#if __GTK_WIN32__ == 1
//#else
//#define __linux__
//#endif

#define nint(f) (int)((f)+0.5)
static void load_cmd(char *prog,char *frmt,int ew,float xazim,float yelev)
{
  int n=0,m=0;
  char *p;
  char df='d';
  if (strchr(frmt,'f')) df='f';

  for (p=frmt; *p; p++)
  {
    if (*p=='%')
    {
      n++;
    }
  }

  for (p=frmt; *p; p++)
  {
    if (*p=='%')
    {
      m++;
    }
    if (m>=1)
    {
      if ((n==3) && (m==1))
      {
        if (strchr("df",*p)) *p='d';
      }
      else
      {
        if (strchr("df",*p)) *p=df;
      }
    }
  }

  if (n==3)
  {
    if (df=='f')
    {
      sprintf(prog,frmt,ew,xazim,yelev);
    }
    else
    {
      sprintf(prog,frmt,ew,nint(xazim),nint(yelev));
    }
  }
  else
  {
    if (df=='f')
      sprintf(prog,frmt,xazim,yelev);
    else
      sprintf(prog,frmt,nint(xazim),nint(yelev));
  }
  if (!strcmp(prog+strlen(prog)-2,"\\r"))      sprintf(prog+strlen(prog)-2,"%c",'\r');
  else if (!strcmp(prog+strlen(prog)-2,"\\n")) sprintf(prog+strlen(prog)-2,"%c",'\n');
  else                                         strcat(prog,"\n");
}

static void send_onepos(SAT *sat,GtkWidget *wnd)
{
  DIRECTION satdir=sat->dir;
  char prog[100];
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
    int pass_e1_w0=sat->pass_e1_w0;
    if (db->force_pass_e1_w0=='e') pass_e1_w0=1; 
    if (db->force_pass_e1_w0=='w') pass_e1_w0=0; 
    load_cmd(prog,db->rs232.command,pass_e1_w0,xazim,yelev);
    if (op)
    {
      char cmd2[100];
      strcpy(cmd2,prog); if (cmd2[strlen(cmd2)-1]=='\n') strcpy(cmd2+strlen(prog)-1,"\\n");
      Set_Entry(Find_Window(wnd,Lab_WNDDebug),LAB_RS232CMD,cmd2);

      if (RS232_Open_Close(-1,0, 0))
        RS232_SendBuf(db->rs232.portnr, (unsigned char*)prog, strlen(prog));
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


static char *type_prcnt(char *p)
{
  while ((*p) && ((isdigit(*p)) || (*p=='.'))) p++;
  return p;
}


static void fillin_satpos(char *frmt,char *name,DIRECTION *satdir)
{
  char *p;
  char *rslt=malloc(strlen(frmt)+strlen(name)+50);

  float elev=R2D(satdir->elev);
  float azim=R2D(satdir->azim);
  float x=R2D(satdir->x);
  float y=R2D(satdir->y);
  for (p=frmt; *p; p++) if (*p=='%')
  {
    if (*(p+1)=='%')
    {
      p+=2; continue;
    }

    *p='?';  // replace '%' by 1
    
  }
  while ((p=strchr(frmt,'?'))) // serach for 1
  {
    *p='%';                  // restore '%'
    if ((p=type_prcnt(p+1))) // skip numbers (e.g. %5.1y)
    {
      switch(*p)
      {
        case 'n': 
          *p='s';
          sprintf(rslt,frmt,name);
          strcpy(frmt,rslt);
        break;
        case 'e': 
          *p='f';
          sprintf(rslt,frmt,elev);
          strcpy(frmt,rslt);
        break;
        case 'a': 
          *p='f';
          sprintf(rslt,frmt,azim);
          strcpy(frmt,rslt);
        break;
        case 'x': 
          *p='f';
          sprintf(rslt,frmt,x);
          strcpy(frmt,rslt);
        break;
        case 'y': 
          *p='f';
          sprintf(rslt,frmt,y);
          strcpy(frmt,rslt);
        break;
        default:
          *p='%';
        break;
      }
    }
  }
  free(rslt);
}

static void extprog(SAT *sat)
{
  DIRECTION satdir=sat->dir;
  char prog[100];
#define GOING_UP   ((prev_elev<db->elev_horiz) && (satdir.elev>=db->elev_horiz))
#define GOING_DOWN ((prev_elev>=db->elev_horiz) && (satdir.elev<db->elev_horiz))
  static float prev_elev;
  if ((sat->max_elev) && (sat->max_elev<db->elev_det)) return; // skip output
  if ((db->out_on) && (db->ext_on))
  {
    if (*db->prog_track)
    {
      strcpy(prog,db->prog_track);
      fillin_satpos(prog,sat->satname,&satdir);
      system(prog);
    }
    if ((*db->prog_trackup) && (satdir.elev>=db->elev_horiz))
    {
      strcpy(prog,db->prog_trackup);
      fillin_satpos(prog,sat->satname,&satdir);
      system(prog);
    }
    if ((*db->prog_up) && (GOING_UP))
    {
      strcpy(prog,db->prog_up);
      fillin_satpos(prog,sat->satname,&satdir);
      system(prog);
    }
    if ((*db->prog_down) && (GOING_DOWN))
    {
      strcpy(prog,db->prog_down);
      fillin_satpos(prog,sat->satname,&satdir);
      system(prog);
    }
  }
  prev_elev=satdir.elev;

}

static void do_onepos(struct tm_ms cur_tm,GtkWidget *wnd,SAT *sat)
{
  EPOINT pos_subsat;
  EPOINT pos_earth;
  DIRECTION satdir;
  if (!sat) return;
  if ((!sat->visible) && (!sat->selected)) return;
  sat->orbit.scan_hor=E_W;
  calc_satrelposms(cur_tm,&pos_subsat,&satdir,&db->refpos,sat,&db->rotor);
  sat->pos.lon=pos_subsat.lon;
  sat->pos.lat=pos_subsat.lat;
  sat->dir.elev=satdir.elev;
  sat->dir.azim=satdir.azim;
  sat->dir.x=satdir.x;
  sat->dir.y=satdir.y;

  if (sat->type==satellite)
  {
    draw_onepos(cur_tm.tm,wnd,sat,pos_subsat,pos_earth);
  }
  else if (sat->type==sun)
  {
    draw_mark(wnd,pos_subsat,'c'*0x100,NULL);
//    draw_vis(wnd,sat,&pos_subsat,TRUE);
  }
  else // moon
  {
    draw_mark(wnd,pos_subsat,'C'*0x100+sat->m_ilum,NULL);
  }
  if (sat->selected)
  {
    char rgb[3];
     
    if (db->out_on) { rgb[0]=0x00; rgb[1]=0xff; rgb[2]=0x00; } // green if on
    else            { rgb[0]=0xff; rgb[1]=0x00; rgb[2]=0x00; } // red if off
    calc_rotpos(sat->dir,&db->rotor,db->elev_horiz,db->reptime);
    Set_Entry(wnd,LAB_LON,"%.1f",R2D(pos_subsat.lon));
    Set_Entry(wnd,LAB_LAT,"%.1f",R2D(pos_subsat.lat));

    if (db->show_torot)
    {
      Set_Entry(wnd,LAB_ELE,"%.1f",db->rotor.to_rotor.elev);
      Set_Entry(wnd,LAB_AZI,"%.1f",db->rotor.to_rotor.azim);
      Set_Entry(wnd,LAB_XXX,"%.1f",db->rotor.to_rotor.x); // rp1);
      Set_Entry(wnd,LAB_YYY,"%.1f",db->rotor.to_rotor.y); // rp2);
    }
    else
    {
      Set_Entry(wnd,LAB_ELE,"%.1f",R2D(satdir.elev));
      Set_Entry(wnd,LAB_AZI,"%.1f",R2D(satdir.azim));
      Set_Entry(wnd,LAB_XXX,"%.1f",R2D(satdir.x));
      Set_Entry(wnd,LAB_YYY,"%.1f",R2D(satdir.y));
    }
    if (db->out_on)
    {
      send_onepos(sat,wnd);
      extprog(sat);
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
  clear_wnd(wnd); // achtegrond opnieuw schrjven

  // calc. pos. and draw selected sat's
  for (sat=db->sat; sat; sat=sat->next)
  {
    do_onepos(db->glob_tm_ms,wnd,sat);
  }
  refresh_wnd(wnd);
}
