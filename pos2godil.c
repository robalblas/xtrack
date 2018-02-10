/**************************************************
 * RCSId: $Id: pos2godil.c,v 1.5 2017/04/11 20:35:56 ralblas Exp $
 *
 * interface to godil module 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: pos2godil.c,v $
 * Revision 1.5  2017/04/11 20:35:56  ralblas
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

#include "sattrack.h"

DISEQC val2diseqc(float val,int deg2step,gboolean inv,int limit)
{
  DISEQC diseqc;
  int steps[2];
  int ival;
  if (!deg2step) deg2step=8;
  diseqc.val=val;
  diseqc.addr=0x31;         // azim rotor
  diseqc.cmd=0x6e;          // goto
  val=MIN(MAX(val,0),180);  //   0...180
  val=90-val;               // -90...+90
  ival=val*deg2step;        // -0x2d0..+0x2d0 of -0x5a0..+0x5a0
  if (inv) ival*=-1;
  if (limit) ival=MAX(MIN(ival,limit),-1*limit);
  steps[0]=ABS(ival)&0xff;
  steps[1]=(ABS(ival)-steps[0])>>8;
  if (ival<0)
    diseqc.data[0]=0xD0+steps[1];
  else
    diseqc.data[0]=0xE0+steps[1];
  diseqc.data[1]=steps[0];
  return diseqc;
}

static int set_displxy(USB_ITEMS *usb,float pos,int n)
{
  int ipos=pos*10;
  if (n==2)
  {
    if (usb_write_reg(usb,A_AZIM,ipos&0xff)) return 1;
    if (usb_write_reg(usb,A_AZIM+1,(ipos>>8)&0xff)) return 1;
  }
  else if (n==1)
  {
    if (usb_write_reg(usb,A_ELEV,ipos&0xff)) return 1;
    if (usb_write_reg(usb,A_ELEV+1,(ipos>>8)&0xff)) return 1;
  }
  if (usb_write_reg(usb,A_DSP,n)) return 1; // show on display: 0=norm, 1=pos1,2=pos2
  return 0;
}

int do_dqc(USB_ITEMS *usb,DISEQC diseqc,gboolean displ_show)
{
  if (diseqc.cmd!=0x6e) displ_show=FALSE;
  if (usb_write_reg(usb,A_DISEQ_ADR,diseqc.addr)) return 1;
  if (usb_write_reg(usb,A_DISEQ_CMD,diseqc.cmd)) return 1;
  if (usb_write_reg(usb,A_DISEQ_DAT1,diseqc.data[0])) return 1;
  if (usb_write_reg(usb,A_DISEQ_DAT2,diseqc.data[1])) return 1; 
  if (diseqc.nr==1)
  {
    if (usb_write_reg(usb,A_SERCTRL,0x60)) return 1; // bit 6 =send, 5=second rotor
    if (displ_show)
    {
      if (set_displxy(usb,diseqc.val,2)) return 1;
    }
    else
    {
      if (set_displxy(usb,diseqc.val,0)) return 1;
    }
  }
  else
  {
    if (usb_write_reg(usb,A_SERCTRL,0x40)) return 1; // bit 6 = send, 5=0=first rotor
    if (displ_show)
    {
      if (set_displxy(usb,diseqc.val,1)) return 1;
    }
    else
    {
      if (set_displxy(usb,diseqc.val,0)) return 1;
    }
  }
  return 0;
}

static int send_diseqcsngl(USB_ITEMS *usb,float xy,int nr,int deg2step,gboolean inv,int limit)
{
  DISEQC diseqc;
  diseqc=val2diseqc(xy,deg2step,inv,limit);
  diseqc.nr=nr;
  return do_dqc(usb,diseqc,FALSE);
}

static int send_diseqcxy(USB_ITEMS *usb,ROTOR *rotor,float x,float y)
{
  if (send_diseqcsngl(usb,x,0,rotor->deg2step,rotor->inv_x,rotor->xy_rotorlim)) return 1;
  if (send_diseqcsngl(usb,y,1,rotor->deg2step,rotor->inv_y,rotor->xy_rotorlim)) return 1;
  return 0;
}



#define DISP_POSONLY 0
int pos2godil(USB_ITEMS *usb,ROTOR *rotor,float xazim,float yelev,int showpos)
{
  static int i;
  if (!usb->ftHandle) usb_init_connect(usb);
  if (!usb->ftHandle) return 1;
  if (send_diseqcxy(usb,rotor,xazim,yelev)) return 1;
  if (usb_write_reg(usb,A_DSP,i)) return 1; // show on display: 0=norm, 1=pos1,2=pos2

  if (showpos)
  {
    if (showpos==2)   // just positions
      i=(i==1? 2 : 1);
    else
      i=(i+1)%3;      // decoder state and positions
  }
  else
  {
    i=0;              // just decoder state
  }
  switch(i)
  {
    case 1: if (set_displxy(usb,xazim,1)) return 1; break;
    case 2: if (set_displxy(usb,yelev,2)) return 1; break;
    default: if (usb_write_reg(usb,A_DSP,0)) return 1;  break;
  }
  return 0;
}
