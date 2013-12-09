
# Makefile
#
ROOTDIR = ../..
include $(ROOTDIR)/Rules.make

plc: plcserver.c
	arm_v5t_le-gcc -o plc plcserver.c -I$(PUBLIC_INCLUDE_DIR) -lpthread

