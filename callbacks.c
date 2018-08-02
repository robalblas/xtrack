/**************************************************
 * RCSId: $Id: callbacks.c,v 1.7 2018/02/09 18:58:31 ralblas Exp $
 *
 * callbacks of button actions 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: callbacks.c,v $
 * Revision 1.7  2018/02/09 18:58:31  ralblas
 * _
 *
 * Revision 1.6  2018/01/12 08:53:22  ralblas
 * _
 *
 * Revision 1.5  2017/04/27 10:46:49  ralblas
 * _
 *
 * Revision 1.4  2017/04/11 20:43:35  ralblas
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
#include "defines.h"
extern DBASE *db;
struct tm tm2showtm(struct tm tm,int rev);

SAT *nearest(SAT *sat,int x,int y)
{
  SAT *sat1;
  SAT *satmin=NULL;
  gint32 dmin,d;
  int x1,y1;
  dmin=-1;
  for (sat1=sat; sat1; sat1=sat1->next)
  {
    sat1->selected=FALSE;
    x1=LON2X(R2D(sat1->pos.lon));
    y1=LAT2Y(R2D(sat1->pos.lat));
    d=(x1-x)*(x1-x)+(y1-y)*(y1-y);
    if ((dmin<0) || (d<dmin))
    {
      dmin=d;
      satmin=sat1;
    }
  }
  if (satmin) satmin->selected=TRUE;
  return satmin;
}

struct tm tm2showtm(struct tm tm,int rev)
{
  if (rev)
  {
    tm.tm_hour-=(int)db->utc_offset;
    tm.tm_min-=(db->utc_offset - (int)(db->utc_offset))*60;
  }
  else
  {
    tm.tm_hour+=(int)db->utc_offset;
    tm.tm_min+=(db->utc_offset - (int)(db->utc_offset))*60;
  }
  mktime_ntz(&tm);
  return tm;
}

// voorkom "hik" op plaatje bij overflow sec->min enz.
void tm2adjust(GtkWidget *wnd,struct tm tm)
{
  db->disable_draw=TRUE; 
  Set_Adjust(wnd,SEC,"%d",tm.tm_sec);
  Set_Adjust(wnd,MINT,"%d",tm.tm_min);
  Set_Adjust(wnd,UUR,"%d",tm.tm_hour);
  Set_Adjust(wnd,DAG,"%d",tm.tm_mday);
  Set_Adjust(wnd,MND,"%d",tm.tm_mon+1);
  Set_Adjust(wnd,JAAR,"%d",tm.tm_year+1900);
  {
    int delta;
    if (tm.tm_sec>0) delta=-1; else delta=+1;
    tm.tm_sec+=delta;
    Set_Adjust(wnd,SEC,"%d",tm.tm_sec); 
    tm.tm_sec-=delta;
    db->disable_draw=FALSE;
    Set_Adjust(wnd,SEC,"%d",tm.tm_sec); 
  }
}


static gboolean sched_update(gpointer data)
{
  GtkWidget *widget=(GtkWidget *)data;
  GtkWidget *window=Find_Parent_Window(widget);
  Update_Main_Wait(widget);
  refresh_wnd(window);
  return FALSE;
}


/*************************************
 * Update window (scheduled)
 *************************************/
void Update_Main(GtkWidget *widget)
{
  g_idle_add(sched_update,widget);
}

static RGBI rgb_bck;
void Update_Main_Wait(GtkWidget *widget)
{
  GtkWidget *window=Find_Parent_Window(widget);
  GtkWidget *main_window=Find_Window(window,Lab_Main);
  GtkWidget *drawing_area=Find_Widget(window,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);
  int i;

  db->redraw=TRUE;
  db->redrawing=TRUE;
  if (Get_Enable_Update(drawing_area))
  {
    db->redraw=FALSE;
    Set_Enable_Update(drawing_area,FALSE); /* Prevent re-drawing while drawing */
    draw_back(main_window);
    if (!db->redraw)
    {
      draw_rast(main_window,TRUE);
 //     Set_Adjust(main_window,SEC,"%d",db->glob_tm.tm_sec); /* force drawing and setting values */
    }
    else
    {
      db->redraw=FALSE;
      Update_Main(widget);
    }
    if (rgbi)
    {
      if (rgb_bck.rgbbuf) free(rgb_bck.rgbbuf);
      rgb_bck.bufsize=rgbi->bufsize;
      rgb_bck.rgbbuf=malloc(rgb_bck.bufsize);
      rgb_bck.width =rgbi->width;
      rgb_bck.height=rgbi->height;
      for (i=0; i<rgb_bck.bufsize; i++) *(rgb_bck.rgbbuf+i)=*(rgbi->rgbbuf+i);
    }
    Set_Enable_Update(drawing_area,TRUE);
  }
  db->redrawing=FALSE;
}

