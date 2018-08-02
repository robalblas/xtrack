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
#include "xtrack.h"
#include "xtrack_func.h"
#include <ctype.h>
#include <stdlib.h>
extern DBASE *db;


static char *type_prcnt(char *p)
{
  while ((*p) && ((isdigit(*p)) || (*p=='.'))) p++;
  return p;
}


static void fillin_satpos(char *frmt,char *name,DIRECTION *satdir,gboolean deg)
{
  char *p;
  char *rslt=malloc(strlen(frmt)+strlen(name)+50);

  float elev=(deg?satdir->elev: R2D(satdir->elev));
  float azim=(deg?satdir->azim: R2D(satdir->azim));
  float x=(deg?satdir->x: R2D(satdir->x));
  float y=(deg?satdir->y: R2D(satdir->y));
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

void pos2extprog(SAT *sat,DIRECTION satdir)
{
//  DIRECTION satdir=sat->dir;
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
      fillin_satpos(prog,sat->satname,&satdir,TRUE);
      system(prog);
    }
    if ((*db->prog_trackup) && (satdir.elev>=db->elev_horiz))
    {
      strcpy(prog,db->prog_trackup);
      fillin_satpos(prog,sat->satname,&satdir,TRUE);
      system(prog);
    }
    if ((*db->prog_up) && (GOING_UP))
    {
      strcpy(prog,db->prog_up);
      fillin_satpos(prog,sat->satname,&satdir,TRUE);
      system(prog);
    }
    if ((*db->prog_down) && (GOING_DOWN))
    {
      strcpy(prog,db->prog_down);
      fillin_satpos(prog,sat->satname,&satdir,TRUE);
      system(prog);
    }
  }
  prev_elev=satdir.elev;

}
