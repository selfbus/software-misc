#HX8347A: this is the old LCD controller sold until b/o 2011
#DEVID = 0x32024001
#new LCD controller sold after b/o 2011
DEVID = 0x32024002
#LCD controller of 2.4" module
#DEVID = 0x32024003
SWVERSIONMAJOR = 1
SWVERSIONMINOR = 19
BOOTLDRSIZE = 0x2000
#FLASHSIZE - BOOTLDRSIZE - 8
BOOTLDRINFOSTART = 0x1DFF8

# Target file name (without extension).
BTARGET = $(PROJ)_$(SWVERSIONMAJOR).$(SWVERSIONMINOR)

PROJ   = EIB_LCD
include C:\ethernut-4.9\nutapp\Makedefs

SRCS =  $(PROJ).c tft_io.c ssd1963.c TPUart.c EIBLayers.c NandFlash.c ScreenCtrl.c Sound.c System.c \
					picture.c page.c e_picture.c e_jumper.c e_button.c addr_tab.c e_led.c e_value.c e_sbutton.c listen.c cyclic.c \
					o_backlight.c o_led.c rc5_io.c ir_button.c 1wire_io.c ds1820.c dht11.c o_button.c o_warning.c \
					EIBObjects.c FATSingleOpt/dos.c FATSingleOpt/dir.c FATSingleOpt/fat.c FATSingleOpt/mmc_spi.c FATSingleOpt/find_x.c

OPT = s
OBJS =  $(SRCS:.c=.o)
LIBS =  $(LIBDIR)/nutinit.o $(MODS) -lnutpro -lnutos -lnutarch -lnutdev -lnutnet -lnutfs -lnutcrt -lnutgorp
# Floating point printf version (requires MATH_LIB = -lm below)
LIBS += -lprintf_flt -lm
LDFLAGS += -Wl,-u,vfprintf
TARG =  $(PROJ).hex
.SECONDARY : $(PROJ).elf

ifneq (, $(findstring h8300, $(ARCH)))
# Libraries specific for H8 port
LIBS += -lnutentry -lc -lgcc
endif

all: $(OBJS) $(TARG) $(ITARG) $(DTARG)

#include C:\Programming\AVR\ethernut-4.9_lcd\nutapp\Makerules
include C:\ethernut-4.9\nutapp\Makerules

HWDEF += -DDEVID=$(DEVID)
HWDEF += -DSWVERSIONMAJOR=$(SWVERSIONMAJOR)
HWDEF += -DSWVERSIONMINOR=$(SWVERSIONMINOR)
#switch to control debug message outputs
#HWDEF += -DLCD_DEBUG
#HWDEF += -DTOUCH_DEBUG
HWDEF += -DHW_DEBUG
#switch to support 5.0" rather than 4.2" displays
#HWDEF += -DTFT_480_800_CTRL_SSD1963
#invert X axis of touch area for modules sold after b/o 2013
HWDEF += -DTFT_480_800_CTRL_SSD1963_INVERT_TOUCH_X
#Switch to support NEW 4.3" modules with SD slot instead of OLD one without (swaps touch x-y & adjusts LCD init)
HWDEF += -DTFT_272_480_CTRL_SSD1963_NEW

LDFLAGS	+= -Wl,--section-start=.bootldrinfo=$(BOOTLDRINFOSTART)
#CFLAGS += -gdwarf-2
#CFLAGS += -g2

clean:
	-rm -f $(OBJS)
	-rm -f $(TARG)
	-rm -f $(PROJ).eep
	-rm -f $(PROJ).obj
	-rm -f $(PROJ).map
	-rm -f $(SRCS:.c=.lst)
	-rm -f $(SRCS:.c=.bak)
	-rm -f $(SRCS:.c=.i)
