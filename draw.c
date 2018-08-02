/**************************************************
 * RCSId: $Id: draw.c,v 1.6 2018/02/04 12:53:45 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: draw.c,v $
 * Revision 1.6  2018/02/04 12:53:45  ralblas
 * _
 *
 * Revision 1.5  2018/02/02 23:11:33  ralblas
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
#include <string.h>
#include "xtrack.h"
#include "xtrack_func.h"
#include "defines.h"
#include "orbit.h"
#include "gif.h"

#if __ADD_JPEG__ == 1
#define JPEGSIGN 0xffd8ffe0
#endif

extern DBASE *db;

float calcele(EPOINT *pos_subsat,ORBIT *orbit,EPOINT *refpos);

void draw_back(GtkWidget *wnd)
{
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);
  GdkColor clr;
  FILE *fp;
  FILE_TYPE ftype;
  GIF_PICINFO *gif_pci;
  CLRMAP clrmap;
  int width,height;
  int picwidth,picheight;
  int picyp=-1;
  guint16 *line_r, *line_g, *line_b;
  guchar *istr=NULL;
  int x,y;
  if (!drawing_area) return;
  if (!rgbi) return;
  gif_pci=calloc(sizeof(GIF_PICINFO),1);
  width=drawing_area->allocation.width;
  height=drawing_area->allocation.height;

  ftype=detect_file(db->map_file);

  if (!(fp=fopen(db->map_file,"rb"))) return;

  switch(ftype)
  {
#ifdef TIFFSIGN
    case TIFF:
      read_tiff_header(fp,&picwidth,&picheight);
    break;
#endif
#ifdef JPEGSIGN
    case JPEG:
    {
      jpg2str_1b(fp,&istr,&picwidth,&picheight);
    }
    break;
#endif
#ifdef GIFSIGN
    case GIF:
      if (read_gif_header1(fp, &clrmap, gif_pci)) 
      {
        free(gif_pci);
        return;
      }
      picwidth=gif_pci->width;
      picheight=gif_pci->height;
    break;
#endif
    default:
      printf("Unknown filetype '%d' of %s\n",ftype,db->map_file);
      return;
    break;
  }

  line_r=malloc(3*picwidth*sizeof(guint16));
  line_g=malloc(3*picwidth*sizeof(guint16));
  line_b=malloc(3*picwidth*sizeof(guint16));

  for (y=0; y<LAT2Y(-90)-LAT2Y(90); y++)
  {
    int picy=(picheight*y)/(LAT2Y(-90)-LAT2Y(90));
    if (y+LAT2Y(90)>=height-RAND) continue;
    if (y+LAT2Y(90)<RAND) continue;
    g_main_iteration(FALSE);            /* Handle GTK actions now and then */
    if (y+LAT2Y(90)>=height-RAND) continue;
    if (y+LAT2Y(90)<RAND) continue;
    if (db->redraw) break;
    if (picy!=picyp)
    {
      switch(ftype)
      {
#ifdef TIFFSIGN
        case TIFF:
          read_tiff_line(fp,picwidth, picy,line_r,line_g,line_b);
        break;
#endif
#ifdef JPEGSIGN
        case JPEG:
        {
          read_jpg_line(istr,picwidth, picy,line_r,line_g,line_b);
        }
        break;
#endif
#ifdef GIFSIGN
        case GIF:
          read_gif_line1(fp,gif_pci,picy,line_r);
        break;
#endif
        default: break;
      }
    }
    picyp=picy;

    for (x=0; x<=LON2X(180)-LON2X(-180); x++)
    {
      int picx=(picwidth*x)/(LON2X(180)-LON2X(-180));
      if (x+LON2X(-180)>=width-RAND) continue;
      if (x+LON2X(-180)<RAND) continue;
      switch(ftype)
      {
        case TIFF:
          clr.red=line_r[picx];
          clr.green=line_g[picx];
          clr.blue=line_b[picx];
        break;
        case JPEG:
          clr.red=line_r[picx];
          clr.green=line_g[picx];
          clr.blue=line_b[picx];
        break;
        case GIF:
        {
          int n=*(line_r+picx);
          n=MIN(MAX(n,0),255);
          if (n<0) n=0;
          clr.red=clrmap.red[n];
          clr.green=clrmap.green[n];
          clr.blue=clrmap.blue[n];
        }
        break;
        default: break;
       }
       draw_rgbpoint(rgbi,&clr,x+LON2X(-180),y+LAT2Y(90));
    }
    Refresh_Rect(drawing_area,0,y+LAT2Y(90),drawing_area->allocation.width-1,1);

  }
  if (istr) free(istr);
  free(gif_pci);
  free(line_r);
  free(line_g);
  free(line_b);
  fclose(fp);
}