void configure_event_main(GtkWidget         *widget,
                          GdkEventConfigure *event)
{
  GtkWidget *window=Find_Parent_Window(widget);
  GtkWidget *drawing_area=Find_Widget(window,"GTK_DRAWING_AREA");

  Renew_RGBBuf(drawing_area);
  Update_Main(widget);
}

void zoom_inout(float *z,int *offset,float zfact,int w,char io)
{
  if (io=='i')
  {
    (*z)*=zfact;
    if (*z<1.) *z=1.;
    *offset=*offset+(int)(w/(*z)*(zfact-1.)/2.);
  }
  else
  {
    *offset=*offset-(int)(w/(*z)*(zfact-1.)/2.);
    (*z)/=zfact;
  }
}

gboolean key_actions(GtkWidget *widget, GdkEventKey *event)
{
  GtkWidget *drawing_area=Find_Widget(widget,"GTK_DRAWING_AREA");
  GtkWidget *window=Find_Parent_Window(widget);
  int width=drawing_area->allocation.width/db->ppl;
  int height=drawing_area->allocation.height/db->ppl;
  gboolean redraw=FALSE;
#define FIX_ZMFACT 2.

  switch ((guchar)event->keyval)
  {
    case 'f':
      db->ox=0;
      db->oy=0;
      db->zx=db->zy=1.;
      redraw=TRUE;
    break;
    case 'I':
      zoom_inout(&db->zx,&db->ox,FIX_ZMFACT,width,'i');
      zoom_inout(&db->zy,&db->oy,FIX_ZMFACT,height,'i');
      redraw=TRUE;
    break;
    case 'i':
      db->zx*=FIX_ZMFACT;
      db->zy*=FIX_ZMFACT;
      db->ox=R2D(db->refpos.lon)+LONOFFSET - (width-1)/(db->zx*2);
      db->oy=LATOFFSET-R2D(db->refpos.lat) - (height-1)/(db->zy*2);
      redraw=TRUE;
    break;
    case 'O':
      zoom_inout(&db->zx,&db->ox,FIX_ZMFACT,width,'o');
      zoom_inout(&db->zy,&db->oy,FIX_ZMFACT,height,'o');
/* To prevent coredump in clear_wnd. rgb_bck.bufsize lacks rgb->bufsize! */
      rgb_bck.bufsize=0;
      redraw=TRUE;
    break;
    case 'o':
      db->zx/=FIX_ZMFACT;
      db->zy/=FIX_ZMFACT;
      db->ox=R2D(db->refpos.lon)+LONOFFSET - (width-1)/(db->zx*2);
      db->oy=LATOFFSET-R2D(db->refpos.lat) - (height-1)/(db->zy*2);

/* To prevent coredump in clear_wnd. rgb_bck.bufsize lacks rgb->bufsize! */
      rgb_bck.bufsize=0;
      redraw=TRUE;
    break;
    case GDK_Left&0xff:
      db->ox-=(width/(db->zx*4));
      redraw=TRUE;
    break;
    case GDK_Right&0xff:
      db->ox+=(width/(db->zx*4));
      redraw=TRUE;
    break;
    case GDK_Up&0xff:
      db->oy-=(height/(db->zy*4));
      redraw=TRUE;
    break;
    case GDK_Down&0xff:
      db->oy+=(height/(db->zy*4));
      redraw=TRUE;
    break;
  }

  if (redraw)
  {
    db->setscrolflag=TRUE;
    Set_Scroll(window,"hbar",2*180,&db->ox,&db->zx,
                      "vbar",2* 90,&db->oy,&db->zy);
    db->setscrolflag=FALSE;
/*
    Update_Main(window);
    refresh_wnd(window);
*/
  }
  return TRUE;
}

void mouse_actions(GtkWidget *widget, GdkEventMotion *event)
{
  GdkModifierType state;
  int x,y;
  static gboolean button1_up;
  if (event->is_hint)
  {
    gdk_window_get_pointer(event->window,&x,&y,&state);
  }
  else
  {
    x=event->x;
    y=event->y;
    state=event->state;
  }
  Set_Entry(widget,LAB_CURSPOS,"%4.1f  %4.1f",Y2LAT(y),X2LON(x));
  if (state & GDK_BUTTON1_MASK)
  {
    if (!button1_up)
    {
      SAT *sat=nearest(db->sat,x,y);
      if (sat) Set_Entry(widget,LAB_SATN,"%s",sat->satname);
    }
    button1_up=TRUE;
  }
  if (!(state & GDK_BUTTON1_MASK))
  {
    button1_up=FALSE;
  }
}

