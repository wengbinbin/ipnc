# (c) Texas Instruments 

include $(BASE_DIR)/COMMON_HEADER.MK
	
libs:
	make -fMAKEFILE.MK -C./csl $(MAKE_TARGET)
	make -fMAKEFILE.MK -C./drv $(MAKE_TARGET)	
	make -fMAKEFILE.MK -C./osa/src MODULE=osa $(MAKE_TARGET)
	make -fMAKEFILE.MK -C./alg/src MODULE=alg $(MAKE_TARGET)	
	make -fMAKEFILE.MK -C./image_tune/src MODULE=image_tune $(MAKE_TARGET)			
ifeq ($(AEWB_ID), AEWB_ENABLE)
	make -fMAKEFILE.MK -C./alg/src/aewb_ap MODULE=alg $(MAKE_TARGET)	
	make -fMAKEFILE.MK -C./alg/src/aewb_ti MODULE=alg $(MAKE_TARGET)	
else	
	make -fMAKEFILE.MK -C./alg/src/aewb_ap MODULE=alg $(MAKE_TARGET)
	make -fMAKEFILE.MK -C./alg/src/aewb_ti MODULE=alg $(MAKE_TARGET)
endif		

ifneq ($(SYSTEM), IPNC)			
ifneq ($(SYSTEM), EVM)		
	make -fMAKEFILE.MK -C./mcvip/src MODULE=mcvip $(MAKE_TARGET)							
endif	
endif	
				
all:
	make -fMAKEFILE.MK depend
	make -fMAKEFILE.MK clean
	make -fMAKEFILE.MK libs
	
clean:
	make -fMAKEFILE.MK libs MAKE_TARGET=clean
	rm -R -f alg/src/alg_config
	
depend:
	make -fMAKEFILE.MK libs MAKE_TARGET=depend  

install:
	make -fMAKEFILE.MK libs MAKE_TARGET=install  
	
.PHONY : install clean depend all libs 
	
