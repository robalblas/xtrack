/**************************************************
 * RCSId: $Id: subwnds.c,v 1.7 2018/02/02 23:13:19 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: subwnds.c,v $
 * Revision 1.7  2018/02/02 23:13:19  ralblas
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
#include "xtrack.h"
#include "defines.h"
#include "xtrack_func.h"
#include "orbit.h"
extern DBASE *db;

#define LAB_SATSEL "!Satselect"
#define LAB_EXIT "Exit"



#define LAB_CLOSE_KE "Close"


#define LAB_CLOSE_PD "Close"
#define LAB_GENPD    "Predict"
#define LAB_GENPR    "Save track selected"
#define LAB_STRT1    "Reset daynr"
#define LAB_STRT2    "Set daynr"
#define LAB_GENPS    "Save as ps"
#define LAB_GENTXT   "Save as txt"

#define LAB_SETSTRT  "Set time"
#define LAB_SPEED    "Speed"
#define LAB_OPENUSB  "Open USB"
#define LAB_STATUSB  "!usbstat"

static struct tm simul_start_tm;

void callback_strttm(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  if (strstr(name,"year"))db->genstart_tm.tm_year=GTK_ADJUSTMENT(widget)->value-1900;
  if (strstr(name,"mon"))db->genstart_tm.tm_mon=GTK_ADJUSTMENT(widget)->value-1;
  if (strstr(name,"day"))db->genstart_tm.tm_mday=GTK_ADJUSTMENT(widget)->value;
  if (strstr(name,"hour"))db->genstart_tm.tm_hour=GTK_ADJUSTMENT(widget)->value;
  if (strstr(name,"min"))db->genstart_tm.tm_min=GTK_ADJUSTMENT(widget)->value;
  if (strstr(name,"sec"))db->genstart_tm.tm_sec=GTK_ADJUSTMENT(widget)->value;
}


void lijst1(GtkContainer *wdgt)
{
  GtkWidget *cur_wdgt;
  char *cur_wdgt_id;
  GList *node;
  GList *List_Childs;
/* Create a list of widget in current container */
  List_Childs = gtk_container_children(wdgt);

/* Loop through all these widgets */
  for (node = List_Childs; node != NULL; node = node->next)
  {
    cur_wdgt = (GtkWidget *)node->data;
    cur_wdgt_id = (char *)gtk_object_get_data(GTK_OBJECT(cur_wdgt),WDGT_ID);
/*    if (cur_wdgt_id) printf("%s\n",cur_wdgt_id);*/

    if (GTK_IS_CONTAINER(cur_wdgt))
    {
      lijst1(GTK_CONTAINER(cur_wdgt));
    }

  }
}

void lijst(GtkContainer *wdgt)
{
  wdgt=(GtkContainer *)Find_Parent_Window(wdgt);
  lijst1(wdgt);
}

#define LAB_SAT "Satellite"
#define LAB_TMSTART "Start prediction"
#define LAB_TMPRRNG "Prediction range"
#define LAB_ELEV_HOR "^Horizon"
#define LAB_ELEV_DET "^Detect"
void callback_rngtm(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  if (strstr(name,"year"))db->genrange_tm.tm_year=GTK_ADJUSTMENT(widget)->value-1900;
  if (strstr(name,"mon"))
  {
    db->genrange_tm.tm_mon=GTK_ADJUSTMENT(widget)->value-1;
    Set_Adjust(Find_Widget(widget,LAB_TMPRRNG),"/^day","%d",0);
  }

  if (strstr(name,"day"))db->genrange_tm.tm_mday=GTK_ADJUSTMENT(widget)->value;
  if (strstr(name,"hour"))db->genrange_tm.tm_hour=GTK_ADJUSTMENT(widget)->value;
  if (strstr(name,"min"))db->genrange_tm.tm_min=GTK_ADJUSTMENT(widget)->value;
  if (strstr(name,"sec"))db->genrange_tm.tm_sec=GTK_ADJUSTMENT(widget)->value;

  if (!strcmp(name,LAB_ELEV_HOR))
  {
    db->elev_horiz=D2R(GTK_ADJUSTMENT(widget)->value);
  }
  if (!strcmp(name,LAB_ELEV_DET))
  {
    db->elev_det=D2R(GTK_ADJUSTMENT(widget)->value);
  }
}

int nr_sats(SAT *s)
{
  int n=0;
  for (s=db->sat; s; s=s->next) if ((s->selected) || (s->visible)) n++;
  return n;
}

static int spd;
static int incr_time(gpointer pntr)
{
  GtkWidget *wnd=(GtkWidget *)pntr;
  static float ms;
  struct tm tm=db->glob_tm_ms.tm;
  ms+=(db->reptime*spd);
  if (ms>=1.) { tm.tm_sec+=(int)ms; ms=ms-(int)ms; }

  tm.tm_hour+=(int)db->utc_offset;
  mktime_ntz(&tm);

  tm2adjust(wnd,tm);
/*
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
    Set_Adjust(wnd,SEC,"%d",tm.tm_sec+delta); 
    db->disable_draw=FALSE;
    Set_Adjust(wnd,SEC,"%d",tm.tm_sec-delta); 
  }
*/
  return 1;
}

#define MAXSPEED 1000
static void run_time(int n,GtkWidget *wnd)
{
  static int to=0;
  spd=MAX(MIN(spd,MAXSPEED),1);
  if ((n) && (!to))
  {
    to=gtk_timeout_add(db->reptime*1000,incr_time,(gpointer)wnd);
  }
  else if (to)
  {
    gtk_timeout_remove(to); to=0;
  }
}

