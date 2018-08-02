/**************************************************
 * RCSId: $Id: misc.c,v 1.1 2018/02/04 11:58:38 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: misc.c,v $
 * Revision 1.1  2018/02/04 11:58:38  ralblas
 * Initial revision
 *
 * Revision 1.4  2018/02/02 23:12:17  ralblas
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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "xtrack.h"
#include "xtrack_func.h"
#include "defines.h"
#include "gif.h"
#if __ADD_JPEG__ == 1
#define JPEGSIGN 0xffd8ffe0
#endif

extern DBASE *db;
void ld_clr(GdkColor *clr,int i,int r,int g,int b)
{
  clr[i].red=r<<8;
  clr[i].green=g<<8;
  clr[i].blue=b<<8;
}
GdkColor rgb2clr(int r,int g,int b)
{
  GdkColor clr;
  clr.red=r;
  clr.green=g;
  clr.blue=b;
  return clr;
}

/*************************************
 * translate days-in-year in tm struct into month and day
 *************************************/
void yday2mday_mon(struct tm *tmref)
{
  int i;
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};
  if (tmref->tm_year/4==tmref->tm_year/4.) days[1]=29;

  tmref->tm_mday=tmref->tm_yday+1;
  tmref->tm_mon=0; 
  for (i=0; i<12; i++)
  {
    if (tmref->tm_mday<=days[i]) break;
    tmref->tm_mon++;
    tmref->tm_mday-=days[i];
  }

}

/*************************************
 * Correct tm-struct and return #seconds since 1970.
 * Similar to mktime, except that dayligt saving is ignored.
 *************************************/
long mktime_ntz(struct tm *tm)
{
  long secs;
  int i;
  int days[]={31,28,31,30,31,30,31,31,30,31,30,31};

  while (tm->tm_sec >= 60) { tm->tm_min++; tm->tm_sec-=60; }
  while (tm->tm_sec <   0) { tm->tm_min--; tm->tm_sec+=60; }
  while (tm->tm_min >= 60) { tm->tm_hour++; tm->tm_min-=60; }
  while (tm->tm_min <   0) { tm->tm_hour--; tm->tm_min+=60; }
  while (tm->tm_hour>= 24) { tm->tm_mday++; tm->tm_hour-=24; }
  while (tm->tm_hour<   0) { tm->tm_mday--; tm->tm_hour+=24; }
  while (tm->tm_mon >= 12) { tm->tm_year++; tm->tm_mon-=12; }
  while (tm->tm_mon <   0) { tm->tm_year--; tm->tm_mon+=12; }
  if (tm->tm_year/4==tm->tm_year/4.) days[1]=29;

  while (tm->tm_mday>days[tm->tm_mon])
  {
    tm->tm_mday-=days[tm->tm_mon]; tm->tm_mon++;
    while (tm->tm_mon>= 12)  { tm->tm_year++; tm->tm_mon-=12; }
    if (tm->tm_year/4==tm->tm_year/4.) days[1]=29; else days[1]=28;
  }

  while (tm->tm_mday<=0)
  {
    tm->tm_mon--;
    tm->tm_mday+=days[tm->tm_mon];
    while (tm->tm_mon< 0)  { tm->tm_year--; tm->tm_mon+=12; }
    if (tm->tm_year/4==tm->tm_year/4.) days[1]=29; else days[1]=28;
  }

  secs=tm->tm_sec+60*(tm->tm_min+60*(tm->tm_hour+24*(tm->tm_mday-1)));
  for (i=0; i<tm->tm_mon; i++) secs+=(days[i]*24*3600);
  secs+=((tm->tm_year-70)*365*24*3600); 
  secs+=((tm->tm_year-70+1)/4)*24*3600; 
  tm->tm_yday=0;
  for (i=0; i<tm->tm_mon; i++) tm->tm_yday+=days[i];
  tm->tm_yday+=(tm->tm_mday-1);
  return secs;
}

#ifdef NIETGEBUIKT
struct tm local2gmt(struct tm *ltm)
{
  time_t t;
  struct tm tm1,tm2;
  struct tm gmtm;
  time(&t);
  tm1=*gmtime(&t);
  tm2=*localtime(&t);
  gmtm=*ltm;
  gmtm.tm_hour+=(tm2.tm_hour-tm1.tm_hour);
  gmtm.tm_min+=(tm2.tm_min-tm1.tm_min);
  gmtm.tm_hour-=(int)(db->tm_off.tz_offset);
  gmtm.tm_min-=(db->tm_off.tz_offset - (int)(db->tm_off.tz_offset))*60;
  mktime_ntz(&gmtm);
  return gmtm;
}
#endif



char *timestr(struct tm *tm)
{
  static char s[50],*p;
  strcpy(s,asctime(tm));
  if ((p=strchr(s,'\n'))) *p=0;
  return s+4;
}

char *tmstrsh(struct tm *tm)
{
  static char s[51];
  strftime(s,50,"%d-%m    %H:%M:%S",tm);
  return s;
}

