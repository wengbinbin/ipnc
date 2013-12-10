
/*
//============================================================================
//    FILE NAME : vs_ti.h
//    ALGORITHM : VS
//    VENDOR    : TI
//    TARGET DSP: C64x+
//    PURPOSE   : This is the top level driver file that exercises the VS code
//============================================================================
*/
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2006 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

#ifndef VS_TI_
#define VS_TI_

#include <ivs1.h>


extern XDAS_Int32 VS_TI_create(IVS1_Params *params);

extern XDAS_Int32 VS_TI_apply(IVS1_InArgs *Inargs, IVS1_OutArgs *Outargs);

extern XDAS_Int32 VS_TI_delete();

#endif	/* VS_TI_ */

/* ======================================================================== */
/* End of file : vs_ti.h                                             */
/* ------------------------------------------------------------------------ */
/*            Copyright (c) 2006 Texas Instruments, Incorporated.           */
/*                           All Rights Reserved.                           */
/* ======================================================================== */

