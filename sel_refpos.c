/**************************************************
 * RCSId: $Id: sel_refpos.c,v 1.3 2018/02/02 23:15:15 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: sel_refpos.c,v $
 * Revision 1.3  2018/02/02 23:15:15  ralblas
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
// read (next) waypoint from GPX file
#include <stdio.h>
#include <stdlib.h>
#include "xtrack.h"
#include "xtrack_func.h"
#define LENNAME 50
extern DBASE *db;

static int rd_wp(FILE *fp,float *lat, float *lon,char *name)
{
  char l[1000],*p;
  int ret=0;
  int flag=0;
  while  (fgets(l,1000,fp))
  {
//   <wpt lat="52.500000" lon="4.400000">
    if (strstr(l,"<wpt")) flag=1;
    if (flag)
    {
      if ((p=strstr(l,"lat=\"")))
      {
        p+=5;
        *lat=atof(p);
        ret|=0x1;
      }
      if ((p=strstr(l,"lon=\"")))
      {
        p+=5;
        *lon=atof(p);
        ret|=0x2;
      }
      if ((p=strstr(l,"<name>")))
      {
        p+=6;
        strncpy(name,p,LENNAME);
        if ((p=strstr(name,"</name>"))) *p=0;
        ret|=0x2;
      }
    }
    if (strstr(l,"</wpt")) break; 
  }
  if (((ret&0x3)==0x3)) return 1;
  return 0;
}

static gchar *slat,*slon,*snam;
static void sel_func(GtkWidget      *clist,
                     gint            row,
                     gint            column,
	             GdkEventButton *event,
                     gpointer        data)
{
  gtk_clist_get_text(GTK_CLIST(clist), row, 0, &snam);
  gtk_clist_get_text(GTK_CLIST(clist), row, 1, &slat);
  gtk_clist_get_text(GTK_CLIST(clist), row, 2, &slon);
//printf("%s  %s\n",slat,slon);
}
#define LAB_USEPOS "Set User location"
#define LAB_POSLON   "!Longitude     "
#define LAB_POSLAT   "!Latitude   "
#define LAB_POSNAM   "!Name       "
#define LAB_ELEDET   "!detect level"

static void sel_func2(GtkWidget *w,gpointer data)
{
  char *name=(char *)data;
  if (!strcmp(name,LAB_USEPOS))
  {
    GtkWidget *pwnd=Find_Window(w,"Preferences");
    if ((slon) && (slat) && (snam))
    {
      Set_Entry(pwnd,LAB_POSNAM,snam);
      Set_Adjust(pwnd,LAB_POSLON,"%f",atof(slon));
      Set_Adjust(pwnd,LAB_POSLAT,"%f",atof(slat));
    }
    else
      Create_Message("Warning","Nothing slected");
  }
}

int list_places(GtkWidget *pwnd)
{
  GtkWidget *wnd,*w[3];
  FILE *fp;
  char *tmp[5];
  char tmp2[2][100];
  char name[LENNAME];
  char placefile[100];
  snam=NULL,slat=NULL; slon=NULL;
  float lon,lat;
  if (!(search_file(db->pref_placesfile,placefile,db->cur_dir,db->home_dir,db->prog_dir))) return 1;
  if (!(fp=fopen(placefile,"r"))) return 1;
  wnd=Create_Window(pwnd,300,200,"",NULL);
  w[1]=Create_Clist("Places",sel_func,NULL,NULL,3,"Name",15,"lat",12,"lon",12,NULL);
  w[2]=Create_Button(LAB_USEPOS,sel_func2);
  w[0]=Pack(NULL,'v',w[1]->parent,1,w[2],1,NULL);
  gtk_container_add(GTK_CONTAINER(wnd),w[0]);
  gtk_widget_show(wnd);
  while (rd_wp(fp,&lat,&lon,name))
  {
    tmp[0]=name;
    sprintf(tmp2[0],"%f",lat);
    tmp[1]=tmp2[0];
    sprintf(tmp2[1],"%f",lon);
    tmp[2]=tmp2[1];
    gtk_clist_append(GTK_CLIST(w[1]), tmp);
  }
  fclose(fp);
  return 0;
}
