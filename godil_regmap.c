#define ADD_GTK 1

#if ADD_GTK
  #include "gtk/gtk.h"
#endif

#include "godil_regmap.h"

#define CTRL_NOP  0x00
#define CTRL_WR   0x40
#define CTRL_RD   0xC0


// return first in list
static char *usb_list_devs()
{
  FT_STATUS ftStatus;
  int iNumDevs = 0;
  char *bufpntr[2];
  static char buf0[64];
  bufpntr[0]=buf0;
  bufpntr[1]=NULL;
  *buf0=0;
  ftStatus = FT_ListDevices(bufpntr, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
  if(ftStatus != FT_OK)
  {
    return NULL;
  }
  return buf0;
}

// Open USB-port
int usb_init_connect(USB_ITEMS *usb)
{
  FT_HANDLE ftHandle;
  FT_STATUS ftStatus;
  char *ftd_id=(usb->usb_use_id?usb->usbport_id : NULL);
  if (usb->ftHandle) return 0;   // was already open (return 'OK')
  // get ID if not defined
  if (!ftd_id)
  {
    if (!(ftd_id=usb_list_devs()))
    {
      return -1; // error open ISB
    }
  }
  if((ftStatus = FT_OpenEx(ftd_id, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle)) != FT_OK)
  {
    return -1;  // error open USB
  }
  ftStatus=FT_SetTimeouts(ftHandle,1000,1000);
  usb->ftHandle=ftHandle;
  return 0; // OK
}

void usb_close_connect(USB_ITEMS *usb)
{
  if (usb->ftHandle) FT_Close(usb->ftHandle);
  usb->ftHandle=0;
}


int usb_reset_regmap(USB_ITEMS *usb)
{
  FT_STATUS ftStatus;
  DWORD nr_bwr;
  unsigned char pcBufWrite[3];
  pcBufWrite[0]=CTRL_NOP;
  pcBufWrite[1]=CTRL_NOP;
  if ((ftStatus=FT_Write(usb->ftHandle, pcBufWrite,2, &nr_bwr)==FT_OK))
  {
    if (nr_bwr==2) return 0; else return 1;
  }
  return -1;
}

int usb_write_reg(USB_ITEMS *usb,int addr,int dat)
{
  FT_STATUS ftStatus;
  DWORD n_bytes;
  unsigned char dBuf[3];
  int was_open=(int)usb->ftHandle;
  if (usb_init_connect(usb)) return -1;
  if (!was_open) g_usleep(100000); 
  dBuf[0]=CTRL_WR|addr;
  dBuf[1]=dat;
  if ((ftStatus=FT_Write(usb->ftHandle, dBuf,2,&n_bytes)!=FT_OK)) return -1;
  if (n_bytes!=2) return 1;
//  if (!was_open) usb_close_connect(usb);
  return 0; // OK
}

int usb_read_reg(USB_ITEMS *usb,int addr,int *dat)
{
  FT_STATUS ftStatus;
  DWORD n_bytes;
  unsigned char dBuf[3];
  int was_open=(int)usb->ftHandle;
  if (usb_init_connect(usb)) return -1;
  if (!was_open) g_usleep(100000); 
  dBuf[0]=CTRL_RD|addr;
  if ((ftStatus=FT_Write(usb->ftHandle,dBuf,1,&n_bytes))!=FT_OK) return -1;
  if (n_bytes!=1) return 1;
  if ((ftStatus=FT_Read (usb->ftHandle,dBuf,1,&n_bytes))!=FT_OK) return -2;
  if (n_bytes!=1) return 2;
  *dat=dBuf[0];
//  if (!was_open) usb_close_connect(usb);
  return 0;
}

int usb_read_regs(USB_ITEMS *usb,int addr,int n,unsigned char *dat)
{
  int i;
  int dat1;
  *dat=0;
  for (i=0; i<n; i++)
  {
    if ((usb_read_reg(usb,addr+i,&dat1))) return -1;
    dat[i]=dat1;
  }
  return 0;
}

void usb_write(USB_ITEMS *usb,int addr,int dat)
{
//  int was_open=(int)usb->ftHandle;
  if (usb_init_connect(usb)) return;
  usb_reset_regmap(usb);
  addr&=0x3f;
  usb_write_reg(usb, addr, dat);

//  if (!was_open) usb_close_connect(usb);
}

// flush 
int usb_flush_tx(USB_ITEMS *usb)
{
  FT_STATUS ftStatus;
  g_usleep(100000); // wait 100ms (10ms too fast)
  ftStatus=FT_Purge (usb->ftHandle, FT_PURGE_RX | FT_PURGE_TX);
  return 0;
}

//FT_SetTimeouts(ftHandle,readtimeout,writetimeout); 
void ena_dat(USB_ITEMS *usb)
{
  usb_write_reg(usb,A_DAT,(usb->fast_ftd? 0x2 : 0x0));
}

void dis_dat(USB_ITEMS *usb)
{
  usb_write_reg(usb,A_DAT,(usb->fast_ftd? 0x3 : 0x1));
}

int usb_read(USB_ITEMS *usb,int addr)
{
  int dat;
//  int was_open=(int)usb->ftHandle;
  if (usb_init_connect(usb)) return FALSE;
  usb_reset_regmap(usb);
  addr&=0x3f;
  // stop data and flush
  dis_dat(usb);
  usb_flush_tx(usb);
  if ((usb_read_reg(usb,addr,&dat))) return -1;
//  ena_dat(usb);
//  if (!was_open) usb_close_connect(usb);
  return dat;
}


int get_firmware(USB_ITEMS *usb,char *fw)
{
//  int was_open=(int)usb->ftHandle;
  if (usb_init_connect(usb)) return -1;
  usb_reset_regmap(usb);
  // stop data and flush
  dis_dat(usb);
  usb_flush_tx(usb);
  if (usb_read_regs(usb,A_VER,4,(unsigned char *)fw)) return -2;
//  if (rdi->recording) ena_dat(usb);
//  if (!was_open) usb_close_connect(usb);
  return 0;
}

void set_sat(USB_ITEMS *usb,int dat)
{
  usb_write(usb,A_CHN,dat); 
}

int get_sat(USB_ITEMS *usb)
{
  dis_dat(usb);
  usb_flush_tx(usb);
  return usb_read(usb,A_STA)&0x07;
}

#if __GTK_WIN32__ == 1
// for some reason linking these basic FT funcs used in another file 
// doesn't work with Windows. Via ft_... it works...???
FT_STATUS ft_Read(FT_HANDLE ftHandle,unsigned char *dBuf,int n,DWORD *n_bytes)
{
  return FT_Read(ftHandle,dBuf,n,n_bytes);
}

FT_STATUS ft_GetDeviceInfoList(FT_DEVICE_LIST_INFO_NODE *ft_dlin,DWORD *numDevs)
{
  return FT_GetDeviceInfoList(ft_dlin,numDevs);
}

FT_STATUS ft_CreateDeviceInfoList(DWORD *numDevs)
{
  return FT_CreateDeviceInfoList(numDevs);
}
#endif

