/**************************************************
 * RCSId: $Id: prefer.c,v 1.6 2018/03/08 09:09:55 ralblas Exp $
 *
 * preferenc related functions 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: prefer.c,v $
 * Revision 1.6  2018/03/08 09:09:55  ralblas
 * _
 *
 * Revision 1.5  2018/01/09 21:21:08  ralblas
 * _
 *
 * Revision 1.4  2017/04/23 14:20:37  ralblas
 * _
 *
 * Revision 1.3  2017/04/11 20:34:34  ralblas
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
/*******************************************************************
 * Content:
 *  int search_file(char *rootfile,char *file,
 *               char *cur_dir,char *home_dir,char *prog_dir)
 *  void init_db(DBASE *db,char loc)
 *  void Read_Prefs(char *progname,DBASE *db)
 *  void Save_Prefs(DBASE *db,gboolean save_sat)
 *  void Create_preferences_wnd(GtkWidget *widget)
 *
 *******************************************************************/
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "xtrack.h"
#include "xtrack_func.h"

#define PREF_FILE ".xtrackrc"
#define PREF_FILE2 "xtrack.ini"

#define SEPCHAR " \n	"
#if __GTK_WIN32__ == 1
  #define PORTMIN 1
  #define PORTMAX 16
#else
  #define PORTMIN 0
  #define PORTMAX 29
#endif
extern DBASE *db;
extern char comports[30][16];


// nearest integer
static int nint(float f)
{
  if (f<-0.1)
    return ((int)(f-0.5));
  else
    return ((int)(f+0.5));
}

static char *get_fullarg(char *w)
{
  char *p;
  if (*w=='"')
  {
    w++;
    if ((p=strchr(w,'"')))
    {
      *p=0;
    }
    else
    {
      p=strtok(NULL,"\"");
      if (p) sprintf(w,"%s %s",w,p);
    }
  }
  return w;
}

static void get_dirs(char *progname,char **cur_dir,char **prog_dir,char **home_dir)
{
  freed(cur_dir);
  freed(prog_dir);
  freed(home_dir);

  *cur_dir=g_get_current_dir();
  *prog_dir=get_path(progname);
  *home_dir=(char *)g_get_home_dir();
  #if __GTK_WIN32__ == 1
  {
    if (!(*home_dir))
    {
      if (!strncmp(*cur_dir+1,":\\",2))
      {
        *home_dir=malloc(10);
        strncpy(*home_dir,*cur_dir,2);
        *((*home_dir)+2)=0;
      }
    }
  }
  #endif
  if (!*cur_dir)  strcpyd(cur_dir,".");
  if (!*prog_dir) strcpyd(prog_dir,".");
  if (!*home_dir) strcpyd(home_dir,".");
}

int search_file(char *rootfile,char *file,
                 char *cur_dir,char *home_dir,char *prog_dir)
{
/* Search pref-file */
  if (cur_dir)
    sprintf(file,"%s%c%s",cur_dir,DIR_SEPARATOR,rootfile);

  if (!(exist_file(file)))
  {
    *file=0;
    if (home_dir)
      sprintf(file,"%s%c%s",home_dir,DIR_SEPARATOR,rootfile);

    if (!(exist_file(file)))
    {
      *file=0;
      if (prog_dir)
        sprintf(file,"%s%c%s",prog_dir,DIR_SEPARATOR,rootfile);

      if (!(exist_file(file)))
      {
        *file=0;
      }
    }
  }
  if (!*file) return 0;
  return 1;
}

static char *search_file1(char *rootfile)
{
  static char *file;
  int len=0;
  len=MAX(len,strlen(db->cur_dir)+strlen(rootfile)+10);
  len=MAX(len,strlen(db->home_dir)+strlen(rootfile)+10);
  len=MAX(len,strlen(db->prog_dir)+strlen(rootfile)+10);
  if (file) free(file);
  file=malloc(len);
  if (!(search_file(rootfile,file,
              db->cur_dir,db->home_dir,db->prog_dir))) return NULL;
  return file;
}

static GdkColor int2gdkclr(int rgb)
{
  GdkColor clr;
  clr.red=(rgb>>16)&0xff;
  clr.green=(rgb>>8)&0xff;
  clr.blue=(rgb>>0)&0xff;
  return clr;
}

static int gdkclr2int(GdkColor clr)
{
  int rgb=0;
  rgb+=(clr.red&0xff)<<16;
  rgb+=(clr.green&0xff)<<8;
  rgb+=(clr.blue&0xff);
  return rgb;
}


#ifdef KOMTNOG
static PREFITEMS *pf;
void Load_Defaults(DBASE *db)
{
  pf=NULL;
  Create_TPref(&pf,"# xtrack.ini");
  Create_FPref(&pf,"Observer_Lon"     ,&db->refpos.lon       ,NULL,5.);  // D2R?
  Create_FPref(&pf,"Observer_Lat"     ,&db->refpos.lat       ,NULL,52.); // D2R?
  Create_SPref(&pf,"Map_file"         ,db->pref_mapfile      ,DEF_MAPFILE);
//  Create_SPref(&pf,""                 ,db->map_file          ,DEF_MAPFILE);
  Create_SPref(&pf,"Places_file"      ,db->pref_placesfile   ,DEF_PLACESFILE);
//  Create_SPref(&pf,""                 ,db->placesfile        ,DEF_PLACESFILE);
  Create_IPref(&pf,"Elev_horizon"     ,&db->elev_horiz         ,NULL,0);      
}
#endif

static void default_colors(COLOR *clrs)
{
  clrs->ssat_pnt  =rgb2clr(0xff,0x00,0x00);   // pos. sel. sat
  clrs->ssat_vis  =rgb2clr(0xff,0x00,0xff);   // visual area sel. sat 
  clrs->ssat_track=rgb2clr(0x40,0x40,0x40);   // track sel. sat
  clrs->ssat_scan =rgb2clr(0x00,0x00,0xff);   // Scanline selected sat

  clrs->usat_pnt  =rgb2clr(0xff,0x00,0x00);   // pos not selected sat.
  clrs->usat_vis  =rgb2clr(0xff,0x00,0x00);   // visual area not selected sat
  clrs->usat_track=rgb2clr(0x40,0x40,0x40);   // track not sel. sat
  clrs->usat_scan =rgb2clr(0x10,0x10,0x10);   // Scanline not sel. sat

  clrs->ref_pnt   =rgb2clr(0xff,0x00,0xff);   // reference
  clrs->ref_vis   =rgb2clr(0x00,0x00,0xff);

  clrs->raster    =rgb2clr(0xf0,0xf0,0xf0);
  clrs->number    =rgb2clr(0x00,0x00,0x00);

  clrs->red       =rgb2clr(0xff,0x00,0x00);
  clrs->green     =rgb2clr(0x00,0xff,0x00);
  clrs->blue      =rgb2clr(0x00,0x00,0xff);
}

void init_db(DBASE *db,char loc)
{
  switch(loc)
  {
    case 'E':
      db->refpos.lon=D2R(-2.);
      db->refpos.lat=D2R(52.);
    break;
    default:
      db->refpos.lon=D2R(5.);
      db->refpos.lat=D2R(52.);
    break;
  }
  strcpy(db->pref_mapfile,DEF_MAPFILE);
  strcpy(db->map_file,DEF_MAPFILE);
  strcpy(db->pref_placesfile,DEF_PLACESFILE);
  strcpy(db->placesfile,DEF_PLACESFILE);

  db->elev_horiz=0;
  db->show_radiohorizon=TRUE;
  db->ppl=FACT;
  db->zx=db->zy=1.;
  db->satsel_bottom=FALSE;
  db->glob_tm_ms=mom_tmms(0.,NULL);
  db->genrange_tm.tm_mon=-1;
  db->genrange_tm.tm_mday=1;
  db->reptime=1.;
  db->decoderdisplinfo=1;
  db->is_gmtime=TRUE;
  db->rs232.portnr=0;
  db->rs232.speed=19200;
  db->rotor.storm.x=90.;
  db->rotor.storm.y=90.;
  db->rotor.storm_wait_x=0;
  db->rotor.storm_wait_y=0;
  db->rotor.storm.y=90.;
  db->rotor.storm.elev=90.;
  db->rotor.storm.azim=0.;
  db->rotor.deg2step=16;
  db->rotor.inv_x=FALSE;
  db->rotor.inv_y=FALSE;
  db->rotor.xy_rotorlim=0;
  strcpy(db->rs232.command,"%d,%d,%d");
  default_colors(&db->clrs);
  db->rastres=15;
  db->shadow_fact=0.8;
  db->enable_shadow=TRUE;
}