static TRACK *seltrack;
void callback_pred(GtkWidget *widget,gpointer data)
{
  GtkWidget *wnd=Find_Window(Find_Parent_Window(widget),Lab_Main );
  char *name=(char *)data;
  if (!strcmp(name,LAB_CLOSE_PD))
  {
    GtkWidget *wnd=gtk_widget_get_toplevel(widget);
    Close_Window(wnd);
  }
  if (!strcmp(name,LAB_GENPD))
  {
    if (nr_sats(db->sat)>1)
      Set_Entry(widget,LAB_SAT,"more than 1");
    else
      Set_Entry(widget,LAB_SAT,db->sat_sel->satname);
    show_predict(widget,db->sat,TRUE,db->genstart_tm,db->genrange_tm);
  }
  if (!strcmp(name,LAB_GENPR))
  {
     if (seltrack)
     {
       FILE *fp;
       fp=fopen("track.txt","w");
      if (!fp) 
      {
        Create_Message("Error","Can't create 'track.txt'.");
        return;
      }
       pri_track(fp,seltrack->sat,seltrack->up_time,seltrack->down_time);
       Create_Message("Info","Saved track of selected in track.txt");
       fclose(fp);
     }
  }

  if (!strcmp(name,LAB_STRT1))
  {
/* Gaat "toevallig" goed 2x "^day"! */
    Set_Adjust(widget,"^day","%d",1);
    Set_Adjust(widget,"^hour","%d",0);
    db->genstart_tm.tm_min=0;
    db->genstart_tm.tm_sec=0;
  }
  if (!strcmp(name,LAB_STRT2))
  {
    db->genstart_tm=mom_tm(0.,&db->tm_off);
    Set_Adjust(widget,"^year","%d",db->genstart_tm.tm_year+1900);
    Set_Adjust(widget,"^month","%d",db->genstart_tm.tm_mon);
    Set_Adjust(widget,"^day","%d",db->genstart_tm.tm_mday);
    Set_Adjust(widget,"^hour","%d",db->genstart_tm.tm_hour);
  }

  if (!strcmp(name,LAB_GENPS))
  {
    generate_ps(db->track,db->sat_sel);
  }
  if (!strcmp(name,LAB_GENTXT))
  {
    FILE *fp;
    fp=fopen("passes.txt","w");
    if (!fp) 
    {
      Create_Message("Error","Can't create 'passes.txt'.");
      return;
    }
    generate_txt(fp,db->track,db->sat_sel);
    Create_Message("Info","Saved passes of selected in 'passes.txt'");
    fclose(fp);
  }
  if (!strcmp(name,LAB_SETSTRT))
  {
    static struct tm tm;
    if (simul_start_tm.tm_year==0) tm=mom_tm(0.,&db->tm_off); else tm=simul_start_tm;
    tm.tm_sec-=2;
    tm.tm_hour+=(int)db->utc_offset;
    mktime_ntz(&tm);
    tm2adjust(wnd,tm);
/*
    Set_Adjust(wnd,SEC,"%d",tm.tm_sec);
    Set_Adjust(wnd,MINT,"%d",tm.tm_min);
    Set_Adjust(wnd,UUR,"%d",tm.tm_hour);
    Set_Adjust(wnd,DAG,"%d",tm.tm_mday);
    Set_Adjust(wnd,MND,"%d",tm.tm_mon+1);
    Set_Adjust(wnd,JAAR,"%d",tm.tm_year+1900);
*/
  }
  if (!strcmp(name,LAB_SIMUL))
  {
    int n;
    Set_Button_if(Find_Window(widget,Lab_Main),LAB_RUN,FALSE);
    Set_Button_if(Find_Window(widget,Lab_WNDDebug),LAB_CONT,FALSE);
    n=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    run_time(n,wnd);
  }
  if (!strcmp(name,LAB_SPEED))
  {
    spd=GTK_ADJUSTMENT(widget)->value;
  }

  #if __ADD_USB__ == 1
  if (!strcmp(name,LAB_OPENUSB))
  {
    int n;
    n=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (n)
      usb_init_connect(&db->usb);
    else 
      usb_close_connect(&db->usb);
  }
  #endif

  {
    static int a=1;
    if (a)
    {
      a=0;
      if (db->usb.ftHandle) 
      {
        Set_Led(widget,LAB_STATUSB,0x0f0);
        Set_Button(widget,LAB_OPENUSB,TRUE);
      }
      else
      {
        Set_Led(widget,LAB_STATUSB,0xf00);
        Set_Button(widget,LAB_OPENUSB,FALSE);
      }
      a=1;
    }
  }
}


