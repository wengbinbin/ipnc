# (c) Texas Instruments 

USERMOD = $(shell pwd)/usermod/src
KERMOD = $(shell pwd)/kermod/build

libs:
	make -C$(KERMOD)  $(MAKE_TARGET)
	make -fMAKEFILE.MK -C$(USERMOD) MODULE=drv $(MAKE_TARGET)
ifeq ($(IMGS_ID), IMGS_MICRON_MT9P031_5MP)	
	make -fMAKEFILE.MK -C$(USERMOD)/imgs_MT9P031_5MP MODULE=drv $(MAKE_TARGET)
endif
ifeq ($(IMGS_ID), IMGS_NONE)
	make -fMAKEFILE.MK -C$(USERMOD)/imgs_none MODULE=drv $(MAKE_TARGET)			
endif

all:
	make -fMAKEFILE.MK depend
	make -fMAKEFILE.MK clean
	make -fMAKEFILE.MK libs
	
clean:
	make -fMAKEFILE.MK libs MAKE_TARGET=clean
	
depend:
	make -fMAKEFILE.MK libs MAKE_TARGET=depend  

install:
	make -fMAKEFILE.MK libs MAKE_TARGET=install  
	
.PHONY : install clean depend all libs 
	
	
