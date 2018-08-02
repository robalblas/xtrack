/**************************************************
 * RCSId: $Id: pos2uart.c,v 1.9 2018/03/08 10:42:32 ralblas Exp $
 *
 * Satellite tracker 
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: pos2uart.c,v $
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
#include "rs232.h"

// doe_open=1: open conn., return conn.
// doe_open=0: close conn., return conn.
// doe_open=-1: do nothing, return current conn.
int RS232_Open_Close(int doe_open,int portnr,int speed)
{
  static int is_open;
  if (doe_open>0)
  {
    if (!is_open)
    {
      if (!(RS232_OpenComport(portnr, speed)))
      {
        is_open=1;
      }
    }
    else
    {
      // al open, doe niets
    }
  }
  else if (doe_open<0)
  {
    return is_open;
  }
  else
  {
    if (is_open)
    {
      RS232_CloseComport(portnr);
      is_open=0;
    }
    else
    {
      // was al dicht, doe niets
    }
  }
  return is_open;
}

// __linux__ is al gedef. op linux-bak
//#if __GTK_WIN32__ == 1
//#else
//#define __linux__
//#endif

#define nint(f) (int)((f)+0.5)

/*
 * expected frmt contains 2x %d  or  2x %f: xazim,yelev
            Or   contains 3x %d  or %d and 2x %f: ew,xazim,yelev
   ==> frmt filled in into prog (should be big enough: length > strlen(frmt)+16)
 */
void load_cmd(char *prog,char *frmt,int ew,float xazim,float yelev)
{
  int n=0,m=0;
  char *p;
  char df='d';
  if (strchr(frmt,'f')) df='f';

  for (p=frmt; *p; p++)
  {
    if (*p=='%')
    {
      n++;
    }
  }

  for (p=frmt; *p; p++)
  {
    if (*p=='%')
    {
      m++;
    }
    if (m>=1)
    {
      if ((n==3) && (m==1))
      {
        if (strchr("df",*p)) *p='d';
      }
      else
      {
        if (strchr("df",*p)) *p=df;
      }
    }
  }

  if (n==3)
  {
    if (df=='f')
    {
      snprintf(prog,strlen(frmt)+8+8,frmt,ew,xazim,yelev);
    }
    else
    {
      snprintf(prog,strlen(frmt)+8+8,frmt,ew,nint(xazim),nint(yelev));
    }
  }
  else
  {
    if (df=='f')
      snprintf(prog,strlen(frmt)+8+8,frmt,xazim,yelev);
    else
      snprintf(prog,strlen(frmt)+8+8,frmt,nint(xazim),nint(yelev));
  }

  if (!strcmp(prog+strlen(prog)-2,"\\r"))      sprintf(prog+strlen(prog)-2,"%c",'\r');
  else if (!strcmp(prog+strlen(prog)-2,"\\n")) sprintf(prog+strlen(prog)-2,"%c",'\n');
  else                                         strcat(prog,"\n");
}

char *pos2uart(int portnr,char *command,float xazim,float yelev,int pass_e1_w0)
{
  static char prog[100];
  load_cmd(prog,command,pass_e1_w0,xazim,yelev);
//printf("%d  %f  %f    %s\n",pass_e1_w0,xazim,yelev,prog);
  if (RS232_Open_Close(-1,0, 0))
    RS232_SendBuf(portnr, (unsigned char*)prog, strlen(prog));
  return prog;
}
