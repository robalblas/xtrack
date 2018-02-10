/**************************************************
 * RCSId: $Id: xtrack.h,v 1.6 2017/08/20 09:18:53 ralblas Exp $
 *
 * main header file 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: xtrack.h,v $
 * Revision 1.6  2017/08/20 09:18:53  ralblas
 * _
 *
 * Revision 1.5  2017/04/23 13:39:30  ralblas
 * _
 *
 * Revision 1.4  2017/04/11 20:55:31  ralblas
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
#define ADD_GTK 1


#define DEF_NORADFILE "norad.txt"
#define DEF_SAT "NOAA-19"
#define DEF_MAPFILE "earthmap.gif"
#define DEF_PLACESFILE "cities.gpx"

#define MAX_FILENAME_LENGTH 512


#if __GTK_WIN32__ == 1
  #define WGETPROG "wget.exe"
#else
  #define WGETPROG "wget"
#endif

#include <float.h>
#ifndef FLT_MAX
  #include <float.h>
#endif

#include <time.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

//typedef enum {mode_unk=0,search_up,search_down,search_maxelev} SEARCH_MODE;

#ifndef MAX
  #define MAX(a,b) ((a)>(b)? (a) : (b))
  #define MIN(a,b) ((a)<(b)? (a) : (b))
#endif


#if ADD_GTK
  #include "gtk/gtk.h"
  #include "gdk/gdkkeysyms.h"
  #include "sgtk.h"

#else
  #ifndef gint32
    typedef long gint32;
  #endif

  #ifndef gboolean
    typedef enum {FALSE=0,TRUE} gboolean;
  #endif
#endif

#include "godil_regmap.h"

#include "sattrack.h"

#define LENSATNAME 40
#define FACT 2.0

#define SEL_LAB "Sel"
#define VIS_LAB "Vis"

typedef enum { unknown_filetype,SUN,TIFF,GIF,JPEG } FILE_TYPE;

#define MAX_PCICLR 4096
typedef struct clrmap
{
  int size;
  int red[MAX_PCICLR],green[MAX_PCICLR],blue[MAX_PCICLR];
} CLRMAP;


typedef struct colors
{
  GdkColor red,green,blue;
  GdkColor ssat_pnt,ssat_vis,ssat_track,ssat_scan;  // selected: point,area,track,scan
  GdkColor usat_pnt,usat_vis,usat_track,usat_scan;  // not selected: same
  GdkColor ref_pnt,ref_vis;
  GdkColor raster,number;
} COLOR;


typedef struct dbase
{
  EPOINT refpos;
  char refpos_name[100];
  EPOINT sun;
  SAT *sat;
  SAT *sat_sel;
  SAT *next_sat;
  int force_pass_e1_w0;
  struct tm glob_tm;
  struct tm_ms glob_tm_ms;
  struct tm_offset tm_off;
//  float time_offset;
//  float time_offset_sec;
  float utc_offset;

  struct tm genstart_tm;
  struct tm genrange_tm;

  char satname[40];
  char norad_file[MAX_FILENAME_LENGTH];
  char pref_noradfile[MAX_FILENAME_LENGTH];
  char map_file[MAX_FILENAME_LENGTH];
  char pref_mapfile[MAX_FILENAME_LENGTH];
  char placesfile[MAX_FILENAME_LENGTH];
  char pref_placesfile[MAX_FILENAME_LENGTH];
  FILE *fp_norad;
  
  struct rotor rotor;
  char ofile[50];
  float ppl;             /* zoom */
  float fact;             /* zoom */
  float zx,zy;
  int ox,oy;
  gboolean redraw;
  gboolean redrawing;
  gboolean setscrolflag;
  float elev_horiz;
  float elev_det;
  gboolean show_radiohorizon;
  struct track *track;
  gboolean hres;
  gboolean start_now;
  gboolean out_on;
  gboolean ext_on;
  gboolean show_torot;
  gboolean to_serial;
  gboolean to_usb;
  gboolean satsel_bottom;
  gboolean auto_select;
  gboolean is_gmtime;
  gboolean disable_draw;
  gboolean enable_shadow;
  float shadow_fact;
  int decoderdisplinfo; // 0: dec-status, 1: rotate status, positions; 2: only positions 
  
  char prog_up[100];
  char prog_down[100];
  char prog_track[100];
  char prog_trackup[100];

  int ps_x_offset,ps_y_offset;
  int ps_max_h;
  int ps_max_t;
  float reptime;
  struct rs232_items rs232;
  struct usb_items usb;
  char used_preffile[MAX_FILENAME_LENGTH];  /* preffile used to read from */
  char *cur_dir;     // loc. where prog. is started
  char *home_dir;    // $HOME in Linux, "\" in Windows (= C:\ or D:\ etc.)
  char *prog_dir;    // location where prog. is installed
  struct colors clrs;
  int rastres;
  int fontsize;
//  int rastclr;
} DBASE;

#ifdef XXXX
typedef struct track
{
  struct track *prev,*next;
  EPOINT up_pos;
  struct tm up_time;

  EPOINT down_pos;
  struct tm down_time;

  EPOINT maxelev_pos;
  struct tm maxelev_time;
  float max_elev;

  gboolean going_south;
} TRACK;
#endif



#define Rewind(s)  while ((s) && ((s)->prev)) (s)=(s)->prev
#define Forward(s) while ((s) && ((s)->next)) (s)=(s)->next

#include "xtrack_gtkfuncs.h"
#include "xtrack_func.h"
//float alpha_2_beta(float ,SCANDIR,int ,int ,float );