static SAT *is_sun_moon(char *name,SAT *sat)
{
  if ((!strcmp(name,"SUN")) || (!strcmp(name,"MOON")))
  {
    sat=Create_Sat(&sat);
    strcpy(sat->satname,name);
    if (!strcmp(name,"SUN"))
    {
      sat->type=sun;
      sat->orbit.height=149600000.*1000.;
    }
    if (!strcmp(name,"MOON"))
    {
      sat->type=moon;
      sat->orbit.height=384422.*1000.;  // 363345...405500
    }
  }
  return sat;
}

void Read_Prefs(char *progname,DBASE *db)
{
  FILE *fp;
  SAT *sat=NULL;
  char l[MAX_FILENAME_LENGTH],*w1,*w2;
  char satname[100];
  
/* Load directory names: current, program location etc. */
  get_dirs(progname,&db->cur_dir,&db->prog_dir,&db->home_dir);
  if (!(search_file(PREF_FILE2,db->used_preffile,
              db->cur_dir,db->home_dir,db->prog_dir)))
    search_file(PREF_FILE,db->used_preffile,
              db->cur_dir,db->home_dir,db->prog_dir);
  if (!(fp=fopen(db->used_preffile,"r")))
  {
    add_to_db(db,sat,FALSE); // geen xtrack.ini, lees alleen zon/maan
    return;
  }

  while (fgets(l,MAX_FILENAME_LENGTH,fp))
  {
    if (((w1=strchr(l,':'))) && (*(w1+1)!='\\')) *w1=' ';
    w1=strtok(l,SEPCHAR);    if (!w1) continue;
    w2=strtok(NULL,SEPCHAR); if (!w2) continue;

    if ((!strcmp(w1,"Norad_file")) && (!*db->norad_file))
    {
      char sep[10];
      sprintf(sep,"%c.",DIR_SEPARATOR);
      strcpy(db->pref_noradfile,w2);

      #if __GTK_WIN32__ == 1
        if (*(db->pref_noradfile+1)==':')
          strcpy(db->norad_file,db->pref_noradfile);
        else
      #endif
        if (strchr(sep,*db->pref_noradfile))
          strcpy(db->norad_file,db->pref_noradfile);
        else
          search_file(db->pref_noradfile,db->norad_file,
                      db->cur_dir,db->home_dir,db->prog_dir);

    }
/*
    if (!strcmp(w1,"Hour_Offset"))  db->hour_offset=atoi(w2);
*/
    if (!strcmp(w1,"Hour_Offset"))  db->tm_off.tz_offset=atof(w2); 
    if (!strcmp(w1,"Sec_Offset"))   db->tm_off.sec_offset=atof(w2); 
    if (!strcmp(w1,"Localtime_UTCOffset"))  db->utc_offset=atof(w2);

    if (!strcmp(w1,"Observer_Name"))
    {
      strncpy(db->refpos_name,w2,90);
      if ((w2=strtok(NULL,"\n")))
      {
        strcat(db->refpos_name," ");
        strncat(db->refpos_name,w2,90);
      }
    }
    if (!strcmp(w1,"Observer_Lon"))  db->refpos.lon=D2R(atof(w2));
    if (!strcmp(w1,"Observer_Lat"))  db->refpos.lat=D2R(atof(w2));
    if (!strcmp(w1,"Rotortype"))     db->rotor.use_xy=(strstr(w2,"X-Y")? 1 : 0);
    if (!strcmp(w1,"X_at_Disc"))     db->rotor.x_at_disc=atoi(w2);
    if (!strcmp(w1,"X_at_Dish"))     db->rotor.x_at_disc=atoi(w2);
    if (!strcmp(w1,"X_Dir_0"))       db->rotor.x_west_is_0=(*w2=='w'? TRUE : FALSE);
    if (!strcmp(w1,"Y_Dir_0"))       db->rotor.y_south_is_0=(*w2=='s'? TRUE : FALSE);

    if (!strcmp(w1,"Elev_detect"))   db->elev_det=D2R((float)atoi(w2));
    if (!strcmp(w1,"Elev_horizon"))  db->elev_horiz=D2R((float)atoi(w2));
    if (!strcmp(w1,"Radio_horizon")) db->show_radiohorizon=(*(w2+1)=='n'? TRUE : FALSE);

    if (!strcmp(w1,"Stormpos_x"))    db->rotor.storm.x=atof(w2);
    if (!strcmp(w1,"Stormpos_y"))    db->rotor.storm.y=atof(w2);
    if (!strcmp(w1,"Storm_wait_x"))  db->rotor.storm_wait_x=atoi(w2);
    if (!strcmp(w1,"Storm_wait_y"))  db->rotor.storm_wait_y=atoi(w2);
    if (!strcmp(w1,"Stormpos_elev")) db->rotor.storm.elev=atof(w2);
    if (!strcmp(w1,"Stormpos_azim")) db->rotor.storm.azim=atof(w2);
    if (!strcmp(w1,"DiSEqC_Deg2step"))     db->rotor.deg2step=atoi(w2);
    if (!strcmp(w1,"DiSEqC_Limit"))        db->rotor.xy_rotorlim=atoi(w2);
    if (!strcmp(w1,"Flip_X"))              db->rotor.inv_x=atoi(w2);
    if (!strcmp(w1,"Flip_Y"))              db->rotor.inv_y=atoi(w2);
    if (!strcmp(w1,"Clr.sat_mark_sel"))    db->clrs.ssat_pnt=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.sat_area_sel"))    db->clrs.ssat_vis=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.sat_track_sel"))   db->clrs.ssat_track=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.sat_scanline_sel"))  db->clrs.ssat_scan=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.sat_mark_nsel"))   db->clrs.usat_pnt=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.sat_area_nsel"))   db->clrs.usat_vis=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.sat_track_nsel"))  db->clrs.usat_track=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.sat_scanline_nsel")) db->clrs.usat_scan=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.ref_mark"))        db->clrs.ref_pnt=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.radio_hor"))       db->clrs.ref_vis=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.raster"))          db->clrs.raster=int2gdkclr(strtol(w2,NULL,16));
    if (!strcmp(w1,"Clr.number"))          db->clrs.number=int2gdkclr(strtol(w2,NULL,16));

    if (!strcmp(w1,"Sunshadow"))           db->enable_shadow=(*w2=='y'? TRUE : FALSE);
    if (!strcmp(w1,"Shadow_fact"))         db->shadow_fact=atof(w2);

    if (!strcmp(w1,"Map_file"))
    {
      char sep[10];
      sprintf(sep,"%c.",DIR_SEPARATOR);
      strcpy(db->pref_mapfile,w2);

      #if __GTK_WIN32__ == 1
        if (*(db->pref_mapfile+1)==':')             // full path
          strcpy(db->map_file,db->pref_mapfile);
        else
      #endif
        if (strchr(sep,*db->pref_mapfile))          // full path
          strcpy(db->map_file,db->pref_mapfile);
        else
          search_file(db->pref_mapfile,db->map_file,
                      db->cur_dir,db->home_dir,db->prog_dir);
    }
    if (!strcmp(w1,"Places_file"))
    {
      char sep[10];
      sprintf(sep,"%c.",DIR_SEPARATOR);
      strcpy(db->pref_placesfile,w2);
      #if __GTK_WIN32__ == 1
        if (*(db->pref_placesfile+1)==':')             // full path
          strcpy(db->placesfile,db->pref_placesfile);
        else
      #endif
        if (strchr(sep,*db->pref_placesfile))          // full path
          strcpy(db->placesfile,db->pref_placesfile);
        else
          search_file(db->pref_placesfile,db->placesfile,
                      db->cur_dir,db->home_dir,db->prog_dir);
    }

    if (!strcmp(w1,"Rep_time"))     db->reptime=atof(w2);
    if (!strcmp(w1,"Dec_display"))  db->decoderdisplinfo=atoi(w2);
    if (!strcmp(w1,"Run_at_start"))
      db->start_now=(!strncmp(w2,"y",1)? TRUE : FALSE);
    if (!strcmp(w1,"Ext_progs"))
      db->ext_on=(!strncmp(w2,"y",1)? TRUE : FALSE);
    if (!strcmp(w1,"Output_pos"))
      db->out_on=(!strncmp(w2,"y",1)? TRUE : FALSE);
    if (!strcmp(w1,"Serial"))
      db->to_serial=(!strncmp(w2,"y",1)? TRUE : FALSE);
    if (!strcmp(w1,"Portnr"))
      db->rs232.portnr=MAX(atoi(w2)-PORTMIN,0);
    if (!strcmp(w1,"Baudrate"))
      db->rs232.speed=atoi(w2);
    if (!strcmp(w1,"Poscmd"))
      strncpy(db->rs232.command,get_fullarg(w2),90);

    if (!strcmp(w1,"USB"))
      db->to_usb=(!strncmp(w2,"y",1)? TRUE : FALSE);

    if (!strcmp(w1,"Prog_up"))      strcpy(db->prog_up,get_fullarg(w2));
    if (!strcmp(w1,"Prog_down"))    strcpy(db->prog_down,get_fullarg(w2));
    if (!strcmp(w1,"Prog_track"))   strcpy(db->prog_track,get_fullarg(w2));
    if (!strcmp(w1,"Prog_trackup")) strcpy(db->prog_trackup,get_fullarg(w2));

    if (!strcmp(w1,"Font_size"))    db->fontsize=atoi(w2);

    if (!strcmp(w1,"Buttonsatsel_position"))
      db->satsel_bottom=(!strcmp(w2,"bottom")? TRUE : FALSE);
    if (!strcmp(w1,"Satellite"))
    {
      SAT *sat1;
      gboolean selected=FALSE;
      gboolean visible=FALSE;
      w2=get_fullarg(w2);
      strcpy(satname,w2);
      if ((w2=strtok(NULL,SEPCHAR))) selected=atoi(w2);
      if ((w2=strtok(NULL,SEPCHAR))) visible=atoi(w2);

// melding weggehaald
      if ((!(db->fp_norad=fopen(db->norad_file,"r"))))
      { 
//        Create_Message("Warning2","Kepler-file '%s' not found.",db->norad_file);
      }

      if ((sat1=is_sun_moon(satname,db->sat)))
      {
      }
      else
      {
        sat1=read_msat(db->fp_norad,satname,db->sat);
      }

      if (!sat)
      {
        sat=sat1;
        if (sat1)
        {
          sat1->selected=selected;
          sat1->visible=visible;
        }
      }
      else if (sat1)
      {
        SAT *s;
        for (s=sat; (s) && (s->next); s=s->next);
        s->next=sat1; sat1->prev=s;
        sat1->selected=selected;
        sat1->visible=visible;
      }
      if (db->fp_norad) fclose(db->fp_norad); db->fp_norad=NULL;
    }
  }

  // db->sat=old, sat=new
  add_to_db(db,sat,FALSE);
