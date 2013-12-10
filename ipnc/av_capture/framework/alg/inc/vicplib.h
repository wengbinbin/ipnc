/* ======================================================================== */
/*  TEXAS INSTRUMENTS, INC.                                                 */
/*                                                                          */
/*  VICP Signal Processing Library                                          */
/*                                                                          */
/*  This library contains proprietary intellectual property of Texas        */
/*  Instruments, Inc.  The library and its source code are protected by     */
/*  various copyrights, and portions may also be protected by patents or    */
/*  other legal protections.                                                */
/*                                                                          */
/*  This software is licensed for use with Texas Instruments TMS320         */
/*  family DSPs.  This license was provided to you prior to installing      */
/*  the software.  You may review this license by consulting the file       */
/*  TI_license.PDF which accompanies the files in this library.             */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*                                                                          */
/*     NAME                                                                 */
/*        vicplib.h -- Signal Processing Library Interface header file      */
/*                                                                          */
/*     DESCRIPTION                                                          */
/*        This file includes the definitions and the interfaces supported   */
/*        by the Library                                                    */
/*                                                                          */
/*     REV                                                                  */
/*        version 0.2.1:  4 Nov , 2009                                      */
/*        Updates to extend VICP Lib API support for DM365                  */
/*                                                                          */
/*        version 0.2.0:  19 Oct , 2009                                     */
/*        Added sad                                                         */
/*                                                                          */
/*        version 0.1.0:  12 June, 2009                                     */
/*        Added cfa interpolation                                           */
/*                                                                          */
/*        version 0.0.9:  29 May, 2009                                      */
/*        Added affine transform                                            */
/*                                                                          */
/*        version 0.0.8:  29 May, 2009                                      */
/*        Added affine transform                                            */
/*                                                                          */
/*        version 0.0.7:  10 April, 2009                                    */
/*        Added integral image, sobel, image pyramid                        */
/*                                                                          */
/*        version 0.0.6:  3rd March, 2009                                   */
/*        Added CPIS_updateSrcDstPtr                                        */
/*                                                                          */
/*        version 0.0.5:  5th Jan, 2009                                     */
/*        Added 2-D median filtering                                        */
/*                                                                          */ 
/*        version 0.0.4:  10th Dec                                          */
/*        Added recursive filtering                                         */
/*                                                                          */ 
/*        version 0.0.3:  19th Nov                                          */
/*        Updated CPIS_init to include staticDMAAlloc option                */
/*                                                                          */
/*        version 0.0.2:  27th Oct                                          */
/*        Adding kernels for release 2.0                                    */
/*                                                                          */
/*        version 0.0.1:  22nd Sep                                          */
/*        Initial version                                                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2008 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

#ifndef _CPISLIB_H
#define _CPISLIB_H

#ifdef __cplusplus
    extern "C" {
#endif

#include <tistdtypes.h>

#define CPIS_MAX_SRC_CHANNELS 8
#define CPIS_MAX_DST_CHANNELS 6

/* 
CPIS_MAX_CHANNELS does not have to be necessarily equal to 
CPIS_MAX_SRC_CHANNELS + CPIS_MAX_DST_CHANNELS
It represents the number of total channels allocatable
by the vicp library, including source an destination channels
hence you can have a combination of: 8 SRC channels + 2 DST channels
or 2 SRC channels + 8 DST channels
*/

#define CPIS_MAX_CHANNELS     14

/* Error symbols used by the library */
#define CPIS_INIT_ERROR                 1
#define CPIS_NOTINIT_ERROR              2
#define CPIS_UNPACK_ERROR               3
#define CPIS_NOSUPPORTFORMAT_ERROR      4
#define CPIS_NOSUPPORTDIM_ERROR         5
#define CPIS_PACK_ERROR                 6
#define CPIS_MAXNUMFUNCREACHED          7
#define CPIS_OUTOFMEM                   8
#define CPIS_NOSUPPORTANGLE_ERROR       9
#define CPIS_NOSUPPORTOP_ERROR          10
#define CPIS_NOT_ENOUGH_EDMACHAN_ERROR  11


/* 
    Maximum processing block size for different functions. 
    For a given <functionName>: 
    procBlockSize.width x procBlockSize.height < MAX_<functionName>_BLOCKSIZE
*/
#define PLAT_DIV 2

