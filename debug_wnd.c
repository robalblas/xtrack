/**************************************************
 * RCSId: $Id: debug_wnd.c,v 1.8 2018/03/07 22:37:48 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: debug_wnd.c,v $
 * Revision 1.8  2018/03/07 22:37:48  ralblas
 * _
 *
 * Revision 1.7  2018/02/02 22:50:09  ralblas
 * _
 *
 * Revision 1.5  2017/04/11 20:42:15  ralblas
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
#include <stdarg.h>
#include <stdlib.h>
#include "xtrack.h"
#include "xtrack_func.h"
#include "defines.h"
#define LAB_ADDR "!address"
#define LAB_DATI "!data in"
#define LAB_OUSB "Open USB"
#define LAB_CUSB "Close USB"
#define LAB_OCU "!ocu"
#define LAB_DEQC     "Test diseqc"
#define LAB_DEQCADDR  "!deqcaddra"
#define LAB_DEQCADDRb "!deqcaddrb"
#define LAB_DEQCCMD   "!commanda"
#define LAB_DEQCCMDb  "!commandb"
#define LAB_DEQCDAT0  "!deqcdat0a"
#define LAB_DEQCDAT0b "!deqcdat0b"
#define LAB_DEQCDAT1  "!deqcdat1a"
#define LAB_DEQCDAT1b "!deqcdat1b"
#define LAB_DEQCPOS   "!deqcpos"
#define LAB_DEQCPOSb  "!deqcposb"
#define LAB_DISLIMITS "Disable limits"
#define LAB_DEQCPOSS  "Postion"
#define LAB_STOP "Stop"
#define LAB_NSTOP "NOODStop"
#define LAB_STORM "Storm"
#define LAB_DISLIM2 "Disable lim."
#define LAB_DRIVEEAST "Drive East"
#define LAB_DRIVEWEST "Drive West"
#define LAB_DEQCSEND1 "diseqc X"
#define LAB_DEQCSEND2 "diseqc Y"
static GtkWidget *wnd;

extern DBASE *db;

#if __ADD_USB__ == 1

static void flick_but(GtkWidget *widget,char *lab_but)
{
  Set_Button(widget,lab_but,TRUE);
  Set_Buttoncolor(Find_Widget(wnd,lab_but),0xf00);
  while (g_main_iteration(FALSE));
  g_usleep(50000);
  Set_Buttoncolor(Find_Widget(wnd,lab_but),0xddd);
  while (g_main_iteration(FALSE));
}

static void send_command(USB_ITEMS *usb,int n)
{
  static int act[2];
  act[n]=!act[n];
  if (!usb) return;

  if (n==1)
  {
    flick_but(Find_Widget(wnd,LAB_DEQCSEND2),LAB_DEQCSEND2);
  }
  else
  {
    flick_but(Find_Widget(wnd,LAB_DEQCSEND1),LAB_DEQCSEND1);
  }
}

static int deqcont(gpointer pointer)
{
  USB_ITEMS *usb=(USB_ITEMS *)pointer;
  send_command(usb,0);
  return 1;
}

static void cont_send(USB_ITEMS *usb,int n)
{
  static int to;
  if ((n) && (usb))
  {
    if (!to) to=gtk_timeout_add(300,deqcont,usb);
  }
  else
  {
    if (to) gtk_timeout_remove(to); to=0;
  }
}

static int dstorm_x(gpointer gp)
{
  GtkWidget *widget=(GtkWidget *)gp;
  flick_but(widget,LAB_DEQCSEND1);
  return 0;
}

static int dstorm_y(gpointer gp)
{
  GtkWidget *widget=(GtkWidget *)gp;
  flick_but(widget,LAB_DEQCSEND2);
  return 0;
}

static void set_posa(GtkWidget *widget,float val,DISEQC *diseqc)
{
  *diseqc=val2diseqc(val,db->rotor.deg2step,db->rotor.inv_x,db->rotor.xy_rotorlim);
  Set_Entry(widget,LAB_DEQCADDR,"%x",diseqc->addr);
  Set_Entry(widget,LAB_DEQCCMD,"%x",diseqc->cmd);
  Set_Entry(widget,LAB_DEQCDAT0,"%x",diseqc->data[0]);
  Set_Entry(widget,LAB_DEQCDAT1,"%x",diseqc->data[1]);
  Set_Adjust(widget,LAB_DEQCPOS,"%d",(int)val);
}
static void set_posb(GtkWidget *widget,float val,DISEQC *diseqc)
{
  *diseqc=val2diseqc(val,db->rotor.deg2step,db->rotor.inv_x,db->rotor.xy_rotorlim);
  Set_Entry(widget,LAB_DEQCADDRb,"%x",diseqc->addr);
  Set_Entry(widget,LAB_DEQCCMDb,"%x",diseqc->cmd);
  Set_Entry(widget,LAB_DEQCDAT0b,"%x",diseqc->data[0]);
  Set_Entry(widget,LAB_DEQCDAT1b,"%x",diseqc->data[1]);
  Set_Adjust(widget,LAB_DEQCPOSb,"%d",(int)val);
}

static void test_usbcommands(GtkWidget *widget, gpointer dat)
{
  USB_ITEMS *usb=&db->usb;
  char *name=(char *)dat;
  static DISEQC diseqc,diseqc1,diseqc2;
  if (!strcmp(name,LAB_OUSB))
  {
    usb_init_connect(&db->usb);
  }
  if (!strcmp(name,LAB_CUSB))
  {
    usb_close_connect(usb);
  }
  if (!strcmp(name,LAB_DEQCADDR))
  {
    diseqc1.addr=strtol(Get_Entry(widget,name),NULL,16);
  }
  if (!strcmp(name,LAB_DEQCADDRb))
  {
    diseqc2.addr=strtol(Get_Entry(widget,name),NULL,16);
  }
  if (!strcmp(name,LAB_DEQCCMD))
  {
    diseqc1.cmd=strtol(Get_Entry(widget,name),NULL,16);
  }
  if (!strcmp(name,LAB_DEQCCMDb))
  {
    diseqc2.cmd=strtol(Get_Entry(widget,name),NULL,16);
  }
  if (!strcmp(name,LAB_DEQCDAT0))
  {
    diseqc1.data[0]=strtol(Get_Entry(widget,name),NULL,16);
  }
  if (!strcmp(name,LAB_DEQCDAT0b))
  {
    diseqc2.data[0]=strtol(Get_Entry(widget,name),NULL,16);
  }
  if (!strcmp(name,LAB_DEQCDAT1))
  {
    diseqc1.data[1]=strtol(Get_Entry(widget,name),NULL,16);
  }
  if (!strcmp(name,LAB_DEQCDAT1b))
  {
    diseqc2.data[1]=strtol(Get_Entry(widget,name),NULL,16);
  }
  if (!strcmp(name,LAB_DISLIMITS))
  {
    diseqc.addr=0x31;  // azim rotor
    diseqc.cmd=0x63;   // limits off
    Set_Entry(widget,LAB_DEQCADDR,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCADDRb,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCCMD,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCCMDb,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCDAT0,"");
    Set_Entry(widget,LAB_DEQCDAT0b,"");
    Set_Entry(widget,LAB_DEQCDAT1,"");
    Set_Entry(widget,LAB_DEQCDAT1b,"");
    diseqc1.addr=diseqc.addr;
    diseqc2.addr=diseqc.addr;
    diseqc1.cmd=diseqc.cmd;
    diseqc2.cmd=diseqc.cmd;
  }
  if (!strcmp(name,LAB_STOP))
  {
    diseqc.addr=0x31;  // azim rotor
    diseqc.cmd=0x60;   // halt
    Set_Entry(widget,LAB_DEQCADDR,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCADDRb,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCCMD,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCCMDb,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCDAT0,"");
    Set_Entry(widget,LAB_DEQCDAT0b,"");
    Set_Entry(widget,LAB_DEQCDAT1,"");
    Set_Entry(widget,LAB_DEQCDAT1b,"");
    diseqc1.addr=diseqc.addr;
    diseqc2.addr=diseqc.addr;
    diseqc1.cmd=diseqc.cmd;
    diseqc2.cmd=diseqc.cmd;
  }
  if (!strcmp(name,LAB_NSTOP))
  {
    diseqc.addr=0x31;  // azim rotor
    diseqc.cmd=0x60;   // halt
    Set_Entry(widget,LAB_DEQCADDR,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCADDRb,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCCMD,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCCMDb,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCDAT0,"");
    Set_Entry(widget,LAB_DEQCDAT0b,"");
    Set_Entry(widget,LAB_DEQCDAT1,"");
    Set_Entry(widget,LAB_DEQCDAT1b,"");
    diseqc1.addr=diseqc.addr;
    diseqc2.addr=diseqc.addr;
    diseqc1.cmd=diseqc.cmd;
    diseqc2.cmd=diseqc.cmd;
    flick_but(widget,LAB_DEQCSEND1);
    flick_but(widget,LAB_DEQCSEND2);
  }
  if (!strcmp(name,LAB_DRIVEEAST))
  {
    diseqc.addr=0x31;  // azim rotor
    diseqc.cmd=0x68;   // east
    Set_Entry(widget,LAB_DEQCADDR,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCADDRb,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCCMD,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCCMDb,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCDAT0,"");
    Set_Entry(widget,LAB_DEQCDAT0b,"");
    Set_Entry(widget,LAB_DEQCDAT1,"");
    Set_Entry(widget,LAB_DEQCDAT1b,"");
    diseqc1.addr=diseqc.addr;
    diseqc2.addr=diseqc.addr;
    diseqc1.cmd=diseqc.cmd;
    diseqc2.cmd=diseqc.cmd;
  }
  if (!strcmp(name,LAB_DRIVEWEST))
  {
    diseqc.addr=0x31;  // azim rotor
    diseqc.cmd=0x69;   // west
    Set_Entry(widget,LAB_DEQCADDR,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCADDRb,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCCMD,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCCMDb,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCDAT0,"");
    Set_Entry(widget,LAB_DEQCDAT0b,"");
    Set_Entry(widget,LAB_DEQCDAT1,"");
    Set_Entry(widget,LAB_DEQCDAT1b,"");
    diseqc1.addr=diseqc.addr;
    diseqc2.addr=diseqc.addr;
    diseqc1.cmd=diseqc.cmd;
    diseqc2.cmd=diseqc.cmd;
  }
  if (!strcmp(name,LAB_DEQCPOSS))
  {
    float val;
    val=Get_Adjust(widget,LAB_DEQCPOS);
    set_posa(widget,val,&diseqc1);
    val=Get_Adjust(widget,LAB_DEQCPOSb);
    set_posb(widget,val,&diseqc2);
  }

  if (!strcmp(name,LAB_DEQCPOS))
  {
    float val=Get_Adjust(widget,name);
    set_posa(widget,val,&diseqc1);
  }
  if (!strcmp(name,LAB_DEQCPOSb))
  {
    float val=Get_Adjust(widget,name);
    set_posb(widget,val,&diseqc2);
  }
  if (!strcmp(name,LAB_STORM))
  {
    set_posa(widget,db->rotor.storm.x,&diseqc1);
    set_posb(widget,db->rotor.storm.y,&diseqc2);

    if (db->rotor.storm_wait_x)
      gtk_timeout_add(db->rotor.storm_wait_x*1000,dstorm_x,(gpointer)widget);
    else
      flick_but(widget,LAB_DEQCSEND1);

    if (db->rotor.storm_wait_y)
      gtk_timeout_add(db->rotor.storm_wait_y*1000,dstorm_y,(gpointer)widget);
    else
      flick_but(widget,LAB_DEQCSEND2);
  }
  if (!strcmp(name,LAB_DISLIM2))
  {
    diseqc.addr=0x31;  // azim rotor
    diseqc.cmd=0x63;   // limits off
    Set_Entry(widget,LAB_DEQCADDR,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCADDRb,"%x",diseqc.addr);
    Set_Entry(widget,LAB_DEQCCMD,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCCMDb,"%x",diseqc.cmd);
    Set_Entry(widget,LAB_DEQCDAT0,"");
    Set_Entry(widget,LAB_DEQCDAT0b,"");
    Set_Entry(widget,LAB_DEQCDAT1,"");
    Set_Entry(widget,LAB_DEQCDAT1b,"");
    diseqc1.addr=diseqc.addr;
    diseqc2.addr=diseqc.addr;
    diseqc1.cmd=diseqc.cmd;
    diseqc2.cmd=diseqc.cmd;
    flick_but(widget,LAB_DEQCSEND1);
    flick_but(widget,LAB_DEQCSEND2);
  }

  if (!strcmp(name,LAB_DEQCSEND1))
  {
    Set_Button_if(Find_Window(widget,Lab_Main),LAB_RUN,FALSE);
    Set_Button_if(Find_Window(widget,LAB_PRED),LAB_SIMUL,FALSE);
    diseqc1.nr=0;
    diseqc1.invert=db->rotor.inv_x;
    Set_Button(widget,LAB_OUSB,TRUE);
    do_dqc(usb,diseqc1,TRUE);
  }

  if (!strcmp(name,LAB_DEQCSEND2))
  {
    Set_Button_if(Find_Window(widget,Lab_Main),LAB_RUN,FALSE);
    Set_Button_if(Find_Window(widget,LAB_PRED),LAB_SIMUL,FALSE);
    diseqc2.nr=1;
    diseqc2.invert=db->rotor.inv_y;
    Set_Button(widget,LAB_OUSB,TRUE);
    do_dqc(usb,diseqc2,TRUE);
  }

  if (!strcmp(name,LAB_CONT))
  {
    Set_Button_if(Find_Window(widget,Lab_Main),LAB_RUN,FALSE);
    Set_Button_if(Find_Window(widget,LAB_PRED),LAB_SIMUL,FALSE);
    Set_Button(Find_Window(widget,LAB_PRED),LAB_SIMUL,FALSE);
    cont_send(usb,Get_Button(widget,LAB_CONT));
  }

  if (usb->ftHandle) 
    Set_Led(widget,LAB_OCU,0x0f0);
  else
    Set_Led(widget,LAB_OCU,0xf00);
}

#endif

static void close_debugwnd(GtkWidget *widget)
{
  destroy_window(widget);
  db->force_pass_e1_w0=0;
#if __ADD_USB__ == 1
  cont_send(NULL,0);
#endif
}

#define LAB_EW_AUTO "auto eastwest"
#define LAB_EAST_FORCE "force east"
#define LAB_WEST_FORCE "force west"

#define LAB_RS232MON "!rs232mon"
#define LAB_USBOPEN "USB port"
static void debugcmd(GtkWidget *widget,gpointer data)
{
  char *name=(char *)data;
  if (!strcmp(name,LAB_EW_AUTO))
  {
    db->force_pass_e1_w0=0;
  }
  if (!strcmp(name,LAB_EAST_FORCE))
  {
    db->force_pass_e1_w0='e';
  }
  if (!strcmp(name,LAB_WEST_FORCE))
  {
    db->force_pass_e1_w0='w';
  }
} 

void Menu_Debug(GtkWidget *widget,gpointer data)
{
  GtkWidget *w[6];
  wnd=Find_Parent_Window(widget);
  wnd=Create_Window(wnd,0,0,Lab_WNDDebug,close_debugwnd);
  if (!wnd) return;
  w[1]=w[2]=w[3]=NULL;
  w[1]=Create_ButtonArray(LAB_RS232MON,debugcmd,1,
                          LABEL,"Monitor RS232 commands",
                          ENTRY,LAB_RS232CMD,"%-30s","(No commands sent yet)",
                          LABEL,"__________________________________________",

                          LABEL,"East-west calculation",
                          RADIOs,LAB_EW_AUTO,
                          RADIOn,LAB_EAST_FORCE,
                          RADIOn,LAB_WEST_FORCE,
                          0);

#if __ADD_USB__ == 1
  w[2]=Create_ButtonArray(LAB_USBOPEN,test_usbcommands,3,
         BUTTON,LAB_OUSB,
         LED,LAB_OCU,0x0,
         BUTTON,LAB_CUSB,
    0);

  w[3]=Create_ButtonArray("Set positions",test_usbcommands,3,
         LABEL,"Address 0x",
         ENTRY,LAB_DEQCADDR ,"%02x ",0x31,
         ENTRY,LAB_DEQCADDRb ,"%02x ",0x31,
         LABEL,"Commands",
         ENTRY,LAB_DEQCCMD ,"%02x ",0x00,
         ENTRY,LAB_DEQCCMDb ,"%02x ",0x00,
         LABEL,"Data0",
         ENTRY,LAB_DEQCDAT0 ,"%02x ",0xE0,
         ENTRY,LAB_DEQCDAT0b ,"%02x ",0xE0,
         LABEL,"Data1",
         ENTRY,LAB_DEQCDAT1 ,"%02x ",0x20,
         ENTRY,LAB_DEQCDAT1b ,"%02x ",0x20,
         LABEL,"",
         LABEL,"",
         LABEL,"",
         LABEL,"GOTOxy",
         SPIN,LAB_DEQCPOS ,"%d%d%d ",90,0,180,
         SPIN,LAB_DEQCPOSb ,"%d%d%d ",90,0,180,
         LABEL,"Send comands",
         BUTTON,LAB_DEQCSEND1,
         BUTTON,LAB_DEQCSEND2,
         LABEL,"Send cont.",
         CHECK,LAB_CONT,
         LABEL,"",
         0);

  w[4]=Create_ButtonArray("Set commands",test_usbcommands,2,
         BUTTON,LAB_DRIVEWEST,
         BUTTON,LAB_STOP,
         BUTTON,LAB_DRIVEEAST,
         BUTTON,LAB_DISLIMITS,
         BUTTON,LAB_DEQCPOSS,
         0);

  w[5]=Create_ButtonArray("Do commands",test_usbcommands,1,
           BUTTON,LAB_NSTOP,
           BUTTON,LAB_STORM,
           BUTTON,LAB_DISLIM2,
           0);
  w[4]=Pack(NULL,'h',w[4],1,w[5],1,NULL);

  w[3]=Pack(LAB_DEQC,'v',w[3],1,w[4],1,NULL);
#endif
  if ((!db->to_serial) && (!db->to_usb))
    w[0]=Pack("No items to show: USB and serial port disabled",'v',NULL);
  else
    w[0]=Pack(NULL,'v',w[1],10,w[2],10,w[3],10,NULL);

  gtk_container_add(GTK_CONTAINER(wnd),w[0]);

  Show_Button(wnd,LAB_RS232MON,db->to_serial);
  Show_Button(wnd,LAB_DEQC,db->to_usb);
  Show_Button(wnd,LAB_USBOPEN,db->to_usb);

  gtk_widget_show(wnd);
  place_window(wnd,0,0,smart_wndpos);
}