// moet nog selected/visible voor maan/zon toevoegen, nu zon 'toevallig' gesel. asl in prefs.
  fprintf(stderr,"Preference file %s found.\n",db->used_preffile);

  if ((*db->pref_mapfile) && (!*db->map_file))
  {
      char sep[10];
      sprintf(sep,"%c.",DIR_SEPARATOR);
  #if __GTK_WIN32__ == 1
    if (*(db->pref_mapfile+1)==':')
      strcpy(db->map_file,db->pref_mapfile);
    else
  #endif
    if (strchr(sep,*db->pref_mapfile))
      strcpy(db->map_file,db->pref_mapfile);
    else
      search_file(db->pref_mapfile,db->map_file,
                  db->cur_dir,db->home_dir,db->prog_dir);
  }
}


void Save_Prefs(DBASE *db,gboolean save_sat)
{
  SAT *sat;
  FILE *fp=stdout;
  if (!(fp=fopen(PREF_FILE2,"w")))
  {
    Create_Message("Error","Can't open pref-file %s for write.",PREF_FILE2);
    return;
  }

  fprintf(fp,"Norad_file     %s\n",db->pref_noradfile);
  fprintf(fp,"Hour_Offset  : %3.2f\n",db->tm_off.tz_offset); //   db->time_offset);
  fprintf(fp,"Sec_Offset   : %3.1f\n",db->tm_off.sec_offset); // db->time_offset_sec);
  fprintf(fp,"Localtime_UTCOffset  : %3.2f\n",db->utc_offset);

/*
  fprintf(fp,"Hour_Offset  : %d\n",db->hour_offset);
*/
  fprintf(fp,"Observer_Name: %s\n",db->refpos_name);
  fprintf(fp,"Observer_Lon : %f\n",R2D(db->refpos.lon));
  fprintf(fp,"Observer_Lat : %f\n",R2D(db->refpos.lat));

  fprintf(fp,"Rotortype    : %s\n",(db->rotor.use_xy? "X-Y": "Elev-Azim"));
  fprintf(fp,"X_at_Disc    : %d\n",db->rotor.x_at_disc);
  fprintf(fp,"X_Dir_0      : %s\n",db->rotor.x_west_is_0?"west":"east");
  fprintf(fp,"Y_Dir_0      : %s\n",db->rotor.y_south_is_0?"south":"north");

  fprintf(fp,"Elev_detect  : %d\n",nint(R2D(db->elev_det)));
  fprintf(fp,"Elev_horizon : %d\n",nint(R2D(db->elev_horiz)));
  fprintf(fp,"Radio_horizon: %s\n",(db->show_radiohorizon? "On" : "Off"));
  fprintf(fp,"Stormpos_x   : %.1f\n",db->rotor.storm.x);
  fprintf(fp,"Stormpos_y   : %.1f\n",db->rotor.storm.y);
  fprintf(fp,"Storm_wait_x : %d\n",db->rotor.storm_wait_x);
  fprintf(fp,"Storm_wait_y : %d\n",db->rotor.storm_wait_y);
  fprintf(fp,"Stormpos_elev : %.1f\n",db->rotor.storm.elev);
  fprintf(fp,"Stormpos_azim : %.1f\n",db->rotor.storm.azim);
  fprintf(fp,"DiSEqC_Deg2step : %d\n",db->rotor.deg2step);
  fprintf(fp,"DiSEqC_Limit    : %d\n",db->rotor.xy_rotorlim);
  fprintf(fp,"Flip_X    : %d\n",db->rotor.inv_x);
  fprintf(fp,"Flip_Y    : %d\n",db->rotor.inv_y);

  fprintf(fp,"Map_file     : %s\n",db->pref_mapfile);
  fprintf(fp,"Places_file  : %s\n",db->pref_placesfile);
  fprintf(fp,"Rep_time     : %.1f\n",db->reptime);
  fprintf(fp,"Dec_display  : %d\n",db->decoderdisplinfo);
  fprintf(fp,"Run_at_start : %s\n",(db->start_now? "y" : "n"));
  fprintf(fp,"Output_pos   : %s\n",(db->out_on? "y" : "n"));
  fprintf(fp,"Ext_progs    : %s\n",(db->ext_on? "y" : "n"));

  fprintf(fp,"Serial       : %s\n",(db->to_serial? "y" : "n"));
  fprintf(fp,"USB          : %s\n",(db->to_usb? "y" : "n"));
  fprintf(fp,"Portnr       : %d\n",db->rs232.portnr+PORTMIN);
  fprintf(fp,"Baudrate     : %d\n",db->rs232.speed);
  fprintf(fp,"Poscmd       : \"%s\"\n",db->rs232.command);
  fprintf(fp,"Prog_up      : \"%s\"\n",db->prog_up);
  fprintf(fp,"Prog_down    : \"%s\"\n",db->prog_down);
  fprintf(fp,"Prog_track   : \"%s\"\n",db->prog_track);
  fprintf(fp,"Prog_trackup : \"%s\"\n",db->prog_trackup);

  fprintf(fp,"Font_size    : %d\n",db->fontsize);
  fprintf(fp,"Buttonsatsel_position: %s\n",
              (db->satsel_bottom?"bottom":"right"));

  fprintf(fp,"Clr.sat_mark_sel     : %06x\n",gdkclr2int(db->clrs.ssat_pnt));
  fprintf(fp,"Clr.sat_area_sel     : %06x\n",gdkclr2int(db->clrs.ssat_vis));
  fprintf(fp,"Clr.sat_track_sel    : %06x\n",gdkclr2int(db->clrs.ssat_track));
  fprintf(fp,"Clr.sat_scanline_sel : %06x\n",gdkclr2int(db->clrs.ssat_scan));
  fprintf(fp,"Clr.sat_mark_nsel    : %06x\n",gdkclr2int(db->clrs.usat_pnt));
  fprintf(fp,"Clr.sat_area_nsel    : %06x\n",gdkclr2int(db->clrs.usat_vis));
  fprintf(fp,"Clr.sat_track_nsel   : %06x\n",gdkclr2int(db->clrs.usat_track));
  fprintf(fp,"Clr.sat_scanline_nsel: %06x\n",gdkclr2int(db->clrs.usat_scan));
  fprintf(fp,"Clr.ref_mark         : %06x\n",gdkclr2int(db->clrs.ref_pnt));
  fprintf(fp,"Clr.radio_hor        : %06x\n",gdkclr2int(db->clrs.ref_vis));
  fprintf(fp,"Clr.raster           : %06x\n",gdkclr2int(db->clrs.raster));
  fprintf(fp,"Clr.number           : %06x\n",gdkclr2int(db->clrs.number));

  fprintf(fp,"Sunshadow            : %s\n",(db->enable_shadow? "y" : "n"));
  fprintf(fp,"Shadow_fact          : %.1f\n",db->shadow_fact);

  if (save_sat)
  {
    for (sat=db->sat; sat; sat=sat->next)
    {
      fprintf(fp,"Satellite: \"%s\" %d %d\n",
                           sat->satname,sat->selected,sat->visible);
    }
  }
  fclose(fp);
}


