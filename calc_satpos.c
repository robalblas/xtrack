/**************************************************
 * RCSId: $Id: calc_satpos.c,v 1.7 2018/04/24 21:21:45 ralblas Exp $
 *
 * Calculate current observation point for polar satellites 
 * Project: SSI
 * Author: R. Alblas
 *
 * History: 
 * $Log: calc_satpos.c,v $
 * Revision 1.7  2018/04/24 21:21:45  ralblas
 * _
 *
 * Revision 1.6  2018/04/22 12:10:10  ralblas
 * _
 *
 * Revision 1.5  2018/03/08 10:34:34  ralblas
 * _
 *
 * Revision 1.4  2017/04/11 19:56:13  ralblas
 * _
 *
 * Revision 1.3  2017/02/17 12:30:10  ralblas
 * _
 *
 * Revision 1.2  2016/11/04 09:24:26  ralblas
 * _
 *
 * Revision 1.1  2016/03/25 16:15:51  ralblas
 * Initial revision
 *
 * Revision 1.1  2015/11/18 18:33:49  ralblas
 * Initial revision
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
/**************************************************
  Main functions:
    xy2lonlat(): translates (x,y) of picture into (lon,lat)
    calc_orbitconst()
 **************************************************/
/**************************************************
  Used formulas to calculate the satellite orbit:

    Used symbols:
      h=height sat
      Rearth=radius earth
      g0=gravity
      to=time 1 loop (in sec)
      lpd=loops per day = 24*3600/to
      orbit_nr=# orbits since reference time (=time Kepler data)
      dt=time elapsed since reference time
      orbit_angle=remainder orbit (orbit_nr - n*2pi)
      k2_n=deviation orbit north-south
      k2_w=deviation orbit west-east

    Common physics:
      to=2*pi*sqrt(Rearth/g0) * (1+h/Rearth)**(1.5)

          { [         to           ](2/3)     }
      h = { [ -------------------- ]      - 1 } * Rearth
          { [ 2*pi*sqrt(Rearth/g0) ]          }


    Satellite movement:
                                      
                     [  Rearth  ](3.5)            2
      k2_n = Const * [ -------- ]      * {2.5*(cos(incl)-1) + 2}
                     [ h+Rearth ]

      dh_n=dt*k2_n

                     [  Rearth  ](3.5)            
      k2_w = Const * [ -------- ]      * cos(incl)
                     [ h+Rearth ]

      dh_w=dt*k2_w

      orbit_nr=(lpd + decay*dt)*dt

      orbit_angle=(orbit_nr-int(orbit_nr))*2*pi + perigee + anomaly

      x1=sin(orbit_angle+dh_n*dt)*cos(incl)
      y1=cos(orbit_angle+dh_n*dt)
      z1=sin(orbit_angle+dh_n*dt)*sin(incl)

  Take into account sensor movement.
  beta=angle "sat -> earth-middle -> current observation point"
      x=x1*cos(beta)+sin(inclination)*sin(beta);
      y=y1*cos(beta);
      z=z1*cos(beta)-cos(inclination)*sin(beta);

   Translate to lon/lat
      sat_lon=atan(y/x)+raan-dh_w
      sat_lat=asin(z)

    Earth rotation:
      rem_e=(rot_year-int(rot_year))*2*pi
      earth_lon=rem_e + Earth_offsetangle
      earth_lat=0;

    Relative rotation. If beta=0 then this is the sub-satellite point.
      rel_lon=sat_lon-earth_lon
      rel_lat=sat_lat

 **************************************************/
#include <math.h>
#include "xtrack_basefunc.h"
//#include "sattrack_funcs.h"

/*************************************
 * Pre-calculate some orbit constants
 *************************************/