GtkWidget *Create_Timespin(char *label,char hv,void func(),struct tm *tm,char *tmspec,gboolean delta_tm)
{
  GtkWidget *w;
  GtkWidget *widgeto;
  widgeto=Pack(NULL,hv,NULL);
  for (; *tmspec; tmspec++)
  {
    gboolean t=(strchr("hH",hv)? TRUE : FALSE);
    w=NULL;
    switch(*tmspec)
    {
      case 'y':
        w=Create_Spin((t?"^year":"year"),func,"%3d%d%d",tm->tm_year+1900,(delta_tm? 0 : 1980),2080);
      break;
      case 'M':
        w=Create_Spin((t?"^month":"month"),func,"%3d%d%d",tm->tm_mon+1,(delta_tm? -12 :1),12);
      break;
      case 'd':
        w=Create_Spin((t?"^day":"day"),func,"%3d%d%d",tm->tm_mday,(delta_tm? -31 :1),31);
      break;
      case 'h':
        w=Create_Spin((t?"^hour":"hour"),func,"%3d%d%d",tm->tm_hour,(delta_tm? -23 :0),23);
      break;
      case 'm':
        w=Create_Spin((t?"^min":"min"),func,"%3d%d%d",tm->tm_min,(delta_tm? -59 :0),59);
      break;
      case 's':
        w=Create_Spin((t?"^sec":"sec"),func,"%3d%d%d",tm->tm_sec,(delta_tm? -59 :0),59);
      break;
    }
    gtk_box_pack_start(GTK_BOX (widgeto), w, FALSE, FALSE, 1);
  }
  gtk_widget_show(widgeto);

  if ((label) && (*label!='!'))
  {
    GtkWidget *frame;
    if (*label)
    {
      frame=gtk_frame_new(label);
      gtk_object_set_data(GTK_OBJECT(frame),WDGT_ID,(gpointer)label);
    }
    else
      frame=gtk_frame_new(NULL);
    gtk_widget_show(frame);
    gtk_container_add(GTK_CONTAINER(frame),widgeto);
    widgeto=frame;
  }
  return widgeto;
}


static void selected_row( GtkWidget      *clist,
                          gint            row,
                          gint            column,
	       	          GdkEventButton *event,
                          gpointer        data)
{
  TRACK *track=db->track;
  GtkWidget *clist_window=Find_Parent_Window(clist);
  GtkWidget *main_window=Find_Window(clist_window,Lab_Main);
  char *text;
  int i;
//warning: passing argument 1 of gtk_clist_get_text from incompatible pointer type
  gtk_clist_get_text((GtkCList *)clist,row,6,&text);
  if (!atoi(text))
  {
    clear_wnd(main_window);
    refresh_wnd(main_window);
    return;
  }
  for (i=0; i<atoi(text)-1; i++)
  {  
    if (track) track=track->next;
  }
  seltrack=track;
  simul_start_tm=track->up_time;
  if (track)
  {
    Set_Button_if(main_window,LAB_RUN,FALSE);
    Set_Button_if(Find_Window(main_window,Lab_WNDDebug),LAB_CONT,FALSE);
    buttons_callback(Find_Widget(main_window,LAB_RUN),LAB_RUN);
    clear_wnd(main_window);
    draw_mark(main_window,track->up_pos,0,"Up");
    draw_mark(main_window,track->down_pos,0,"Down");
    draw_mark(main_window,track->maxelev_pos,0,"Max");
    calcdraw_sensor(track->up_time,main_window,track->sat);
    calcdraw_sensor(track->down_time,main_window,track->sat);
    calcdraw_sensor(track->maxelev_time,main_window,track->sat);
    if (track->sat) Set_Button(main_window,track->sat->satname,TRUE);
    draw_track(main_window,track->sat,track->up_time,track->down_time);
    Set_Local_Button(Find_Widget(main_window,track->sat->satname),SEL_LAB,TRUE);
    refresh_wnd(main_window);
  }
}

static void close_predwnd(GtkWidget *widget)
{
  destroy_window(widget);
  run_time(0,NULL);
}

GtkWidget *Create_Toggle_c(char *name,        /* name of the button */
                          void func(),       /* function to be executed */
                          gboolean *act,
                          GdkColor *normal,
                          GdkColor *prelight,
                          GdkColor *active);

