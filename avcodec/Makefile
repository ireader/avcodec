ROOT:=../../sdk
CONTRIB:=../../3rd

#--------------------------------Output------------------------------
# OUTTYPE: 0-exe, 1-dll, 2-static
#--------------------------------------------------------------------
OUTTYPE = 2
OUTFILE = libavcodec.a

#-------------------------------Include------------------------------
#
# INCLUDES = $(addprefix -I,$(INCLUDES)) # add -I prefix
#--------------------------------------------------------------------
INCLUDES = . \
					./include \
					$(ROOT)/include \
					$(CONTRIB) \
					$(CONTRIB)/opus \
					$(CONTRIB)/faac \
					$(CONTRIB)/x264 \
					$(CONTRIB)/openh264 \
					$(CONTRIB)/freetype/include \
					$(CONTRIB)/twolame

#-------------------------------Source-------------------------------
#
#--------------------------------------------------------------------
SOURCE_PATHS = source
SOURCE_FILES = $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.cpp))
SOURCE_FILES += $(foreach dir,$(SOURCE_PATHS),$(wildcard $(dir)/*.c))

_SOURCE_FILES = source/text-render-gdi.c

ifndef X264_ENABLE
_SOURCE_FILES += source/x262enc.c
_SOURCE_FILES += source/x264enc.c
endif

ifndef OPENH264_ENABLE
_SOURCE_FILES += source/openh264enc.cpp
endif

ifndef FAAC_ENABLE
_SOURCE_FILES += source/faac-decoder.c
_SOURCE_FILES += source/faac-encoder.c
endif

ifndef OPUS_ENABLE
_SOURCE_FILES += source/opus-decoder.c
_SOURCE_FILES += source/opus-encoder.c
endif

ifndef MP2_ENABLE
_SOURCE_FILES += source/mp2lame-encoder.c
endif

ifndef FREETYPE_ENABLE
_SOURCE_FILES += source/text-render-freetype.c
endif

SOURCE_FILES := $(filter-out $(_SOURCE_FILES),$(SOURCE_FILES))

#-----------------------------Library--------------------------------
#
# LIBPATHS = $(addprefix -L,$(LIBPATHS)) # add -L prefix
#--------------------------------------------------------------------
LIBPATHS =
ifdef RELEASE
# relase library path
LIBPATHS += 
else
LIBPATHS +=
endif

LIBS =

STATIC_LIBS =

ifdef X264_ENABLE
STATIC_LIBS += $(CONTRIB)/x264/libx264.a
endif

ifdef OPENH264_ENABLE
STATIC_LIBS += $(CONTRIB)/openh264/libopenh264.a
endif

ifdef FAAC_ENABLE
STATIC_LIBS += $(CONTRIB)/faac/libfaac.a
STATIC_LIBS += $(CONTRIB)/faac/libfaad2.a
endif

ifdef OPUS_ENABLE
STATIC_LIBS += $(CONTRIB)/opus/libopus.a
endif

ifdef MP2_ENABLE
STATIC_LIBS += $(CONTRIB)/twolame/libtwolame.a
endif

ifdef FREETYPE_ENABLE
LIBS += freetype
endif

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