int calc_orbitconst(KEPLER *kepler,ORBIT *orbit)
{
  struct tm momtm;
  if (kepler->motion<0.1) return 0;

  orbit->loop_time=24.*3600./kepler->motion;
  orbit->height=(pow(orbit->loop_time/PIx2/sqrt(Rearth/G0),2./3.)-1)*Rearth;
  orbit->k2_n=Const * pow(Rearth/(Rearth+orbit->height),3.5) * 
                          (2.5*pow(cos(kepler->inclination),2.)-0.5);

  orbit->k2_w=Const * pow((Rearth/(Rearth+orbit->height)),3.5) * 
                                            cos(kepler->inclination);

  orbit->k2_n=D2R(orbit->k2_n);
  orbit->k2_w=D2R(orbit->k2_w);

/* Translate reference time into tm struct */
  orbit->ref_tm.tm_isdst=0;
  orbit->ref_tm.tm_year=kepler->epoch_year;
  orbit->ref_tm.tm_yday=kepler->epoch_day-1;
  yday2mday_mon(&orbit->ref_tm);
  orbit->ref_tm.tm_hour=  (kepler->epoch_day-1-orbit->ref_tm.tm_yday)*24;
  orbit->ref_tm.tm_min = ((kepler->epoch_day-1-orbit->ref_tm.tm_yday)*24-
                                               orbit->ref_tm.tm_hour)*60;
  orbit->ref_tm.tm_sec =(((kepler->epoch_day-1-orbit->ref_tm.tm_yday)*24-
                                               orbit->ref_tm.tm_hour)*60-
                                               orbit->ref_tm.tm_min )*60;

  orbit->ref_time=mktime_ntz(&orbit->ref_tm);
  orbit->ref_tm.tm_wday=0;
  orbit->ref_ms=0;
  momtm=mom_tm(0.,NULL);
  orbit->data_age=(mktime_ntz(&momtm)-orbit->ref_time)/(24.*3600);

/* Set valid-flag */
  orbit->valid=TRUE;
  return 1;
}

/*************************************
 * Calculate position of satellite from 
 *   current time and reference time (ref-time is in KEPLER-data)
 * Result in pos_sat->x,y,z
 * raan and dh_w NOT taken into account. These have to be added to the lon.
 * Return: 0 if calculation has failed 
 *         1 if OK
 *************************************/
int calcpossat(struct tm *cur_tm, int cur_ms,  /* current time */
               KEPLER *kepler,                 /* kepler data */
               ORBIT *orbit,                   /* orbit data */
               EPOINT *pos_sat)                /* position satellite */
{
  double dift;
  double loop_nr;
  float orbit_angle,dh_n,dh_w;

/* Determine time difference with reference in sec */
  dift=mktime_ntz(cur_tm)-orbit->ref_time;
  dift=dift+(float)(cur_ms-orbit->ref_ms)/1000.;

/* Translate difference into days */
  dift/=(24.*3600.);

/* Calculate # loops since ref-time. */
  loop_nr=(kepler->motion+kepler->decay_rate*dift)*dift; 
  if (ABS(loop_nr/10) >= FLT_MAX) return 0;     /* loop_nr too large -> error */

/* Calc orbit-angle in radians */
  orbit_angle=(loop_nr-(int)loop_nr)*PIx2;

/* Add starting point */
  orbit_angle=orbit_angle+kepler->perigee+kepler->anomaly;

/* Shift result to [0 ... pi*2] */
  while (orbit_angle<0)    orbit_angle+=PIx2;
  while (orbit_angle>PIx2) orbit_angle-=PIx2;

/* Determine corrections */
  dh_n=dift*orbit->k2_n;
  dh_w=dift*orbit->k2_w;

  dh_n=dh_n-(int)(dh_n/PIx2)*PIx2;
  dh_w=dh_w-(int)(dh_w/PIx2)*PIx2;
  orbit->dh_w=dh_w;

/* Translate to orthonormal coordinates */
  pos_sat->x=sin(orbit_angle+dh_n)*cos(kepler->inclination);
  pos_sat->y=cos(orbit_angle+dh_n);
  pos_sat->z=sin(orbit_angle+dh_n)*sin(kepler->inclination);

  return 1;
}

/*************************************
 * Calculate observation point of satellite.
 * Takes into account sensor movement.
 * Result in pos_sat->lon,lat
 *************************************/