void Create_predict_wnd(GtkWidget *widget)
{
  GtkWidget *main_window=gtk_widget_get_toplevel(widget);
  GtkWidget *wnd,*wa,*wb,*wc,*wd,*we,*wf,*wg,*wl,*wq,*wx,*wy,*wz;

  if (!(db->sat_sel))
  {
    Create_Message("Error","No satellite selected.");
    return;
  }
  db->genstart_tm=mom_tm(0.,&db->tm_off);
  if (!(wnd=Create_Window(main_window,950,240,LAB_PRED,close_predwnd))) return;
  wa=Create_Entry(LAB_SAT,NULL,"%10s",(nr_sats(db->sat)>1? "more than 1" : db->sat_sel->satname));
  wb=Create_Timespin(LAB_TMSTART,'h',callback_strttm,&db->genstart_tm,"yMdh",FALSE);
  wc=Create_Timespin(LAB_TMPRRNG,'h',callback_rngtm,&db->genrange_tm,"Md",TRUE);
  wd=Create_Spin(LAB_ELEV_HOR,callback_rngtm,"%1d%1d%1d%1d",1,(int)(R2D(db->elev_horiz)+.5),-2,50);
  we=Create_Spin(LAB_ELEV_DET,callback_rngtm,"%1d%1d%1d%1d",1,(int)(R2D(db->elev_det)+.5),-2,80);
  wd=Pack("Elevation",'h',wd,1,we,1,NULL);

  wc=Pack(NULL,'h',wc,1,wd,1,NULL);
  wd=Create_Button(LAB_GENPD,callback_pred);
  we=Create_Button(LAB_GENPR,callback_pred);
  wd=Pack(NULL,'h',wd,10,we,10,NULL);
  wf=Create_ButtonArray("Generate Prediction overview",callback_pred,2,
    BUTTON,LAB_STRT1,
    BUTTON,LAB_STRT2,
    BUTTON,LAB_GENPS,
    BUTTON,LAB_GENTXT,
    NULL);

  {
    GtkWidget *wgg[6];
    GdkColor prelight;
    GdkColor active;
    active.red=0xffff; active.green=active.blue=0x8888;
    prelight.red=0xffff; prelight.green=0xaaaa; prelight.blue=0xaaaa;
    wgg[1]=Create_Button(LAB_SETSTRT,callback_pred);
    wgg[2]=Create_Toggle_c(LAB_SIMUL,callback_pred,FALSE,NULL,&prelight,&active);

    wgg[5]=Create_Spin(LAB_SPEED,callback_pred,"%d%d%d",1,1,MAXSPEED);
 //   wgg[3]=Create_Button(LAB_OPENUSB,callback_pred);
    wgg[3]=Create_Toggle_c(LAB_OPENUSB,callback_pred,FALSE,NULL,&prelight,&active);
    wgg[4]=Create_Led(LAB_STATUSB,0x000);


    wgg[1]=Pack(NULL,'h',wgg[1],1,wgg[2],1,NULL);
    wgg[3]=Pack(NULL,'h',wgg[3],1,wgg[4],1,wgg[5],1,NULL);
    wg=Pack("Simulate",'v',wgg[1],1,wgg[3],1,NULL);
  }
/*
  wg=Create_ButtonArray("Simulate",callback_pred,3,
    BUTTON,LAB_SETSTRT,
    TOGGLE,LAB_SIMUL,TRUE,
    SPIN,LAB_SPEED,"%d%d%d",1,1,100,
    NULL);
*/

  wl=Create_Clist("Updown",selected_row,NULL,NULL,7,
                                               "Sat",10,
                                               "Uptime",18,
                                               "Downtime",18,
                                               "Maxtime",18,
                                               "Max elev",8,
                                               "Freq",8,
                                               "",1,
                                                NULL);

  wq=Create_Button(LAB_CLOSE_PD,callback_pred);
  wx=Pack(NULL,'v',wa,5,wb,5,wc,5,wd,5,wf,5,wg,5,NULL);
  wy=Pack(NULL,'h',wx,1,wl,1,NULL);

  wz=Pack(NULL,'v',wy,5,wq,5,NULL);
  gtk_clist_set_column_visibility((GtkCList *)wl,6,FALSE);
  gtk_container_add(GTK_CONTAINER(wnd),wz);
  gtk_widget_show(wnd);
/*
  {
    GtkWidget *ww=Find_Widget(wnd,LAB_SIMUL);
    GtkRcStyle *rc_style;
    GdkColor prelight;
    GdkColor active;
printf("%x\n",ww);
    active.red=0xffff; active.green=active.blue=0x8888;
    prelight.red=0xffff; prelight.green=0xaaaa; prelight.blue=0xaaaa;
    rc_style=Create_RcStylex(NULL,&prelight,&active);
    gtk_widget_modify_style(ww,rc_style);
   gtk_widget_show_all(wnd);
printf("XX: %d\n",Get_Button(wnd,LAB_SIMUL));
  }
*/
}

/*************************************
 * Copy current Kepler/Orbit vars to buttons. 
 * kepler or orbit may be NULL, related vars will not be updated.
 *************************************/
void Set_Keplervals(GtkWidget *wnd,SAT *sat)
{
  GtkWidget *kepler_wnd=Find_Window(wnd,Lab_Keplerwnd);
  KEPLER *kepler=&sat->kepler;
  ORBIT *orbit=&sat->orbit;

/* Update Kepler data */
  if ((kepler) && (kepler_wnd))
  {
    Set_Adjust(kepler_wnd,LAB_EY,"%d",kepler->epoch_year);
    Set_Adjust(kepler_wnd,LAB_ED,"%f",kepler->epoch_day);
    Set_Adjust(kepler_wnd,LAB_DR,"%f",kepler->decay_rate);
    Set_Adjust(kepler_wnd,LAB_IN,"%f",R2D(kepler->inclination));
    Set_Adjust(kepler_wnd,LAB_RA,"%f",R2D(kepler->raan));
    Set_Adjust(kepler_wnd,LAB_EC,"%f",kepler->eccentricity);
    Set_Adjust(kepler_wnd,LAB_PE,"%f",R2D(kepler->perigee));
    Set_Adjust(kepler_wnd,LAB_AN,"%f",R2D(kepler->anomaly));
    Set_Adjust(kepler_wnd,LAB_MO,"%f",kepler->motion);

    Set_Entry(kepler_wnd,LAB_HE,"%f",orbit->height/1000);
    Set_Entry(kepler_wnd,LAB_SN,"%s",sat->satname);

  }
}


#define LAB_Accept "Accept"
#define LAB_CLOSE_SS "Close"
#define LAB_SAVE_SS "Save"
void select_sat_func(GtkWidget *widget, gpointer data)
{
  GtkWidget *select_window=gtk_widget_get_toplevel(widget);
  GtkWidget *main_window=Find_Window(select_window,Lab_Main);
  GtkWidget *wr=Find_Widget(widget,Dlist_RightID);
  char *name=(char *)data;

/* Activate advanced orbit window */
  if (!strcmp(name,LAB_CLOSE_SS))
  {
    Set_Entry(main_window,LAB_SATN,"%s",db->satname);
    Close_Window(select_window);
    fclose(db->fp_norad); db->fp_norad=NULL;
  }

  if (!strcmp(name,LAB_SAVE_SS))
  {
    Save_Prefs(db,TRUE);
  }

  if (!strcmp(name,LAB_Accept))
  {
    SAT *sat;
    int row;
    char *text;

    for (sat=db->sat; sat; sat=sat->next)
    {
      Remove_Widget(main_window,sat->satname);
    }

    sat=NULL;
    for (row=0; ; row++)
    {
      if (!(gtk_clist_get_text(GTK_CLIST(wr),row,0,&text))) break;
      sat=read_msat(db->fp_norad,text,sat);
    }
    add_to_db(db,sat,FALSE);
    update_satbut(main_window,LAB_SATSEL,db->sat);
  }

}

