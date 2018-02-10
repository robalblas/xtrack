/***********************************************
 * USB regmap 
 * Zie usb.vhd voor adressen.
 ***********************************************/
//                 address       type      comment
#ifndef GODIL_REGMAPHDR
#define GODIL_REGMAPHDR

#define A_DAT       0         // read: HRPT-data, write: stop data
#define A_CHN       1         // control   [2:0] = dectype set
#define A_STA       2         // status    [2:0] = dectype read (from pins if auto=0)
#define A_DSP       3         // control   [7:0] = display-mode: values 0,1,2
#define A_VER       4         // status    version code: 4,5 (2 bytes)
#define A_DTE       6         // status    date code: 6,7  (year,month)

#define A_ELEV      8         // control   elevation: 8,9 (2 bytes)
#define A_AZIM     10         // control   azimuth: 10,11 (2 bytes) + east/west bit
#define A_SERCTRL  12         // control   serial interface: i2c, rs232
#define A_SERSTAT  13         // status    serial interface: i2c
#define A_SERDATA  14         // control   data i2c

#define A_DISEQ_ADR  16
#define A_DISEQ_CMD  17
#define A_DISEQ_DAT1 18
#define A_DISEQ_DAT2 19

#define RS232_SEND 0x80 

#if __GTK_WIN32__
  #include "windows.h"
  #undef WINAPI
// needed to get correct links, even if WINAPI is already defined...
// Must be behind windows.h; windows.h still needed...
  #define WINAPI
  #include "ftd2xx_win.h"
#else
  #include "ftd2xx.h"
#endif
typedef struct usb_items
{
  FT_HANDLE ftHandle;
  int usb_use_id;
  char usbport_id[30];
  gboolean fast_ftd;
} USB_ITEMS;

int usb_write_reg(USB_ITEMS *usb,int addr,int dat);
int usb_init_connect(USB_ITEMS *usb);
#if __GTK_WIN32__ == 1
FT_STATUS ft_Read(FT_HANDLE ftHandle,unsigned char *dBuf,int n,DWORD *n_bytes);
FT_STATUS ft_GetDeviceInfoList(FT_DEVICE_LIST_INFO_NODE *ft_dlin,DWORD *numDevs);
FT_STATUS ft_CreateDeviceInfoList(DWORD *numDevs);
#endif
#endif