static void calc_satobs(KEPLER *kepler,ORBIT *orbit,
                 EPOINT *pos_sati,EPOINT *pos_sato,float beta)
{
/* No need to calc. if beta=0. Just to speed up calcs. */

  if (beta)
  {
    pos_sato->x=pos_sati->x*cos(beta)+sin(kepler->inclination)*sin(beta);
    pos_sato->y=pos_sati->y*cos(beta);
    pos_sato->z=pos_sati->z*cos(beta)-cos(kepler->inclination)*sin(beta);
  }
  else
  {
    pos_sato->x=pos_sati->x;
    pos_sato->y=pos_sati->y;
    pos_sato->z=pos_sati->z;
  }
}

/*************************************
 * Calculate position of earth since jan 1 
 *************************************/
void calcposearth(struct tm *cur_tm, int ms, EPOINT *pos_earth)
{
  double days,rot_earth;
/* days since jan 1 */
  days=cur_tm->tm_yday+1+cur_tm->tm_hour/24.+
                             cur_tm->tm_min/1440.+
                             (double)cur_tm->tm_sec/(double)86400.+
                             (float)ms/86400000.;

/* earth rotations since jan 1. Translate to 365.24 days/year. */
  rot_earth=days*LEN_LEAPYEAR/LEN_YEAR;

/* Translate into rotations. Ignore full rotations and add offset. */
  pos_earth->lon=PIx2*(rot_earth-(int)rot_earth)+D2R(Earth_offsetangle);

/* Translate to range [0 ... 2*pi] */
  while (pos_earth->lon>=PIx2) pos_earth->lon-=PIx2;

/* earth rotates in lon direction only */
  pos_earth->lat=0;
}

/*************************************
 * Calculate position of satellite relative to earth
 *************************************/
void calcposrel(KEPLER *kepler,ORBIT *orbit,
                EPOINT *pos_sat,EPOINT *pos_earth,EPOINT *pos_rel)
{
/* Translate to long/lat */

  if ((kepler) && (orbit))
  {
    pos_rel->lon=atan2(pos_sat->x,pos_sat->y)+
                            kepler->raan-orbit->dh_w-pos_earth->lon;
    pos_rel->lat=asin(pos_sat->z);
  }
  else
  {
    pos_rel->lon=pos_sat->lon-pos_earth->lon;
    pos_rel->lat=pos_sat->lat;
  }

/* Shift to range [-pi..+pi] */  
  while (pos_rel->lon < -1*PI) pos_rel->lon+=PIx2;
  while (pos_rel->lon >    PI) pos_rel->lon-=PIx2;
  while (pos_rel->lat < -1*PI) pos_rel->lat+=PIx2;
  while (pos_rel->lat >    PI) pos_rel->lat-=PIx2;
}

/**************************************************
  Translation to elevation/azimuth:
    Rsat=h+Rearth;
    h1=sin(obs_lat)*sin(sat_lat) +cos(obs_lat)*cos(sat_lat)*cos(sat_lon-obs_lon);
    h=acos(h1);
    gamma=2*asin(sqrt((1-(h1))/2.) );

    dist=sqrt(Rearth*Rearth+Rsat*Rsat-2*Rearth*Rsat*cos(gamma));
    sat_elev=acos((Rearth-Rsat*cos(gamma))/dist)-pi/2

    sat_azim=acos((sin(sat_lat)-cos(h)*sin(obs_lat))/(cos(obs_lat)*sin(h)));

 **************************************************/

/*************************************
 * Calculate elevation/azimuth wrt observer position
 *************************************/