#define PIJLO "[^]"
#define PIJLN "[v]"
#define NR_COL 3
static gint sortfunc(GtkCList *clist,gconstpointer *ptr1, gconstpointer *ptr2)
{
  const GtkCListRow *row1 = (const GtkCListRow *) ptr1;
  const GtkCListRow *row2 = (const GtkCListRow *) ptr2;
  char *s1,*s2;

  s1 = GTK_CELL_TEXT(row1->cell[clist->sort_column])->text;
  s2 = GTK_CELL_TEXT(row2->cell[clist->sort_column])->text;
  if ((!s1) || (!s2)) return 0;
  return strcmp(s1,s2);
}


static int colwidth[]={30,10,10};
static void sort_col(GtkWidget      *widget,
                     gint            column,
                     gboolean        as_desn,
                     gpointer        data)
{
  GtkCList *w_cl=(GtkCList *)widget;
  char str[50],*p;
  int c;
  gtk_clist_set_sort_column(w_cl,column);
  gtk_clist_set_compare_func(w_cl,(GtkCListCompareFunc)sortfunc);
  if (as_desn)
    gtk_clist_set_sort_type(w_cl,GTK_SORT_DESCENDING);
  else
    gtk_clist_set_sort_type(w_cl,GTK_SORT_ASCENDING);
  gtk_clist_sort(w_cl);

  for (c=0; c<NR_COL; c++)
  {
    char *strcol=gtk_clist_get_column_title(w_cl,c);
    if (!strcol) continue;
    strcpy(str,strcol); 
    while (strlen(str)<colwidth[c]) strcat(str," ");
    if ((p=strstr(str,PIJLO))) *p=0;
    if ((p=strstr(str,PIJLN))) *p=0;
    if (c==column)
    {
      if (as_desn)
        strcat(str,PIJLO);
      else
        strcat(str,PIJLN);
    }
    gtk_clist_set_column_title(w_cl,c,str);
  }
}

static void sort_col1(GtkWidget      *widget,
                      gint            icol,
                      gpointer        data)
{
  static gboolean lastcol_sorted;
  static gboolean as_desn=FALSE;
  int column;
  if (!widget)
  {
    as_desn=TRUE;
    return;
  }
  if (icol<0)
  {
    column=lastcol_sorted;
  }
  else
  {
    column=icol;
    as_desn=!as_desn;
  }
  sort_col(widget,column,as_desn,data);
  if (icol>=0)
  {
    lastcol_sorted=column;
  }
}

void select_sat(GtkWidget *widget)
{
  char sat_name[100];
  KEPLER kepler;
  int n=0;

  GtkWidget *main_window=gtk_widget_get_toplevel(widget);
  GtkWidget *wnd,*wa,*wb[3],*wl,*wr,*wz;
  if (!(wnd=Create_Window(main_window,600,200,"Select satellite",NULL))) return;
  wa=Create_Dlist(NULL,sort_col1,"Satellite",12,"Ref time",16,NULL);

  wb[1]=Create_Button(LAB_Accept,select_sat_func);
  wb[2]=Create_Button(LAB_SAVE_SS,select_sat_func);
  wb[3]=Create_Button(LAB_CLOSE_SS,select_sat_func);
  wb[0]=SPack(NULL,"hH",wb[1],"f5",wb[2],"f5",wb[3],"f5",NULL);
/*
  wb[0]=Create_ButtonArray("",select_sat_func,2,BUTTON,LAB_Accept,
                                                BUTTON,LAB_SAVE_SS,
                                                BUTTON,LAB_CLOSE_SS,
                                                NULL);
*/
  wz=SPack(NULL,"v",wa,"fe1",wb[0],"1",NULL);
  gtk_container_add(GTK_CONTAINER(wnd),wz);
  gtk_widget_show(wnd);
  wl=Find_Widget(wnd,Dlist_LeftID);
  wr=Find_Widget(wnd,Dlist_RightID);

  if (!(db->fp_norad=fopen(db->norad_file,"r")))
  {
    Create_Message("Warning1","Kepler-file '%s' not found.",db->norad_file);
    Close_Window(wnd);
    return;
  }
  rewind(db->fp_norad);
  while (read_norad_next_keps(db->fp_norad,sat_name,&kepler))
  {
    char date[100];
    char *tmp[4];

    n++;
/* For 'safety' reasons: Stop at a ridiculous amount */
    if (n>1000)
    {
      tmp[0]="More...";
      tmp[1]="(truncated at 1000)";
      gtk_clist_append(GTK_CLIST(wl), tmp);
      break;
    }
    sprintf(date,"Day %d year %d",(int)kepler.epoch_day,kepler.epoch_year-100);
/* column is sat-name, column 2 shows reference time */
    tmp[0]=sat_name;
    tmp[1]=date;
    if (find_sat(db->sat,sat_name))
    {
      gtk_clist_append(GTK_CLIST(wr), tmp);
    }
    else
    {
      gtk_clist_append(GTK_CLIST(wl), tmp);
    }
  }


/* Check if Kepler data is found */
  if (n==0)
  {
    Create_Message("Warning","No Kepler-data found in:\n  %s.",db->norad_file);
    Close_Window(wnd);
    fclose(db->fp_norad); db->fp_norad=NULL;
  }
  else
  {
    place_window(wnd,0,0,smart_wndpos);
    sort_col1(NULL,0,NULL);
    sort_col1(wl,0,NULL);
  }

}


/*************************************
 * Manage buttons in orbit edit window 
 * Connected to Create_kepleredt_wnd and Create_orbitedt_wnd
 *************************************/
