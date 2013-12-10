#ifndef _CSLR__VPSS_001_H_
#define _CSLR__VPSS_001_H_

#include <cslr.h>

#include <tistdtypes.h>


/* Minimum unit = 4 bytes */

/**************************************************************************\
* Register Overlay Structure
\**************************************************************************/
typedef struct {
  volatile Uint32 PCCR;
  volatile Uint32 TEST;
  volatile Uint32 MISR;
  volatile Uint32 DATST1;
  volatile Uint32 DATST2;
  volatile Uint32 RSVD0[7];
  volatile Uint32 CKE0;
  volatile Uint32 RSVD1[3];
  volatile Uint32 CKE1;
  volatile Uint32 RSVD2[3];
  volatile Uint32 CKE2;
  volatile Uint32 RSVD3[3];
  volatile Uint32 CKE3;
  volatile Uint32 RSVD4[3];
  volatile Uint32 CKE4;
  volatile Uint32 RSVD5[3];
  volatile Uint32 CKE5;
} CSL_VpssRegs;

/**************************************************************************\
* Field Definition Macros
\**************************************************************************/

/* PCCR */


#define CSL_VPSS_PCCR_LDC_CLK_SEL_MASK (0x00000080u)
#define CSL_VPSS_PCCR_LDC_CLK_SEL_SHIFT (0x00000007u)
#define CSL_VPSS_PCCR_LDC_CLK_SEL_RESETVAL (0x00000000u)

#define CSL_VPSS_PCCR_OSD_CLK_SEL_MASK (0x00000040u)
#define CSL_VPSS_PCCR_OSD_CLK_SEL_SHIFT (0x00000006u)
#define CSL_VPSS_PCCR_OSD_CLK_SEL_RESETVAL (0x00000000u)


#define CSL_VPSS_PCCR_FDIF_CLK_ENABLE_MASK (0x00000010u)
#define CSL_VPSS_PCCR_FDIF_CLK_ENABLE_SHIFT (0x00000004u)
#define CSL_VPSS_PCCR_FDIF_CLK_ENABLE_RESETVAL (0x00000001u)

#define CSL_VPSS_PCCR_LDC_CLK_ENABLE_MASK (0x00000008u)
#define CSL_VPSS_PCCR_LDC_CLK_ENABLE_SHIFT (0x00000003u)
#define CSL_VPSS_PCCR_LDC_CLK_ENABLE_RESETVAL (0x00000001u)

#define CSL_VPSS_PCCR_CLKSEL_VENC_MASK (0x00000004u)
#define CSL_VPSS_PCCR_CLKSEL_VENC_SHIFT (0x00000002u)
#define CSL_VPSS_PCCR_CLKSEL_VENC_RESETVAL (0x00000000u)


#define CSL_VPSS_PCCR_VPBE_CLK_ENABLE_MASK (0x00000001u)
#define CSL_VPSS_PCCR_VPBE_CLK_ENABLE_SHIFT (0x00000000u)
#define CSL_VPSS_PCCR_VPBE_CLK_ENABLE_RESETVAL (0x00000001u)

#define CSL_VPSS_PCCR_RESETVAL (0x00000019u)

/* TEST */


#define CSL_VPSS_TEST_MISR_CTRL_MASK (0x00000003u)
#define CSL_VPSS_TEST_MISR_CTRL_SHIFT (0x00000000u)
#define CSL_VPSS_TEST_MISR_CTRL_RESETVAL (0x00000000u)
/*----MISR_CTRL Tokens----*/
#define CSL_VPSS_TEST_MISR_CTRL_IDLE (0x00000000u)
#define CSL_VPSS_TEST_MISR_CTRL_LOAD (0x00000001u)
#define CSL_VPSS_TEST_MISR_CTRL_RUNLOOPBACK (0x00000002u)
#define CSL_VPSS_TEST_MISR_CTRL_RUN (0x00000003u)

#define CSL_VPSS_TEST_RESETVAL (0x00000000u)