static double calceleazim(EPOINT *pos_subsat,ORBIT *orbit,EPOINT *refpos,
              DIRECTION *satdir,ROTOR *rot)
{
  double gamma;
  double lon_o,lat_o,lon_s,lat_s;
  double dlon,h,h1;
  double dist,Rsat;
  lon_s=pos_subsat->lon; lat_s=pos_subsat->lat;
  lon_o=refpos->lon;     lat_o=refpos->lat;
  dlon=lon_s-lon_o;
  Rsat=orbit->height+Rearth;
  h1=sin(lat_o)*sin(lat_s) +cos(lat_o)*cos(lat_s)*cos(dlon);
  h=acos(h1);
  gamma=2*asin(sqrt((1-(h1))/2.) );

  dist=sqrt(Rearth*Rearth+Rsat*Rsat-2*Rearth*Rsat*cos(gamma));
  satdir->elev=acos((Rearth-Rsat*cos(gamma))/dist)-PI/2.;

  {
    double tmp=(sin(lat_s)-cos(h)*sin(lat_o))/(cos(lat_o)*sin(h));
    if (tmp>1.) tmp=1.; if (tmp<-1.) tmp=-1.;
    satdir->azim=acos(tmp);
  }

  if (satdir->azim<0) satdir->azim+=(PI*2);
  if (dlon<0) satdir->azim=PI*2-satdir->azim;

  if ((rot) && (rot->x_at_disc))
  {
    satdir->y=atan2(cos(satdir->azim),tan(satdir->elev));
    satdir->x=asin(sin(satdir->azim)*cos(satdir->elev));
  }
  else
  {
    satdir->x=atan2(sin(satdir->azim),tan(satdir->elev));
    satdir->y=asin(cos(satdir->azim)*cos(satdir->elev));
  }

  // x/y: -90...+90 ==> 0...180
  if ((rot) && (rot->x_west_is_0))
    satdir->x+=D2R(90.);
  else
    satdir->x=D2R(90.)-satdir->x;

  if ((rot) && (rot->y_south_is_0))
    satdir->y+=D2R(90.);
  else
    satdir->y=D2R(90.)-satdir->y;
  return dist;
}


void calc_x2satobs(KEPLER *kepler,ORBIT *orbit,
                   EPOINT *pos_sati,EPOINT *pos_sato,
                   int width_original,float max_sens_angle,
                   int x)
{
  float beta;
  beta=x2earthangle(orbit->height,E_W,
                    x,width_original/2,max_sens_angle);
  calc_satobs(kepler,orbit,pos_sati,pos_sato,beta);
}

/* Calc. position of satellite at time 'cur_tm' */
void calc_onepos(SAT *sat,struct tm cur_tm,int cur_ms,
                 EPOINT *pos_earth,EPOINT *pos_sat,EPOINT *pos_subsat,EPOINT *refpos)
{
  struct tm cur_gmtm;
  cur_gmtm=cur_tm;
  switch(sat->type)
  {
#if __ADD_SUNMOON__ == 1
    case sun:
      *pos_subsat=calc_sun(cur_tm);
    break;
    case moon:
    {
      EPOINT pos_moon;
      pos_moon=calc_moon(cur_tm,&sat->m_ilum,refpos); // sat->m_ilum>100: ilum east
      calcposearth(&cur_gmtm,cur_ms,pos_earth);
      calcposrel(NULL,NULL,&pos_moon,pos_earth,pos_subsat);
    }
    break;
#endif
    case satellite:
    default:
      calcposearth(&cur_gmtm,cur_ms,pos_earth);
      calcpossat(&cur_gmtm,cur_ms,&sat->kepler,&sat->orbit,pos_sat);
      calc_satobs(&sat->kepler,&sat->orbit,pos_sat,pos_sat,0);
      calcposrel(&sat->kepler,&sat->orbit,pos_sat,pos_earth,pos_subsat);
    break;
  }
  if (sat->do_forcepos)
  {
    pos_subsat->lon=sat->forced_pos.lon;
    pos_subsat->lat=sat->forced_pos.lat;
  }
}

// calc. sat. relative to pos. on earth; calc elevation/azimuth/x/y
void calc_satrelpos(struct tm cur_tm,EPOINT *pos_subsat,DIRECTION *satdir,EPOINT *refpos,
     SAT *sat,ROTOR *rot)
{
  EPOINT pos_sat;
  EPOINT pos_earth;
  calc_onepos(sat,cur_tm,0,&pos_earth,&pos_sat,pos_subsat,refpos);
  satdir->dist=calceleazim(pos_subsat,&sat->orbit,refpos,satdir,rot);
}

