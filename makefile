##############################################################
# makefile to create xtrack
# Needed:
#   gtk2.0      always (lib files)
#   sgtk        always (object files, see github.com/robalblas/sgtk)
#   jpeg812     if background pic. is jpeg (lib file, see github.com/robalblas/jpeg812
#               Or use pre-compiled so
#   ftd2xx      if DiSEqC command via GODIL (lib file from ftd)
#
# To re-compile with different settings, start with:
#   make clean
#
# Linus: set OS="linux"
# Windows: set OS="windows"; use mingw
##############################################################
RELEASE1=2018.1
RELEASE=\"$(RELEASE1)\"
#RELEASE=\"2018.1\"

#Choose linux or windows
OS="linux"
#OS="windows"

GTK_REL="2.0"

#Add USB interface for GODIL connection
ADD_USB  ="FALSE"
ADD_RS232="TRUE"
ADD_JPEG ="TRUE"
ADD_SUNMOON="TRUE"
#DBGFLAG="-g"
##################################################################
SUBDIR=..

ifneq ($(OS),"linux")
  ifneq ($(OS),"windows")
all:
	echo "Wrong choice $(OS)."

  endif
endif

ifeq ($(ADD_USB),"TRUE")
  USBLIBS=-L$(SUBDIR)/libs -lftd2xx 
else
  USBLIBS=
endif


#Compile command
CC=gcc -Wall -Wno-format-y2k $(DBGFLAG)

ifeq ($(OS),"linux")
########################## Defs for Linux ##########################
  PROGRAM=xtrack

  ifeq ($(GTK_REL),"2.0")
    GTKCONFIG_CFLAG=`pkg-config --cflags gtk+-2.0` -D__GTK_20__ 
    GTKCONFIG_LLIBS=`pkg-config --libs gtk+-2.0`
  else
    GTKCONFIG_CFLAG=`pkg-config --cflags gtk+`
    GTKCONFIG_LLIBS=`pkg-config --libs gtk+`
  endif
  LOC_JPEGSRC=$(SUBDIR)/jpeg812
######################################################################
else
########################## Defs for Windows ##########################
#needed: libs/ftd2xx.dll, jpeg812/libjpeg812.a
  PROGRAM=xtrack.exe
  ifeq ($(GTK_REL),"2.0")
    GTKCONFIG_CFLAG=-mms-bitfields -Ic:/tools/wingtk20/include/gtk-2.0 -Ic:/tools/wingtk20/lib/gtk-2.0/include -Ic:/tools/wingtk20/include/atk-1.0 -Ic:/tools/wingtk20/include/cairo -Ic:/tools/wingtk20/include/pango-1.0 -Ic:/tools/wingtk20/include/glib-2.0 -Ic:/tools/wingtk20/lib/glib-2.0/include -Ic:/tools/wingtk20/include/libpng12
    GTKCONFIG_LLIBS= -Lc:/tools/wingtk20/lib -lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lpangowin32-1.0 -lgdi32 -lpangocairo-1.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lintl -mwindows
    CFLAGS_EXTRA=-D__GTK_WIN32__=1 -D__GTK_20__ -D__WIN32__=1
  else
    GTKCONFIG_CFLAG=-Ic:/bin/wingnu/gtk/include/gppm3 -Ic:/bin/wingnu/gtk/include/gtkp -Ic:/bin/wingnu/gtk/include/gtkp/gdk
    GTKCONFIG_LLIBS=-Lc:/bin/wingnu/gtk/lib -lgtk-1.3 -lgdk-1.3 -lglib-1.3  -mwindows
    CFLAGS_EXTRA=-D__GTK_WIN32__=1
  endif
  
  LOC_JPEGSRC=$(SUBDIR)/jpeg812
######################################################################
endif

ifeq ($(ADD_USB),"TRUE")
  CFLAGS_EXTRA+=-D__ADD_USB__=1
endif

ifeq ($(ADD_RS232),"TRUE")
  CFLAGS_EXTRA+=-D__ADD_RS232__=1
endif

ifeq ($(ADD_SUNMOON),"TRUE")
  CFLAGS_EXTRA+=-D__ADD_SUNMOON__=1
endif

ifeq ($(ADD_JPEG),"TRUE")
  CFLAGS_EXTRA+=-D__ADD_JPEG__=1
  DLIB_JPEG=-L$(LOC_JPEGSRC) -ljpeg812
#else
#  DLIB_JPEG=
endif


#Source locations
LOC_SGTK=$(SUBDIR)/sgtk
LOC_INCS=-I$(LOC_SGTK) -I$(LOC_JPEGSRC)

