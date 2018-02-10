/*******************************************************************
 * Copyright (C) 2000 SSI. 
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
 * RCSId: $Id: calc_sunmoon.c,v 1.3 2017/02/17 22:13:10 ralblas Exp $
 *
 * Calculate current observation point for polar satellites 
 * Project: SSI
 * Author: R. Alblas
 *
 * History: 
 * $Log: calc_sunmoon.c,v $
 * Revision 1.3  2017/02/17 22:13:10  ralblas
 * _
 *
 * Revision 1.2  2016/03/25 15:41:56  ralblas
 * _
 *
 * Revision 1.1  2015/11/18 18:33:49  ralblas
 * Initial revision
 *
 **************************************************/
#include <math.h>
#include <float.h>
#include <string.h>
#include "sattrack.h"
#include "xtrack.h"
#include "xtrack_func.h"

/********************************************************
 * Calc. sun
 ********************************************************/
  //# Astronomer's almanach time is the number of 
  //# days since (noon, 1 January 2000)
static double astronomersAlmanacTime(struct tm tm)
{
  struct tm tm1;
  memset(&tm1,0,sizeof(tm1));
  tm1.tm_mday=1;
  tm1.tm_mon=0;
  tm1.tm_year=2000-1900;
  tm1.tm_hour=12;
  long tx=mktime_ntz(&tm);
  long ty=mktime_ntz(&tm1);
  double dt=difftime(tx,ty);
  return dt/(24.*3600.);
}

static float hourOfDay(struct tm tm)
{
  float h=tm.tm_hour+tm.tm_min/60.+tm.tm_sec/3600.;
  return h;
}

static float meanLongitudeDegrees(float time)
{
  float ret;
  ret=(280.460 + 0.9856474 * time);
  while (ret>360) ret-=360;
  while (ret<0) ret+=360;
  return ret;
}

static float meanAnomalyRadians(float time)
{
  float ret;
  ret=357.528 + 0.9856003 * time;
  while (ret>360) ret-=360;
  while (ret<0) ret+=360;
  return D2R(ret);
}

static float eclipticLongitudeRadians(float mnlong, float mnanom)
{
  float ret;
  ret=mnlong + 1.915 * sin(mnanom) + 0.020 * sin(2 * mnanom);
  while (ret>360) ret-=360;
  while (ret<0) ret+=360;
  return D2R(ret);
}

static float eclipticObliquityRadians(float time)
{
  return D2R(23.439 - 0.0000004 * time);
}

static float rightAscensionRadian(float oblqec, float eclong)
{
  float num,den,ra;
  num=cos(oblqec) * sin(eclong);
  den=cos(eclong);
  ra=atan2(num,den);
  return ra;
}

static float rightDeclinationRadians(float oblqec, float eclong)
{
  return asin(sin(oblqec) * sin(eclong));
}


static float greenwichMeanSiderealTimeHours(float time, float hour)
{
  float ret;
  ret=(6.697375 + 0.0657098242 * time + hour);
  while (ret>24) ret-=24;
  while (ret<0) ret+=24;
  return ret;
}

static float localMeanSiderealTimeRadians(float gmst, float lon)
{
  float ret=(gmst + lon / 15.);
  while (ret>24) ret-=24;
  while (ret<0) ret+=24;
  return D2R(15.*ret);
}

static float hourAngleRadians(float lmst, float ra)
{
  float ret;
  ret=lmst - ra + PI;
  while (ret>2.*PI) ret-=2.*PI;
  return ret-PI;
}


EPOINT calc_sun(struct tm tm)
{
  EPOINT zon;
  float time,fhour;
  float lon=0.;
  float mnlong,mnanom,eclong,oblqec;
  float ra,dec,gmst,lmst;
  float ha;
  time=astronomersAlmanacTime(tm);
  fhour = hourOfDay(tm);

  //# Ecliptic coordinates  
  mnlong = meanLongitudeDegrees(time);  
  mnanom = meanAnomalyRadians(time);
  eclong = eclipticLongitudeRadians(mnlong, mnanom);   
  oblqec = eclipticObliquityRadians(time);

  //# Celestial coordinates
  ra =rightAscensionRadian(oblqec, eclong);
  dec = rightDeclinationRadians(oblqec, eclong);

  //# Local coordinates
  gmst = greenwichMeanSiderealTimeHours(time, fhour) ; 
  lmst = localMeanSiderealTimeRadians(gmst, lon);

  //# Hour angle
  ha = hourAngleRadians(lmst, ra);

  zon.lon=-1.*ha;
  zon.lat=dec;

  return zon;
}

