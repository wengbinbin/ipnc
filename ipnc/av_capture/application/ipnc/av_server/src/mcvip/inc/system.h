

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <alg.h>
#include <drv.h>

#ifdef BOARD_UD_DVR
#define TVP5158_A_I2C_ADDR    (0xB6)
#define TVP5158_B_I2C_ADDR    (0xBE)
#endif

#ifdef BOARD_TI_EVM
#define TVP5158_A_I2C_ADDR    (0xB0)
#define TVP5158_B_I2C_ADDR    (0xB2)
#endif

//#define CAPTURE_DEBUG
//#define ENCODE_DEBUG //add by sxh for debug
//#define WRITER_DEBUG
//#define DECODE_DEBUG

//#define CAPTURE_DEBUG_RUNNING
//#define ENCODE_DEBUG_RUNNING
//#define WRITER_DEBUG_RUNNING
//#define DISPLAY_DEBUG_RUNNING
//#define DECODE_DEBUG_RUNNING

int SYSTEM_init(int debug);
int SYSTEM_exit();

char SYSTEM_getInput();

int SYSTEM_profileInfoShow();

#endif 
