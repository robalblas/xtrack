/**************************************************
 * RCSId: $Id: xtrack.c,v 1.6 2018/03/08 09:57:40 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: xtrack.c,v $
 * Revision 1.6  2018/03/08 09:57:40  ralblas
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
#define SCR1
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "xtrack.h"
#include "xtrack_func.h"
#include "defines.h"
#include "orbit.h"

DBASE *db;

/* defaults */

#ifndef RELEASE
#define RELEASE "Unknown"
#endif

void help(char *p)
{
  fprintf(stderr,"%s release %s\n",p,RELEASE);
  fprintf(stderr,"%s [options]\n",p);
  fprintf(stderr,"   [options:]\n");
//  fprintf(stderr,"     -f <norad-filename> (default: %s)\n",DEF_NORADFILE);
 // fprintf(stderr,"     -s <sat_name> (default: %s)\n",DEF_SAT);
  fprintf(stderr,"     -rlon <ref-pos lon> (default: %d)\n",5);
  fprintf(stderr,"     -rlat <ref-pos lat> (default: %d)\n",52);
  fprintf(stderr,"     -rep_time <rep. time> (default: %.1f)\n",db->reptime);
  fprintf(stderr,"     -run_at_start\n");
  fprintf(stderr,"     -m <map-filename> (default: %s)\n",DEF_MAPFILE);
  fprintf(stderr,"     -ppl <value> (default: %2.0f)\n",FACT);
/*
  fprintf(stderr,"     -width <value> (default: %d)\n",720);
  fprintf(stderr,"     -rotordrv <program>\n");
*/
  fprintf(stderr,"  ppl=pixels per longitude.\n");
  fprintf(stderr,"  ppl=1 means: 1 pixel=1 degree, so size is 360x180 pixels.\n");
  fprintf(stderr,"  width=# pixels in x (lon) direction.\n");
}

GtkRcStyle *Create_RcStylex(GdkColor *normal,GdkColor *prelight,GdkColor *active)
{
  GtkRcStyle *rc_style;
  rc_style=gtk_rc_style_new();
  if (normal)
  {
    rc_style->bg[GTK_STATE_NORMAL]=*normal;
    rc_style->color_flags[GTK_STATE_NORMAL] |= GTK_RC_BG;
  }
  if (prelight)
  {
    rc_style->bg[GTK_STATE_PRELIGHT]=*prelight;
    rc_style->color_flags[GTK_STATE_PRELIGHT] |= GTK_RC_BG;
  }
  if (active)
  {
    rc_style->bg[GTK_STATE_ACTIVE]=*active;
    rc_style->color_flags[GTK_STATE_ACTIVE] |= GTK_RC_BG;
  }
  return rc_style;
}

GtkWidget *Create_Toggle_c(char *name,        /* name of the button */
                          void func(),       /* function to be executed */
                          gboolean *act,
                          GdkColor *normal,
                          GdkColor *prelight,
                          GdkColor *active)
{
  GtkWidget *button;
  GtkRcStyle *rc_style;

  button=Create_Toggle(name,func,FALSE);
  rc_style=Create_RcStylex(normal,prelight,active);
  gtk_widget_modify_style(button,rc_style);


  return button;
}

#ifdef SCR1
void scroll_actions(GtkWidget *widget, gpointer data)
{
  GtkWidget *window=Find_Parent_Window(widget);
  GtkWidget *drawing_area=Find_Widget(window,"GTK_DRAWING_AREA");
  char *name=(char *)data;
  float value,lower,upper;
  value=GTK_ADJUSTMENT(widget)->value;
  upper=GTK_ADJUSTMENT(widget)->upper;
  lower=GTK_ADJUSTMENT(widget)->lower;

  if (!db->setscrolflag)       /* To prevent conflict ox/ox ch. in Set_Scrol */
  {
    if (!strcmp(name,"hbar"))
    {
      db->ox=value-lower;
    }
    if (!strcmp(name,"vbar"))
    {
      db->oy=value-lower;
    }
  }
  Update_Main(widget);
  expose_event(drawing_area,NULL);
}
#endif

#define BKNOPPEN_H 200
#define RKNOPPEN_W 200

void Menu_Help(GtkWidget *widget,gpointer data)
{
#if __GTK_WIN32__ == 1
  Create_Message("Program info","xtrack release %s\n%c R. Alblas\nwerkgroep Kunstmanen\nNederland\n",RELEASE,'@');
#else
  Create_Message("Program info","xtrack release %s\nBuild at %s, %s\n%c R. Alblas\nwerkgroep Kunstmanen\nNederland\n",
           RELEASE,__DATE__,__TIME__,'@');
#endif
}

