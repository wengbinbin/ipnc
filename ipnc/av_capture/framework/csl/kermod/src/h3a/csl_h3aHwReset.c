/*
    DM360 Evaluation Software

    (c)Texas Instruments 2007
*/

#include <csl_h3a.h>


CSL_Status CSL_h3aHwReset(CSL_H3aHandle hndl)
{
  if (hndl == NULL)
    return CSL_EFAIL;

  hndl->regs->PID = CSL_H3A_PID_RESETVAL;
  hndl->regs->PCR = CSL_H3A_PCR_RESETVAL;
  hndl->regs->AFPAX1 = CSL_H3A_AFPAX1_RESETVAL;
  hndl->regs->AFPAX2 = CSL_H3A_AFPAX2_RESETVAL;
  hndl->regs->AFPAXSTART = CSL_H3A_AFPAXSTART_RESETVAL;
  hndl->regs->AFIIRSH = CSL_H3A_AFIIRSH_RESETVAL;
  hndl->regs->AFBUFST = CSL_H3A_AFBUFST_RESETVAL;
  hndl->regs->AFCOEF010 = CSL_H3A_AFCOEF010_RESETVAL;
  hndl->regs->AFCOEF032 = CSL_H3A_AFCOEF032_RESETVAL;
  hndl->regs->AFCOEFF054 = CSL_H3A_AFCOEFF054_RESETVAL;
  hndl->regs->AFCOEFF076 = CSL_H3A_AFCOEFF076_RESETVAL;
  hndl->regs->AFCOEFF098 = CSL_H3A_AFCOEFF098_RESETVAL;
  hndl->regs->AFCOEFF0010 = CSL_H3A_AFCOEFF0010_RESETVAL;
  hndl->regs->AFCOEF110 = CSL_H3A_AFCOEF110_RESETVAL;
  hndl->regs->AFCOEF132 = CSL_H3A_AFCOEF132_RESETVAL;
  hndl->regs->AFCOEFF154 = CSL_H3A_AFCOEFF154_RESETVAL;
  hndl->regs->AFCOEFF176 = CSL_H3A_AFCOEFF176_RESETVAL;
  hndl->regs->AFCOEFF198 = CSL_H3A_AFCOEFF198_RESETVAL;
  hndl->regs->AFCOEFF1010 = CSL_H3A_AFCOEFF1010_RESETVAL;
  hndl->regs->AEWWIN1 = CSL_H3A_AEWWIN1_RESETVAL;
  hndl->regs->AEWINSTART = CSL_H3A_AEWINSTART_RESETVAL;
  hndl->regs->AEWINBLK = CSL_H3A_AEWINBLK_RESETVAL;
  hndl->regs->AEWSUBWIN = CSL_H3A_AEWSUBWIN_RESETVAL;
  hndl->regs->AEWBUFST = CSL_H3A_AEWBUFST_RESETVAL;
  hndl->regs->RSDR_ADDR = CSL_H3A_RSDR_ADDR_RESETVAL;
  hndl->regs->LINE_START = CSL_H3A_LINE_START_RESETVAL;
  hndl->regs->VFV_CFG1 = CSL_H3A_VFV_CFG1_RESETVAL;
  hndl->regs->VFV_CFG2 = CSL_H3A_VFV_CFG2_RESETVAL;
  hndl->regs->VFV_CFG3 = CSL_H3A_VFV_CFG3_RESETVAL;
  hndl->regs->VFV_CFG4 = CSL_H3A_VFV_CFG4_RESETVAL;
  hndl->regs->HFV_THR = CSL_H3A_HFV_THR_RESETVAL;

  return CSL_SOK;
}