CCFLAGS=$(LOC_INCS) $(GTKCONFIG_CFLAG) $(CFLAGS_EXTRA)

#Main files
SRC_XMAIN=xtrack.c callbacks.c dbase.c dbase_prims.c predict.c predict_xtrack.c drawpos.c calc_satpos.c \
          calc_sunmoon.c parse_norad.c write_ps.c draw.c misc.c subwnds.c \
          read_norad.c prefer.c gif_read.c rs232.c use_pos.c  jpeg.c \
          debug_wnd.c sel_refpos.c satmap.c
OBJ_XMAIN = $(subst .c,.o,$(SRC_XMAIN))

INC_JPEG=$(LOC_JPEGSRC)/jpeglib.h $(LOC_JPEGSRC)/cdjpeg.h $(LOC_JPEGSRC)/jinclude.h \
         $(LOC_JPEGSRC)/jmorecfg.h $(LOC_JPEGSRC)/jerror.h $(LOC_JPEGSRC)/jconfig.h $(LOC_JPEGSRC)/cderror.h 

INC_XMAIN=xtrack.h defines.h orbit.h gif.h rs232.h \
          xtrack_func.h  xtrack_basefunc.h xtrack_gtkfuncs.h \
          sattrack.h sattrack_funcs.h

#USB-stuff if needed
ifeq ($(ADD_USB),"TRUE")
  SRC_USB=godil_regmap.c pos2godil.c
  OBJ_USB = $(subst .c,.o,$(SRC_USB))
  ifeq ($(OS),"windows")
    INC_USB=ftd2xx_win.h godil_regmap.h
  else
    INC_USB=ftd2xx.h WinTypes.h godil_regmap.h
  endif
endif

#GTK sgtk files
SRC_SGTK=$(LOC_SGTK)/sgtk_misc.c \
         $(LOC_SGTK)/windows.c \
         $(LOC_SGTK)/filemngr.c \
         $(LOC_SGTK)/canvas.c \
         $(LOC_SGTK)/buttons.c \
         $(LOC_SGTK)/listtree.c \
         $(LOC_SGTK)/adjust.c \
         $(LOC_SGTK)/menus.c \
         $(LOC_SGTK)/packing.c \
         $(LOC_SGTK)/rgbdraw.c \
         $(LOC_SGTK)/gtk_led.c 

OBJ_SGTK = $(subst .c,.o,$(SRC_SGTK))
INC_SGTK=$(LOC_SGTK)/sgtk.h $(LOC_SGTK)/gtk_led.h $(LOC_SGTK)/sgtk_functions.h

SRC_ALL=$(SRC_XMAIN) $(SRC_SGTK)  $(SRC_USB)
OBJ_ALL= $(SRC_ALL:.c=.o)
INC_ALL=$(INC_XMAIN) $(INC_SGTK) $(INC_JPEG) $(INC_USB)

$(PROGRAM): $(OBJ_ALL) 
	$(CC) -o $@ $(OBJ_ALL) -lm $(GTKCONFIG_CFLAG) $(GTKCONFIG_LLIBS) $(DLIB_JPEG) $(USBLIBS)

%.o: %.c $(INC_ALL) 
	$(CC) -c $(CCFLAGS) -o $@ $<

xtrack.o: xtrack.c makefile $(INC_ALL)
	$(CC) -c xtrack.c $(CCFLAGS)  -DRELEASE=$(RELEASE) 


web: xtrack_$(RELEASE1).tgz
xtrack_$(RELEASE1).tgz: xtrack
	tar -czf xtrack_$(RELEASE1).tgz xtrack

arch: xtrack_src.zip
xtrack_src.zip: $(SRC_ALL) $(INC_ALL)  xtrack_func.h makefile
	zip xtrack_src.zip $(SRC_ALL) $(INC_ALL) xtrack_func.h makefile

git: xtrack_git_src.zip
xtrack_git_src.zip: $(SRC_XMAIN) $(SRC_USB) $(INC_XMAIN) $(INC_USB) makefile
	zip xtrack_git_src.zip $(SRC_XMAIN) $(SRC_USB) $(INC_XMAIN) $(INC_USB) makefile

icon: xtrack_icon.exe
xtrack_icon.exe: xtrack.exe
	mv xtrack.exe xtrack_noicon.exe
	ResHacker -add xtrack_noicon.exe,xtrack.exe,wbol.ico,ICONGROUP,ikoon,

clean:
	rm $(OBJ_ALL) 2>/dev/null