#define W_CHAR 8
#define RASTW 15
void draw_rast(GtkWidget *wnd,gboolean rgbm)
{
  GdkGC **gc=Get_Gc(wnd);
  GdkPixmap *px=Find_Widget(wnd,"GTK_DRAWING_AREA")->window;
  int lon,lat;
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);

  GdkColor rast1,rast2,number;
  rast1=db->clrs.raster;
  rast2.red=db->clrs.raster.red/1.6;
  rast2.green=db->clrs.raster.green/1.6;
  rast2.blue=db->clrs.raster.blue/1.6;
  number=db->clrs.number;

  if (rgbm)
  {
    char tmp[10];
    for (lon=-180; lon<=180; lon+=RASTW)
    {
      draw_rgbline(rgbi,&rast2,LON2X(lon),LAT2Y(-90),LON2X(lon),LAT2Y(90));
      sprintf(tmp,"%4d",lon);
      if ((ABS(lon)<180) && (!(lon%30)))
        draw_rgbstring(rgbi,&number,LON2X(lon)-2*W_CHAR,4,tmp);
    }
    draw_rgbline(rgbi,&rast1,LON2X(0),LAT2Y(-90),LON2X(0),LAT2Y(90));

    for (lat=-90; lat<=90; lat+=RASTW)
    {
      draw_rgbline(rgbi,&rast2,LON2X(-180),LAT2Y(lat),LON2X(180),LAT2Y(lat));
      sprintf(tmp,"%3d",lat);
      if ((ABS(lat)<90) && (!(lat%30)))
        draw_rgbstring(rgbi,&number,4,LAT2Y(lat)-W_CHAR,tmp);
    }
    draw_rgbline(rgbi,&rast1,LON2X(-180),LAT2Y(0),LON2X(180),LAT2Y(0));
  }
  else
  {
    for (lon=-180; lon<=180; lon+=RASTW)
      gdk_draw_line(px,gc[RASCOL],LON2X(lon),LAT2Y(-90),LON2X(lon),LAT2Y(90));
    gdk_draw_line(px,gc[RASCOL2],LON2X(0),LAT2Y(-90),LON2X(0),LAT2Y(90));

    for (lat=-90; lat<=90; lat+=RASTW)
      gdk_draw_line(px,gc[RASCOL],LON2X(-180),LAT2Y(lat),LON2X(180),LAT2Y(lat));
    gdk_draw_line(px,gc[RASCOL2],LON2X(-180),LAT2Y(0),LON2X(180),LAT2Y(0));
  }
}

#define SIZEREF 8
void draw_ref(GtkWidget *wnd,float lon,float lat,gboolean rgbm)
{
  int ilon,ilat;
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);
  GdkColor clr1;
  int x1,y1,x2,y2;
  ilon=R2D(lon);
  ilat=R2D(lat);
  x1=LON2X(ilon)-SIZEREF/2;
  x2=LON2X(ilon)+SIZEREF/2;
  y1=LAT2Y(ilat)-SIZEREF/2;
  y2=LAT2Y(ilat)+SIZEREF/2;
  if (rgbm)
  {
    clr1=db->clrs.ref_pnt;
    draw_rgbline(rgbi,&clr1,x1,y1,x1,y2);
    draw_rgbline(rgbi,&clr1,x1,y2,x2,y2);
    draw_rgbline(rgbi,&clr1,x2,y2,x2,y1);
    draw_rgbline(rgbi,&clr1,x2,y1,x1,y1);
  }
  else // werkt niet?
  {
    GdkGC **gc=Get_Gc(wnd);
    GdkPixmap *px=Find_Widget(wnd,"GTK_DRAWING_AREA")->window;
    gdk_draw_arc(px,gc[REFCOL],FALSE,x1,y1,SIZEREF,SIZEREF,0,360*64);
  }
}


