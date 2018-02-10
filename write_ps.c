/**************************************************
 * RCSId: $Id: write_ps.c,v 1.2 2018/02/02 23:10:30 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: write_ps.c,v $
 * Revision 1.2  2018/02/02 23:10:30  ralblas
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
#include <time.h>
#include <string.h>
#include "defines.h"
#include "xtrack.h"

extern DBASE *db;

void setfont(FILE *fp,int fs)
{
  fprintf(fp,"/Times-Roman findfont\n");
  fprintf(fp,"  %d scalefont\n",fs);
  fprintf(fp,"  setfont\n");
}

void ps_hdr(FILE *fp)
{
  setfont(fp,10);
  fprintf(fp,"  0 300 translate\n");
  fprintf(fp,"  1.0 1.0 scale\n");
  fprintf(fp,"newpath\n");
}

void ps_ldr(FILE *fp)
{
  fprintf(fp,"stroke\n");
  fprintf(fp,"showpage\n");
}

void draw_line(FILE *fp,int x,int y,int x2,int y2)
{
  fprintf(fp,"  %d %d moveto\n",x,y);
  fprintf(fp,"  %d %d lineto\n",x2,y2);
}

void draw_rect(FILE *fp,int x,int y,int dx,int dy,int clr,gboolean fill)
{
  float r,g,b;
  r=((clr>>8)&0xf)/15.;
  g=((clr>>4)&0xf)/15.;
  b=((clr>>0)&0xf)/15.;

  fprintf(fp,"  %.2f %.2f %.2f setrgbcolor\n",r,g,b);
  fprintf(fp,"  %d %d moveto\n",x,y);
  fprintf(fp,"  %d %d rlineto\n",dx,0);
  fprintf(fp,"  %d %d rlineto\n",0,dy);
  fprintf(fp,"  %d %d rlineto\n",-dx,0);
  fprintf(fp,"  %d %d rlineto\n",0,-dy);

  if (fill) fprintf(fp,"  fill\n");
  fprintf(fp,"  stroke\n");
  fprintf(fp,"  0 setgray\n");
}

void draw_text(FILE *fp,int x,int y,char *frmt,...)
{
  va_list ap;
  va_start(ap,frmt);
  fprintf(fp,"  %d %d moveto\n",x,y);
  fprintf(fp,"  (");
  vfprintf(fp,frmt,ap);
  fprintf(fp,") show\n");
  va_end(ap);
}

void ps_rast(FILE *fp,int min_hour,int max_hour,int maxday)
{
  int t;
  setfont(fp,(db->ps_max_h>260? 10 : 8));
  fprintf(fp,".6 .6 .6 setrgbcolor\n");

  for (t=min_hour*60; t<=max_hour*60; t+=60)
  {
    draw_line(fp,t2x(t),t2y(1),t2x(t),t2y(maxday));
  }
  for (t=1; t<=maxday; t+=1)
  {
    draw_line(fp,t2x(min_hour*60),t2y(t),t2x(max_hour*60),t2y(t));
  }
  fprintf(fp,"0 stroke\n");
  fprintf(fp,"0 setgray\n");

  for (t=min_hour*60; t<=max_hour*60; t+=60)
  {
    draw_text(fp,t2x(t)-4,t2y(0),"%2d",t/60);
  }
  for (t=1; t<=maxday; t+=1)
  {
    draw_text(fp,t2x(-40+min_hour*60),t2y(t),"%2d",t);
  }
}

int titel(FILE *fp,DBASE *db,int n)
{
  char maand[50],*p,*y;
  int marr[]={31,28,31,30,31,30,31,31,30,31,30,31};

  struct tm tm=db->genstart_tm;
  tm.tm_mon+=n;
  mktime_ntz(&tm);
  sprintf(maand,asctime(&tm));
  p=strtok(maand," ");
  p=strtok(NULL," ");
  y=strtok(NULL," ");
  y=strtok(NULL," ");
  y=strtok(NULL," ");

  setfont(fp,13);
  fprintf(fp,"  %d %d moveto\n",100,t2y(0)+20);
  fprintf(fp,"  (%s   %s  %s) show\n",db->sat_sel->satname,p,y);

  fprintf(fp,"  %d %d moveto\n",300,t2y(0)+20);
  fprintf(fp,"  (groundstation: [%.0f , %.0f]) show\n",
                    R2D(db->refpos.lon),R2D(db->refpos.lat));
  return marr[tm.tm_mon];
}