/* MISR */

#define CSL_VPSS_MISR_MISR_MASK (0xFFFFFFFFu)
#define CSL_VPSS_MISR_MISR_SHIFT (0x00000000u)
#define CSL_VPSS_MISR_MISR_RESETVAL (0x00000000u)

#define CSL_VPSS_MISR_RESETVAL (0x00000000u)

/* DATST1 */


#define CSL_VPSS_DATST1_DACTDOA_MASK (0x0FFF0000u)
#define CSL_VPSS_DATST1_DACTDOA_SHIFT (0x00000010u)
#define CSL_VPSS_DATST1_DACTDOA_RESETVAL (0x00000000u)


#define CSL_VPSS_DATST1_DACTDOB_MASK (0x00000FFFu)
#define CSL_VPSS_DATST1_DACTDOB_SHIFT (0x00000000u)
#define CSL_VPSS_DATST1_DACTDOB_RESETVAL (0x00000000u)

#define CSL_VPSS_DATST1_RESETVAL (0x00000000u)

/* DATST2 */


#define CSL_VPSS_DATST2_DACTDOA_MASK (0x00000FFFu)
#define CSL_VPSS_DATST2_DACTDOA_SHIFT (0x00000000u)
#define CSL_VPSS_DATST2_DACTDOA_RESETVAL (0x00000000u)

#define CSL_VPSS_DATST2_RESETVAL (0x00000000u)

/* CKE0 - Customer Key Enable */


#define CSL_VPSS_CKE0_IPIPENIKE_MASK (0x00000001u)
#define CSL_VPSS_CKE0_IPIPENIKE_SHIFT (0x00000000u)
#define CSL_VPSS_CKE0_IPIPENIKE_RESETVAL (0x00000001u)

#define CSL_VPSS_CKE0_RESETVAL (0x00000001u)

/* CKE1 - Customer Key Enable */


#define CSL_VPSS_CKE1_ISIFNIKE_MASK (0x00000001u)
#define CSL_VPSS_CKE1_ISIFNIKE_SHIFT (0x00000000u)
#define CSL_VPSS_CKE1_ISIFNIKE_RESETVAL (0x00000001u)

#define CSL_VPSS_CKE1_RESETVAL (0x00000001u)

/* CKE2 - Customer Key Enable */


#define CSL_VPSS_CKE2_ISIFNOKE_MASK (0x00000001u)
#define CSL_VPSS_CKE2_ISIFNOKE_SHIFT (0x00000000u)
#define CSL_VPSS_CKE2_ISIFNOKE_RESETVAL (0x00000001u)

#define CSL_VPSS_CKE2_RESETVAL (0x00000001u)

/* CKE3 - Customer Key Enable */


#define CSL_VPSS_CKE3_IPIPEIFNOKE_MASK (0x00000001u)
#define CSL_VPSS_CKE3_IPIPEIFNOKE_SHIFT (0x00000000u)
#define CSL_VPSS_CKE3_IPIPEIFNOKE_RESETVAL (0x00000001u)

#define CSL_VPSS_CKE3_RESETVAL (0x00000001u)

/* CKE4 - Customer Key Enable */


#define CSL_VPSS_CKE4_H3AKOKE_MASK (0x00000001u)
#define CSL_VPSS_CKE4_H3AKOKE_SHIFT (0x00000000u)
#define CSL_VPSS_CKE4_H3AKOKE_RESETVAL (0x00000001u)

#define CSL_VPSS_CKE4_RESETVAL (0x00000001u)

/* CKE5 - Customer Key Enable */


#define CSL_VPSS_CKE5_H3ANIKE_MASK (0x00000001u)
#define CSL_VPSS_CKE5_H3ANIKE_SHIFT (0x00000000u)
#define CSL_VPSS_CKE5_H3ANIKE_RESETVAL (0x00000001u)

#define CSL_VPSS_CKE5_RESETVAL (0x00000001u)

#endif