static guint idle_handler_id=0;

static int repeated_calc(gpointer p)
{
  struct tm tm;

  GtkWidget *wnd=(GtkWidget *)p;
  db->tm_off.reptime=db->reptime;
  db->glob_tm_ms=mom_tmms(0.,&db->tm_off);
  db->glob_tm=db->glob_tm_ms.tm; //  mom_tm(0.);

  tm=tm2showtm(db->glob_tm_ms.tm,0);

  tm2adjust(wnd,tm);
/*
  Set_Adjust(wnd,SEC,"%d",tm.tm_sec);
  Set_Adjust(wnd,MINT,"%d",tm.tm_min);
  Set_Adjust(wnd,UUR,"%d",tm.tm_hour);
  Set_Adjust(wnd,DAG,"%d",tm.tm_mday);
  Set_Adjust(wnd,MND,"%d",tm.tm_mon+1);
  Set_Adjust(wnd,JAAR,"%d",tm.tm_year+1900);
*/
  Set_Entry(wnd,LAB_OU,"%.2f",db->utc_offset);
//  do_onepos_all(wnd);
  idle_handler_id=0;
  return 0;
}

static int repeated_calc1(gpointer p)
{
  if (idle_handler_id==0)
  {
    idle_handler_id=g_idle_add(repeated_calc,p);
  }
  return 1;
}

// see rs232.c, def. for linux AND windows!
extern char comports[][16];

void buttons_callback(GtkWidget *widget, gpointer data)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  char *name=(char *)data;
  static int to=0;

  if (!strcmp(name,LAB_RUN))
  {
    Set_Button_if(Find_Window(wnd,LAB_PRED),LAB_SIMUL,FALSE);
    Set_Button_if(Find_Window(wnd,Lab_WNDDebug),LAB_CONT,FALSE);
    if ((!to) && (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))))
    {
      int msreptime=(int)(db->reptime*1000);
      if (msreptime<=0) msreptime=1;
      if (wnd)
        to=gtk_timeout_add(msreptime,repeated_calc1,(gpointer)wnd);
    }
    else
    {
      if ((to) && (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))))
      {
        gtk_timeout_remove(to); to=0;
      }
    }
  }
  if (!strcmp(name,ASEL))
  {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
      db->auto_select=TRUE;
    else
      db->auto_select=FALSE;
  }
  if (!strcmp(name,LAB_EXT))
  {
    Update_Togglelabel(widget);
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
    {
      db->out_on=TRUE;
#if __ADD_RS232__
      if (db->to_serial)
      
        if (!RS232_Open_Close(1,db->rs232.portnr, db->rs232.speed))
        {
          Set_Button(wnd,LAB_EXT,0);
          db->out_on=FALSE;
          Create_Message("ERROR","Can't open RS232 port '%s'",comports[db->rs232.portnr]);
//          Create_Info(NULL,"POEP",0,0,NULL,NULL);
        }
#endif
    }
    else
    {
      db->out_on=FALSE;
#if __ADD_RS232__
      if (db->to_serial)
        RS232_Open_Close(0,db->rs232.portnr, db->rs232.speed);
#endif
    }
  }
  if (!strcmp(name,LAB_DISPLROT))
  {
    Update_Togglelabel(widget);
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
    {
      db->show_torot=TRUE;
    }
    else
    {
      db->show_torot=FALSE;
    }
  }

  if (!strcmp(name,SEL))
  {
    select_sat(wnd);
  }
  if (!strcmp(name,EKP))
  {
    Create_kepleredt_wnd(wnd);
  }
}

#ifdef NIETGEBRUIKT
static void file_ok_sel(GtkWidget *widget, GtkFileSelection *fs)
{
  GtkWidget *file_window=Find_Parent_Window(widget);
  if (file_selection_get_filename())
  {
    strcpy(db->norad_file,(char *)file_selection_get_filename());
    strcpy(db->pref_noradfile,db->norad_file);
  }

  if (db->fp_norad) fclose(db->fp_norad);
  db->fp_norad=fopen(db->norad_file,"r");
  if (db->fp_norad)
  {
    Close_Fileselect(file_window); 
    fclose(db->fp_norad); db->fp_norad=NULL;
  }
  else
  {
    Create_Message("Error","Can't open file\n%s.\n",db->norad_file);
  }
}
#endif