#define LAB_SAVE "Save preferences"
#define LAB_CLOSE "Close"
#define LAB_LOPREF "Local preference file"
#define LAB_GLPREF "Used preference file"
#define LAB_ISGM     "Show time as GMT "
#define LAB_POSLON   "!Longitude     "
#define LAB_POSLAT   "!Latitude   "
#define LAB_POSNAM   "!Name       "
#define LAB_ELEVHOR "!Start tracking at "
#define LAB_ELEVDET "!Only track if reaches "
#define LAB_RADIOHOR "Radio horizon"

#define LAB_RTYPE     "Rotortype:"
#define LAB_RTYPEB    "Elev-Azim/ X-Y "

#define LAB_RADISC  "Dish at:"
#define LAB_XATDISC  "  Y  /  X  "
#define LAB_X_DIR   "east/west"
#define LAB_Y_DIR   "north/south"

#define LAB_STORMX    "!X pos:"
#define LAB_STORMY    "!Y pos:"
#define LAB_STORMXW   "!Wait X:"
#define LAB_STORMYW   "!Wait Y:"

#define LAB_STORME    "!E pos:"
#define LAB_STORMA    "!A pos:"

#define LAB_PROGTRACK   "!Run always          "
#define LAB_PROGUP      "!Run 1x at sat. up   "
#define LAB_PROGDOWN    "!Run 1x at sat. down "
#define LAB_PROGTRACKUP "!Run during sat. up  "

#define LAB_KEPF "!Kepler file"
#define LAB_MAPF "!Map file   "
#define LAB_LOCF "!Locations  "

#define LAB_POSSATSEL   "Button position"
#define LAB_POSSATSEL_R "Right"
#define LAB_POSSATSEL_B "Bottom"

#define LAB_REPTIM      "Interval [sec]"
#define LAB_START       "Run at program start"
#define LAB_OUTON       "Enable at program start"
#define LAB_EXTPROG     "Use external progs"
#define LAB_SERIAL      "Use serial/COM"
#define LAB_FREQ        "L Freq/H Freq"
#define LAB_USB         "Use FTD USB"

#define LAB_USE_EXTPROG "Use ext. prog"
#define LAB_USE_RS232   "Use RS232"

#define LAB_FONTS       "Font size"
#define LAB_OFFSETTM    "!Offset UTC time (hours)"       // als PC-tijd afwijkt van UTC
#define LAB_OFFSETMS    "!Offset UTC time (secs)"        //
#define LAB_OFFSETUTC   "!Presented time: offset UTC "   // invloed getoonde tijd

#if __GTK_WIN32 == 1
  #define LAB_COMPORT  "!RS232 portnr: COM"
#else
  #define LAB_COMPORT  "!RS232 portnr  "
#endif
#define LAB_COMNAME "!portname"

#define LAB_COMSPEED "RS232 baudrate"
#define LAB_COMMAND "!RS232 cmd"

#define LAB_EXAMPLE1 "2 integers"
#define LAB_EXAMPLE2 "2 floats"
#define LAB_EXAMPLE3 "3 integers"
#define LAB_EXAMPLE4 "1 integer, 2 floats"

#define LAB_DEG2STEP "deg2step"
#define LAB_DEQCLIMIT "!Limit"
#define LAB_DEQCLIMITDEG "!Limit deg"
#define LAB_SETLIM "Set lim HH390"

#define LAB_DEQCINVX "Flip X"
#define LAB_DEQCINVY "Flip Y"

#define LAB_PLLIST "!Cities"

#define LAB_SEARCHFILES "Search"

#define LAB_CLRRAST "!lum rast"

#define LAB_STATPOS "status+position"
#define LAB_DECSTAT "Status"
#define LAB_POSITION "Positions"
static float round_to_qh(float t)
{
  int tmp=((int)(t*4.));
  return (float)tmp/4.;
}