#define MAX_ALPHABLEND_GLOBAL_ALPHA_BLOCKSIZE   (2*1024/PLAT_DIV)
#define MAX_ALPHABLEND_BLOCKSIZE                (2*630/PLAT_DIV)
#define MAX_COLORSPCCONV_BLOCKSIZE              (744/PLAT_DIV)
#define MAX_ROTATION_BLOCKSIZE                  (2048/PLAT_DIV)   
#define MAX_FILLMEM_BLOCKSIZE                   (8188/PLAT_DIV)   
#define MAX_ARRAYOP_BLOCKSIZE                   (4096/PLAT_DIV)
#define MAX_ARRAYSCALAROP_BLOCKSIZE             (8192/PLAT_DIV)
#define MAX_ARRAYCONDWRITE_BLOCKSIZE            (2730/PLAT_DIV)
#define MAX_YCBCRPACK_BLOCKSIZE                 (1364/PLAT_DIV)
#define MAX_YCBCRUNPACK_BLOCKSIZE               (1364/PLAT_DIV)
#define MAX_MATMUL_BLOCKSIZE                    (4096/PLAT_DIV)
#define MAX_SUM_BLOCKSIZE                       (8188/PLAT_DIV)
#define MAX_SUMCFA_BLOCKSIZE                    (8176/PLAT_DIV)
#define MAX_LUT_BLOCKSIZE                       (8192/PLAT_DIV)
#define MAX_LUT_SIZE                            (32768/PLAT_DIV)
#define MAX_BLKAVERAGE_BLOCKSIZE                (8188/PLAT_DIV)
#define MAX_MEDIANFILTER_ROW_BLOCKSIZE          (8192/PLAT_DIV)
#define MAX_MEDIANFILTER_COL_BLOCKSIZE          (8192/PLAT_DIV)
#define MAX_FILTER_BLOCKSIZE                    (8192/PLAT_DIV)
#define MAX_RGBPACK_BLOCKSIZE                   (1638/PLAT_DIV)
#define MAX_RGBUNPACK_BLOCKSIZE                 (1638/PLAT_DIV)
#define MAX_MEDIAN2D_BLOCKSIZE                  (8192/PLAT_DIV)
#define MAX_SOBEL_BLOCKSIZE                     (4096/PLAT_DIV)
#define MAX_PYRAMID_BLOCKSIZE                   (8192/PLAT_DIV)
#define MAX_AFFINE_BLOCKSIZE                    (5000/PLAT_DIV)
#define MAX_CFA_BLOCKSIZE                       (2730/PLAT_DIV)
#define MAX_SAD_BLOCKSIZE                       (8192/PLAT_DIV)
#define MAX_SAD_TEMPLATESIZE                    (32768/PLAT_DIV)

extern Uint32 CPIS_errno;

/* 
    The below enums represent the various color space types that 
    are supported by the YCbCrPack routine
*/
typedef enum {
    CPIS_444_16BIT_TO_422_8BIT=0,
    CPIS_422_16BIT_TO_422_8BIT,
    CPIS_420_16BIT_TO_422_8BIT,
    CPIS_422V_16BIT_TO_422_8BIT,
    CPIS_444_16BIT_TO_444_8BIT,
    CPIS_422_16BIT_TO_444_8BIT,
    CPIS_420_16BIT_TO_444_8BIT,
    CPIS_422V_16BIT_TO_444_8BIT,

    CPIS_444_8BIT_TO_422_8BIT=0x8000,
    CPIS_422_8BIT_TO_422_8BIT,
    CPIS_420_8BIT_TO_422_8BIT,
    CPIS_422V_8BIT_TO_422_8BIT,
    CPIS_444_8BIT_TO_444_8BIT,
    CPIS_422_8BIT_TO_444_8BIT,
    CPIS_420_8BIT_TO_444_8BIT,
    CPIS_422V_8BIT_TO_444_8BIT

} CPIS_ColorSpacePack;