void orbitedt_cmds(GtkWidget *widget, gpointer data)
{
  GtkWidget *wnd=Find_Parent_Window(widget);
  char *name=(char *)data;
  SAT *sat=db->sat_sel;
  if (!sat) return;

/* ===== Manage Kepler window buttons ===== */
/* Take over entered kepler values */
  if (!strcmp(name,LAB_EY))
  {
    sat->kepler.epoch_year=GTK_ADJUSTMENT(widget)->value;
  }
  if (!strcmp(name,LAB_ED))
  {
    sat->kepler.epoch_day=GTK_ADJUSTMENT(widget)->value;
  }
  if (!strcmp(name,LAB_DR))
  {
    sat->kepler.decay_rate=GTK_ADJUSTMENT(widget)->value;
  }
  if (!strcmp(name,LAB_IN))
  {
    sat->kepler.inclination=D2R(GTK_ADJUSTMENT(widget)->value);
  }
  if (!strcmp(name,LAB_RA))
  {
    sat->kepler.raan=D2R(GTK_ADJUSTMENT(widget)->value);
  }
  if (!strcmp(name,LAB_EC))
  {
    sat->kepler.eccentricity=GTK_ADJUSTMENT(widget)->value;
  }
  if (!strcmp(name,LAB_PE))
  {
    sat->kepler.perigee=D2R(GTK_ADJUSTMENT(widget)->value);
  }
  if (!strcmp(name,LAB_AN))
  {
    sat->kepler.anomaly=D2R(GTK_ADJUSTMENT(widget)->value);
  }
  if (!strcmp(name,LAB_MO))
  {
    sat->kepler.motion=GTK_ADJUSTMENT(widget)->value;
  }

  calc_orbitconst(&sat->kepler,&sat->orbit);
  Set_Entry(wnd,LAB_HE,"%f",sat->orbit.height/1000);

  if (!strcmp(name,LAB_CLOSE_KE))
  {
    Close_Window(wnd);
  }
}

/*************************************
 * Define orbit advanced edit window.  
 *************************************/
void Create_kepleredt_wnd(GtkWidget *widget)
{
  GtkWidget *main_window=gtk_widget_get_toplevel(widget);
  GtkWidget *wa,*wb,*wc,*wd,*wx,*wy,*wz;
  GtkWidget *w1;
  GtkWidget *kepler_wnd;
  SAT *sat=db->sat_sel;
  KEPLER *kepler;
  ORBIT *orbit;
  char *frmt="%9.7f%9.7f%9.7f";
  if (!sat) return;

  kepler=&sat->kepler;
  orbit=&sat->orbit;

/* Create new window or pop-up and refresh it if it already exist. */
  if (!(kepler_wnd=Create_Window(main_window,0,0,Lab_Keplerwnd,NULL))) 
  {
    kepler_wnd=Find_Window(main_window,Lab_Keplerwnd);
    if (kepler_wnd) Set_Keplervals(kepler_wnd,sat);
    return;
  }

/* Go on with defining the Kepler edit window */
// uh... waarom volgende regel? Hierdoor kan je bij spin geen getal meer invoeren!
//  gtk_signal_connect(GTK_OBJECT(kepler_wnd), "key_press_event",
//		              GTK_SIGNAL_FUNC(orbitedt_cmds), NULL);
/* Add Kepler edit buttons */
  wa=Create_ButtonArray("Kepler",orbitedt_cmds,3,
              SPIN,LAB_EY,"%9d%9d%9d",kepler->epoch_year,1,300,
              SPIN,LAB_ED,frmt,kepler->epoch_day,0.,366.,
              SPIN,LAB_DR,frmt,kepler->decay_rate,0.,1.,
              SPIN,LAB_IN,frmt,R2D(kepler->inclination),-360.,360.,
              SPIN,LAB_RA,frmt,R2D(kepler->raan),-180.,360.,
              SPIN,LAB_EC,frmt,kepler->eccentricity,0.,1.,
              SPIN,LAB_PE,frmt,R2D(kepler->perigee),-360.,360.,
              SPIN,LAB_AN,frmt,R2D(kepler->anomaly),-360.,360.,
              SPIN,LAB_MO,frmt,kepler->motion,0.,30.,
              0
              );

  wb=Create_Entry(LAB_SN,NULL,"%8s",sat->satname);
  wc=Create_Entry(LAB_HE,NULL,"%.3f",orbit->height/1000.);
  wd=Create_Entry(LAB_AG,NULL,"%d",orbit->data_age);
  wx=Pack(NULL,'h',wb,1,wc,1,wd,1,NULL);

/* Exit button */

  w1=Create_Button(LAB_CLOSE_KE,orbitedt_cmds);
  wy=SPack("","h",w1,"fe5",NULL);

  wz=Pack(NULL,'v',wx,5,wa,5,wy,5,NULL);

  gtk_container_add(GTK_CONTAINER(kepler_wnd),wz);
  gtk_widget_show(kepler_wnd);
  place_window(kepler_wnd,0,0,smart_wndpos);

  return;
}

#define IS_GEO(h) ((h > 33000.) && (h<39000.))
void report_nextpass(GtkWidget *widget,SAT *sat)
{
  struct tm start_tm=db->glob_tm_ms.tm;
  struct tm stop_tm;
  struct tm maxelev_time;
  GtkWidget *text;
  text=Find_Widget(widget,LAB_TEXT);
#ifdef __GTK_20__
  if (text) text=text->parent;
#endif
  if (text)
  {
    TRACK track;
    memset(&track,0,sizeof(TRACK));
    Clear_Text(text);

    if (!IS_GEO(sat->orbit.height/1000.))
    {
      if (predict(sat,&start_tm,&stop_tm,&track,&db->refpos,&db->rotor,db->elev_horiz,db->elev_det,FALSE))
      {
        if (track.max_elev>=db->elev_det)
        {
          int difmin,difsec;
          start_tm.tm_hour+=db->utc_offset; mktime_ntz(&start_tm);
          stop_tm.tm_hour+=db->utc_offset; mktime_ntz(&stop_tm);
          maxelev_time=track.maxelev_time;
          maxelev_time.tm_hour+=db->utc_offset; mktime_ntz(&maxelev_time);
          Add_Text(text,0,"Next pass: %s\n",sat->satname);
          Add_Text(text,0,"Start: %s (%02d-%02d)\n",tmstrshx(&start_tm,"HMS"),start_tm.tm_mday,start_tm.tm_mon+1);
          Add_Text(text,0,"Max. : %s @ %.0f deg.(%s)\n",tmstrshx(&maxelev_time,"HMS"),R2D(track.max_elev),
                                (track.pass_e1_w0? "E" : "W"));
          Add_Text(text,0,"Stop : %s\n",tmstrshx(&stop_tm,"HMS"));
    //      Add_Text(text,0,"Max. elevation: %.0f deg.\n",R2D(track.max_elev));
          difsec=mktime_ntz(&stop_tm)-mktime_ntz(&start_tm);
          difmin=difsec/60; difsec=difsec-(difmin*60);
          Add_Text(text,0,"Pass length: %d:%02d (min:sec)\n",difmin,difsec);
          if (db->hres)
          {
            if (sat->hfreq) Add_Text(text,0,"Freq.: %.1f MHz\n",sat->hfreq);
          }
          else
          {
            if (sat->lfreq) Add_Text(text,0,"Freq.: %.3f MHz\n",sat->lfreq);
          }
        }
      }
      else
      {
        Add_Text(text,db->fontsize,"%s Can't predict\n",sat->satname);
      }
    }
    else // geostat.
    {
      if (predict_colin(sat,&start_tm,&track,&db->refpos))
      {
        start_tm.tm_hour+=db->utc_offset; mktime_ntz(&start_tm);
        Add_Text(text,db->fontsize,
          "Next colinearity %s with sun :\n  %02d-%02d-%4d at %s\n",
            sat->satname,start_tm.tm_mday,start_tm.tm_mon+1,start_tm.tm_year+1900,
            tmstrshx(&start_tm,"HMS"));
        Add_Text(text,db->fontsize,"(pos. w.r.t. observer: %s)",(sat->pass_e1_w0? "E" : "W"));
      }
      else
      {
        Add_Text(text,db->fontsize,"%s Can't predict colin\n",sat->satname);
        Add_Text(text,db->fontsize,"(pos. w.r.t. observer: %s)",(sat->pass_e1_w0? "E" : "W"));
      }
    }
  }
}

void cmd1(GtkWidget *widget, gpointer data)
{
  SAT *sat;
  char *bname=(char *)data;
  char *satname;
  satname=gtk_object_get_data(GTK_OBJECT(widget->parent),WDGT_ID);
  for (sat=db->sat; sat; sat=sat->next)
  {
    GtkWidget *wg;
    wg=Find_Widget(widget,sat->satname);
    if (!wg) continue;
    sat->visible=Get_Local_Button(wg,VIS_LAB);
    sat->selected=Get_Local_Button(wg,SEL_LAB);
    if (!strcmp(bname,SEL_LAB))
    {
      if (sat->selected)
      {
        sat->db->sat_sel=sat;
        report_nextpass(widget,sat);
      }
    }
  }
}

void update_satbut(GtkWidget *w,char *name,SAT *sat)
{
  GtkWidget *w1,*w2,*w3;
  SAT *sat1;
  gboolean r=TRUE;

  if (!w) return;
  if (!sat) return;

  for (sat1=sat; sat1; sat1=sat1->next)
  {
    w1=Create_Check(VIS_LAB,cmd1,sat1->visible);
    w2=Create_Radio(SEL_LAB,r,cmd1);
    w3=Pack(sat1->satname,(db->satsel_bottom?'v' : 'h'),w1,1,w2,1,NULL);
    Add_Widget(w,w3,name,NULL);
    if (sat1->selected)
    {
      sat->db->sat_sel=sat1;
    }
    Set_Local_Button(w3,SEL_LAB,sat1->selected);


    r=FALSE;
  }

}

#define HTTP_KEPLER "http://celestrak.com/NORAD/elements"
#define Lab_Downloadwnd "Kepler file"
#define LAB_Browse "Browse"
#define LAB_Downld "Download"
#define LAB_Use "Use"
#define LAB_KFN "(none)"
#define LAB_KF1 "weather.txt"
#define LAB_KF2 "geo.txt"
#define LAB_KF3 "stations.txt"
#define LAB_DNWLDINFO "^Download info"
#define LAB_KFINFO "^Currently used Kepler file"
#define LAB_Sav "Save"
#define LAB_VIEW "View"

static char kepfile[MAX_FILENAME_LENGTH];
static void file_ok_sel(GtkWidget *widget, GtkFileSelection *fs)
{
  FILE *fp;
  GtkWidget *file_window=Find_Parent_Window(widget);
  if (file_selection_get_filename())
  {
    strcpy(kepfile,(char *)file_selection_get_filename());
  }

  fp=fopen(kepfile,"r");
  if (fp)
  {
    Close_Fileselect(file_window); 
    fclose(fp);
    Set_Entry(widget,LAB_KFINFO,"Selected %s",kepfile);
  }
  else
  {
    Create_Message("Error","Can't open file\n%s.\n",kepfile);
  }
}