#define SIZESAT 8
void draw_sat(GtkWidget *wnd,SAT *sat,float lon,float lat,gboolean rgbm)
{
  int x1,y1,x2,y2;
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);
  GdkColor clr1;
  GdkGC **gc=Get_Gc(wnd);
  GdkPixmap *px=Find_Widget(wnd,"GTK_DRAWING_AREA")->window;

  lon=R2D(lon);
  lat=R2D(lat);
  x1=LON2X(lon)-SIZESAT/2;
  x2=LON2X(lon)+SIZESAT/2;
  y1=LAT2Y(lat)-SIZESAT/2;
  y2=LAT2Y(lat)+SIZESAT/2;
  if (rgbm)
  {
    int xtxtpos[2];
    clr1=(sat->selected? db->clrs.ssat_pnt : db->clrs.usat_pnt);
    draw_rgbline(rgbi,&clr1,x1,y1,x1,y2);
    draw_rgbline(rgbi,&clr1,x1,y2,x2,y2);
    draw_rgbline(rgbi,&clr1,x2,y2,x2,y1);
    draw_rgbline(rgbi,&clr1,x2,y1,x1,y1);
    xtxtpos[0]=x1+2*W_CHAR;
    xtxtpos[1]=x1+(2+strlen(sat->satname))*W_CHAR;
    if (xtxtpos[1]>=rgbi->width) xtxtpos[0]-=(xtxtpos[1]-rgbi->width);
    draw_rgbstring(rgbi,&clr1,xtxtpos[0],y1+1*W_CHAR,sat->satname);
  }
  else
  {
    gdk_draw_arc(px,gc[SATCOL],FALSE,x1,y1,SIZESAT,SIZESAT,0,360*64);
  }
}

// part:  0... 100: west (100=100%)
//       -0...-100: east (-100=100%)
static void draw_rgbcircle(RGBI *rgbi,GdkColor *clr1,GdkColor *clr2,int x0,int y0,int r,int part)
{
  int i,j;
  int x1,y1;
  int s=1;
#ifdef XXX
  int x2,y2;
  if (r<5) s=5;
  for (; r>0; r--)
  {
    for (i=0; i<=360; i+=s)
    {
      x1=x0+r*cos(D2R((float)i));
      y1=y0+r*sin(D2R((float)i));
      if (i)
      {
        if ((part>0) && (i>part))
          draw_rgbline(rgbi,clr1,x2,y2,x1,y1);
        else
          draw_rgbline(rgbi,clr2,x2,y2,x1,y1);
      }
      x2=x1; y2=y1;
    }
  }
#elif YYY
  int x2,y2;
  if (r<5) s=5;
  for (i=0; i<=180; i+=s)
  {
    y1=y0+r*sin(D2R((float)i));
    y2=y0-r*sin(D2R((float)i));
    x1=x0+r*cos(D2R((float)i));  // x1=x0+r ... x0: start east
    x2=x0-r*cos(D2R((float)i));  // x2=x0-r ... x0: start west
    if (part>0)           // west ilum
    {
      if (i>part*1.8)
        draw_rgbline(rgbi,clr2,x2,y1,x2,y2); // dark
      else
        draw_rgbline(rgbi,clr1,x2,y1,x2,y2); // ilum
    }
    else                 // east
    {
      if (i>(part*-1.8))
        draw_rgbline(rgbi,clr2,x1,y1,x1,y2);
      else
        draw_rgbline(rgbi,clr1,x1,y1,x1,y2);
    }
  }
#else
  if (r<5) s=5;
  for (j=0; j<=180; j+=s)
  {
    for (i=-90; i<=90; i+=s)
    {
      y1=y0+r*sin(D2R((float)i));
      x1=x0+cos(D2R((float)j))*r*cos(D2R((float)i));  // x1=x0+r ... x0: start east
      if (part>0)           // west ilum
      {
        if (j>part*1.8)
          draw_rgbpoint(rgbi,clr2,x1,y1);
        else
          draw_rgbpoint(rgbi,clr1,x1,y1);
      }
      else
      {
        if (j>(part*-1.8))
          draw_rgbpoint(rgbi,clr2,x1,y1);
        else
          draw_rgbpoint(rgbi,clr1,x1,y1);
      }
    }
  }
#endif
}