/* 
    The below enums represent the various color space types that 
    are supported by the YCbCrUnPack routine
*/
typedef enum {
    CPIS_422_TO_444_8BIT=0x8000,
    CPIS_422_TO_422_8BIT,
    CPIS_422_TO_420_8BIT,
    CPIS_444_TO_444_8BIT,
    CPIS_444_TO_422_8BIT,
    CPIS_444_TO_420_8BIT,

    CPIS_422_TO_444_16BIT=0x0000,
    CPIS_422_TO_422_16BIT,
    CPIS_422_TO_420_16BIT,
    CPIS_444_TO_444_16BIT,
    CPIS_444_TO_422_16BIT,
    CPIS_444_TO_420_16BIT

} CPIS_ColorSpaceUnpack;

typedef enum {
    CPIS_DS_NONE,
    CPIS_DS_SKIP,
    CPIS_DS_AVERAGE,
    CPIS_DS_FOR_ALPHABLEND
} CPIS_ColorDsMode;

/* The various arithmetic and logical operations supported by the library */
typedef enum {
    CPIS_OP_MPY=0, 
    CPIS_OP_ABDF,     
    CPIS_OP_ADD,     
    CPIS_OP_SUB,     
    CPIS_OP_TLU,     
    CPIS_OP_AND,     
    CPIS_OP_OR,      
    CPIS_OP_XOR,     
    CPIS_OP_MIN,     
    CPIS_OP_MAX, 
    CPIS_OP_MINSAD,     /* Only available on DM64x/DM64xx platform */  
    CPIS_OP_MAXSAD,     /* Only available on DM64x/DM64xx platform */
    CPIS_OP_MEDIAN,     /* Only available on DM64x/DM64xx platform */
    CPIS_OP_BINLOG,     /* Only available on DM64x/DM64xx platform */
    CPIS_OP_3DLUT,      /* Only available on DM64x/DM64xx platform */
    CPIS_OP_CONDWR,     /* Only available on DM64x/DM64xx platform */
    CPIS_OP_NOT         /* Only available on DM355/DM365 platform */  
} CPIS_Operation;


/* The conditions supprted by the array conditional write API */
typedef enum {
    CPIS_WR_ZERO=0, 
    CPIS_WR_NOTZERO,     
    CPIS_WR_SAT,     
    CPIS_WR_NOTSAT     
} CPIS_WriteMode;

/* Various data formats supported by the library */
typedef enum {
    CPIS_YUV_420P=0, /* Planar symbols must be listed first */
    CPIS_YUV_422P,
    CPIS_YUV_444P,
    CPIS_YUV_411P,
    CPIS_YUV_422VP,  /* Vertical subsampling */
    CPIS_RGB_P,
    CPIS_BAYER_P,
    CPIS_YUV_422IBE,
    CPIS_YUV_422ILE,
    CPIS_RGB_555,
    CPIS_RGB_565,
    CPIS_BAYER,
    CPIS_YUV_444IBE,
    CPIS_YUV_444ILE,
    CPIS_RGB_888,
    CPIS_YUV_GRAY,
    CPIS_8BIT,
    CPIS_16BIT,
    CPIS_32BIT,
    CPIS_64BIT,
    CPIS_U8BIT,
    CPIS_U16BIT,
    CPIS_U32BIT,
    CPIS_U64BIT,
    CPIS_1BIT,
    CPIS_YUV_420SP    
} CPIS_Format;

typedef enum {
    CPIS_TOP2BOTTOM,
    CPIS_BOTTOM2TOP,
    CPIS_LEFT2RIGHT,
    CPIS_RIGHT2LEFT
} CPIS_FilterDir;

typedef enum {
    CPIS_USE_BOUNDARY,
    CPIS_USE_PASSED_VALUES
} CPIS_FilterInitialMode;

#define CPIS_ALPHA 0x8000
#define CPIS_ALPHA_SEPARATE_PLANE 0x4000
#define CPIS_FOREGROUND_ALPHA_ARGB888_PLANE	0x2000
#define CPIS_BACKGROUND_ALPHA_ARGB888_PLANE	0x1000

/* Enum that controls the synchronous or async operation of the library APIs */
typedef enum {
    CPIS_SYNC,
    CPIS_ASYNC
} CPIS_ExecType;

/* Initialization structure for the library */
typedef void (*Cache_wbInv)(void *, Uint32, Bool);
typedef void (*VicpLock)(void*);
typedef void (*VicpUnlock)(void*);