void Menu_Debug(GtkWidget *widget,gpointer data);

/*
void Menu_Debugd(GtkWidget *widget,gpointer data)
{
  GtkWidget *wnd=Find_Parent_Window(widget),*w;
  wnd=Create_Window(wnd,0,0,Lab_WNDDebug,NULL);
  if (!wnd) return;
  w=Create_ButtonArray(NULL,NULL,1,
                          LABEL,"Monitor RS232 commands",
                          ENTRY,LAB_RS232CMD,"%-30s","(No commands sent yet)",
                          0);
  gtk_container_add(GTK_CONTAINER(wnd),w);
  gtk_widget_show(wnd);
  place_window(wnd,0,0,smart_wndpos);
}
*/
#include <locale.h>
int main(int argc, char **argv)
{
  GtkWidget *main_window,*canvas,*menu;
  GdkColor clr[MAXCOL];
  GdkColormap clrmap;
  
  int i;
  int width=0;
  int height=0;
  int scrw;
  int scrh;
  char fn[100];
  int cvsw,cvsh;

  gboolean all_on=FALSE;
//puts(setlocale(LC_NUMERIC,""));
  gtk_disable_setlocale();  // always use decimal point, never comma!
  gtk_init(&argc, &argv);
  gdk_rgb_init();
  db=(DBASE *)calloc(1,sizeof(*db));
  if (!db)  { printf("OUT OF MEM!\n"); exit(1); }
  scrw=gdk_screen_width();
  scrh=gdk_screen_height();

  init_db(db,0);
  Read_Prefs(argv[0],db);
  if (!strchr(db->map_file,DIR_SEPARATOR))  // kale bestandsnaam
  {                                         // dus zoek
    strcpy(fn,db->map_file);
    search_file(fn,db->map_file,
              db->cur_dir,db->home_dir,db->prog_dir);
  }

  db->fontsize=set_fontsize(db->fontsize);

  width=660+RKNOPPEN_W;
  height=(width-RKNOPPEN_W)*190/370+BKNOPPEN_H;
  for (i=1; i<argc; i++)
  {
    if ((!strcmp(argv[i],"-h")))              { help(argv[0]); return 0; }
    if ((!strcmp(argv[i-1],"-m")))            strcpy(db->map_file,argv[i]);
    if ((!strcmp(argv[i-1],"-rlon")))         db->refpos.lon=D2R(atof(argv[i]));
    if ((!strcmp(argv[i-1],"-rlat")))         db->refpos.lat=D2R(atof(argv[i]));
    if ((!strcmp(argv[i],"-all_on")))         all_on=TRUE;
    if ((!strcmp(argv[i-1],"-rep_time")))     db->reptime=atof(argv[i]);
    if ((!strcmp(argv[i],"-run_at_start")))   db->start_now=TRUE;
    if ((!strcmp(argv[i-1],"-ppl")))          db->ppl=atof(argv[i]);
    if ((!strcmp(argv[i-1],"-offset")))       db->tm_off.tz_offset=atof(argv[i]);
    if ((!strcmp(argv[i-1],"-prog_up")))      strcpy(db->prog_up,argv[i]);
    if ((!strcmp(argv[i-1],"-prog_down")))    strcpy(db->prog_down,argv[i]);
    if ((!strcmp(argv[i-1],"-prog_track")))   strcpy(db->prog_track,argv[i]);
    if ((!strcmp(argv[i-1],"-prog_trackup"))) strcpy(db->prog_trackup,argv[i]);
    if ((!strcmp(argv[i-1],"-width")))
    {
      width=atof(argv[i])+RKNOPPEN_W;
      height=(width-RKNOPPEN_W)*190/370+BKNOPPEN_H;
    }
#ifdef XXX
    if ((!strcmp(argv[i-1],"-f")))
    {
      strcpy(db->norad_file,argv[i]);
      if ((!(db->fp_norad=fopen(db->norad_file,"r"))))
      { printf("No file '%s'!\n",db->norad_file); return 1; }
    }
    if ((!strcmp(argv[i-1],"-s")))
    {
      SAT *sat=NULL;
      strcpy(db->satname,argv[i]);
      sat=read_msat(db->fp_norad,db->satname,NULL);
      if (!sat)
      { printf("No sat %s!\n",db->satname); return 1; }
    }
#endif

  }
  db->fact=db->ppl;
/* Clip fact: 0.1 ... 8 */
  db->fact=MAX(MIN(db->fact,8),0.1);

  if ((width<0) && (370*db->fact > scrw))
  {
    width=scrw; height=width*190/370+BKNOPPEN_H;
  }

  cvsw=(360+RAND)*db->fact;
  cvsh=(180+RAND)*db->fact;

  clrmap.colors=clr;
  clrmap.size=MAXCOL;

  ld_clr(clr,SATCOL,0xff,0,0);
  ld_clr(clr,TRACKCOL,0x40,0x40,0x40);
  ld_clr(clr,RASCOL,0x7f,0x7f,0x7f);
  ld_clr(clr,RASCOL2,0xff,0xff,0xff);

  ld_clr(clr,SCANCOL,0x00,0x00,0xff);
  ld_clr(clr,SCANCOL2,0x00,0x00,0xff);

  ld_clr(clr,REFCOL,0xff,0,0xff);
  ld_clr(clr,VISCOL,0xff,0,0);
#ifdef SCR1
  width=height=0;
#endif
  main_window=Create_Window(NULL,width,height,Lab_Main,gtk_main_quit);
  menu=Create_Menu(main_window, MENU_FILE    , NULL          ,0,
                                  MENU_KF    , menu_callback ,BUTTON,
                                  MENU_Q     , gtk_main_quit ,BUTTON,
                                MENU_EDIT    , NULL          ,0,
                                  MENU_SF    , menu_callback ,BUTTON,
                                  MENU_EK    , menu_callback ,BUTTON,
                                  MENU_EDSEP ,NULL           ,0,
                                  MENU_PR    , menu_callback ,BUTTON,
                                MENU_ZOOM    , NULL          ,BUTTON,
                                  MENU_ZMFULL, menu_callback ,BUTTON,
                                  MENU_ZMIN  , menu_callback ,BUTTON,
                                  MENU_ZMOUT , menu_callback ,BUTTON,
                                  MENU_ZMIN2 , menu_callback ,BUTTON,
                                  MENU_ZMOUT2, menu_callback ,BUTTON,
                                MENU_PD      , menu_callback ,BUTTON,
                                MENU_HELP    ,NULL           ,BUTTON,
                                  MENU_PINFO ,Menu_Help      ,BUTTON,
                                  MENU_DEBUG ,Menu_Debug     ,BUTTON,
                                0);


/* Create the canvas */
  canvas=Create_Canvas(main_window,
                cvsw,cvsh,
                configure_event_main,
                key_actions,mouse_actions,mouse_actions);

#ifdef SCR1
  canvas=Add_Scrollbars(canvas,"hbar","vbar",scroll_actions);
#endif
  {
    GtkWidget *w2,*w3,*w4,*w5;
    GtkWidget *wa,*wb,*wc,*wd;
    GtkWidget *wtijd,*wpos,*wx,*wy,*wz;
    GdkColor clr1,clr2;
    clr1.red=0xffff; clr1.green=clr1.blue=0x8888;
    clr2.red=0xffff; clr2.green=0xaaaa; clr2.blue=0xaaaa;

    w2=Create_Toggle_c(LAB_RUN,buttons_callback,FALSE,NULL,&clr2,&clr1);
    gtk_widget_set_tooltip_text(w2,"Start/stop calculations");

    w3=Create_Toggle(ASEL,buttons_callback,FALSE);
    gtk_widget_set_tooltip_text(w3,"Satellite to follow");

    w4=Create_Toggle_c(LAB_EXT,buttons_callback,FALSE,NULL,&clr2,&clr1);
    gtk_widget_set_tooltip_text(w4,"Send position to rotors");

    w5=Create_Toggle_c(LAB_DISPLROT,buttons_callback,FALSE,NULL,&clr2,&clr1);
    gtk_widget_set_tooltip_text(w5,"Display sat dir. or rotor pos.");

    wa=SPack(NULL,"v",w2,"fe1",w3,"1",w4,"1",w5,"1",NULL);

    wtijd=Create_ButtonArray(NULL,time_callback,0,
                              SPIN,JAAR,"%01d%01d%d"  ,db->glob_tm_ms.tm.tm_year+1900,1980,2040,
                              SPIN,MND ,"%01d%01d%d"  ,db->glob_tm_ms.tm.tm_mon+1,0,13,
                              SPIN,DAG ,"%01d%01d%d"  ,db->glob_tm_ms.tm.tm_mday,0,32,
                              SPIN,UUR ,"%01d%01d%01d",db->glob_tm_ms.tm.tm_hour,-1,24,
                              SPIN,MINT,"%01d%01d%01d",db->glob_tm_ms.tm.tm_min,-1,60,
                              SPIN,SEC ,"%01d%01d%01d",db->glob_tm_ms.tm.tm_sec,-1,60,
                              0
                            );
    wtijd=Pack("",'h',wtijd,1,NULL);
    wpos=Create_ButtonArray(NULL,NULL,0,
                             ENTRY,LAB_LON,"%05.1f",0.,
                             ENTRY,LAB_LAT,"%05.1f",0.,
                             ENTRY,LAB_ELE,"%05.1f",0.,
                             ENTRY,LAB_AZI,"%05.1f",0.,
#define XYCALC
#ifdef XYCALC
                             ENTRY,LAB_XXX,"%05.1f",0.,
                             ENTRY,LAB_YYY,"%05.1f",0.,
#endif
                             ENTRY,LAB_DIST,"%-9d",0,
                             0
                           );

wb=NULL;
    wb=Create_Entry(LAB_OU,NULL,"%.2f",db->utc_offset);
    wb=Pack("",'h',wtijd,1,wb,1,NULL);
    wb=Pack(NULL,'v',wpos,1,wb,1,NULL);

    wc=SPack(LAB_SATSEL,(db->satsel_bottom?"hs" : "vs"),NULL);
    gtk_widget_set_usize(wc,120,0);
    wd=Create_Text(LAB_TEXT,FALSE,"Courier 10");
    gtk_widget_set_usize(wd,200,0);

    wx=Pack(NULL,'h',wa,5,wb,5,wd,5,NULL);
    if (db->satsel_bottom)
    {
      wz=Pack(NULL,'v',menu,1,canvas,1,wx,1,wc,1,NULL);
    }
    else
    {
      wy=Pack(NULL,'v',menu,1,canvas,1,wx,1,NULL);
      wz=SPack(NULL,"h",wy,"1",wc,"fe1",NULL);
    }

    gtk_container_add(GTK_CONTAINER(main_window),wz);
    update_satbut(wc,LAB_SATSEL,db->sat);
  }

  place_window(main_window,0,0,left_top);

  gtk_widget_show_all(main_window);
  gtk_widget_realize(main_window);

  Set_Enable_Update(Find_Widget(main_window,"GTK_DRAWING_AREA"),TRUE);
  configure_event_main(Find_Widget(main_window,"GTK_DRAWING_AREA"),NULL);
  refresh_wnd(main_window);

  if (db->start_now)
  {
    Set_Button(main_window,LAB_RUN,TRUE);
    buttons_callback(Find_Widget(main_window,LAB_RUN), LAB_RUN);
  }
  if (db->out_on) Set_Button(main_window,LAB_EXT,TRUE);
  if (db->show_torot) Set_Button(main_window,LAB_DISPLROT,TRUE);

  gtk_main();
#if __ADD_RS232__
  RS232_Open_Close(0,0,0);
#endif
  return 0;
}

void draw_points(GtkWidget *wnd,char *fn)
{
  FILE *fp;
  char l[100],*p;
  float lon, lat;
  if (!(fp=fopen(fn,"r"))) return;
  while (fgets(l,100,fp))
  {
    if (*l=='#') continue;
    if (!(p=strtok(l," "))) continue;
    lat=atof(p);
    if (!(p=strtok(NULL," "))) continue;
    lon=atof(p);
    draw_ref(wnd,D2R(lon),D2R(lat),TRUE);
  }
  fclose(fp);
}
#define TEKENPUNTENn

void refresh_wnd(GtkWidget *wnd)
{
  GdkColor clr;
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  int wnd_width;
  int wnd_height;
  clr=db->clrs.ref_vis; //rgb2clr(0x88,0x88,0x88);
  wnd_width=drawing_area->allocation.width;
  wnd_height=drawing_area->allocation.height;
  draw_ref(wnd,db->refpos.lon,db->refpos.lat,TRUE);
  if (db->show_radiohorizon) draw_vis(wnd,db->sat_sel,&db->refpos,TRUE,clr);
#ifdef TEKENPUNTEN
draw_points(wnd,"scat.txt");
#endif
  Refresh_Rect(drawing_area,0,0,wnd_width,wnd_height);
}