void calc_satrelposms(struct tm_ms cur_tm,EPOINT *pos_subsat,DIRECTION *satdir,EPOINT *refpos,
     SAT *sat,ROTOR *rot)
{
  EPOINT pos_sat;
  EPOINT pos_earth;
  calc_onepos(sat,cur_tm.tm,cur_tm.ms,&pos_earth,&pos_sat,pos_subsat,refpos);
  satdir->dist=calceleazim(pos_subsat,&sat->orbit,refpos,satdir,rot);
}

static int do_xystorm(ROTOR *rot,float reptime)
{
  static int tx,ty;
  int ret=0;
  if (reptime)
  {
    tx=rot->storm_wait_x/reptime;
    ty=rot->storm_wait_y/reptime;
  }
  else
  {
    ret=tx+ty;
    if (tx) tx--; else rot->to_rotor.x=rot->storm.x;
    if (ty) ty--; else rot->to_rotor.y=rot->storm.y;
  }
//w_dbg("  storm: tx=%d  ty=%d  ret=%d\n",tx,ty,ret);
  return ret;
}

// set pos. to send in 'rotor'
// return:  1: above horizon
//         -1: below horizon, but needs more time to set otor to storm
//          0: below horizon, ready
int calc_rotpos(DIRECTION satdir,gboolean will_track,ROTOR *rotor,float elev_horiz,float reptime)
{
  int ret=0;
  if ((satdir.elev>=elev_horiz) && (will_track))
  {
    ret=1;
    do_xystorm(rotor,reptime);     // set wait time x/y storm
    rotor->to_rotor.x=R2D(satdir.x);
    rotor->to_rotor.y=R2D(satdir.y);
    rotor->to_rotor.azim=R2D(satdir.azim);
    rotor->to_rotor.elev=R2D(satdir.elev);
  }
  else
  {
    if (do_xystorm(rotor,0.)) ret=-1; else ret=0;
    rotor->to_rotor.azim=rotor->storm.azim;
    rotor->to_rotor.elev=rotor->storm.elev;
  }
  
  // add offsets
  rotor->to_rotor.x+=rotor->offset.x;
  rotor->to_rotor.y+=rotor->offset.y;
  rotor->to_rotor.azim+=rotor->offset.azim;
  rotor->to_rotor.elev+=rotor->offset.elev;

  if (rotor->to_rotor.azim<0) rotor->to_rotor.azim+=360;
  if (rotor->to_rotor.azim>360) rotor->to_rotor.azim-=360;
  return ret;
}

void calc_satrelinfo(struct tm_ms cur_tmms,SAT *sat,ROTOR *rotor,EPOINT *refpos,gboolean calc_velo)
{
  EPOINT pos_subsat;
  DIRECTION satdir;
  struct tm_ms cur_tm_p=cur_tmms;
  double distp;
  float lat;

  if (calc_velo)
  {
    // determine speed
    cur_tm_p.tm.tm_sec--;
    mktime_ntz(&cur_tm_p.tm);
    calc_satrelposms(cur_tm_p,&pos_subsat,&satdir,refpos,sat,rotor);
    distp=satdir.dist;
    lat=pos_subsat.lat;
  }

  calc_satrelposms(cur_tmms,&pos_subsat,&satdir,refpos,sat,rotor);

  satdir.velo=0.;
  if (calc_velo)
  {
    satdir.velo=(distp-satdir.dist)*3.6;
  }

  if (lat < pos_subsat.lat)
  {
    sat->orbit.scan_ver=S_N;
    sat->orbit.scan_hor=E_W;
  }
  else
  {
    sat->orbit.scan_ver=N_S;
    sat->orbit.scan_hor=W_E;
  }
  sat->pos=pos_subsat;
  sat->dir=satdir;
}
