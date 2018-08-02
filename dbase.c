/**************************************************
 * RCSId: $Id: dbase.c,v 1.5 2017/04/23 14:29:41 ralblas Exp $
 *
 * database functions 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: dbase.c,v $
 * Revision 1.5  2017/04/23 14:29:41  ralblas
 * _
 *
 * Revision 1.4  2017/04/11 20:38:39  ralblas
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
#include "xtrack.h"
#include "sattrack.h"

SAT *add_sun(SAT **sat)
{
  SAT *snw;
  snw=Create_Sat(sat);
  snw->type=sun;
  snw->orbit.height=150000000.*1000.;

  strcpy(snw->satname,"SUN");
  return snw;
}

SAT *add_moon(SAT **sat)
{
  SAT *snw;
  snw=Create_Sat(sat);
  snw->type=moon;
  snw->orbit.height=150000000.;

  strcpy(snw->satname,"MOON");
  return snw;
}

#define ZON
#define MAAN
// sat1: only check if sun/moon already in list
// return: linked sat with only sun/moon
static SAT *add_sunmoon(SAT *sat1)
{
  SAT *sat=NULL;
#ifdef ZON
  if (!Find_Sat(sat1,"SUN"))
    add_sun(&sat);  // first in row
#endif
#ifdef MAAN
  if (!Find_Sat(sat1,"MOON"))
    add_moon(&sat); // second in row
#endif
  return sat;
}

list_sats(SAT *sat)
{
  Rewind(sat);
  for (; sat; sat=sat->next) printf("%s\n",sat->satname);
}

// replace db->sat by sat; keep flags in db->sat
int add_to_db(DBASE *db,SAT *sat,gboolean on)
{
  SAT *sat_new,*sat_old,*sunmoon=NULL;
  Rewind(sat);
  db->sat_sel=NULL;
  // copy flags from sat's in db->sat to sat from input

#if __ADD_SUNMOON__ == 1
  sunmoon=add_sunmoon(sat);
#endif
  // connect sat to sun/moon
  for (sat_new=sunmoon; ((sat_new) && (sat_new->next)); sat_new=sat_new->next);
  if (sat_new)
  {
    sat_new->next=sat;
    if (sat) sat->prev=sat_new;
    sat=sat_new;
  }
  Rewind(sat);
  for (sat_new=sat; sat_new; sat_new=sat_new->next)
  {
    if ((sat_old=Find_Sat(db->sat,sat_new->satname)))
    {
      sat_new->type=sat_old->type;
      sat_new->visible=sat_old->visible;
      sat_new->selected=sat_old->selected;
      if (sat_new->selected) db->sat_sel=sat_new;
    }
    if (on) sat_new->visible=TRUE;
    sat_new->db=db;
  }

  do
  {
// waar is dit voor???
//    if ((db->sat) && (db->sat->track)) { db->sat=NULL; puts("AAAA"); }
  } while ((db->sat=Remove_Sat(db->sat)));

  sat_new=NULL;
  // goto last sat in tree
  for (sat_new=db->sat; ((sat_new) && (sat_new->next)); sat_new=sat_new->next);

  // add new sat set
  if (sat_new)                // add rest
  {
    sat_new->next=sat;
    if (sat) sat->prev=sat_new;
  }
  else                        // no sun/moon
  {
    db->sat=sat;
  }

  // make sure at least 1 is selected
  db->sat_sel=NULL;
  if (!db->sat_sel)
  {
    db->sat_sel=db->sat;
    if (db->sat) db->sat->selected=TRUE;
  }
  return 1;
}

void Set_Button_if(GtkWidget *wnd,char *lab,gboolean set)
{
  int val=Get_Button(wnd,lab);
  if (val==1)
//  if (set!=val)
  {
    Set_Button(wnd,lab,set);
  }
}