#define CPIS_INIT_CE                1
#define CPIS_INIT_RMAN              (1<<1)
#define CPIS_INIT_VICP_IRES         (1<<2)
#define CPIS_INIT_EDMA3_IRES        (1<<3)
#define CPIS_INIT_ADDRSPACE_IRES    (1<<4)
#define CPIS_INIT_FC_ALL            (CPIS_INIT_CE | CPIS_INIT_RMAN | CPIS_INIT_VICP_IRES |CPIS_INIT_EDMA3_IRES | CPIS_INIT_ADDRSPACE_IRES)
#define CPIS_INIT_FC_NONE           0

typedef struct {
    Uint16 maxNumProcFunc;
    void *mem;
    Uint32 memSize;
    Cache_wbInv cacheWbInv;
    Uint16 staticDmaAlloc;
    Uint16 initFC;          /* Initialize framework components according to bitmask combination:
                                   CPIS_INIT_CE: initialize CE, need to set member engineName as well
                                   CPIS_INIT_RMAN: initialize RMAN
                                   CPIS_INIT_VICP_IRES: register VICP res manager 
                                   CPIS_INIT_EDMA3_IRES: register EDMA3 res manager
                                   CPIS_INIT_ADDRSPACE_IRES: register ADDRSPACE res manager
                                All these previous symbols can be ORed to form a bitmask.
                                In addition the symbol CPIS_INIT_FC_ALL can be used to initialize
                                all the Framework components.
                                CPIS_INIT_FC_NONE can be used to disable all FC initialization by
                                the VICP library.   
                             */
    char *engineName;        /* Codec engine name, must match name used in vicpLib365.cfg
                                normally, it is "alg_server"  
                                This member is irrelevant if initFC's bit #0 is set to 0   
                             */
    void *codecEngineHandle; /* Codec engine handle */
    VicpLock lock;
    void *lockArg;
    VicpUnlock unlock;
    void *unlockArg;
    Uint16 maxNumDma;       /* Maximum number of EDMA channels that vicplib can use.
                               This allow the application integrator to optimize
                               the EDMA channel usage with the rest of the system */
} CPIS_Init;


/* Structures used to convey information regarding the various blocks */
typedef struct {
    Uint32 width;   /* In number of pixels */
    Uint32 height;  /* In number of lines */
} CPIS_Size;

typedef struct {
    Uint8 *ptr;
    Uint32 stride;  /* In number of pixels */
} CPIS_Buffer;


/* Base paramters common to all APIs */

typedef struct {
    CPIS_Format srcFormat[CPIS_MAX_SRC_CHANNELS];
    CPIS_Buffer srcBuf[CPIS_MAX_SRC_CHANNELS];
    CPIS_Format dstFormat[CPIS_MAX_DST_CHANNELS];
    CPIS_Buffer dstBuf[CPIS_MAX_DST_CHANNELS];
    CPIS_Size roiSize;
    CPIS_Size procBlockSize;
    Uint16 numInput;    /* Number of input buffers */
    Uint16 numOutput;   /* Number of ouput buffers */
} CPIS_BaseParms;

typedef void* CPIS_Handle;

/* ColorSpaceConv API params */
typedef struct {
    Int16 matrix[9];
    Uint32 qShift;
    Int16 preOffset[3];     /* offset to add to each component before matrix multiplication */
    Int16 postOffset[3];    /* offset to add to each component after matrix multiplication */
    Int16 signedInput[3];
    Int16 signedOutput[3];
    CPIS_ColorDsMode colorDsMode;  /* color downsampling mode */
} CPIS_ColorSpcConvParms;

/* AlphaBlendig API params */
typedef struct {
    /* 
        if set to 1 then alpha value specified next is used for entire image
        otherwise if 0, use alpha plane passed as src[1]
    */
    Uint16 useGlobalAlpha; 
    Uint16 alphaValue;  /* global alpha value, 0-255, 255 let see foreground, 0 let see background */
    CPIS_Buffer background;
} CPIS_AlphaBlendParms;

/* Rotation API params */
typedef struct {
    Int16 angle;
} CPIS_RotationParms;

/* FillMem API params */
typedef struct {
    Uint8 * constData;
} CPIS_FillMemParms;

/* ArrayOperation API params */
typedef struct {
    Uint16 qShift;
    CPIS_Operation operation;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
} CPIS_ArrayOpParms;

