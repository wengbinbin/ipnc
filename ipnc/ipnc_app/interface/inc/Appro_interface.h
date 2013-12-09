/* ===========================================================================
* @file Appro_interface.h
*
* @path $(IPNCPATH)/include/
*
* @desc Appro_interface common include file
* .
* Copyright (c) Appro Photoelectron Inc.  2008
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied
*
* =========================================================================== */
/**
  \file Appro_interface.h

  \brief Appro_interface common include file
*/

#ifndef _APPRO_INTERFACE_
#define _APPRO_INTERFACE_

#if defined (__cplusplus)
extern "C" {
#endif
#include <Msg_Def.h>

#if DEBUG==1
#define dbg(x...)		fprintf(stderr,x)
#else
#define dbg(x...)
#endif

/**
  \defgroup AV_INTERFACE Appro interface datatypes, data structures, functions

  The section defines some common datatypes, data structures, callbacks which are used by all parts of the Appro interface framework.
  System init APIs are also defined in this section.

  \section AV_COMMON_DIR_STRUCT Directory Structure

  \subsection AV_COMMON_INTERFACE Interface

  \code
  Appro_interface.h
  \endcode

  \subsection AV_COMMON_SOURCE Source

  \code
  Appro_interface.c
  \endcode


  @{
*/


/* return value */
#define RET_SUCCESS 		0		/**< error code : no error */
#define RET_NO_VALID_DATA   -1		/**< error code : request data not ready */
#define RET_NEW_MJPEG       -2		/**< error code : new jpeg(not use now) */
#define RET_INVALID_PRM     -100	/**< error code : invalid parameter */
#define RET_OVERWRTE        -102	/**< error code : request data loss */
#define RET_NO_MEM          -103	/**< error code : not enough memory for locking frame */
#define RET_ERROR_OP		-104	/**< error code : error operation */
#define RET_INVALID_COMMAND -200	/**< error code : invalid command */
#define RET_UNKNOWN_ERROR	-400	/**< error code : unknow error */



/* definition for field */
#define AV_OP_LOCK_MJPEG 				1 /**< Lock one MJPEG*/
#define AV_OP_BOOK_MJPEG 				2 /**< BOOK one MJPEG (not support now)*/
#define AV_OP_UNLOCK_MJPEG 				3 /**< Unlock one MJPEG */
#define AV_OP_LOCK_ULAW 				4 /**< Lock a period of sound */
#define AV_OP_UNLOCK_ULAW 				5 /**< Unlock a period of sound */
#define AV_OP_LOCK_MP4_VOL 				6 /**< Lock VOL of MPEG4 */
#define AV_OP_LOCK_MP4_1_VOL 			39 /**< Lock VOL of MPEG4 */
#define AV_OP_LOCK_MP4_2_VOL 			40 /**< Lock VOL of MPEG4 */
#define AV_OP_LOCK_MP4_3_VOL 			41 /**< Lock VOL of MPEG4 */



#define AV_OP_UNLOCK_MP4_VOL 			7 /**< Unlock VOL of MPEG4 */
#define AV_OP_UNLOCK_MP4_1_VOL 			45 /**< Unlock VOL of MPEG4 */
#define AV_OP_UNLOCK_MP4_2_VOL 			46 /**< Unlock VOL of MPEG4 */
#define AV_OP_UNLOCK_MP4_3_VOL 			47 /**< Unlock VOL of MPEG4 */



#define AV_OP_LOCK_MP4 					8 /**< Lock one MPEG4 frame */
#define AV_OP_LOCK_MP4_1 				33 /**< Lock one MPEG4 frame */
#define AV_OP_LOCK_MP4_2 				34 /**< Lock one MPEG4 frame */
#define AV_OP_LOCK_MP4_3 				35 /**< Lock one MPEG4 frame */



#define AV_OP_LOCK_MP4_IFRAME 			9 /**< Lock one MPEG4 I frame*/
#define AV_OP_LOCK_MP4_1_IFRAME 			42 /**< Lock one MPEG4 I frame*/
#define AV_OP_LOCK_MP4_2_IFRAME 			43 /**< Lock one MPEG4 I frame*/
#define AV_OP_LOCK_MP4_3_IFRAME 			44 /**< Lock one MPEG4 I frame*/



#define AV_OP_UNLOCK_MP4 				10 /**< Unlock one MPEG4 frame */
#define AV_OP_UNLOCK_MP4_1 				36 /**< Unlock one MPEG4 frame */
#define AV_OP_UNLOCK_MP4_2 				37 /**< Unlock one MPEG4 frame */
#define AV_OP_UNLOCK_MP4_3 				38 /**< Unlock one MPEG4 frame */



#define AV_OP_GET_MJPEG_SERIAL 			11 /**< Get latest MJPEG serial number */
#define AV_OP_GET_MPEG4_SERIAL 			12 /**< Get latest MPEG4 serial number */
#define AV_OP_GET_MPEG4_1_SERIAL 			24 /**< Get latest MPEG4 serial number */
#define AV_OP_GET_MPEG4_2_SERIAL 			25 /**< Get latest MPEG4 serial number */
#define AV_OP_GET_MPEG4_3_SERIAL 			26 /**< Get latest MPEG4 serial number */



#define AV_OP_GET_ULAW_SERIAL 			13 /**< Get latest AUDIO serial number */
#define AV_OP_WAIT_NEW_MJPEG_SERIAL 	14 /**< Get new MJPEG serial number with wait (do not use now)*/
#define AV_OP_WAIT_NEW_MPEG4_SERIAL 	15 /**< Get new MPEG4 serial number with wait (do not use now)*/
#define AV_OP_WAIT_NEW_MPEG4_1_SERIAL 	27 /**< Get new MPEG4 serial number with wait (do not use now)*/
#define AV_OP_WAIT_NEW_MPEG4_2_SERIAL 	28 /**< Get new MPEG4 serial number with wait (do not use now)*/
#define AV_OP_WAIT_NEW_MPEG4_3_SERIAL 	29 /**< Get new MPEG4 serial number with wait (do not use now)*/




#define AV_OP_LOCK_MP4_CIF_VOL 				16 /**< Lock VOL of MPEG4_2 */
#define AV_OP_LOCK_MP4_1_CIF_VOL 				48 /**< Lock VOL of MPEG4_2 */
#define AV_OP_LOCK_MP4_2_CIF_VOL 				49 /**< Lock VOL of MPEG4_2 */
#define AV_OP_LOCK_MP4_3_CIF_VOL 				50 /**< Lock VOL of MPEG4_2 */



#define AV_OP_UNLOCK_MP4_CIF_VOL 			17 /**< Unlock VOL of MPEG4_2 */
#define AV_OP_UNLOCK_MP4_1_CIF_VOL 			51 /**< Unlock VOL of MPEG4_2 */
#define AV_OP_UNLOCK_MP4_2_CIF_VOL 			52 /**< Unlock VOL of MPEG4_2 */
#define AV_OP_UNLOCK_MP4_3_CIF_VOL 			53 /**< Unlock VOL of MPEG4_2 */



#define AV_OP_LOCK_MP4_CIF 					18 /**< Lock one MPEG4_2 frame */
#define AV_OP_LOCK_MP4_1_CIF 					54 /**< Lock one MPEG4_2 frame */
#define AV_OP_LOCK_MP4_2_CIF 					55 /**< Lock one MPEG4_2 frame */
#define AV_OP_LOCK_MP4_3_CIF 					56 /**< Lock one MPEG4_2 frame */



#define AV_OP_LOCK_MP4_CIF_IFRAME 			19 /**< Lock one MPEG4_2 I frame*/
#define AV_OP_LOCK_MP4_1_CIF_IFRAME 			57 /**< Lock one MPEG4_2 I frame*/
#define AV_OP_LOCK_MP4_2_CIF_IFRAME 			58 /**< Lock one MPEG4_2 I frame*/
#define AV_OP_LOCK_MP4_3_CIF_IFRAME 			59 /**< Lock one MPEG4_2 I frame*/



#define AV_OP_UNLOCK_MP4_CIF 				20 /**< Unlock one MPEG4_2 frame */
#define AV_OP_UNLOCK_MP4_1_CIF 				60 /**< Unlock one MPEG4_2 frame */
#define AV_OP_UNLOCK_MP4_2_CIF 				61 /**< Unlock one MPEG4_2 frame */
#define AV_OP_UNLOCK_MP4_3_CIF 				62 /**< Unlock one MPEG4_2 frame */



#define AV_OP_GET_MPEG4_CIF_SERIAL 			21 /**< Get latest MPEG4_2 serial number */
#define AV_OP_GET_MPEG4_1_CIF_SERIAL 			63 /**< Get latest MPEG4_2 serial number */
#define AV_OP_GET_MPEG4_2_CIF_SERIAL 			64 /**< Get latest MPEG4_2 serial number */
#define AV_OP_GET_MPEG4_3_CIF_SERIAL 			65 /**< Get latest MPEG4_2 serial number */



#define AV_OP_WAIT_NEW_MPEG4_CIF_SERIAL 	22 /**< Get new MPEG4_2 serial number with wait (do not use now)*/
#define AV_OP_WAIT_NEW_MPEG4_1_CIF_SERIAL 	30 /**< Get new MPEG4_2 serial number with wait (do not use now)*/
#define AV_OP_WAIT_NEW_MPEG4_2_CIF_SERIAL 	31 /**< Get new MPEG4_2 serial number with wait (do not use now)*/
#define AV_OP_WAIT_NEW_MPEG4_3_CIF_SERIAL 	32 /**< Get new MPEG4_2 serial number with wait (do not use now)*/



#define AV_OP_WAIT_NEW_ULAW_SERIAL			23 /**< Get new AUDIO serial number with wait (do not use now)*/


/* definition for AV_DATA.flags */
#define AV_FLAGS_MP4_I_FRAME 			0x00001 /**< First frame of GOP */
#define AV_FLAGS_MP4_LAST_FRAME 		0x00002 /**< Last frame of GOP */
/* definition for AV_DATA.frameType */
#define AV_FRAME_TYPE_I_FRAME	0x1	/**< frame type is I frame */
#define AV_FRAME_TYPE_P_FRAME	0x2	/**< frame type is P frame */

/**
 * @brief frame information data structure for GetAVData()
 */
typedef struct _av_data
{
	unsigned int serial;	/**< frame serial number */
	unsigned int size;		/**< frame size */
	unsigned int width;		/**< frame width */
	unsigned int height;	/**< frame height */
	unsigned int quality;	/**< frame quality */
	unsigned int flags;		/**< frame serial number */
	unsigned int frameType;	/**< either AV_FRAME_TYPE_I_FRAME  or AV_FRAME_TYPE_P_FRAME */
	unsigned int timestamp;	/**< get frame time stamp */
	int			 ref_serial[FMT_MAX_NUM];	/**<  all frame serial record for reference */
	unsigned char * ptr;	/**<  pointer for data ouput */
} AV_DATA;

/* interface functions */
int func_get_mem( void *pdata );
int GetAVData( unsigned int field, int serial, AV_DATA * ptr );
int ApproInterfaceExit();

/* @} */

#if defined (__cplusplus)
}
#endif



#endif /*_APPRO_INTERFACE_*/