static void callback_pref(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  static char *kfn,*mfn;
  char *opt1=soption_menu_get_history(Find_Widget(widget,LAB_COMSPEED));
  char *opt2=soption_menu_get_history(Find_Widget(widget,LAB_POSSATSEL));
  if (!strcmp(name,LAB_OFFSETUTC))
  {
    db->utc_offset=GTK_ADJUSTMENT(widget)->value;
  }
  if (!strcmp(name,LAB_OFFSETTM))
  {
    db->tm_off.tz_offset=GTK_ADJUSTMENT(widget)->value;
    db->tm_off.tz_offset=round_to_qh(db->tm_off.tz_offset);
    Set_Adjust(widget,LAB_OFFSETTM,"%f",(float)db->tm_off.tz_offset);
  }
  if (!strcmp(name,LAB_OFFSETMS))
  {
    db->tm_off.sec_offset=GTK_ADJUSTMENT(widget)->value;
  }
  if (!strcmp(name,LAB_ISGM))
  {
    db->is_gmtime=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  }
  
  if (!strcmp(name,LAB_POSNAM))
  {
    strncpy(db->refpos_name,Get_Entry(widget,LAB_POSNAM),90);
  }

  if (!strcmp(name,LAB_POSLON))
  {
    db->refpos.lon=D2R(GTK_ADJUSTMENT(widget)->value);
  }
  if (!strcmp(name,LAB_POSLAT))
  {
    db->refpos.lat=D2R(GTK_ADJUSTMENT(widget)->value);
  }
  if (!strcmp(name,LAB_PLLIST))
  {
    if (list_places(Find_Parent_Window(widget)))
      Create_Message("Error","Waypoint file '%s' not found.",db->pref_placesfile);
  }

  if (!strcmp(name,LAB_ELEVDET))
  {
    db->elev_det=D2R(GTK_ADJUSTMENT(widget)->value);

  }
  if (!strcmp(name,LAB_ELEVHOR))
  {
    db->elev_horiz=D2R(GTK_ADJUSTMENT(widget)->value);
  }

  if (!strcmp(name,LAB_RADIOHOR))
  {
    db->show_radiohorizon=Get_Button(widget,LAB_RADIOHOR);
  }

  if (!strcmp(name,LAB_RTYPEB))
  {
    Update_Togglelabel(widget);
    db->rotor.use_xy=Get_Button(widget,LAB_RTYPEB);
    Sense_Button(widget,LAB_XATDISC,db->rotor.use_xy);
    Sense_Button(widget,LAB_X_DIR,db->rotor.use_xy);
    Sense_Button(widget,LAB_Y_DIR,db->rotor.use_xy);
    Sense_Button(widget,LAB_STORMX,db->rotor.use_xy);
    Sense_Button(widget,LAB_STORMY,db->rotor.use_xy);
    Sense_Button(widget,LAB_STORMXW,db->rotor.use_xy);
    Sense_Button(widget,LAB_STORMYW,db->rotor.use_xy);

    Sense_Button(widget,LAB_STORME,!db->rotor.use_xy);
    Sense_Button(widget,LAB_STORMA,!db->rotor.use_xy);
  }

  if (!strcmp(name,LAB_XATDISC))
  {
    Update_Togglelabel(widget);
    db->rotor.x_at_disc=Get_Button(widget,LAB_XATDISC);
  }

  if (!strcmp(name,LAB_X_DIR))
  {
    Update_Togglelabel(widget);
    db->rotor.x_west_is_0=Get_Button(widget,LAB_X_DIR);
  }
  if (!strcmp(name,LAB_Y_DIR))
  {
    Update_Togglelabel(widget);
    db->rotor.y_south_is_0=Get_Button(widget,LAB_Y_DIR);
  }

  if (!strcmp(name,LAB_DEG2STEP))
  {
    db->rotor.deg2step=GTK_ADJUSTMENT(widget)->value;
  }
  if (!strcmp(name,LAB_DEQCINVX))
  {
    db->rotor.inv_x=Get_Button(widget,LAB_DEQCINVX);
  }
  if (!strcmp(name,LAB_DEQCINVY))
  {
    db->rotor.inv_y=Get_Button(widget,LAB_DEQCINVY);
  }
  if (!strcmp(name,LAB_DEQCLIMIT))
  {
    db->rotor.xy_rotorlim=GTK_ADJUSTMENT(widget)->value;
    Set_Adjust(widget,LAB_DEQCLIMITDEG,"%f",(float)db->rotor.xy_rotorlim/db->rotor.deg2step);
  }
  if (!strcmp(name,LAB_DEQCLIMITDEG))
  {
    db->rotor.xy_rotorlim=GTK_ADJUSTMENT(widget)->value*db->rotor.deg2step;
    Set_Adjust(widget,LAB_DEQCLIMIT,"%d",db->rotor.xy_rotorlim);
  }
  // lim.: deg2step=8:  lim= 728
  //       deg2step=16: lim=1279
  if (!strcmp(name,LAB_SETLIM))
  {
    db->rotor.xy_rotorlim=db->rotor.deg2step*91;
    db->rotor.xy_rotorlim=MIN(db->rotor.xy_rotorlim,0x4ff);

    Set_Adjust(widget,LAB_DEQCLIMIT,"%d",db->rotor.xy_rotorlim);
    if (db->rotor.deg2step)
      Set_Adjust(widget,LAB_DEQCLIMITDEG,"%f",(float)db->rotor.xy_rotorlim/db->rotor.deg2step);
  }

  if (!strcmp(name,LAB_STORMX))
  {
    db->rotor.storm.x=GTK_ADJUSTMENT(widget)->value;
  }

  if (!strcmp(name,LAB_STORMY))
  {
    db->rotor.storm.y=GTK_ADJUSTMENT(widget)->value;
  }
  if (!strcmp(name,LAB_STORMXW))
  {
    db->rotor.storm_wait_x=GTK_ADJUSTMENT(widget)->value;
  }

  if (!strcmp(name,LAB_STORMYW))
  {
    db->rotor.storm_wait_y=GTK_ADJUSTMENT(widget)->value;
  }

  if (!strcmp(name,LAB_STORME))
  {
    db->rotor.storm.elev=GTK_ADJUSTMENT(widget)->value;
  }

  if (!strcmp(name,LAB_STORMA))
  {
    db->rotor.storm.azim=GTK_ADJUSTMENT(widget)->value;
  }

  if (!strcmp(name,LAB_REPTIM))
  {
    db->reptime=GTK_ADJUSTMENT(widget)->value;
  }

  if (!strcmp(name,LAB_PROGUP))
  {
    strcpy(db->prog_up,Get_Entry(gtk_widget_get_toplevel(widget),LAB_PROGUP));
  }
  if (!strcmp(name,LAB_PROGDOWN))
  {
    strcpy(db->prog_down,Get_Entry(gtk_widget_get_toplevel(widget),LAB_PROGDOWN));
  }
  if (!strcmp(name,LAB_PROGTRACK))
  {
    strcpy(db->prog_track,Get_Entry(gtk_widget_get_toplevel(widget),LAB_PROGTRACK));
  }
  if (!strcmp(name,LAB_PROGTRACKUP))
  {
    strcpy(db->prog_trackup,Get_Entry(gtk_widget_get_toplevel(widget),LAB_PROGTRACKUP));
  }

  if (!strcmp(name,LAB_KEPF))
  {
    kfn=Get_Entry(gtk_widget_get_toplevel(widget),LAB_KEPF);
    if (kfn) strcpy(db->pref_noradfile,kfn);
  }
  if (!strcmp(name,LAB_MAPF))
  {
    mfn=Get_Entry(gtk_widget_get_toplevel(widget),LAB_MAPF);
    if (mfn) strcpy(db->pref_mapfile,mfn);
  }
  if (!strcmp(name,LAB_LOCF))
  {
    char *lfn=Get_Entry(gtk_widget_get_toplevel(widget),LAB_LOCF);
    if (lfn) strcpy(db->pref_placesfile,lfn);
  }

  if (!strcmp(name,LAB_FONTS))
  {
    db->fontsize=GTK_ADJUSTMENT(widget)->value;
  }

  if ((opt2) && (!strcmp(name,opt2)))         /* suppress deselect */
  {
    if (!strcmp(name,LAB_POSSATSEL_R))
      db->satsel_bottom=FALSE;
    if (!strcmp(name,LAB_POSSATSEL_B))
      db->satsel_bottom=TRUE;
  }

  if (!strcmp(name,LAB_START))
  {
    db->start_now=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  }
  if (!strcmp(name,LAB_OUTON))
  {
    db->out_on=Get_Button(widget,LAB_OUTON);
  }
  if (!strcmp(name,LAB_EXTPROG))
  {
    db->ext_on=Get_Button(widget,LAB_EXTPROG);
  }
  if (!strcmp(name,LAB_SERIAL))
  {
    db->to_serial=Get_Button(widget,LAB_SERIAL);
  }
  #if __ADD_USB__ == 1
  if (!strcmp(name,LAB_USB))
  {
    db->to_usb=Get_Button(widget,LAB_USB);
  }
  #endif
  if (!strcmp(name,LAB_FREQ))
  {
    Update_Togglelabel(widget);
    db->hres=Get_Button(widget,LAB_FREQ);
  }
  if (!strcmp(name,LAB_COMPORT))
  {
    db->rs232.portnr=GTK_ADJUSTMENT(widget)->value-PORTMIN;
    Set_Entry(widget,LAB_COMNAME,comports[db->rs232.portnr]);
  }
  if ((opt1) && (!strcmp(name,opt1)))         /* suppress deselect */
  {
    db->rs232.speed=atoi(opt1);
//    db->rs232.speed=GTK_ADJUSTMENT(widget)->value;
  }
  if (!strcmp(name,LAB_COMMAND))
  {
    strcpy(db->rs232.command,Get_Entry(gtk_widget_get_toplevel(widget),LAB_COMMAND));
  }

  if (!strcmp(name,LAB_EXAMPLE1))
  {
    Set_Entry(widget,LAB_COMMAND,"%s","%d,%d");
  }
  if (!strcmp(name,LAB_EXAMPLE2))
  {
    Set_Entry(widget,LAB_COMMAND,"%s","%05.1f,%05.1f");
  }
  if (!strcmp(name,LAB_EXAMPLE3))
  {
    Set_Entry(widget,LAB_COMMAND,"%s","%d,%d,%d");
  }
  if (!strcmp(name,LAB_EXAMPLE4))
  {
    Set_Entry(widget,LAB_COMMAND,"%s","%d,%05.1f,%05.1f");
  }
  if (!strcmp(name,LAB_SEARCHFILES))
  {
    char *fn;
    fn=search_file1(db->pref_noradfile);
    Set_Entry(widget,"!fnkeps","%s",(fn? fn : "Not found"));
    fn=search_file1(db->pref_mapfile);
    Set_Entry(widget,"!fnmaps","%s",(fn? fn : "Not found"));
    fn=search_file1(db->pref_placesfile);
    Set_Entry(widget,"!fnlocs","%s",(fn? fn : "Not found"));
  }

  if (!strcmp(name,LAB_SAVE))
  {
    if (kfn) strcpy(db->pref_noradfile,kfn);
    if (mfn) strcpy(db->pref_mapfile,mfn);
    Save_Prefs(db,TRUE);
  }
  if (!strcmp(name,LAB_CLOSE))
  {
    GtkWidget *wnd=gtk_widget_get_toplevel(widget);
    kfn=NULL;
    mfn=NULL;
    Close_Window(wnd);
  }

  if (!strcmp(name,LAB_STATPOS))
  {
    db->decoderdisplinfo=1;
  } 
  if (!strcmp(name,LAB_DECSTAT))
  {
    db->decoderdisplinfo=0;
  } 
  if (!strcmp(name,LAB_POSITION))
  {
    db->decoderdisplinfo=2;
  } 
  
}

#ifdef XXX
typedef struct tm FunctionType(struct tm *);

#define LAB_HOUR "^hour"
#define LAB_MIN "^min"
#define LAB_SEC "^sec"
#define LAB_MSEC "^msec"
#define FUNCID "FUNCTIM"
static GtkWidget *ww;
static void Create_TimeArray_func(GtkWidget *widget, gpointer data)
{
  struct tm tm;
  static int msec;
  static int stop;
  char *name=(char *)data;

  FunctionType* func=gtk_object_get_data(GTK_OBJECT(ww),FUNCID);
  tm=func(NULL);

  if (stop) return;

  tm.tm_sec=Get_Adjust(widget,LAB_SEC);
  tm.tm_min=Get_Adjust(widget,LAB_MIN);
  tm.tm_hour=Get_Adjust(widget,LAB_HOUR);
  if (!strcmp(name,LAB_MSEC))
  {
    msec=GTK_ADJUSTMENT(widget)->value;
  }

  if (!strcmp(name,LAB_SEC))
  {
    tm.tm_sec=GTK_ADJUSTMENT(widget)->value;
  }

  if (!strcmp(name,LAB_MIN))
  {
    tm.tm_min=GTK_ADJUSTMENT(widget)->value;
  }

  if (!strcmp(name,LAB_HOUR))
  {
    tm.tm_hour=GTK_ADJUSTMENT(widget)->value;
  }
  if (msec>=1000) { tm.tm_sec++; msec-=1000; }
  if (msec<0) { tm.tm_sec--; msec+=1000; }
  mktime_ntz(&tm);

  stop=1;
  Set_Adjust(widget,LAB_MSEC,"%d" ,msec);
  Set_Adjust(widget,LAB_SEC,"%d" ,tm.tm_sec);
  Set_Adjust(widget,LAB_MIN,"%d",tm.tm_min);
  Set_Adjust(widget,LAB_HOUR,"%d" ,tm.tm_hour);
  stop=0;
  func(&tm);
}
GtkWidget *Create_TimeArray(char *name,struct tm func(),struct tm tm,int msec)
{
  GtkWidget *w;
  w=Create_ButtonArray(name,Create_TimeArray_func,4,
      SPIN,LAB_HOUR  ,"%d%d%d",tm.tm_hour,-23,23,
      SPIN,LAB_MIN   ,"%d%d%d",tm.tm_min,-1,60,
      SPIN,LAB_SEC   ,"%d%d%d",tm.tm_sec,-1,60,
      SPIN,LAB_MSEC  ,"%d%d%d%d",100,msec,-1000,1000,
      NULL);

  func(&tm);
  gtk_object_set_data(GTK_OBJECT(w),FUNCID,(gpointer)func);
ww=w;
  return w;
}

