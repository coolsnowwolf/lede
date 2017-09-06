#include <AT91RM9200_inc.h>

#define ARM_MODE_USER           0x10
#define ARM_MODE_FIQ            0x11
#define ARM_MODE_IRQ            0x12
#define ARM_MODE_SVC            0x13
#define ARM_MODE_ABORT          0x17
#define ARM_MODE_UNDEF          0x1B
#define ARM_MODE_SYS            0x1F

#define I_BIT                   0x80
#define F_BIT                   0x40
#define T_BIT                   0x20


/* -----------------------------------------------------------------------------
   AT91F_ASM_MCI_Handler
   ---------------------
      Handler called by the AIC
        
      Save context
        Call C handler
    Restore context
   ----------------------------------------------------------------------------- */
		
.global AT91F_ASM_MCI_Handler  

AT91F_ASM_MCI_Handler:
/*  Adjust and save LR_irq in IRQ stack */
 	sub         r14, r14, #4
	stmfd       sp!, {r14}

/*  Write in the IVR to support Protect Mode
  No effect in Normal Mode
  De-assert the NIRQ and clear the source in Protect Mode */
	ldr         r14, =AT91C_BASE_AIC
	str         r14, [r14, #AIC_IVR]

/*  Save SPSR and r0 in IRQ stack */
	mrs         r14, SPSR
	stmfd       sp!, {r0, r14}

/*  Enable Interrupt and Switch in SYS Mode */
	mrs         r0, CPSR
	bic         r0, r0, #I_BIT
	orr         r0, r0, #ARM_MODE_SYS
	msr         CPSR_c, r0
 
/* Save scratch/used registers and LR in User Stack */
	stmfd       sp!, { r1-r3, r12, r14}

	ldr     r1, =AT91F_MCI_Handler
	mov     r14, pc
	bx      r1

/*  Restore scratch/used registers and LR from User Stack */
	ldmia       sp!, { r1-r3, r12, r14}

/*  Disable Interrupt and switch back in IRQ mode */
	mrs         r0, CPSR
	bic         r0, r0, #ARM_MODE_SYS
	orr         r0, r0, #I_BIT | ARM_MODE_IRQ
	msr         CPSR_c, r0

/*  Mark the End of Interrupt on the AIC */
	ldr         r0, =AT91C_BASE_AIC
	str         r0, [r0, #AIC_EOICR]

/*  Restore SPSR_irq and r0 from IRQ stack */
	ldmia       sp!, {r0, r14}
	msr         SPSR_cxsf, r14

/*  Restore adjusted  LR_irq from IRQ stack directly in the PC */
	ldmia       sp!, {pc}^