// Next needed for WIndows, to prevent set window behind others (caused by running wget)
static void set_above(GtkWidget *w,gboolean n)
{
#if __GTK_WIN32__ == 1
  w=Find_Parent_Window(w);
  if (!GTK_IS_WINDOW(w)) return;
  gtk_window_set_keep_above(w,n);
#endif
}

#define TMP_KEP "tmp_kepfile.tmp"
void func_kepfile(GtkWidget *w,gpointer pntr)
{
  char *name=(char *)pntr;
  char cmd[201],*kf_nopath;
  set_above(w,FALSE);
  if (!strcmp(name,LAB_Browse))
  {
    Create_Fileselectf(w,"Kepler-file",NULL,file_ok_sel,NULL,NULL,NULL,0,NULL);
  }
  else if (!strcmp(name,LAB_Downld))
  {
    if (!*kepfile)
    {
      Set_Entry(w,LAB_DNWLDINFO,"ERROR: No Keplerfile selected");
      return;
    }

    if (test_downloadprog())
    {
      Set_Entry(w,LAB_DNWLDINFO,"ERROR: Download problem; see preferences, tab 'Files'.");
      set_above(w,TRUE);
      return;
    }
    set_above(w,TRUE);

    if (!(kf_nopath=strrchr(kepfile,'/'))) kf_nopath=kepfile; else kf_nopath++;

#if __GTK_WIN32__ == 1
      snprintf(cmd,200,"%s\\%s -q --tries=3 --timeout=10 -O %s %s/%s",db->prog_dir,WGETPROG,TMP_KEP,HTTP_KEPLER,kepfile);
#else
      snprintf(cmd,200,"%s -q --tries=3 --timeout=10 -O %s %s/%s",WGETPROG,TMP_KEP,HTTP_KEPLER,kepfile);
#endif

    if (system(cmd))
    {
      Set_Entry(w,LAB_DNWLDINFO,"Download of %s failed",kf_nopath);
      remove(TMP_KEP);
    }
    else
    {
      Set_Entry(w,LAB_DNWLDINFO,"Download %s OK",kf_nopath);
      remove(kf_nopath);
      rename(TMP_KEP,kf_nopath);
    }
  }
  else if (!strcmp(name,LAB_Use))
  {
    strcpy(db->norad_file,kepfile);
    strcpy(db->pref_noradfile,db->norad_file);
    if (db->fp_norad) fclose(db->fp_norad);
    db->fp_norad=fopen(db->norad_file,"r");
    if (db->fp_norad)
    {
      fclose(db->fp_norad); db->fp_norad=NULL;
    }
  }
  else if (!strcmp(name,LAB_Sav))
  {
    Save_Prefs(db,TRUE);
  }

  else if (!strcmp(name,LAB_VIEW))
  {
#if __GTK_WIN32__ != 1
    snprintf(cmd,100,"gedit %s",db->norad_file);
    system(cmd);
#endif
  }

  else if ((!strcmp(name,LAB_KF1)) || (!strcmp(name,LAB_KF2)) || (!strcmp(name,LAB_KF3)))
  {
    strcpy(kepfile,name);
  }
  else if (!strcmp(name,LAB_KFN))
  {
    *kepfile=0;
  }

  if (*db->norad_file)
  {
    if (exist_file(db->norad_file))
      Set_Entry(w,LAB_KFINFO,"%s",db->norad_file);
    else
      Set_Entry(w,LAB_KFINFO,"Not found: %s",db->norad_file);
  }
  else
  {
    Set_Entry(w,LAB_KFINFO,"Nothing selected.");
  }
 
}

void Get_Kepler(GtkWidget *wnd)
{
  GtkWidget *wa,*wb,*wc,*wz;
  char *fn;
  if (!(fn=strrchr(db->norad_file,DIR_SEPARATOR))) fn=db->norad_file; else fn++;
  if (!(wnd=Create_Window(wnd,0,0,Lab_Downloadwnd,NULL))) return; 
  wa=Create_ButtonArray("Kepler",func_kepfile,2,
              RADIOs,LAB_KFN,LABEL,"",
              RADIOn,LAB_KF1,LABEL,"",
              RADIOn,LAB_KF2,LABEL,"",
              RADIOn,LAB_KF3,LABEL,"",
              BUTTON,LAB_Browse,ENTRY_NOFUNC,"!XXX","%-20s","on your PC",
              BUTTON,LAB_Downld,ENTRY_NOFUNC,"!XXX","%-20s","from Celestrak.com",
              BUTTON,LAB_Use,LABEL,"",
              BUTTON,LAB_Sav,LABEL,"",
#if __GTK_WIN32__ != 1
              BUTTON,LAB_VIEW,LABEL,"",
#endif
              NULL
              );
  wb=Create_Entry(LAB_DNWLDINFO,NULL,"%30s","");
  wc=Create_Entry(LAB_KFINFO,NULL,"%30s",db->norad_file);
  wz=Pack(NULL,'v',wa,5,wb,5,wc,5,NULL);
  if (!strcmp(fn,LAB_KF1)) Set_Button(wa,LAB_KF1);
  if (!strcmp(fn,LAB_KF2)) Set_Button(wa,LAB_KF2);
  if (!strcmp(fn,LAB_KF3)) Set_Button(wa,LAB_KF3);
  gtk_container_add(GTK_CONTAINER(wnd),wz);
  gtk_widget_show(wnd);
  place_window(wnd,0,0,smart_wndpos);

  return; 
}