struct tm abc(struct tm *itm)
{
  static struct tm tm;
  if (itm) tm=*itm;
  return tm;
}
#endif

static GtkWidget *pref_obs()
{
  GtkWidget *wa[5];
  wa[1]=Create_ButtonArray("Location observer:",callback_pref,2,
    LABEL,LAB_POSNAM+1,
    ENTRY,LAB_POSNAM,db->refpos_name,
    LABEL,LAB_POSLON+1,
    SPIN,LAB_POSLON  ,"%.3f%f%f%f",1.,R2D(db->refpos.lon),-180.,180.,
    LABEL,LAB_POSLAT+1,
    SPIN,LAB_POSLAT  ,"%.3f%f%f%f",1.,R2D(db->refpos.lat),-90.,90.,
    LABEL,LAB_PLLIST+1,
    BUTTON,LAB_PLLIST,
    NULL);

  wa[2]=Create_ButtonArray("Elevation levels",callback_pref,2,
                           LABEL,LAB_ELEVDET+1,
                           SPIN,LAB_ELEVDET,"%d%d%d",(int)R2D(db->elev_det),-10,90,
                           LABEL,LAB_ELEVHOR+1,
                           SPIN,LAB_ELEVHOR,"%d%d%d",(int)R2D(db->elev_horiz),-10,90,
                           NULL
                        );
  wa[3]=Create_Check(LAB_RADIOHOR,callback_pref,db->show_radiohorizon);

  wa[4]=Create_ButtonArray("Time",callback_pref,2,
    LABEL,"Time used for calc. is UTC.",
    LABEL,"Time zone",
    LABEL,LAB_OFFSETTM+1,
    SPIN,LAB_OFFSETTM,"%3.2f%3.2f%f%f",0.25,db->tm_off.tz_offset,-23.,23., //  db->time_offset,-23.,23.,
    LABEL,"__________________________",
    LABEL,"____________",

    LABEL,"Compensate rotor delay.",
    LABEL,"",
    LABEL,LAB_OFFSETMS+1,
    SPIN,LAB_OFFSETMS,"%3.1f%f%f%f",0.1,db->tm_off.sec_offset,-60.,60.,   // db->time_offset_sec,-60.,60.,
    LABEL,"__________________________",
    LABEL,"____________",

    LABEL,LAB_OFFSETUTC+1,
    SPIN,LAB_OFFSETUTC,"%3.2f%f%f%f",0.25,db->utc_offset,-23.,23.,
    0);

  wa[1]=Pack(NULL,'h',wa[1],1,wa[2],1,wa[3],1,NULL);
  wa[0]=Pack(NULL,'v',wa[1],1,wa[4],1,NULL);
  return wa[0];
}

static GtkWidget *pref_rotor()
{
  GtkWidget *wb[4];
  wb[1]=Create_ButtonArray("Rotorconfig",callback_pref,-4,
    LABEL,LAB_RTYPE,
    TOGGLE,LAB_RTYPEB,FALSE,
    LABEL,LAB_RADISC,
    TOGGLE,LAB_XATDISC,FALSE,
    LABEL,"  ",
    LABEL,"  ",
    LABEL,"  ",
    LABEL,"  ",

    LABEL,"X=0: ",
    TOGGLE,LAB_X_DIR,FALSE,
    LABEL,"Y=0: ",
    TOGGLE,LAB_Y_DIR,FALSE,
    NULL);

  wb[2]=Create_ButtonArray("DiSEqC config",callback_pref,6,
        SPIN,LAB_DEG2STEP,"%d%d%d%d",8,db->rotor.deg2step,8,16,
        LABEL,"Limit",
        SPIN,LAB_DEQCLIMIT,"%d%d%d",db->rotor.xy_rotorlim,0,0xffff,
        BUTTON,LAB_SETLIM,
        LABEL,"  ",
        CHECK,LAB_DEQCINVX,

        LABEL,"  ",
        LABEL,"Limit deg.",
        SPIN,LAB_DEQCLIMITDEG,"%.1f%.1f%.1f",(float)db->rotor.xy_rotorlim/(float)db->rotor.deg2step,0.,91.,
        LABEL,"  ",
        LABEL,"  ",
        CHECK,LAB_DEQCINVY,
    NULL);

  wb[3]=Create_ButtonArray("Storm position",callback_pref,-4,
    LABEL,LAB_STORMX+1,
    SPIN,LAB_STORMX,"%d%d%d%d",1,(int)db->rotor.storm.x,0,180,
    LABEL,LAB_STORMY+1,
    SPIN,LAB_STORMY,"%d%d%d%d",1,(int)db->rotor.storm.y,0,180,

    LABEL,LAB_STORMXW+1,
    SPIN,LAB_STORMXW,"%d%d%d%d",1,(int)db->rotor.storm_wait_x,0,120,
    LABEL,LAB_STORMYW+1,
    SPIN,LAB_STORMYW,"%d%d%d%d",1,(int)db->rotor.storm_wait_y,0,120,

    LABEL,LAB_STORME+1,
    SPIN,LAB_STORME,"%f%f%f%f",1.,db->rotor.storm.elev,-30.,180.,
    LABEL,LAB_STORMA+1,
    SPIN,LAB_STORMA,"%d%d%d%d",1,(int)db->rotor.storm.azim,0,180,
    NULL);

  wb[0]=Pack("",'v',wb[1],10,wb[2],10,wb[3],10,NULL);
  return wb[0];
}

// search dirs: db->cur_dir,db->prog_dir,db->home_dir
static GtkWidget *pref_files()
{
  GtkWidget *wc[4];
  char lbl[100];
  int test_wget;

  wc[1]=Create_ButtonArray("Files",callback_pref,3,
    LABEL,LAB_KEPF+1,
    ENTRY,LAB_KEPF,"%-15s",db->pref_noradfile,
    ENTRY_NOFUNC,"!fnkeps","%35s","",
    LABEL,LAB_MAPF+1,
    ENTRY,LAB_MAPF,"%-15s",db->pref_mapfile,
    ENTRY_NOFUNC,"!fnmaps","%35s","",
    LABEL,LAB_LOCF+1,
    ENTRY,LAB_LOCF,"%-15s",db->pref_placesfile,
    ENTRY_NOFUNC,"!fnlocs","%35s","",
    BUTTON,LAB_SEARCHFILES,
    0);

  test_wget=test_downloadprog();
  switch(test_wget)
  {
    case 1: strcpy(lbl,"Can't run");                         break;
    case 2: snprintf(lbl,90,"Not found in %s",db->prog_dir); break;
    case 3: strcpy(lbl,"Not found");                         break;
    default: strcpy(lbl,"Found, OK");                        break;
  }

  wc[2]=Create_ButtonArray("Download program",NULL,3,
    LABEL,"External program",
    ENTRY,"!xx","%-20s",WGETPROG,
    LABEL,lbl,
    0);

  wc[3]=Create_ButtonArray("Search locations",NULL,2,
    LABEL,"1. Current location .....",
    ENTRY,"!yy","%-30s",db->cur_dir,
    LABEL,"2. Program location ...",
    ENTRY,"!yy","%-30s",db->prog_dir,
    LABEL,"3. Home location ........",
    ENTRY,"!yy","%-30s",db->home_dir,
    0);

  wc[0]=Pack("",'v',wc[1],10,wc[2],10,wc[3],10,NULL);
  return wc[0];
}

