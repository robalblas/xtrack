/**************************************************
 * RCSId: $Id: xtrack_gtkfuncs.h,v 1.4 2018/02/04 15:11:00 ralblas Exp $
 *
 * func defs for GUI callback funcs
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: xtrack_gtkfuncs.h,v $
 * Revision 1.4  2018/02/04 15:11:00  ralblas
 * _
 *
 * Revision 1.2  2017/04/11 20:58:13  ralblas
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
/* Callbacks */
gboolean key_actions(GtkWidget *, GdkEventKey *);
void mouse_actions(GtkWidget *, GdkEventMotion *);
void buttons_callback(GtkWidget *, gpointer );
void menu_callback(GtkWidget *, gpointer );
void time_callback(GtkWidget *, gpointer );
void configure_event_main(GtkWidget *,GdkEventConfigure *);

void show_predict(GtkWidget *,SAT *,gboolean,struct tm ,struct tm );
void Create_predict_wnd(GtkWidget *);

void Update_Main(GtkWidget *);
void Update_Main_Wait(GtkWidget *);
void tm2adjust(GtkWidget *wnd,struct tm tm);

void draw_onepos(struct tm cur_tm,GtkWidget *wnd,SAT *sat,EPOINT pos_sat);
void draw_onepos_all(GtkWidget *);

void report_nextpass(GtkWidget *,SAT *);
void do_onepos_all(GtkWidget *wnd);
void draw_mark(GtkWidget *,EPOINT ,int , char *);
void draw_track(GtkWidget *,SAT *,struct tm ,struct tm );
void calcdraw_sensor(struct tm ,GtkWidget *,SAT *);

void clear_wnd(GtkWidget *);
void refresh_wnd(GtkWidget *);
void select_sat(GtkWidget *);
void Set_Button_if(GtkWidget *wnd,char *lab,gboolean set);
void Get_Kepler(GtkWidget *wnd);
void draw_rast(GtkWidget *,gboolean);
void draw_ref(GtkWidget *,float ,float ,gboolean);
void draw_horizon(GtkWidget *wnd,float lon,float lat);

void draw_sat(GtkWidget *,SAT *,float ,float ,gboolean);
void draw_sensor(GtkWidget *,SAT *sat,EPOINT *,EPOINT *,gboolean);
void draw_sattrack(GtkWidget *wnd,EPOINT p1,EPOINT p2,gboolean rgbm,int p);
void draw_vis(GtkWidget *,SAT *,EPOINT *,gboolean,GdkColor,float);
void draw_back(GtkWidget *);
void redraw(GtkWidget *,struct tm cur_tm);
void Create_kepleredt_wnd(GtkWidget *);
void select_sat_func(GtkWidget *, gpointer );
int Remove_Widget(GtkWidget *, char *);
void update_satbut(GtkWidget *,char *,SAT *);
// sel_refpos
int list_places(GtkWidget *pwnd);
void Create_preferences_wnd(GtkWidget *);