void menu_callback(GtkWidget *widget, gpointer data)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  GtkWidget *drawing_area=Find_Widget(widget,"GTK_DRAWING_AREA");
  int width=drawing_area->allocation.width/db->ppl;
  int height=drawing_area->allocation.height/db->ppl;

  char *name=(char *)data;
  gboolean redraw=FALSE;
  if (!strcmp(name,MENU_KF))
  {
    Get_Kepler(wnd);
  }
/*
  if (!strcmp(name,MENU_KF))
  {
    Create_Fileselectf(wnd,"Kepler-file",NULL,file_ok_sel,NULL,NULL,NULL,0,NULL);
  }
*/

  if (!strcmp(name,MENU_SF))
  {
    select_sat(wnd);
  }
  if (!strcmp(name,MENU_EK))
  {
    Create_kepleredt_wnd(wnd);
  }
  if (!strcmp(name,MENU_PR))
  {
    Create_preferences_wnd(wnd);
  }
  if (!strcmp(name,MENU_PD))
  {
    Create_predict_wnd(wnd);
  }
  if (!strcmp(name,MENU_ZMFULL))
  {
    db->zx=1.;
    db->zy=1.;
    redraw=TRUE;
  }
  if (!strcmp(name,MENU_ZMIN))
  {
    db->zx*=FIX_ZMFACT;
    db->zy*=FIX_ZMFACT;
    db->ox=R2D(db->refpos.lon)+LONOFFSET - (width-1)/(db->zx*2);
    db->oy=LATOFFSET-R2D(db->refpos.lat) - (height-1)/(db->zy*2);
    redraw=TRUE;
  }
  if (!strcmp(name,MENU_ZMOUT))
  {
    db->zx/=FIX_ZMFACT;
    db->zy/=FIX_ZMFACT;
    db->ox=R2D(db->refpos.lon)+LONOFFSET - (width-1)/(db->zx*2);
    db->oy=LATOFFSET-R2D(db->refpos.lat) - (height-1)/(db->zy*2);
    redraw=TRUE;
  }
  if (!strcmp(name,MENU_ZMIN2))
  {
    zoom_inout(&db->zx,&db->ox,FIX_ZMFACT,width,'i');
    zoom_inout(&db->zy,&db->oy,FIX_ZMFACT,height,'i');
    redraw=TRUE;
  }
  if (!strcmp(name,MENU_ZMOUT2))
  {
    zoom_inout(&db->zx,&db->ox,FIX_ZMFACT,width,'o');
    zoom_inout(&db->zy,&db->oy,FIX_ZMFACT,height,'o');
    redraw=TRUE;
  }
  if (redraw)
  {
    Set_Scroll(wnd,"hbar",2*180 ,&db->ox,&db->zx,"vbar",2*90,&db->oy,&db->zy);
/*
    Update_Main(wnd);
*/
  }
}

#define IS_GEO(h) ((h > 33000.) && (h<39000.))
/****************************************************************
  Get next visible sat. 
  sat           linked list of sats.
  genstart_tm   start searching from this time
  genstop_tm    stop searching time 
  Return:       track of first visible sat
 ****************************************************************/
static SAT *get_next_sat(SAT *sati,struct tm *start_tm)
{
  TRACK track;
  SAT *next_sat=NULL,*sat;
  struct tm next_tm=*start_tm;
  int nrsat=0;
  next_tm.tm_year++;
  for (sat=sati; sat; sat=sat->next)
  {
    if (!sat->visible) continue;
    if (sat->type==sun) continue;
    if (sat->type==moon) continue;
    if (IS_GEO(sat->orbit.height/1000)) continue;
    nrsat++;
  }
  for (sat=sati; sat; sat=sat->next)
  {
    struct tm tm=*start_tm;

    if (!sat->visible) continue;
    if ((sat->type==sun) && (nrsat>0)) continue;  // sats selected; skip sun
    if ((sat->type==moon) && (nrsat>0)) continue; // sats selected; skip moon
    if (IS_GEO(sat->orbit.height/1000)) continue;
    predict(sat,&tm,NULL,&track,&db->refpos,&db->rotor,db->elev_horiz,db->elev_det,FALSE);
//if (track.max_elev<D2R(2)) { printf("SKIP: %s  %f\n",sat->satname,R2D(track.max_elev)); }
    if (mktime_ntz(&tm) < mktime_ntz(&next_tm))
    {
      next_sat=sat;
      next_tm=tm;
    }
  }
  return next_sat;
}
 