#define SIZE_MARK 8
void draw_mark(GtkWidget *wnd,EPOINT p,int vorm,char *t)
{
  int x0,y0,x1,y1,x2,y2;
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);
  GdkColor clr1,clr2;
  float lon,lat;
  int ilum;
  lon=R2D(p.lon);
  lat=R2D(p.lat);
  x0=LON2X(lon);
  y0=LAT2Y(lat);

  x1=x0-SIZE_MARK/2;
  x2=x0+SIZE_MARK/2;
  y1=y0-SIZE_MARK/2;
  y2=y0+SIZE_MARK/2;
  if ((vorm&0xff00) == 'C'*0x100)
  {
    ilum=vorm&0x00ff; // 0=200=0%, 100=100%
    vorm=vorm&0xff00;
    if (ilum>100) ilum=(200-ilum)*-1; // > 100 ==> <0: ilum east 
//    ilum*=1.8;
  }

  switch(vorm)
  {
    case 'c'*0x100:
      clr1.red=0xff; clr1.green=0xff; clr1.blue=0x00;
      draw_rgbcircle(rgbi,&clr1,&clr1,x0,y0,10,100);
    break;
    case 'C'*0x100:
      clr1.red=0xff; 
      clr1.green=clr1.red; clr1.blue=clr1.red;
      clr2.red=0x99; 
      clr2.green=clr2.red; clr2.blue=clr2.red;
      draw_rgbcircle(rgbi,&clr1,&clr2,x0,y0,10,ilum);
    break;
    default:
      clr1=db->clrs.red;
      draw_rgbline(rgbi,&clr1,x1,y1,x2,y2);
      draw_rgbline(rgbi,&clr1,x1,y2,x2,y1);
    break;
  }
  if (t) draw_rgbstring(rgbi,&clr1,x1+2*W_CHAR,y1+1*W_CHAR,t);
}

void pijl(GdkPixmap *px,GdkGC *gc,int x,int y,int r)
{
  int dx1,dy1,dx2,dy2;

  dx1=10*cos(D2R(r-30));
  dy1=10*sin(D2R(r-30));
  dx2=10*cos(D2R(r+30));
  dy2=10*sin(D2R(r+30));
  gdk_draw_line(px,gc,x+dx1,y+dy1,x,y);
  gdk_draw_line(px,gc,x+dx2,y+dy2,x,y);
}

void pijlrgb(RGBI *rgbi,GdkColor *clr,int x,int y,int r)
{
  int dx1,dy1,dx2,dy2;

  dx1=10*cos(D2R(r-30));
  dy1=10*sin(D2R(r-30));
  dx2=10*cos(D2R(r+30));
  dy2=10*sin(D2R(r+30));
  draw_rgbline(rgbi,clr,x+dx1,y+dy1,x,y);
  draw_rgbline(rgbi,clr,x+dx2,y+dy2,x,y);
}

void draw_sattrack(GtkWidget *wnd,EPOINT p1,EPOINT p2,gboolean rgbm,int p)
{
  int x1,y1,x2,y2;
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);
  GdkColor clr1;
  GdkGC **gc=Get_Gc(wnd);
  GdkPixmap *px=Find_Widget(wnd,"GTK_DRAWING_AREA")->window;

  x1=LON2X(R2D(p1.lon));
  x2=LON2X(R2D(p2.lon));
  y1=LAT2Y(R2D(p1.lat));
  y2=LAT2Y(R2D(p2.lat));

  if (ABS(x2-x1)>90) return;
  if (rgbm)
  {
    clr1=db->clrs.ssat_track;
    draw_rgbline(rgbi,&clr1,x1,y1,x2,y2);
    if (p)
    {
      pijlrgb(rgbi,&clr1,x1,y1,(int)R2D(atan2(y2-y1,x2-x1)));
    }
  }
  else
  {
    gdk_draw_line(px,gc[TRACKCOL],x1,y1,x2,y2);
    if (p)
    {
      pijl(px,gc[TRACKCOL],x1,y1,(int)R2D(atan2(y2-y1,x2-x1)));
    }
  }
}

