/**************************************************
 * RCSId: $Id: xtrack_func.h,v 1.5 2018/02/09 18:58:55 ralblas Exp $
 *
 * function defs for xrack
 * Project: xtrack
 * Author: R. Alblas
 *
 * History: 
 * $Log: xtrack_func.h,v $
 * Revision 1.5  2018/02/09 18:58:55  ralblas
 * _
 *
 * Revision 1.4  2017/04/11 20:45:31  ralblas
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

#if __ADD_JPEG__==1
int jpg2str_1b(FILE *fpi,guchar **str,int *W,int *H);
void read_jpg_line(guchar *istr,int picwidth, int picy,guint16 *line_r,guint16 *line_g,guint16 *line_b);
#endif

#include "xtrack_basefunc.h"

/**/
SAT *nearest(SAT *,int,int);
void select_this_sat(SAT *);
void calc_onepos(SAT *,struct tm ,int,EPOINT *,EPOINT *,EPOINT *,EPOINT *);

DISEQC val2diseqc(float val,int deg2step,gboolean inv,int limit);
int do_dqc(USB_ITEMS *usb,DISEQC diseqc,gboolean);
void set_displ1(float pos);
void set_displ2(float pos);

char *tmstrsh();
int strcmpwild(char *,char *);


void calcposrel(KEPLER *kepler,ORBIT *orbit,EPOINT *pos_sat,EPOINT *pos_earth,EPOINT *pos_rel);
char *tmstrshx(struct tm *,char *);

int search_file(char *rootfile,char *file,
                 char *cur_dir,char *home_dir,char *prog_dir);
GdkColor rgb2clr(int r,int g,int b);
void init_db(DBASE *,char);
int get_satkeps();
void pri_norad();
char *timestr();
int add_to_db(DBASE *db,SAT *sat,gboolean on);
void ld_clr(GdkColor *,int ,int ,int ,int );
struct tm local2gmt(struct tm *);
void generate_txt(FILE *fp,TRACK *track,SAT *sat);
void pri_track(FILE *fp,SAT *sat,struct tm tma,struct tm tmb);
struct tm mom_tm0(float);
FILE_TYPE detect_file(char *fn);


SAT *find_sat(SAT *,char *);
gboolean predict_colin(SAT *sat,struct tm *start_tm1,TRACK *tr,EPOINT *refpos);

void ps_hdr(FILE *);
void ps_ldr(FILE *);
void setfont(FILE *,int );
void draw_rect(FILE *,int ,int ,int ,int ,int ,gboolean );
void draw_text(FILE *,int ,int ,char *,...);
int titel(FILE *,DBASE *,int );
void ps_rast(FILE *,int ,int ,int );

void generate_ps(TRACK *,SAT *);
char *dbl_prct(char *);
int exist_file(char *);
int exist_file_in_dir(char *dir,char *fn);
int test_downloadprog();
int search_file(char *,char *,char *,char *,char *);

int RS232_Open_Close(int doe_open,int portnr,int speed);
int pos2godil(USB_ITEMS *,ROTOR *,float d1,float d2,int);
void pos2extprog(SAT *sat,DIRECTION satdir);
char *pos2uart(float portnr,char *command,float xazim,float yelev,int pass_e1_w0);

// godil_regmap
int usb_init_connect(USB_ITEMS *usb);
void usb_close_connect(USB_ITEMS *usb);
int usb_reset_regmap(USB_ITEMS *usb);
int usb_write_reg(USB_ITEMS *usb,int addr,int dat);
int usb_read_reg(USB_ITEMS *usb,int addr,int *dat);
int usb_read_regs(USB_ITEMS *usb,int addr,int n,unsigned char *dat);
void usb_write(USB_ITEMS *usb,int addr,int dat);
int usb_flush_tx(USB_ITEMS *usb);
void dis_dat(USB_ITEMS *usb);
int usb_read(USB_ITEMS *usb,int addr);
void ena_dat(USB_ITEMS *usb);
int get_firmware(USB_ITEMS *usb,char *fw);
void set_sat(USB_ITEMS *usb,int dat);
int get_sat(USB_ITEMS *usb);

void Read_Prefs(char *,DBASE *);
void Save_Prefs(DBASE *,gboolean);

int strcpyd(char **sp,char *s);
void freed(char **s);

int read_norad_next_keps(FILE *fp,char *sat_name,KEPLER *kepler);
int calc_orbitconst(KEPLER *kepler,ORBIT *orbit);
SAT *Find_Sat(SAT *sat,char *satname);
void remove_tracks(TRACK *track);

//EPOINT calc_sun(struct tm tm);
//EPOINT calc_moon(struct tm tm,int *,EPOINT *);
//SAT *add_sun(SAT **sat);

