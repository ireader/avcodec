ROOT:=../../sdk
CONTRIB:=../../3rd
MEDIASERVER:=../../media-server

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 1
OUTFILE = libffutils.so

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . \
			./include \
			../avcodec/include \
			../avbsf/include \
			../h264/include \
			../h265/include \
			$(CONTRIB)/ffmpeg/include

#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))

#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS = $(CONTRIB)/ffmpeg/linux64
ifdef RELEASE
# relase library path
LIBPATHS += 
else
LIBPATHS +=
endif

LIBS = avcodec avformat avutil swscale

STATIC_LIBS = ../avbsf/$(BUILD).$(PLATFORM)/libavbsf.a \
				../avcodec/$(BUILD).$(PLATFORM)/libavcodec.a \
				$(MEDIASERVER)/libflv/$(BUILD).$(PLATFORM)/libflv.a \
				../h264/$(BUILD).$(PLATFORM)/libh264.a \
				../h265/$(BUILD).$(PLATFORM)/libh265.a

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES =

include $(ROOT)/gcc.mk

GCC_VER_GTE44 := $(shell echo `gcc -dumpversion | cut -f1-2 -d.` \< 4.4 | bc )
ifeq ($(GCC_VER_GTE44),1)
CFLAGS += -march=i586
endif

CFLAGS += -fvisibility=default
LDFLAGS += -Wl,--version-script=ffutils.map
