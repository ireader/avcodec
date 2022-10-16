ARCHBITS ?=  # 32/64 bits

ifeq ($(shell uname -m), x86_64)
	ARCHBITS = 64
else ifeq ($(shell getconf LONG_BIT), 64)
	ARCHBITS = 64
else ifeq ($(shell arch), x86_64)
	ARCHBITS = 64
endif

ifdef PLATFORM
	CROSS:=$(PLATFORM)-
else 
	CROSS:=
	PLATFORM:=linux$(ARCHBITS)
endif

ifeq ($(RELEASE),1)
	BUILD:=release
else
	BUILD:=debug
endif

all:
	$(MAKE) -C h264
	$(MAKE) -C h265
	$(MAKE) -C avbsf
	$(MAKE) -C avcodec
	
clean:
	$(MAKE) -C h264 clean
	$(MAKE) -C h265 clean
	$(MAKE) -C avbsf clean
	$(MAKE) -C avcodec clean
	
.PHONY : test
test:
	$(MAKE) -C ../sdk
	$(MAKE) -C test
