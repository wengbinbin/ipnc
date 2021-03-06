
include $(BASE_DIR)/COMMON_HEADER.MK
include $(BASE_DIR)/INCLUDES.MK

ifeq ($(SYSTEM), EVM)
IMAGE_TUNE_LIB := $(LIB_DIR)/image_tune.a
endif

ifeq ($(SYSTEM), DVR)
IMAGE_TUNE_LIB := $(LIB_DIR)/image_tune.a
endif

LIBS=$(LIB_DIR)/av_server.a $(LIB_DIR)/mcvip.a $(LIB_DIR)/alg.a $(LIB_DIR)/drv.a $(DM365MM_LIB) $(IMAGE_TUNE_LIB) $(LIB_DIR)/osa.a $(CMEM_LIB) $(LIB_DIR)/csl.a
LIBS+=$(CODEC_LIBS) 
#*********add by sxh
LIBS+=$(APP_LIB_DIR)/msg_util.a
LIBS+=$(APP_LIB_DIR)/alarm_msg_drv.a
#*********add by sxh

INCLUDE+=$(ALG_INC)
INCLUDE+=$(MCVIP_INC)
INCLUDE+=$(DRV_INC)
INCLUDE+=$(CSL_INC)

#*********add by sxh
INCLUDE+=$(AV_SERVER_INC)
INCLUDE+=$(IMAGE_TUNE_INC)
#*********add by sxh

include $(BASE_DIR)/COMMON_FOOTER.MK


