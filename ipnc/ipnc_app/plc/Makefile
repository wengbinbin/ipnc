# Makefile
#

all: install

ROOTDIR = ../..
include $(ROOTDIR)/Rules.make

TARGET = plc

INSTALLDIR = $(PLC_DIR)

C_FLAGS += -Wall -g -O3

CPP_FLAGS += -I$(PUBLIC_INCLUDE_DIR) \
		-D_GNU_SOURCE \
		-D_LARGEFILE64_SOURCE \
		-D_FILE_OFFSET_BITS=64 \
                $(XDC_FLAGS)

LD_FLAGS += -lfreetype -lpthread

COMPILE.c = $(MVTOOL_PREFIX)gcc $(C_FLAGS) $(CPP_FLAGS)-c
LINK.c = $(MVTOOL_PREFIX)gcc $(LD_FLAGS)

HEADERS = $(wildcard *.h) $(PUBLIC_INCLUDE_DIR)/*.h

install: plcTester plcServer

plcTester: plcTester.o plcI.o
	$(LINK.c) -o $@ $^
	install $@ $(INSTALLDIR)
	install $@ $(DVR_TARGET_DIR)/filesys/opt/plc	

plcTester.o: plcTester.c
	$(COMPILE.c) -o $@ $^

plcI.o: plcI.c
	$(COMPILE.c) -o $@ $^

plcServer: main.o plcServer.o
	$(LINK.c) -o $@ $^ 
	install $@ $(INSTALLDIR)
	install $@ $(DVR_TARGET_DIR)/filesys/opt/plc	

plcServer.o: plcServer.c $(HEADERS)
	$(COMPILE.c) -o $@ $<

main.o: main.c $(HEADERS)
	$(COMPILE.c) -o $@ $<