/* ArrayScalarOperation API params */
typedef struct {
    Uint16 qShift;
    CPIS_Operation operation;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 mask[2][2];
} CPIS_ArrayScalarOpParms;

/* ArrayConditionalWrite API params */
typedef struct {
    Uint16 qShift;
    CPIS_WriteMode writeMode;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
} CPIS_ArrayCondWriteParms;

/* YCbCrPack API params */
typedef struct {
    Uint16 qShift;
    CPIS_ColorSpacePack colorSpace;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 scale;
} CPIS_YCbCrPackParms;

/* YCbCrUnPack API params */
typedef struct {
    Uint16 qShift;
    CPIS_ColorSpaceUnpack colorSpace;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 scale;
} CPIS_YCbCrUnpackParms;

/* MatrixMutiply API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 matWidth;
    Int16 matHeight;
    void * matPtr;
    CPIS_Format matFormat;
} CPIS_MatMulParms;

/* Sum API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 * scalarPtr;
    CPIS_Format scalarFormat;
} CPIS_SumParms;

/* Sum CFA API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 * scalarPtr;
    CPIS_Format scalarFormat;
} CPIS_SumCFAParms;

/* LUT API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 * lutPtr;
    CPIS_Format lutFormat;
    Int16 numLUT;
    Int16 LUTSize;
} CPIS_LUTParms;

/* Blk Average API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
} CPIS_BlkAverageParms;

/* Median Filter Row API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 median_size;
} CPIS_MedianFilterRowParms;

/* Median Filter Col API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 median_size;
} CPIS_MedianFilterColParms;

/* Filter API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 coeff_width;
    Int16 coeff_height;
    Int16 * coeffPtr;
    CPIS_Format coeffFormat;
} CPIS_FilterParms;

/* RGBPack API params */
typedef struct {
    Int8 reserved; /* Not used */
} CPIS_RGBPackParms;

/* RGBUnpack API params */
typedef struct {
    Int8 reserved; /* Not used */
} CPIS_RGBUnpackParms;

/* Vertical/Horizontal recursive filter params */
typedef struct {
    CPIS_FilterDir direction;
    CPIS_FilterInitialMode initialMode;
    CPIS_Buffer initialValues;
    Uint16 alpha;
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
} CPIS_RecursiveFilterParms;

/* Median 2D Filter */
typedef struct {
    Int16 filterWidth;
    Int16 filterHeight;
} CPIS_Median2DParms;

/* Integral image params */
typedef struct {
    Uint16 *scratch; /* scratch buffer of size 2*2*base->roiSize.width * base->roiSize.height */
    Uint16 stage;   /* Image integral stage: 1 or 2 */
} CPIS_IntegralImageParms;

/* Sobel API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
} CPIS_SobelParms;

/* Pyramid API params */
typedef struct {
    Uint16 gaussFilterSize; /* Can be 3, 5, 7 */
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
} CPIS_PyramidParms;


/* 
Affine transform params 

                          
 X      m0 m1     x   tx
 Y   =  m2 m3  *  y + ty
       
      
*/
#define CPIS_AFFINE_PRIVATE_VAR_SIZE 12

typedef struct {
    Uint8 *privateVars; /* Pointer to scratch of size 12 bytes */ 
    Uint8 *scratch; /* point to a region of size scratchSize bytes */
    Uint32 scratchSize; /* size of scratch in bytes, obtained by calling  _CPIS_affineTransformGetSize */
    Int16 skipOutside; /* skip processing of region outside of the ROI */
    Int16 m0;
    Int16 m1;
    Int16 m2;
    Int16 m3;
    Int16 tx;
    Int16 ty;
    Int16 m0inv;  /* Coefficient of the inverse matrix */
    Int16 m1inv;
    Int16 m2inv;
    Int16 m3inv;
    Int16 txinv;
    Int16 tyinv;
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;

} CPIS_AffineTransformParms;

typedef struct {
    Uint16 width;
    Uint16 height;
    Uint16 stride;
    Uint16 blockWidth;
    Uint16 blockHeight;
} CPIS_AffineTransformOutputROI;

