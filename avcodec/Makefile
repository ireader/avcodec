ROOT:=../../sdk

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 2
OUTFILE = libcodec.a

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . \
					./include \
					$(ROOT)/include \
					./ffmpeg/include \
					/usr/include/freetype2

#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))

_SOURCE_FILES = source/text-render-gdi.c
_SOURCE_FILES += source/faac-decoder.c
_SOURCE_FILES += source/faac-encoder.c
SOURCE_FILES := $(filter-out $(_SOURCE_FILES),$(SOURCE_FILES))

#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS = x264 openh264
ifdef RELEASE
# relase library path
LIBPATHS += 
else
LIBPATHS +=
endif

LIBS = freetype x264 openh264

STATIC_LIBS =

#-----------------------------DEFINES--------------------------------
#
# DEFINES := $(addprefix -D,$(DEFINES)) # add -L prefix
#--------------------------------------------------------------------
DEFINES = NO_CRYPTO

include $(ROOT)/gcc.mk

GCC_VER_GTE44 := $(shell echo `gcc -dumpversion | cut -f1-2 -d.` \< 4.4 | bc )
ifeq ($(GCC_VER_GTE44),1)
CFLAGS += -march=i586
endif