void time_callback(GtkWidget *widget, gpointer data)
{
  char *name=(char *)data;
  struct tm tm=tm2showtm(db->glob_tm_ms.tm,0);
  GtkWidget *wnd=Find_Parent_Window(widget);
  static gboolean dis=FALSE;
  gboolean manual=FALSE;
  if (dis) return;
  dis=TRUE;
  if (!strcmp(name,SEC))
  {
    tm.tm_sec=GTK_ADJUSTMENT(widget)->value;
    manual=TRUE;
  }

  else if (!strcmp(name,MINT))
  {
    tm.tm_min=GTK_ADJUSTMENT(widget)->value;
    manual=TRUE;
  }

  else if (!strcmp(name,UUR))
  {
    tm.tm_hour=GTK_ADJUSTMENT(widget)->value;
    manual=TRUE;
  }
  else if (!strcmp(name,DAG))
  {
    tm.tm_mday=GTK_ADJUSTMENT(widget)->value;
    manual=TRUE;
  }
  else if (!strcmp(name,MND))
  {
    tm.tm_mon=GTK_ADJUSTMENT(widget)->value-1;
    manual=TRUE;
  }
  else if (!strcmp(name,JAAR))
  {
    tm.tm_year=GTK_ADJUSTMENT(widget)->value-1900;
    manual=TRUE;
  }
  if (manual)
  {
    mktime_ntz(&tm);
  
    db->glob_tm_ms.tm=tm2showtm(tm,1);

    // hier NIET tm2adjust gebruiken!!!
    Set_Adjust(wnd,SEC,"%d",tm.tm_sec);
    Set_Adjust(wnd,MINT,"%d",tm.tm_min);
    Set_Adjust(wnd,UUR,"%d",tm.tm_hour);
    Set_Adjust(wnd,DAG,"%d",tm.tm_mday);
    Set_Adjust(wnd,MND,"%d",tm.tm_mon+1);
    Set_Adjust(wnd,JAAR,"%d",tm.tm_year+1900);

  }
  {
    GtkWidget *w;
    if (db->auto_select)
    {
      db->next_sat=get_next_sat(db->sat,&db->glob_tm_ms.tm);
    }
    else
    {
      db->next_sat=NULL;
    }

    if (!db->sat_sel) { dis=FALSE; return; }

    w=Find_Widget(wnd,db->sat_sel->satname);
    if (w) Set_Local_Button(w,SEL_LAB,TRUE);

    do_onepos_all(wnd);
  }
  dis=FALSE;

  return;
}
#include <math.h>

float calcele1(EPOINT *pos_subsat,ORBIT *orbit,EPOINT *refpos)
{
  float gamma;
  float lon_o,lat_o,lon_s,lat_s;
  float dlon,h1;
  float Rsat;
  float elev;
  lon_s=pos_subsat->lon; lat_s=pos_subsat->lat;
  lon_o=refpos->lon;     lat_o=refpos->lat;
  dlon=lon_s-lon_o;
  Rsat=orbit->height+Rearth;
  h1=sin(lat_o)*sin(lat_s) + cos(lat_o)*cos(lat_s)*cos(dlon);
  gamma=2*asin(sqrt((1-(h1))/2.) );
  elev=acos((Rearth-Rsat*cos(gamma))/Rsat)-PI/2.;
  return elev;
}

// Sun shadow
#define SCHADUWFACT 1.2
void clear_wnd(GtkWidget *wnd)
{
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);
  int pp;
  float ele=1.;
  EPOINT refpos;
  float klon,klat,schad;
  ORBIT orbit;
  SAT *zon=Find_Sat(db->sat,"SUN");
  EPOINT sunpos;
  int x,y;
  if (zon) sunpos=zon->pos;
  for (y=0; y<rgbi->height; y++)   // y:== -90...+90
  {
    for (x=0; x<rgbi->width; x++)  // x:== -180...+180
    {
      if (zon)
      {
        if (!(x%4))
        {
          orbit.height=150000000.*1000.;
          klon=X2LON(x);
          klat=Y2LAT(y);
          if ((klon<-180) || (klon>180) || (klat<-90) || (klat>90)) continue;
          refpos.lon=D2R(klon);
          refpos.lat=D2R(klat); 
          ele=calcele1(&sunpos,&orbit,&refpos);
        }
      }
// hier juiste 'pp' (pixpos) berekenen!
      pp=(x+y*rgbi->width)*3;
      if ((ele<0) && (db->enable_shadow) && (db->shadow_fact))
        schad=db->shadow_fact;
      else
        schad=1.;

      *(rgbi->rgbbuf+pp)=*(rgb_bck.rgbbuf+pp)*schad;
      *(rgbi->rgbbuf+pp+1)=*(rgb_bck.rgbbuf+pp+1)*schad;
      *(rgbi->rgbbuf+pp+2)=*(rgb_bck.rgbbuf+pp+2)*schad;
    }
  }
}