#ifdef NIET
// From:
// https://github.com/alexfeseto/moonrise/blob/master/lib/moonrise/moon.rb

int T45AD[] = {0, 2, 2, 0, 0, 0, 2, 2, 2, 2,
               0, 1, 0, 2, 0, 0, 4, 0, 4, 2,
               2, 1, 1, 2, 2, 4, 2, 0, 2, 2,
               1, 2, 0, 0, 2, 2, 2, 4, 0, 3,
               2, 4, 0, 2, 2, 2, 4, 0, 4, 1,
               2, 0, 1, 3, 4, 2, 0, 1, 2, 2};

int T45AM[] = {0,  0,  0,  0,  1,  0,  0, -1,  0, -1,
               1,  0,  1,  0,  0,  0,  0,  0,  0,  1,
               1,  0,  1, -1,  0,  0,  0,  1,  0, -1,
               0, -2,  1,  2, -2,  0,  0, -1,  0,  0,
               1, -1,  2,  2,  1, -1,  0,  0, -1,  0,
               1,  0,  1,  0,  0, -1,  2,  1,  0,  0};

int T45AMP[60] = { 1, -1,  0,  2,  0,  0, -2, -1,  1,  0,
                  -1,  0,  1,  0,  1,  1, -1,  3, -2, -1,
                   0, -1,  0,  1,  2,  0, -3, -2, -1, -2,
                   1,  0,  2,  0, -1,  1,  0, -1,  2, -1,
                   1, -2, -1, -1, -2,  0,  1,  4,  0, -2,
                   0,  2,  1, -2, -3,  2,  1, -1,  3, -1};


int T45AF[]  = { 0,  0,  0,  0,  0,  2,  0,  0,  0,  0,
                 0,  0,  0, -2,  2, -2,  0,  0,  0,  0,
                 0,  0,  0,  0,  0,  0,  0,  0,  2,  0,
                 0,  0,  0,  0,  0, -2,  2,  0,  2,  0,
                 0,  0,  0,  0,  0, -2,  0,  0,  0,  0,
                -2, -2,  0,  0,  0,  0,  0,  0,  0, -2};

int T45AL[] = {6288774, 1274027, 658314, 213618, -185116,
               -114332,   58793,  57066,  53322,   45758,
                -40923,  -34720, -30383,  15327,  -12528,
                 10980,   10675,  10034,   8548,   -7888,
                 -6766,   -5163,   4987,   4036,    3994,
                  3861,    3665,  -2689,  -2602,    2390,
                 -2348,    2236,  -2120,  -2069,    2048,
                 -1773,   -1595,   1215,  -1110,    -892,
                  -810,     759,   -713,   -700,     691,
                   596,     549,    537,    520,    -487,
                  -399,    -381,    351,   -340,     330,
                   327,    -323,    299,    294,       0};

int T45AR[] = {-20905355, -3699111, -2955968, -569925,   48888,
                   -3149,   246158,  -152138, -170733, -204586,
                 -129620,   108743,   104755,   10321,       0,
                   79661,   -34782,   -23210,  -21636,   24208,
                   30824,    -8379,   -16675,  -12831,  -10445,
                  -11650,    14403,    -7003,       0,   10056,
                    6322,    -9884,     5751,       0,   -4950,
                    4130,        0,    -3958,       0,    3258,
                    2616,    -1897,    -2117,    2354,       0,
                       0,    -1423,    -1117,   -1571,   -1739,
                       0,    -4421,        0,       0,       0,
                       0,     1165,        0,       0,    8752};

// Meeus table 45B latitude of the moon

int T45BD[] = {0, 0, 0, 2, 2, 2, 2, 0, 2, 0,
               2, 2, 2, 2, 2, 2, 2, 0, 4, 0,
               0, 0, 1, 0, 0, 0, 1, 0, 4, 4,
               0, 4, 2, 2, 2, 2, 0, 2, 2, 2,
               2, 4, 2, 2, 0, 2, 1, 1, 0, 2,
               1, 2, 0, 4, 4, 1, 4, 1, 4, 2};

int T45BM[] = { 0,  0,  0,  0,  0,  0,  0, 0,  0,  0,
               -1,  0,  0,  1, -1, -1, -1, 1,  0,  1,
                0,  1,  0,  1,  1,  1,  0, 0,  0,  0,
                0,  0,  0,  0, -1,  0,  0, 0,  0,  1,
                1,  0, -1, -2,  0,  1,  1, 1,  1,  1,
                0, -1,  1,  0, -1,  0,  0, 0, -1, -2};

int T45BMP[] = {0,  1, 1,  0, -1, -1,  0,  2,  1,  2,
                0, -2, 1,  0, -1,  0, -1, -1, -1,  0,
                0, -1, 0,  1,  1,  0,  0,  3,  0, -1,
                1, -2, 0,  2,  1, -2,  3,  2, -3, -1,
                0,  0, 1,  0,  1,  1,  0,  0, -2, -1,
                1, -2, 2, -2, -1,  1,  1, -1,  0,  0};

int T45BF[] = { 1,  1, -1, -1,  1, -1,  1,  1, -1, -1,
               -1, -1,  1, -1,  1,  1, -1, -1, -1,  1,
                3,  1,  1,  1, -1, -1, -1,  1, -1,  1,
               -3,  1, -3, -1, -1,  1, -1,  1, -1,  1,
                1,  1,  1, -1,  3, -1, -1,  1, -1, -1,
                1, -1,  1, -1, -1, -1, -1, -1, -1,  1};

int T45BL[] = {5128122, 280602, 277693, 173237, 55413,
                 46271,  32573,  17198,   9266,  8822,
                  8216,   4324,   4200,  -3359,  2463,
                  2211,   2065,  -1870,   1828, -1794,
                 -1749,  -1565,  -1491,  -1475, -1410,
                 -1344,  -1335,   1107,   1021,   833,
                   777,    671,    607,    596,   491,
                  -451,    439,    422,    421,  -366,
                  -351,    331,    315,    302,  -283,
                  -229,    223,    223,   -220,  -220,
                  -185,    181,   -177,    176,   166,
                  -164,    132,   -119,    115,   107};

#endif
#ifdef NIET
double rev(double angle){return angle-(int)(angle/360.0)*360.0;}
double sind(double angle){return sin((angle*PI)/180.0);}
double cosd(double angle){return cos((angle*PI)/180.0);}
double tand(double angle){return tan((angle*PI)/180.0);}
double asind(double c){return (180.0/PI)*asin(c);}
//float acosd(double c){return (180.0/PI)*acos(c);}
double atan2d(double y,double x){return (180.0/PI)*atan2(y,x)-180.0*(x<0);}

EPOINT calc_moon3(struct tm tm)
{
  EPOINT maan;
  double JD=astronomersAlmanacTime(tm);
  double T2=(JD-2451545)/36525;
  double lambdam = 218.316 + 481267.881*T2 + 6.29*sind(134.9 + 477198.85*T2) -1.27*sind(259.2-413335.38*T2) + 
                  0.66*sind(235.7 + 890534.23*T2)+ 0.21*sind(269.9 + 954397.7*T2)- 0.19*sind(357.5 + 35999.05*T2) -
                  0.11*sind(186.6 + 966404.05*T2);

  double betam = 5.13*sind(93.3 + 483202.03*T2) + 0.28*sind(228.2 + 960400.87*T2) -0.28*sind(318.3 + 6003.18*T2)- 
                 0.17*sind(217.6-407332.2*T2);


lambdam=rev(lambdam); // (lambdam-((int)(lambdam/360.))*360);
  maan.lon=D2R(lambdam);
  maan.lat=D2R(betam);
//printf("l=%f  b=%f\n",lambdam,betam);
  return maan;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
#define RADS 0.0174532925199433
#define DEGS 57.2957795130823
#define TPI 6.28318530717959

/* returns an angle in rads in the range 0 to two pi */
double rangerad(double x) 
{
  double a, b;
  b = x / TPI;
  a = TPI * (b - floor(b));
  if (a < 0)
    a = TPI + a;
  return(a);
}


/* returns an angle in degrees in the range 0 to 360 */
double range(double x) 
{
  double a, b;
  b = x / 360;
  a = 360 * (b - floor(b));
  if (a < 0)
    a = 360 + a;
  return(a);
}



/*
moonpos() takes days from J2000.0 and returns ecliptic coordinates
of moon in the pointers. Note call by reference.
This function is within a couple of arcminutes most of the time,
and is truncated from the Meeus Ch45 series, themselves truncations of
ELP-2000. Returns moon distance in earth radii.
Terms have been written out explicitly rather than using the
table based method as only a small number of terms is
retained.
*/
void moonpos(double d, double *lambda, double *beta, double *rvec) 
{
  double dl, dB, dR, L, D, M, M1, F, e, lm, bm, rm, t;

  t = d / 36525;

  L = range(218.3164591  + 481267.88134236  * t) * RADS;
  D = range(297.8502042  + 445267.1115168  * t) * RADS;
  M = range(357.5291092  + 35999.0502909  * t) * RADS;
  M1 = range(134.9634114  + 477198.8676313  * t - .008997 * t * t) * RADS;
  F = range(93.27209929999999  + 483202.0175273  * t - .0034029  * t * t) * RADS;
  e = 1 - .002516 * t;

  dl =      6288774 * sin(M1);
  dl +=     1274027 * sin(2 * D - M1);
  dl +=      658314 * sin(2 * D);
  dl +=      213618 * sin(2 * M1);
  dl -=  e * 185116 * sin(M);
  dl -=      114332 * sin(2 * F) ;
  dl +=       58793 * sin(2 * D - 2 * M1);
  dl +=   e * 57066 * sin(2 * D - M - M1) ;
  dl +=       53322 * sin(2 * D + M1);
  dl +=   e * 45758 * sin(2 * D - M);
  dl -=   e * 40923 * sin(M - M1);
  dl -=       34720 * sin(D) ;
  dl -=   e * 30383 * sin(M + M1) ;
  dl +=       15327 * sin(2 * D - 2 * F) ;
  dl -=       12528 * sin(M1 + 2 * F);
  dl +=       10980 * sin(M1 - 2 * F);
  lm = rangerad(L + dl / 1000000 * RADS);

  dB =   5128122 * sin(F);
  dB +=   280602 * sin(M1 + F);
  dB +=   277693 * sin(M1 - F);
  dB +=   173237 * sin(2 * D - F);
  dB +=    55413 * sin(2 * D - M1 + F);
  dB +=    46271 * sin(2 * D - M1 - F);
  dB +=    32573 * sin(2 * D + F);
  dB +=    17198 * sin(2 * M1 + F);
  dB +=     9266 * sin(2 * D + M1 - F);
  dB +=     8822 * sin(2 * M1 - F);
  dB += e * 8216 * sin(2 * D - M - F);
  dB +=     4324 * sin(2 * D - 2 * M1 - F);
  bm = dB / 1000000 * RADS;

  dR =    -20905355 * cos(M1);
  dR -=     3699111 * cos(2 * D - M1);
  dR -=     2955968 * cos(2 * D);
  dR -=      569925 * cos(2 * M1);
  dR +=   e * 48888 * cos(M);
  dR -=        3149 * cos(2 * F);
  dR +=      246158 * cos(2 * D - 2 * M1);
  dR -=  e * 152138 * cos(2 * D - M - M1) ;
  dR -=      170733 * cos(2 * D + M1);
  dR -=  e * 204586 * cos(2 * D - M);
  dR -=  e * 129620 * cos(M - M1);
  dR +=      108743 * cos(D);
  dR +=  e * 104755 * cos(M + M1);
  dR +=       79661 * cos(M1 - 2 * F);
  rm = 385000.56  + dR / 1000;

  *lambda = lm;
  *beta = bm;
  /* distance to Moon must be in Earth radii */
  *rvec = rm / 6378.14;
}

double epsilon(double d) 
{
  double t = d/ 36525;
  return((23.4392911111111 - (t* (46.8150 + 0.00059*t)/3600)) *RADS);
}

/*
gets the atan2 function returning angles in the right
order and  range
*/
double atan22(double y, double x) 
{
  double a;

  a = atan2(y, x);
  if (a < 0) a += TPI;
  return(a);
}

void equatorial(double d, double *lon, double *lat, double *r) 
{
  double  eps, ceps, seps, l, b;

  l = *lon;
  b = * lat;
  eps = epsilon(d);
  ceps = cos(eps);
  seps = sin(eps);
  *lon = atan22(sin(l)*ceps - tan(b)*seps, cos(l));
  *lat = asin(sin(b)*ceps + cos(b)*seps*sin(l));
}

void libration(double day, double lambda, double beta, double alpha, double *l, double *b, double *p) 
{
  double i, f, omega, w, y, x, a, t, eps;
  t = day / 36525;
  i = 1.54242 * RADS;
  eps = epsilon(day);
  f = range(93.2720993 + 483202.0175273 * t - .0034029 * t * t) * RADS;
  omega = range(125.044555 - 1934.1361849 * t + .0020762 * t * t) * RADS;
  w = lambda - omega;
  y = sin(w) * cos(beta) * cos(i) - sin(beta) * sin(i);
  x = cos(w) * cos(beta);
  a = atan22(y, x);
  *l = a - f;

  /*  kludge to catch cases of 'round the back' angles  */
  if (*l < -90 * RADS) *l += TPI;
  if (*l > 90 * RADS)  *l -= TPI;
  *b = asin(-sin(w) * cos(beta) * sin(i) - sin(beta) * cos(i));

  /*  pa pole axis - not used for Sun stuff */
  x = sin(i) * sin(omega);
  y = sin(i) * cos(omega) * cos(eps) - cos(i) * sin(eps);
  w = atan22(x, y);
  *p = rangerad(asin(sqrt(x*x + y*y) * cos(alpha - w) / cos(*b)));
}

void illumination(double day, double lra, double ldec, double dr, double sra, double sdec, double *pabl, double *ill) 
{
  double x, y, phi, i;
  y = cos(sdec) * sin(sra - lra);
  x = sin(sdec) * cos(ldec) - cos(sdec) * sin(ldec) * cos (sra - lra);
  *pabl = atan22(y, x);
  phi = acos(sin(sdec) * sin(ldec) + cos(sdec) * cos(ldec) * cos(sra-lra));
  i = atan22(sin(phi) , (dr - cos(phi)));
  *ill = 0.5*(1 + cos(i));
}

void ecliptic(double d, double *lon, double *lat, double *r) 
{
  double  eps, ceps, seps, alp, dec;
  alp = *lon;
  dec = *lat;
  eps = epsilon(d);
  ceps = cos(eps);
  seps = sin(eps);
  *lon = atan22(sin(alp)*ceps + tan(dec)*seps, cos(alp));
  *lat = asin(sin(dec)*ceps - cos(dec)*seps*sin(alp));
}


void topo(double lst, double glat, double *alp, double *dec, double *r) 
{
  double x, y, z, r1;
  x = *r * cos(*dec) * cos(*alp) - cos(glat) * cos(lst);
  y = *r * cos(*dec) * sin(*alp) - cos(glat) * sin(lst);
  z = *r * sin(*dec)  - sin(glat);
  r1 = sqrt(x*x + y*y + z*z);
  *alp = atan22(y, x);
  *dec = asin(z / r1);
  *r = r1;
}

void sunpos(double d, double *lambda, double *beta, double *rvec) 
{
  double L, g, ls, bs, rs;

  L = range(280.461 + .9856474 * d) * RADS;
  g = range(357.528 + .9856003 * d) * RADS;
  ls = L + (1.915 * sin(g) + .02 * sin(2 * g)) * RADS;
  bs = 0;
  rs = 1.00014 - .01671 * cos(g) - .00014 * cos(2 * g);
  *lambda = ls;
  *beta = bs;
  *rvec = rs;
}

double alt(double glat, double ha, double dec) 
{
  return(asin(sin(dec) * sin(glat) + cos(dec) * cos(glat) * cos(ha)));
}

double gst( double d) 
{
  double t = d / 36525;
  double theta;
  theta = range(280.46061837 + 360.98564736629 * d + 0.000387933 * t * t);
  return(theta * RADS);
}

#define ER_OVER_AU 0.0000426352325194252
EPOINT calc_moon(struct tm tm,int *pill,EPOINT *refpos) 
{
  EPOINT maan;
  double d,mlambda, mbeta, mrv;
  double malpha, mdelta;
  d=astronomersAlmanacTime(tm);
  moonpos(d, &mlambda, &mbeta, &mrv);
  malpha = mlambda;
  mdelta = mbeta;
  equatorial(d, &malpha, &mdelta, &mrv);
  *pill=100; // 0...100: east; 200-100: west
  if (1) // dit werkt nog niet goed; wat is 'b'? 
  {
    double slambda, sbeta, srv;
    double tlambda, tbeta, trv;
    double glat=0.,glong=0.;
    double lst, sunalt, shr, day, Co;
    double salpha, sdelta, mhr, moonalt, l, b;
    double ls, bs, hlambda, hbeta, dratio, paxis, dummy, pabl, ill;
    char sunchar, moonchar;
    day=d;
    if (refpos)
    {
      glat=refpos->lat;
      glong=refpos->lat;
    }
    lst = gst(day) + glong;
    topo(lst, glat, &malpha, &mdelta, &mrv);
    mhr = rangerad(lst - malpha);
    moonalt = alt(glat, mhr, mdelta);
    /* find sun altitude character */
    sunpos(day, &slambda, &sbeta, &srv);
    salpha = slambda;
    sdelta = sbeta;
    equatorial(day, &salpha, &sdelta, &srv);
    shr = rangerad(lst - salpha);
    sunalt = alt(glat, shr, sdelta);
    if (sunalt > 0 * RADS) sunchar = '*';
    if (sunalt < 0 * RADS) sunchar = 'c';
    if (sunalt < - 6 * RADS) sunchar = 'n';
    if (sunalt < -12 * RADS) sunchar = 'a';
    if (sunalt < -18 * RADS) sunchar = ' ';

    /* Optical libration and Position angle of the Pole */

    tlambda = malpha;
    tbeta = mdelta;
    trv = mrv;
    ecliptic(day, &tlambda, &tbeta, &trv);
    libration(day, tlambda, tbeta, malpha,  &l,  &b, &paxis);

    /* Selen Colongitude and latitude of sub solar point */

    dratio = mrv / srv * ER_OVER_AU;
    hlambda = slambda + PI + dratio * cos(mbeta) * sin(slambda - mlambda);
    hbeta = dratio * mbeta;
    libration(day, hlambda, hbeta, salpha, &ls,  &bs, &dummy);
    ls = rangerad(ls);
    if(ls < 90 * RADS) Co = 90 * RADS - ls;
    else Co = 450 * RADS - ls;
    if(Co < 90 * RADS || Co > 270 * RADS) moonchar = 'r';
    else moonchar = 's';

    /* PA of bright limb, and percentage illumination */

    illumination(day, malpha, mdelta, dratio, salpha, sdelta, &pabl, &ill);
    *pill=ill*100;
// onderstaande is ONZIN! Hoe bepalen welke kant maan belicht wordt?
    if (l<0) (*pill)=200-(*pill); // <100: left, >100: right; 0=200=dark
  }

  maan.lon = malpha;
  maan.lat = mdelta;
  return maan;
}









#ifdef WEG
EPOINT calc_moon(struct tm tm,int *pilum)
{
return calc_moon2(tm,pilum);
//printf("B: lon=%f  lat=%f\n",R2D(maan.lon),R2D(maan.lat));
  EPOINT maan;
  float d;
  d=astronomersAlmanacTime(tm);
  float N = (125.1228 - 0.0529538083 * d) / 180 * PI;
  float i = 5.1454;
  float w = (318.0634 + 0.1643573223 * d) / 180 * PI;
  float e = 0.054900; //eccentricity
  float M = (115.3654 + 13.0649929509 * d) / 180.0f * PI ; //mean anomaly

  float E = M + e * sin(M) * ( 1.0 + e * cos(M) );
  E = E - ( E - e * sin(E) - M ) / ( 1 - e * cos(E) );

  float xv = ( cos(E) - e );
  float yv = ( sqrt(1.0 - e*e) * sin(E) );

  float v = atan2( yv, xv );

  float xh = ( cos(N) * cos(v+w) - sin(N) * sin(v+w) * cos(i) );
  float yh = ( sin(N) * cos(v+w) + cos(N) * sin(v+w) * cos(i) );
  float zh = ( sin(v+w) * sin(i) );

  maan.lon = atan2( yh, xh ); //longitude
  maan.lat = atan2( zh, sqrt(xh*xh+yh*yh) ); //latitude
  return maan;
}
#endif
