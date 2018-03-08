/**************************************************
 * RCSId: $Id: sattrack.h,v 1.5 2018/03/08 10:34:54 ralblas Exp $
 *
 *  
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: sattrack.h,v $
 * Revision 1.5  2018/03/08 10:34:54  ralblas
 * _
 *
 * Revision 1.4  2017/04/23 20:03:39  ralblas
 * _
 *
 * Revision 1.3  2017/04/11 20:56:29  ralblas
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
#ifndef SATTRACK_HDR
#define SATTRACK_HDR
#define ADD_GTK 1

#include <malloc.h>
#include <time.h>

#if ADD_GTK
  #include "gtk/gtk.h"
  #if __GTK_WIN32__
    #include "windows.h"
    #define LPCTSTR LPCSTR
    #include "ftd2xx_win.h"
  #else
    #include "ftd2xx.h"
  #endif
#else
  #ifndef gboolean
    typedef enum {FALSE=0,TRUE} gboolean;
  #endif
#endif

#include "godil_regmap.h"

#ifndef __SCANDIR__
#define  __SCANDIR__
typedef enum {scan_unk=0,N_S,S_N,W_E,E_W} SCANDIR;
#endif

#ifndef PI
#define PI 3.14159265
#endif
#define PIx2 (PI*2.)           /* not present in math.h */
#define D2R(g) ((g)*PI/180.)     /* degree --> radians */
#define R2D(g) ((g)*180./PI)     /* radians --> degree */
#ifndef ABS
#define ABS(a) ((a)<0? (-1*a) : (a))
#endif

typedef enum {satellite=0,sun,moon} SATTYPE;

/*************************************
 * Some constants
 *************************************/
#define G0    9.798                     /* gravity constant */
#define Rearth 6378160.                 /* radius earth (meters) */       
#define Const 9.95                      /* ??? */
#define Earth_offsetangle (98.967440+1) /* correctie! ??? 9-10-2013: extra cor. +1 */
#define LEN_YEAR 365.2422
#define LEN_LEAPYEAR 366.2422

#ifndef Rewind
#define Rewind(s)  while ((s) && ((s)->prev)) (s)=(s)->prev
#endif
#ifndef FORWARD
#define Forward(s) while ((s) && ((s)->next)) (s)=(s)->next
#endif

typedef struct tm_ms
{
  struct tm tm;
  int ms;
} TM_MS;

typedef struct tm_offset
{
  // To correct time from cpu to UTC ("wrong" timezone in CPU)
  float tz_offset;  // timezone (hours); steps of 15 minutes

  // To add small sec. offset and compensate for mech. delay rotor
  float sec_offset; // fixed sec. offset

  int sec_xoffset;  // temp. extra offset, normally 0
  float reptime;    // rep. time in secs to do interpolation (ms)
} TM_OFFSET;

typedef struct kepler
{
  int    epoch_year;
  float  epoch_day;
  float  decay_rate;
  float  inclination;
  float  raan;
  float  eccentricity;
  float  perigee;
  float  anomaly;
  float  motion;
  long   epoch_rev;
} KEPLER;

typedef struct orbit
{
  gboolean valid;                  /* flag: kepler data valid? */

/* Start time record (used) */
  struct tm start_tm;              /* record start time  */
  int       start_ms;              /* record start time msec */

/* Start time record (satellite) */
  struct tm start_tm_sat;          /* record start time  */
  int       start_ms_sat;          /* record start time msec */

/* Start time record (computer) */
  struct tm start_tm_comp;          /* record start time  */

/* Reference time Kepler data */
  long    ref_time;              /* total reference time  */
  struct tm ref_tm;                /* reference time year ... sec */
  int       ref_ms;                /* reference time msec */

  float height;                    /* sat height [meters] */ 
  int   data_age;
  float loop_time;                 /* time 1 loop [seconds] */
  float k2_n;
  float k2_w;
  float dh_w;

/* Some satellite dependent constants */
  float max_sens_angle;
  SCANDIR scan_hor,scan_ver;       /* Movement of satellite */
  int width_original;

  gboolean use_sattime;
  int offset_gmt;
  int offset_sec;
  float offset_lon;
} ORBIT;

typedef struct epoint
{
  float x,y,z;
  float lon,lat;
} EPOINT;

typedef struct direction
{
  float elev,azim;
  float x,y;
  double dist;
  double velo;
} DIRECTION;

typedef struct rotor
{
  gboolean x_at_disc;
  gboolean inv_x,inv_y;   // to correct "wrong" rotor direction
  int xy_rotorlim;
  int deg2step;
  gboolean x_west_is_0;   // def.: east=0
  gboolean y_south_is_0;  // def.: north=0
  gboolean use_xy;
  DIRECTION to_rotor;
  DIRECTION storm;
  int storm_wait_x;
  int storm_wait_y;
} ROTOR;

typedef struct sat
{
  struct sat *prev,*next;
  struct dbase *db;
  struct tm up_time;
  char satname[40];
  int m_ilum;            // moon: % iluminated
  KEPLER kepler;
  ORBIT orbit;
  EPOINT pos;
  DIRECTION dir;
  float max_elev;
  int pass_e1_w0;
  gboolean visible; 
  gboolean selected; 
  SATTYPE type;
  float hfreq,lfreq;
  struct track *track;
  double dist;
  double velo;
} SAT;

typedef struct track
{
  struct track *prev,*next;
  struct sat *sat;
  EPOINT up_pos;
  struct tm up_time;

  EPOINT down_pos;
  struct tm down_time;

  EPOINT maxelev_pos;
  struct tm maxelev_time;
  float max_elev;
  int pass_e1_w0;
  gboolean going_south;
  gboolean done;
} TRACK;


typedef struct diseqc
{
  int addr;
  int cmd;
  int data[2];
  int nr;
  int invert;
  float val;
} DISEQC;

typedef struct rs232_items
{
  int portnr;
  int speed;
  char command[100];
} RS232_ITEMS;

#endif
