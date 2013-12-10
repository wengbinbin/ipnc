/*
 *  Copyright 2008
 *  Texas Instruments Incorporated
 *
 *  All rights reserved.  Property of Texas Instruments Incorporated
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */

/**
 *  @file       ti/xdais/dm/ivs1.h
 *
 *  @brief      This header defines all types, constants, and functions
 *              shared by all implementations of the video stabilization
 *              algorithms.
 */
/**
 *  @defgroup   ti_xdais_dm_IVS1   xDM 1.00 Video Stabilization Interface
 *
 *  This is the xDM 1.00 Video Stabilization Interface.
 */

#ifndef ti_xdais_dm_IVS1_
#define ti_xdais_dm_IVS1_

#include <ti/xdais/ialg.h>
#include <ti/xdais/xdas.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup    ti_xdais_dm_IVS1 */
/*@{*/

#define IVS1_EOK            IALG_EOK
#define IVS1_EFAIL          IALG_EFAIL
#define IVS1_EUNSUPPORTED   IALG_EFAIL

/**
 *  @brief      Defines the creation time parameters for
 *              all IVS1 instance objects.
 *
 *  @extensibleStruct
 */
typedef struct IVS1_Params {
    XDAS_Int32  size;             /**< @sizeField */

    XDAS_UInt32 bscInWidth;       /**< Width of the frame input to BSC module */
    XDAS_UInt32 bscInHeight;      /**< Height of the frame input to BSC module */

    XDAS_UInt32  row_hpos;
    XDAS_UInt32  row_vpos;
    XDAS_UInt32  row_hskip;
    XDAS_UInt32  row_vskip;
    XDAS_UInt32  row_hnum;
    XDAS_UInt32  row_vnum;
    XDAS_UInt32  row_vct;
    XDAS_UInt32  row_shf;

    // Column sum vector parameters
    XDAS_UInt32  col_hpos;
    XDAS_UInt32  col_vpos;
    XDAS_UInt32  col_hskip;
    XDAS_UInt32  col_vskip;
    XDAS_UInt32  col_hnum;
    XDAS_UInt32  col_vnum;
    XDAS_UInt32  col_vct;
    XDAS_UInt32  col_shf;

} IVS1_Params;

/**
 *  @brief      Defines the input arguments for all IVS1 instance
 *              process function.
 *
 *  @extensibleStruct
 *
 *  @sa         IVS1_Fxns::process()
 */
typedef struct IVS1_InArgs {
    XDAS_Int32  size;             /**< @sizeField */

    XDAS_UInt32 CurBscMemAddrH;    /**< Memory Address where the BSC data for the current frame is present */
    XDAS_UInt32 CurBscMemAddrV;    /**< Memory Address where the BSC data for the current frame is present */
    XDAS_UInt32 PrevBscMemAddrH;   /**< Memory Address where the BSC data for the previous frame is present */
    XDAS_UInt32 PrevBscMemAddrV;    /**< Memory Address where the BSC data for the current frame is present */

} IVS1_InArgs;

/**
 *  @brief      Defines the run time output arguments for all IVS1
 *              instance objects.
 *
 *  @extensibleStruct
 *
 *  @sa         IVS1_Fxns::process()
 */
typedef struct IVS1_OutArgs {
    XDAS_Int32  size;             /**< @sizeField */
    XDAS_Int32  extendedError;    /**< @extendedErrorField */

    XDAS_UInt32 startX;           /**< horizontal offset required to stabilize the video frame */
    XDAS_UInt32 startY;           /**< vertcal offset required to stabilize the video frame */

    XDAS_UInt32 winWidth;         /**< width of the stabilized video frame */
    XDAS_UInt32 winHeight;        /**< Height of the stabilized video frame */

} IVS1_OutArgs;

/*@}*/

#ifdef __cplusplus
}
#endif

#endif  /* ti_xdais_dm_IVS1_ */
/*
 *  @(#) ti.xdais.dm; 1, 0, 4,150; 11-25-2007 20:44:08; /db/wtree/library/trees/dais-i23x/src/
 */