static GtkWidget *pref_rotdrive(char progs[4][100])
{
  GtkWidget *wd[3];
   wd[1]=Create_Text("x",FALSE,NULL);
   Clear_Text(wd[1]);
   Add_Text(wd[1],120,"Commands: <program_name+options>\n");
   Add_Text(wd[1],120,"  Options:\n");
   Add_Text(wd[1],120,"    %%n: replaced by name satellite\n");
   Add_Text(wd[1],120,"    for elevation/azimuth system:\n");
   Add_Text(wd[1],120,"      %%e: replaced by elevation\n");
   Add_Text(wd[1],120,"      %%a: replaced by azimuth\n");
   Add_Text(wd[1],120,"    for X/Y system:\n");
   Add_Text(wd[1],120,"      %%x: replaced by X position\n");
   Add_Text(wd[1],120,"      %%y: replaced by Y position\n");

  wd[2]=Create_ButtonArray("Commands:",callback_pref,2,
    LABEL,LAB_PROGTRACK+1,
    ENTRY,LAB_PROGTRACK  ,"%-30s",progs[0],
    LABEL,LAB_PROGUP+1,
    ENTRY,LAB_PROGUP     ,"%-30s",progs[1],
    LABEL,LAB_PROGDOWN+1,
    ENTRY,LAB_PROGDOWN   ,"%-30s",progs[2],
    LABEL,LAB_PROGTRACKUP+1,
    ENTRY,LAB_PROGTRACKUP,"%-30s",progs[3],
                         0);
  wd[0]=Pack("External rotor drivers",'v',wd[1],1,wd[2],1,NULL);
  return wd[0];
}

static GtkWidget *pref_run_opt()
{
  GtkWidget *we[7];
  we[1]=Create_Check(LAB_START,callback_pref,FALSE);
  we[2]=Create_Spin(LAB_REPTIM,callback_pref,"%3.1f%3.1f%3.1f",db->reptime,0.1,10000.);
  we[6]=Create_ButtonArray("decoder display",callback_pref,1,
                        RADIOs,LAB_STATPOS,
                        RADIOn,LAB_DECSTAT,
                        RADIOn,LAB_POSITION,
                        0);

  we[3]=Create_ButtonArray("Antenna direction to output:",callback_pref,1,
                        CHECK,LAB_OUTON,
                        CHECK,LAB_EXTPROG,
                        #if __ADD_RS232__
                        CHECK,LAB_SERIAL,
                        #endif
                        #if __ADD_USB__
                        CHECK,LAB_USB,
                        #endif
                        0);
  we[4]=Create_Toggle(LAB_FREQ,callback_pref,FALSE);
  we[4]=SPack("Receiver freq.:","v",we[4],"f1",NULL);

  we[0]=Pack("",'v',we[1],1,we[2],1,we[6],1,NULL);
  we[3]=Pack("",'v',we[3],1,we[4],1,NULL);
  we[0]=Pack("",'h',we[0],1,we[3],1,NULL);
  return we[0];
}

static GtkWidget *pref_rs232()
{
  GtkWidget *wf[4];

  wf[1]=Create_ButtonArray("",callback_pref,2,
                        LABEL,LAB_COMPORT+1,
                        SPIN,LAB_COMPORT,"%3d%3d%3d",db->rs232.portnr+PORTMIN,PORTMIN,PORTMAX,
                        LABEL,LAB_COMNAME+1,
                        ENTRY_NOFUNC,LAB_COMNAME,"%-16s",comports[db->rs232.portnr],
//                        LABEL,LAB_COMSPEED+1,
//                        SPIN,LAB_COMSPEED,"%3d%3d%3d%3d",9600,db->rs232.speed,9600,115200,
                        LABEL,"parity,stop: fixed",
                        LABEL,"parity=N,stop=1",
                        0);

  wf[2]=Create_Optionmenu(LAB_COMSPEED,callback_pref,
                (db->rs232.speed==115200? 2 : db->rs232.speed==19200? 1 : 0),
                          "9600","19200","115200",0);
  wf[1]=Pack(NULL,'v',wf[1],1,wf[2],1,NULL);
  
  {
    char tmp[100],*p;
    *tmp=0;
    for (p=db->rs232.command; *p; p++)
    {
      if (*p=='%') strcat(tmp,"%");
      sprintf(tmp,"%s%c",tmp,*p);
    }
    wf[2]=Create_ButtonArray("",callback_pref,2,
                        LABEL,LAB_COMMAND+1,
                        ENTRY,LAB_COMMAND,"%-20s",tmp,
                        0);
  }
  wf[3]=Create_ButtonArray("Generate RS232 command examples",callback_pref,2,
                        LABEL,"",
                        LABEL,"",
                        LABEL,"2 numbers:",
                        LABEL,"azim+elev or x+y      ",
                        LABEL,"3 numbers:",
                        LABEL,"e/w pass + azim+elev",
                        LABEL,"",
                        LABEL,"",
                        BUTTON,LAB_EXAMPLE1,
                        BUTTON,LAB_EXAMPLE2,
                        BUTTON,LAB_EXAMPLE3,
                        BUTTON,LAB_EXAMPLE4,
                        0);

  wf[2]=Pack("",'v',wf[2],5,wf[3],5,NULL);
  wf[0]=Pack("",'h',wf[1],1,wf[2],1,NULL);
  return wf[0];
}

#define LAB_CLRSPNT    "!Satpos"
#define LAB_CLRUPNT    "!uSatpos"
#define LAB_CLRSVIS    "!Area"
#define LAB_CLRUVIS    "!uArea"
#define LAB_CLRSTRK    "!Track"
#define LAB_CLRUTRK    "!uTrack"
#define LAB_CLRSSNS    "!Scanline"
#define LAB_CLRUSNS    "!uScanline"
#define LAB_CLRREFPNT  "!Refpos"
#define LAB_CLRREFVIS  "!Radiohor"
#define LAB_CLRRASTER  "!Raster"
#define LAB_CLRNUMBER  "!Number"

#define LAB_SHADOW_ONOFF "Sun shadow"
#define LAB_SHADOWFACT   "Shadow fact"

static GdkColor gdkclr_1628(GdkColor ci)
{
  GdkColor co;
  co.red  =(ci.red>>8);
  co.green=(ci.green>>8);
  co.blue =(ci.blue>>8);
  return co;
}

static GdkColor *pgdkclr_8216(GdkColor *ci)
{
  static GdkColor co;
  co.red  =(ci->red<<8);
  co.green=(ci->green<<8);
  co.blue =(ci->blue<<8);
  return &co;
}

static void clrfunc(GtkColorButton *widget, gpointer data)
{
  char *name=(char *)data;
  if (!strcmp(name,LAB_CLRSPNT))
  {
    gtk_color_button_get_color(widget,&db->clrs.ssat_pnt);
    db->clrs.ssat_pnt=gdkclr_1628(db->clrs.ssat_pnt);
  }
  if (!strcmp(name,LAB_CLRSVIS))
  {
    gtk_color_button_get_color(widget,&db->clrs.ssat_vis);
    db->clrs.ssat_vis=gdkclr_1628(db->clrs.ssat_vis);
  }
  if (!strcmp(name,LAB_CLRSTRK))
  {
    gtk_color_button_get_color(widget,&db->clrs.ssat_track);
    db->clrs.ssat_track=gdkclr_1628(db->clrs.ssat_track);
  }
  if (!strcmp(name,LAB_CLRSSNS))
  {
    gtk_color_button_get_color(widget,&db->clrs.ssat_scan);
    db->clrs.ssat_scan=gdkclr_1628(db->clrs.ssat_scan);
  }

  if (!strcmp(name,LAB_CLRUPNT))
  {
    gtk_color_button_get_color(widget,&db->clrs.usat_pnt);
    db->clrs.usat_pnt=gdkclr_1628(db->clrs.usat_pnt);
  }
  if (!strcmp(name,LAB_CLRUVIS))
  {
    gtk_color_button_get_color(widget,&db->clrs.usat_vis);
    db->clrs.usat_vis=gdkclr_1628(db->clrs.usat_vis);
  }
  if (!strcmp(name,LAB_CLRUTRK))
  {
    gtk_color_button_get_color(widget,&db->clrs.usat_track);
    db->clrs.usat_track=gdkclr_1628(db->clrs.usat_track);
  }
  if (!strcmp(name,LAB_CLRUSNS))
  {
    gtk_color_button_get_color(widget,&db->clrs.usat_scan);
    db->clrs.usat_scan=gdkclr_1628(db->clrs.usat_scan);
  }

  if (!strcmp(name,LAB_CLRREFPNT))
  {
    gtk_color_button_get_color(widget,&db->clrs.ref_pnt);
    db->clrs.ref_pnt=gdkclr_1628(db->clrs.ref_pnt);
  }
  if (!strcmp(name,LAB_CLRREFVIS))
  {
    gtk_color_button_get_color(widget,&db->clrs.ref_vis);
    db->clrs.ref_vis=gdkclr_1628(db->clrs.ref_vis);
  }
  if (!strcmp(name,LAB_CLRRASTER))
  {
    gtk_color_button_get_color(widget,&db->clrs.raster);
    db->clrs.raster=gdkclr_1628(db->clrs.raster);
  }
  if (!strcmp(name,LAB_CLRNUMBER))
  {
    gtk_color_button_get_color(widget,&db->clrs.number);
    db->clrs.number=gdkclr_1628(db->clrs.number);
  }
}