void draw_sensor(GtkWidget *wnd,SAT *sat,
                                 EPOINT *pos_sat,EPOINT *pos_earth,gboolean rgbm)
{
  EPOINT pos_subsat;
  int x;
  float lon,lat;
  float lonp,latp;
  EPOINT pos_sat2;
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);
  GdkColor clr1;
  GdkGC **gc=Get_Gc(wnd);
  GdkPixmap *px=Find_Widget(wnd,"GTK_DRAWING_AREA")->window;
  if (!sat->orbit.max_sens_angle) return;

  for (x=0; x<2048; x+=128)
  {
    calc_x2satobs(&sat->kepler,&sat->orbit,
                  pos_sat,&pos_sat2,
                  sat->orbit.width_original,sat->orbit.max_sens_angle,
                  x);
    calcposrel(&sat->kepler,&sat->orbit,&pos_sat2,pos_earth,&pos_subsat);
    pos_subsat.lon-=D2R(sat->orbit.offset_lon);
    lon=R2D(pos_subsat.lon);
    lat=R2D(pos_subsat.lat);
    if (rgbm)
    {
      clr1=(sat->selected? db->clrs.ssat_scan : db->clrs.usat_scan);

      if ((x>0) && (ABS(lon-lonp) <30))
      {
        draw_rgbline(rgbi,&clr1,LON2X(lonp),LAT2Y(latp),LON2X(lon),LAT2Y(lat));
      }
      else
        draw_rgbpoint(rgbi,&clr1,LON2X(lon),LAT2Y(lat));
    }
    else
    {
      int c;
      if (x<1024) c=SCANCOL; else c=SCANCOL2;
      if ((x>0) && (ABS(lon-lonp) <30))
      {
        gdk_draw_line(px,gc[c],LON2X(lonp),LAT2Y(latp),LON2X(lon),LAT2Y(lat));
      }
      else
        gdk_draw_point(px,gc[c],LON2X(lon),LAT2Y(lat));
    }
    lonp=lon; latp=lat;
  }
}

// draw visibility line around sat
void draw_vis(GtkWidget *wnd,SAT *sat,EPOINT *refpos,gboolean rgbm,GdkColor clr1,float elev)
{
  float d;
  float c;
  float lon,lat;
  float lonp=0,latp=0;
  gboolean first=TRUE;
  gboolean normal=TRUE;
  GtkWidget *drawing_area=Find_Widget(wnd,"GTK_DRAWING_AREA");
  RGBI *rgbi=Get_RGBI(drawing_area);
  GdkGC **gc=Get_Gc(wnd);
  GdkPixmap *px=Find_Widget(wnd,"GTK_DRAWING_AREA")->window;
  if (!sat) return;
  d=acos(Rearth/(sat->orbit.height+Rearth)*cos(elev))-elev;
  for (c=0; c<=360; c+=1)
  {
    float cc=D2R(c);
    lat=asin(sin(refpos->lat)*cos(d) + cos(refpos->lat)*sin(d)*cos(cc));
    lon=asin(sin(cc)*sin(d)/cos(lat));
    // volgende OK voor sats, maar refpos=observer en sat=geo geeft rare knik.
    // Met test <PI/3. is dat weg, maar dan elders fout.
    // --> Waarom deze test zo?

    if (((ABS(refpos->lat+d*cos(cc))) < PI/2.))
    {
      lon=R2D(lon);
      lat=R2D(lat);
      normal=TRUE;
    }
    else
    {
      if (lon>0) lon=180-R2D(lon); else lon=-180-R2D(lon);
      lat=R2D(lat);
      normal=FALSE;
    }
//printf("%.0f  %f  %f  %d  ",c,lon,lat,normal,R2D(refpos->lat));
//printf("%f  %f  %f\n",R2D(refpos->lat+d*cos(cc)),R2D(refpos->lat),d*cos(cc));
    lon+=R2D(refpos->lon);
    if (lon>180) lon-=360;
    if (lon<-180) lon+=360;
    if (rgbm)
    {
      if ((!first) && (ABS(lonp-lon) < 60))
      {
        draw_rgbline(rgbi,&clr1,LON2X(lonp),LAT2Y(latp),LON2X(lon),LAT2Y(lat));
      }
      else
      {
        draw_rgbpoint(rgbi,&clr1,LON2X(lon),LAT2Y(lat));
      }
    }
    else
    {
      if ((!first) && (ABS(lonp-lon) < 60))
      {
        gdk_draw_line(px,gc[VISCOL],LON2X(lonp),LAT2Y(latp),LON2X(lon),LAT2Y(lat));
      }
      else
      {
        gdk_draw_point(px,gc[VISCOL],LON2X(lon),LAT2Y(lat));
      }
    }
    first=FALSE;
    lonp=lon; latp=lat;
  }
}