/* CFA API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
} CPIS_CFAParms;

/* Sum-of-absolute differences API params */
typedef struct {
    Uint16 qShift;
    Int32 sat_high;
    Int32 sat_high_set;
    Int32 sat_low;
    Int32 sat_low_set;
    Int16 *templatePtr; /* Pointer to template */
    CPIS_Format templateFormat; /* format of each data element composing the template. Can be CPIS_U8BIT, CPIS_8BIT, CPIS_16BIT, CPIS_U16BIT .  */
    Int16 loadTemplate; /* Enable(1)/Disable(0) pre-loading of template into coefficient memory */
    Uint16 loadTemplateStride;
    CPIS_Size templateRoiSize; /* Size of the template's ROI */
    CPIS_Size templateSize;     /* Size of the template */
    Int16 templateStartOfst; /* Offset from location pointed by templatePtr to the first data point of the template */
} CPIS_SadParms;

/* APIs supported by the library */
Int32 CPIS_getMemSize(Uint16 maxNumProc);
Int32 CPIS_init(CPIS_Init *init);
Int32 CPIS_deInit();

Int32 CPIS_colorSpcConv(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_ColorSpcConvParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_alphaBlend(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_AlphaBlendParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_alphaBlendRGB(
 CPIS_Handle *handle,
 CPIS_BaseParms *base,
 CPIS_AlphaBlendParms *params,
 CPIS_ExecType execType
);

Int32 CPIS_rotation(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_RotationParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_fillMem(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_FillMemParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_arrayOp(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_ArrayOpParms *params,
    CPIS_ExecType execType
);


Int32 CPIS_arrayScalarOp(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_ArrayScalarOpParms *params,
    CPIS_ExecType execType
);


Int32 CPIS_arrayCondWrite(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_ArrayCondWriteParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_YCbCrPack(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_YCbCrPackParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_YCbCrUnpack(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_YCbCrUnpackParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_matMul(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_MatMulParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_sum(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_SumParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_sumCFA(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_SumCFAParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_table_lookup(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_LUTParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_blkAverage(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_BlkAverageParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_medianFilterRow(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_MedianFilterRowParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_medianFilterCol(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_MedianFilterColParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_filter(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_FilterParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_RGBPack(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_RGBPackParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_RGBUnpack(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_RGBUnpackParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_recursiveFilter(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_RecursiveFilterParms *params,
    CPIS_ExecType execType
);


Int32 CPIS_median2D(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_Median2DParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_integralImage(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_IntegralImageParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_sobel(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_SobelParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_pyramid(
    CPIS_Handle *handle,
    CPIS_BaseParms *base,
    CPIS_PyramidParms *params,
    CPIS_ExecType execType
);

Int32 CPIS_affineTransformGetSize(CPIS_BaseParms *base,
    CPIS_AffineTransformParms *params, CPIS_AffineTransformOutputROI *outputROI );

Int32 CPIS_affineTransform(
 CPIS_Handle *handle,
 CPIS_BaseParms *base,
 CPIS_AffineTransformParms *params,
 CPIS_ExecType execType
);

Int32 CPIS_sad(
 CPIS_Handle *handle,
 CPIS_BaseParms *base,
 CPIS_SadParms *params,
 CPIS_ExecType execType
);

Int32 CPIS_setSadTemplateOffset(CPIS_Handle *handle, Uint16 templateStartOfst);

Int32 CPIS_loadRecursiveFilterInitialValues(CPIS_Handle *handle, void *src, CPIS_Format format, Uint32 stride);

Int32 CPIS_setRecursiveFilterAlphaCoef(CPIS_Handle *handle, Uint16 alpha);

Int32 CPIS_start(CPIS_Handle handle);

Int32 CPIS_wait(CPIS_Handle handle);

Int32 CPIS_reset(CPIS_Handle handle);

Int32 CPIS_updateSrcDstPtr(CPIS_Handle handle, CPIS_BaseParms *base);

Int32 CPIS_isBusy();

Int32 CPIS_setWaitCB(Int32 (*waitCB)(void*arg), void* waitCBarg);

Int32 CPIS_setWaitCBArg(void* waitCBarg);

Int32 CPIS_delete(CPIS_Handle handle);

#ifdef __cplusplus
 }
#endif

#endif /* #define _CPISLIB_H */

/* ======================================================================== */
/*                       End of file                                        */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2008 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */
