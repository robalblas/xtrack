/**************************************************
 * RCSId: $Id: defines.h,v 1.5 2018/03/08 09:48:54 ralblas Exp $
 *
 * menu/button names etc.
 * Project: xtrack 
 * Author: R. Alblas
 *
 * History: 
 * $Log: defines.h,v $
 * Revision 1.5  2018/03/08 09:48:54  ralblas
 * _
 *
 * Revision 1.4  2017/04/11 20:53:13  ralblas
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
#define LAB_SATSEL "!Satselect"

#define RASCOL 0
#define RASCOL2 1
#define SATCOL 2
#define TRACKCOL 8
#define VISCOL 6
#define SCANCOL 3
#define SCANCOL2 4
#define REFCOL 5
#define MAXCOL 10
#define LONOFFSET 180
#define LATOFFSET 90
/*
#define FACT 2
*/
#define RAND 0
#define LON2X(l) ((int)((LONOFFSET+l-db->ox)*db->fact*db->zx)+RAND)
#define LAT2Y(l) ((int)((LATOFFSET-l-db->oy)*db->fact*db->zy)+RAND)
#define X2LON(x) ((float)((x-RAND)/(db->fact*db->zx)-LONOFFSET+db->ox))
#define Y2LAT(y) ((float)(LATOFFSET-db->oy-(y-RAND)/(db->fact*db->zy)))

#define dagtijd(t) (t.tm_hour*60+t.tm_min)
#define mndtijd(t) (t.tm_mday)
#define MAX_W 500
#define t2x(t) ((t-db->ps_x_offset*60)*MAX_W/db->ps_max_t+50)
#define t2y(t) ((31-t)*db->ps_max_h/31 + db->ps_y_offset)

#define Lab_Main "xtrack"
#define LAB_SATN "Satellite"

#define MENU_FILE  "/File"
#define MENU_KF    "File/Keplerfile"
#define MENU_Q     "File/Quit"

#define MENU_EDIT   "/Edit"
#define MENU_EDSEP  "Edit/"
#define MENU_SF     "Edit/Select sat"
#define MENU_EK     "Edit/Kepler data"
#define MENU_PR     "Edit/Preferences"

#define MENU_PD     "Predict"

#define MENU_ZOOM   "/Zoom"
#define MENU_ZMFULL "Zoom/100% (f)"
#define MENU_ZMIN   "Zoom/In pos (i)"
#define MENU_ZMOUT  "Zoom/Out pos (o)"
#define MENU_ZMIN2  "Zoom/In (I)"
#define MENU_ZMOUT2 "Zoom/Out (O)"

#define MENU_HELP   "/Help"
#define MENU_PINFO  "Help/Prog info"
#define MENU_DEBUG  "Help/Debug"

#define Lab_WNDDebug "Debug"
#define LAB_RS232CMD "Last sent:"


#define LAB_LON "^lon"
#define LAB_LAT "^lat"
#define LAB_ELE "^elev"
#define LAB_AZI "^azim"
#define LAB_XXX "^X"
#define LAB_YYY "^Y"
#define LAB_DIST "^Distance"
#define LAB_OU "^+utc"
#define LAB_RUN "Run"
#define ASEL "Auto sel"
#define LAB_EXT "Disabled/Enabled"
#define LAB_DISPLROT "sat/rot"
#define SEL "Select sat"
#define EKP "Edit kepler"
#define JAAR "^year"
#define MND "^month"
#define DAG "^day"
#define UUR "^hour"
#define MINT "^min"
#define SEC "^sec"
#define LAB_TEXT "Textje"

#define LAB_PRED "Predict "
#define LAB_SIMUL "Simulate "
#define LAB_CONT "!send cont 1"
