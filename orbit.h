/**************************************************
 * RCSId: $Id: orbit.h,v 1.1 2015/11/18 18:27:12 ralblas Exp $
 *
 * Header orbit-related files.
 * Project: SSI
 * Author: R. Alblas
 *
 * History: 
 * $Log: orbit.h,v $
 * Revision 1.1  2015/11/18 18:27:12  ralblas
 * Initial revision
 *
 **************************************************/
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
#ifndef ORBIT_HDR
#define ORBIT_HDR

/*************************************
 * Some constants
 *************************************/

//#define SIGN(a) ((a)>0? 1 : (a)<0? -1 : 0)

#define LAB_EY "^Epoch year"
#define LAB_ED "^Epoch day"
#define LAB_DR "^Decay"
#define LAB_IN "^Inclination (deg)"
#define LAB_RA "^Raan (deg)"
#define LAB_EC "^Eccentricity"
#define LAB_PE "^Perigee (deg)"
#define LAB_AN "^Mean anomaly"
#define LAB_MO "^Mean motion"

#define LAB_OH "Offset gmt"
#define LAB_OS "Offset sec"
#define LAB_OL "Offset lon"
#define LAB_SN "Sat:"
#define LAB_HE "Height"
#define LAB_AG "days"

#define LAB_Advwnd "Advanced"

#define LAB_RFY "year"
#define LAB_RFN "mon"
#define LAB_RFD "day"
#define LAB_RFH "hour"
#define LAB_RFM "min"
#define LAB_RFS "sec"
#define LAB_COMPTM "Computer time"
#define LAB_SATTM "Satellite time"
#define LAB_TM "!Sel_starttime"
#define LAB_SATNMF "!sat"

#define LAB_SRCHSATTM "Search time"
#define LAB_SRCHRSLT "!from_line"
#define LAB_USESATTM "Use time"

#define Lab_Orbitwnd "Orbit edit"
#define Lab_Keplerwnd "Kepler data"
#define LAB_Exit   " Exit "
#define LAB_Cancel "Cancel"
#define LAB_Save "Save"
#define LAB_Redraw "Redraw"
#define LAB_Undo "Undo"
#define LAB_Reload "Reload"
#define LAB_Save "Save"

#endif