char *tmstrshx(struct tm *tm,char *what)
{
  static char s[51];
  char frmt[20];
  strcpy(frmt,"");
  if (strchr(what,'m')) strcat(frmt,"%d-%m ");
  if (strchr(what,'H')) strcat(frmt,"%H");
  if (strchr(what,'M')) strcat(frmt,":%M");
  if (strchr(what,'S')) strcat(frmt,":%S");
  strftime(s,50,frmt,tm);
  return s;
}


char *dbl_prct(char *p)
{
  static char tmp[1000];
  char *q=tmp;
  *tmp=0;
  for (; *p; p++)
  {
    if (*p=='%') { *q='%'; q++; }
    *q=*p; q++; *q=0;
  }
  return tmp;
}

/* Detect if file exist */
int exist_file(char *fn)
{
  FILE *fp;
  if (!(*fn)) return 0;
  if (!(fp=fopen(fn,"r"))) return 0;
  fclose(fp);
  return 1;
}

void freed(char **s)
{
  if ((s) && (*s)) free(*s);
  *s=NULL;
}

/* Allocate and copy. 2 extra bytes allocated: for closing 0 and maybe 1 extra char to add. */
int strcpyd(char **sp,char *s)
{
  if (!sp) return 1;
  if (!s) return 1;
  if (!(*sp=(char *)malloc(strlen(s)+2))) return 1;
  strcpy(*sp,s);
  return 0;
}

/* Allocate and concat. 2 extra bytes allocated: for closing 0 and maybe 1 extra char to add. */
int strcatd(char **sp,char *s)
{
  if (!sp) return 1;
  if (!*sp)
  {
    strcpyd(sp,s);
    return 0;
  }
  
  if (!(*sp=(char *)realloc(*sp,strlen(*sp)+strlen(s)+2))) return 1;
  strcat(*sp,s);
  return 0;
}
void strcatc(char *s,char c)
{
  *(s+strlen(s)+1)=0;
  *(s+strlen(s))=c;
}

#define RCHAR(l) l[strlen(l)-1]
void finish_path(char *path)
{
  if (!path) return;
  if (RCHAR(path) != DIR_SEPARATOR)
    strcatc(path,DIR_SEPARATOR);
}

int exist_file_in_dir(char *dir,char *fn)
{
  char *tmp=NULL;
  int res;
  strcpyd(&tmp,dir);
  finish_path(tmp);
  strcatd(&tmp,fn);
  res=exist_file(tmp);
  free(tmp);
  return res;
}

int test_downloadprog()
{
  char cmd[200];
  int err=0;
#if __GTK_WIN32__ == 1
  snprintf(cmd,200,"%s\\%s --version",db->prog_dir,WGETPROG);
  if ((system(cmd)))
  {
    err=1;                         // can't run
    if (!(exist_file(cmd))) err=2; // not found
  }
  else
  {
    err=0;                         // OK
  }
#else
  snprintf(cmd,200,"%s --version > /dev/null",WGETPROG);
  if (system(cmd)) err=3;          // not found/can't run
#endif
  return err;
}

#define LENSIGN 10
FILE_TYPE detect_file(char *fn)
{

  char sign[LENSIGN];
#ifdef SUNSIGN
  guint32 *sign1=(guint32 *)sign;
#else
#ifdef JPEGSIGN
  guint32 *sign1=(guint32 *)sign;
#endif
#endif

  FILE_TYPE file_type=unknown_filetype;
  FILE *fp;

  if ((fp=fopen(fn,"rb")) == NULL) return unknown_filetype;
  fread(sign,4,1,fp);
  if (FALSE);
#ifdef SUNSIGN
  else if (GUINT32_FROM_BE(*sign1) == SUNSIGN)  file_type=SUN;
#endif
#ifdef GIFSIGN
  else if (!strncmp(sign,GIFSIGN,3)) file_type=GIF;
#endif
#ifdef JPEGSIGN
  else if (GUINT32_FROM_BE(*sign1) == JPEGSIGN)  file_type=JPEG;
#endif
  fclose(fp);

  return file_type;
}

#define MAXLEN 150
void w_dbg(char *frmt,...)
{
  static GtkWidget *w[3];
  static int n;
  char str[MAXLEN];
return;
  str[MAXLEN-5]=0;
  va_list arg;
  if (!w[0])
  {
    w[0]=Create_Window(NULL,200,200,"xx",NULL);
    w[1]=Create_Text("x",FALSE,NULL);
    w[2]=Pack(NULL,'h',w[1],1,NULL);
    gtk_container_add(GTK_CONTAINER(w[0]),w[2]);
    gtk_widget_show_all(w[0]);
  }

  if (!strcasecmp(frmt,"clear"))
  {
    Clear_Text(w[1]);
    return;
  }
  
  va_start(arg,frmt);
  sprintf(str,"%02d ",n++);
  vsnprintf(str+3,MAXLEN-5,frmt,arg);
  va_end(arg);
  
  if (strlen(str) == MAXLEN-5-1) strcat(str,"...\n");
  Add_Text(w[1],120,str);
}