static void Set_Colorsel(GtkWidget *widget,char *name,GdkColor *color)
{
  GtkColorButton *w=(GtkColorButton *)Find_Widget(widget,name);
  gtk_color_button_set_color(w,pgdkclr_8216(color));
}

#define LAB_SETDEF "Reset to default"
static void deffunc(GtkWidget *widget, gpointer data)
{
  char *name=(char *)data;
  if (!strcmp(name,LAB_SETDEF))
  {
    default_colors(&db->clrs);
    Set_Colorsel(widget,LAB_CLRSPNT,&db->clrs.ssat_pnt);
    Set_Colorsel(widget,LAB_CLRSVIS,&db->clrs.ssat_vis);
    Set_Colorsel(widget,LAB_CLRSTRK,&db->clrs.ssat_track);
    Set_Colorsel(widget,LAB_CLRSSNS,&db->clrs.ssat_scan);
    Set_Colorsel(widget,LAB_CLRUPNT,&db->clrs.usat_pnt);
    Set_Colorsel(widget,LAB_CLRUVIS,&db->clrs.usat_vis);

    Set_Colorsel(widget,LAB_CLRUTRK,&db->clrs.usat_track);

    Set_Colorsel(widget,LAB_CLRUSNS,&db->clrs.usat_scan);
    Set_Colorsel(widget,LAB_CLRREFPNT,&db->clrs.ref_pnt);
    Set_Colorsel(widget,LAB_CLRREFVIS,&db->clrs.ref_vis);
    Set_Colorsel(widget,LAB_CLRRASTER,&db->clrs.raster);
    Set_Colorsel(widget,LAB_CLRNUMBER,&db->clrs.number);
  } 
  if (!strcmp(name,LAB_SHADOW_ONOFF))
  {
    db->enable_shadow=Get_Button(widget,LAB_SHADOW_ONOFF);
  }
  if (!strcmp(name,LAB_SHADOWFACT))
  {
    db->shadow_fact=GTK_ADJUSTMENT(widget)->value;
  }
}


static GtkWidget *pref_fonts_clrs()
{
  GtkWidget *table,*tbut,*wg[5];
  int h=0,v=0;
  table=gtk_table_new(3,1,FALSE);

  tbut=Create_Label("");
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Label(LAB_CLRSPNT+1);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Label(LAB_CLRSVIS+1);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Label(LAB_CLRSTRK+1);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Label(LAB_CLRSSNS+1);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  h=0; v++;
  tbut=Create_Label("  Selected  ");
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Colorsel(LAB_CLRSPNT,&db->clrs.ssat_pnt,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Colorsel(LAB_CLRSVIS,&db->clrs.ssat_vis,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;
  tbut=Create_Colorsel(LAB_CLRSTRK,&db->clrs.ssat_track,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Colorsel(LAB_CLRSSNS,&db->clrs.ssat_scan,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  h=0; v++;
  tbut=Create_Label(" Unselected");
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Colorsel(LAB_CLRUPNT,&db->clrs.usat_pnt,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Colorsel(LAB_CLRUVIS,&db->clrs.usat_vis,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;
  tbut=Create_Label("");
//  tbut=Create_Colorsel(LAB_CLRUTRK,&db->clrs.usat_track,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Colorsel(LAB_CLRUSNS,&db->clrs.usat_scan,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  wg[1]=Pack("",'h',table,1,NULL);
//////////////////////////////////////////////////////////////////
  h=0; v=0;
  table=gtk_table_new(3,1,FALSE);

  tbut=Create_Label(LAB_CLRREFPNT+1);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Label(LAB_CLRREFVIS+1);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Label(LAB_CLRRASTER+1);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Label(LAB_CLRNUMBER+1);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  h=0; v++;

  tbut=Create_Colorsel(LAB_CLRREFPNT,&db->clrs.ref_pnt,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  tbut=Create_Colorsel(LAB_CLRREFVIS,&db->clrs.ref_vis,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;
  tbut=Create_Colorsel(LAB_CLRRASTER,&db->clrs.raster,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;
  tbut=Create_Colorsel(LAB_CLRNUMBER,&db->clrs.number,clrfunc);
  gtk_table_attach(GTK_TABLE(table),tbut,v,v+1,h,h+1,
                           GTK_FILL,GTK_FILL,1,1);
  h++;

  wg[2]=Pack("",'h',table,1,NULL);

  wg[3]=Create_Button(LAB_SETDEF,deffunc);

  wg[1]=Pack(NULL,'h',wg[1],1,wg[2],1,NULL);
  wg[1]=Pack("Colours",'v',wg[1],1,wg[3],1,NULL);
  Set_Widgetcolor(wg[1],'b',0,0,0xff);


  wg[3]=Create_ButtonArray("Sunshadow",deffunc,1,
                            CHECKI,LAB_SHADOW_ONOFF,db->enable_shadow,
                            SPIN,LAB_SHADOWFACT,"%.1f%.1f%.1f",db->shadow_fact,0.1,1.,
                            0
                          );
  Set_Widgetcolor(wg[3],'b',0,0,0xff);

  wg[4]=Create_Optionmenu(LAB_POSSATSEL,callback_pref,(db->satsel_bottom? 1 : 0),
                        LAB_POSSATSEL_R,LAB_POSSATSEL_B,
                        0);
  wg[4]=Pack("Satellite choice buttons",'v',wg[4],1,NULL);
  Set_Widgetcolor(wg[4],'b',0,0,0xff);

  wg[0]=Pack(NULL,'v',wg[1],3,wg[3],3,wg[4],3,NULL);
  return wg[0];
}

void Create_preferences_wnd(GtkWidget *widget)
{
  GtkWidget *main_window=gtk_widget_get_toplevel(widget);
  GtkWidget *wnd,*wt[3],*wx,*wy1,*wy2,*wy,*wz;
  char progs[4][100];

  strcpy(progs[0],dbl_prct(db->prog_track));
  strcpy(progs[1],dbl_prct(db->prog_up));
  strcpy(progs[2],dbl_prct(db->prog_down));
  strcpy(progs[3],dbl_prct(db->prog_trackup));

  if (!(wnd=Create_Window(main_window,0,0,"Preferences",NULL))) return;

  if (exist_file(PREF_FILE2))
    wt[1]=Create_Entry(LAB_LOPREF,NULL,PREF_FILE2); 
  else if (exist_file(PREF_FILE))
    wt[1]=Create_Entry(LAB_LOPREF,NULL,PREF_FILE); 
  else
    wt[1]=Create_Entry(LAB_LOPREF,NULL,"Not found."); 

  if (exist_file(db->used_preffile))
    wt[2]=Create_Entry(LAB_GLPREF,NULL,db->used_preffile); 
  else
    wt[2]=Create_Entry(LAB_GLPREF,NULL,"Not found"); 

  wt[0]=Pack("Preference file",'v',wt[1],5,wt[2],5,NULL);

/* The notebook */
  wx=Create_Notebook(NULL,GTK_POS_TOP,
                     "Observator"         ,pref_obs(),
                     "Rotorconfig"        ,pref_rotor(),
                     "Files"              ,pref_files(),
                     "Rotor drive"        ,pref_rotdrive(progs),
                     "Run options"        ,pref_run_opt(),
                     "RS232"              ,pref_rs232(),
                     "Colours etc."       ,pref_fonts_clrs(),
                     NULL);

  wy1=Create_Button(LAB_SAVE,callback_pref);
  wy2=Create_Button(LAB_CLOSE,callback_pref);
  wy=SPack("","h",wy1,"fe5",wy2,"fe5",NULL);

  wz=Pack(NULL,'v',wt[0],5,wx,5,wy,5,NULL);

  Set_Button(wz,LAB_RTYPEB,db->rotor.use_xy);
  Set_Button(wz,LAB_XATDISC,db->rotor.x_at_disc);
  Set_Button(wz,LAB_X_DIR,db->rotor.x_west_is_0);
  Set_Button(wz,LAB_Y_DIR,db->rotor.y_south_is_0);
  Set_Button(wz,LAB_OUTON,db->out_on);
  Set_Button(wz,LAB_EXTPROG,db->ext_on);
  Set_Button(wz,LAB_START,db->start_now);
  Set_Button(wz,LAB_SERIAL,db->to_serial);
  Set_Button(wz,LAB_USB,db->to_usb);
  Set_Adjust(wz,LAB_COMPORT,"%d",db->rs232.portnr+PORTMIN);
  Set_Button(wz,LAB_DEQCINVX,db->rotor.inv_x);
  Set_Button(wz,LAB_DEQCINVY,db->rotor.inv_y);
  Set_Button(wz,LAB_SEARCHFILES);
//  Set_Adjust(wz,LAB_COMSPEED,"%d",db->rs232.speed);

  gtk_container_add(GTK_CONTAINER(wnd),wz);
  gtk_widget_show_all(wnd);
}
