#include <mach/platform.h>
#include <mach/hardware.h>
#include <asm/memory.h>
#include <linux/linkage.h>

#define __virt_to_phys(x)   ((x) - PAGE_OFFSET + PLAT_PHYS_OFFSET)
#define __phys_to_virt(x)   ((x) - PLAT_PHYS_OFFSET + PAGE_OFFSET)

#define STACK_TOP       0xc00
/* size for each cpu*/
#define PM_PER_CPU_STORE_SIZE	(0x400)

#define MCU_START_CTRL  0xf840f000      /* mcu start control */

#define SRAM_BASE_VIRT  0xff000000      /* according to io.h, asm file can not use complex IO_ADDRESS */

#define OTP_IDWORD_ADDR 0xf8ab0128      /* OTP shadow register to indicate if it is advca chipset */
#define OTP_CA_ID_WORD  0x6EDBE953	/* ID_WORD for CA chipset */
/* General CPU constants */
#define MODE_USR        0x10
#define MODE_FIQ        0x11
#define MODE_IRQ        0x12
#define MODE_SVC        0x13
#define MODE_ABT        0x17
#define MODE_UND        0x1B
#define MODE_SYS        0x1F
#define I_Bit           0x80
#define F_Bit           0x40

@@ ************************************************************************@@
@@                                                                         @@
@@                                                                         @@
@@ General Interupt Controller (GIC) - register offsets                    @@
@@ ====================================================                    @@
@@ ICD register offsets with respect to the GIC Distributor base address   @@
@@ ICC register offsets with respect to the GIC CPU Interface base address @@
@@                                                                         @@
@@ ************************************************************************@@

#define ICC_ICR   0x0   @ ICC control (banked in Security Extns)
#define ICC_PMR   0x4   @ interrupt priority mask
#define ICC_BPR   0x8   @ binary point (banked in Security Extns)
#define ICC_ABPR  0x1C  @ aliased bianry point (Security Extns)

#define ICD_DCR   0x0   @ ICD control (banked in Security Extns)
#define ICD_ICTR  0x4   @ type information (RO)
#define ICD_ISR   0x80  @ interrupt security registers
#define ICD_ISER  0x100 @ set-enable registers
#define ICD_IPR   0x400 @ priority registers
#define ICD_IPTR  0x800 @ processor target registers
#define ICD_ICFR  0xC00 @ interrupt configuration registers

/* General SCU constants */
#define SCU_CONTROL_OFFSET              (0x00)
#define SCU_CONFIG_OFFSET               (0x04)
#define SCU_POWER_STATE_OFFSET          (0x08)
#define SCU_SEC_INVALID_REG_OFFSET      (0x0c)
#define SCU_FILTER_START_OFFSET         (0x40)
#define SCU_FILTER_END_OFFSET           (0x44)
#define SCU_ACCESS_CONTROL_OFFSET       (0x50)
#define SCU_NONSEC_CONTROL_OFFSET       (0x54)

.macro mode_save, rd
	@ save current mode r0~r12, r14, so we can use them
	STMIA	\rd!, {sp}

	@ Save banked ARM registers
	MRS	r4, CPSR
	MRS	r5, SPSR

	@ save the current CPSR and SPSR
	STMIA	\rd!, {r4 - r5}

	CPS	#MODE_SYS /*usr mode*/
	MRS	r4, SPSR
	STMIA	\rd!, {r4, r8 - r12,sp, lr}

	@ switch to Abort mode
	CPS	#MODE_ABT
	MRS	r4, SPSR
	STMIA	\rd!, {r4, sp, lr}

	@ switch to Undefined mode
	CPS	#MODE_UND
	MRS	r4, SPSR
	STMIA	\rd!, {r4, sp, lr}

	CPS	#MODE_IRQ
	MRS	r4, SPSR
	STMIA	\rd!, {r4, sp, lr}

	CPS	#MODE_FIQ
	MRS	r4, SPSR
	STMIA	\rd!, {r4, r8 - r12, sp, LR}

	CPS	#MODE_SVC
.endm

.macro mode_restore, rd
	CPS	#MODE_FIQ
	LDMDB	\rd!, {r4, r8 - r12, sp, LR}
	MSR	SPSR, r4

	CPS	#MODE_IRQ
	LDMDB	\rd!, {r4, sp, lr}
	MSR	SPSR, r4

	@ switch to Undefined mode
	CPS	#MODE_UND
	LDMDB	\rd!, {r4, sp, lr}
	MSR	SPSR, r4

	@ switch to Abort mode
	CPS	#MODE_ABT
	LDMDB	\rd!, {r4, sp, lr}
	MSR	SPSR, r4

	CPS	#MODE_SYS
	LDMDB	\rd!, {r4, r8 - r12, sp, LR}
	MSR	SPSR, r4

	CPS	#MODE_SVC

	@ restore the current CPSR and SPSR
	LDMDB	\rd!, {r4 - r5}
	MSR	CPSR_fcx, r4
	MSR	SPSR, r5

	@ restore sp
	LDMDB	\rd!, {sp}

.endm

.macro cp15_save, rd
	/* c1 control register */
	mrc	p15, 0, r4, c1, c0, 0		@ Save control register
	stmia	\rd!, {r4}

	/* c1 and c2 registers */
	mrc	p15, 0, r4, c1, c0, 2		@ CPACR
	mrc	p15, 0, r5, c2, c0, 0		@ TTBR0
	mrc	p15, 0, r6, c2, c0, 1		@ TTBR1
	mrc	p15, 0, r7, c2, c0, 2		@ TTBCR
	stmia	\rd!, {r4-r7}

	/* c3 and c10 registers */
	mrc	p15, 0, r4, c3, c0, 0		@ DACR
	mrc	p15, 0, r5, c10, c2, 0		@ PRRR
	mrc	p15, 0, r6, c10, c2, 1		@ NMRR
	mrc	p15, 0, r7, c1, c0, 1		@ ACTLR
	stmia	\rd!,{r4-r7}

	/* c12, c13 and CPSR registers */
	mrc	p15, 0, r4, c13, c0, 1		@ Context ID (CONTEXTIDX)
	mrc	p15, 0, r5, c13, c0, 2		@ User r/w thread ID (TPIDRURW)
	mrc	p15, 0, r6, c12, c0, 0		@ Secure or NS VBAR
	mrc	p15, 0, r7, c13, c0, 4		@ PL1 only Thread ID (TPIDRPRW), used by percpu offset
	stmia	\rd!, {r4-r7}

.endm

.macro cp15_restore, rd
	/* c12, c13 and CPSR registers */
	ldmdb	\rd!, {r4-r7}
	mcr	p15, 0, r4, c13, c0, 1		@ Context ID
	mcr	p15, 0, r5, c13, c0, 2		@ User r/w thread ID
	mcr	p15, 0, r6, c12, c0, 0		@ Secure or NS VBAR
	mcr	p15, 0, r7, c13, c0, 4		@ PL1 only Thread ID (TPIDRPRW), used by percpu offset

	/* c3 and c10 registers */
	ldmdb	\rd!,{r4-r7}
	mcr	p15, 0, r4, c3, c0, 0		@ DACR
	mcr	p15, 0, r5, c10, c2, 0		@ PRRR
	mcr	p15, 0, r6, c10, c2, 1		@ NMRR
	mcr	p15, 0, r7, c1, c0, 1		@ ACTLR

	/* c1 and c2 registers */
	ldmdb	\rd!,{r4-r7}
	mcr	p15, 0, r4, c1, c0, 2		@ CPACR
	mcr	p15, 0, r5, c2, c0, 0		@ TTBR0
	mcr	p15, 0, r6, c2, c0, 1		@ TTBR1
	mcr	p15, 0, r7, c2, c0, 2		@ TTBCR

	/* c1 control register */
	ldmdb	\rd!, {r4}
	bic	r4, #5				@ disable mmu and C bit
	mcr	p15, 0, r4, c1, c0, 0		@ Save control register
.endm

@scu_save r1 - scu reg base
.macro scu_save, rd
	LDR	r2,[r1,#SCU_FILTER_START_OFFSET]
	LDR	r3,[r1,#SCU_FILTER_END_OFFSET]
	LDR	r4,[r1,#SCU_ACCESS_CONTROL_OFFSET]
	LDR	r5,[r1,#SCU_NONSEC_CONTROL_OFFSET]
	LDR	r6,[r1,#SCU_CONTROL_OFFSET]
	LDR	r7,[r1,#SCU_POWER_STATE_OFFSET]
	STMIA	\rd!, {r2-r7}
.endm

.macro scu_restore, rd
	LDMDB	\rd!, {r2-r7}
	STR	r2,[r1,#SCU_FILTER_START_OFFSET]
	STR	r3,[r1,#SCU_FILTER_END_OFFSET]
	STR	r4,[r1,#SCU_ACCESS_CONTROL_OFFSET]
	STR	r5,[r1,#SCU_NONSEC_CONTROL_OFFSET]

	LDR	r8, =0xFFFF
	STR	r8, [r1, #SCU_SEC_INVALID_REG_OFFSET]	@invalidate the duplicate TAG store

	STR	r6,[r1,#SCU_CONTROL_OFFSET]
	STR	r7,[r1,#SCU_POWER_STATE_OFFSET]
.endm


/*
 *-------------------------------------------------------------------------
 *   Function: hilpm_godpsleep
 *
 *   this function is the low level interface when deep sleep.
 *
 */
ENTRY (hi_pm_sleep)
store_current:
	stmfd	sp!, {r0-r12, lr} @ Save registers on stack

	ldr	r1, =hi_pm_ddrbase
	ldr	r0, [r1]

	/*--->save mode , cp15*/
	mode_save r0
	cp15_save r0

	ldr	r2, =scureg_base
	ldr	r1, [r2]

	scu_save r0

	mov	r6, r0

	/*
	 * Flush all data from the L1 data cache before disabling
	 * SCTLR.C bit.
	 * Used Register: R0/R1/R2/R3/R4/R5/R7/R9/R10
	 */
	bl	v7_flush_dcache_all

	/* disable C bit*/
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #(1 << 2)		@ Disable the C bit
	mcr	p15, 0, r0, c1, c0, 0
	isb

	/* SMP->AMP*/
	mrc	p15, 0, r0, c1, c1, 2		@Read NSACR data
	tst	r0, #(1 << 18)
	mrcne	p15, 0, r0, c1, c0, 1
	bicne	r0, r0, #(1 << 6)		@SMP-->AMP
	mcrne	p15, 0, r0, c1, c0, 1
	isb

	/** write domain access to get the domain access right  **/
	ldr	r0, =0xFFFFFFFF
	mcr	p15, 0, r0, c3, c0, 0

	@ save end addr
	ldr	r1, =hi_pm_cpu0_len
	ldr	r2, =hi_pm_ddrbase
	ldr	r0, [r2]
	sub	r0, r6, r0
	str	r0, [r1]

store_resume:
	ldr	r5, =(__virt_to_phys(resume_code))
	ldr	r4, =hi_sc_virtbase
	ldr	r4, [r4]

	/*Store resume address for non-advca chipset*/
	str	r5, [r4, #REG_SC_GEN30]

	/* if it is not advca chipset, jump to enable_mcu */
	ldr	r1, =IO_ADDRESS(OTP_IDWORD_ADDR)
	ldr	r2, [r1]
	ldr r1, = OTP_CA_ID_WORD
	cmp r1, r2
	bne	enable_mcu

/*Wakeup process for advca chipset*/
entry_ddr_wakeup_check:
	/*Resume address is different for advca chipset*/
	str	r5, [r4, #REG_SC_GEN9]
	ldr	r0, = _ddr_wakeup_check_code_begin
	ldr	r0, [r0]
	ldr	r1, =hi_sram_virtbase
	ldr 	r1, [r1]
	ldr	r2, = _ddr_wakeup_check_code_end
	ldr	r2, [r2]
loop_copy_to_ddr:
	ldmia	r0!, {r3-r10}
	stmia	r1!, {r3-r10}
	cmp	r0,r2
	ble	loop_copy_to_ddr

	@disable mmu
	MRC	p15, 0, r1, c1, c0, 0
	bic	r1, #1
	MCR	p15, 0, r1, c1, c0, 0

	@addr: 0xffff2000
	mov	r1, #0x2000
	movt	r1, #0xffff
	BX	r1
	b .

enable_mcu:
	ldr	r1, =IO_ADDRESS(MCU_START_CTRL)
	ldr	r2, [r1]
	orr	r2, #0x1
	str	r2, [r1]
	ldr	r1, =hi_sc_virtbase
	ldr	r1, [r1]
	ldr	r2, =0x12345678
	str	r2, [r1, #REG_SC_GEN13]

go_wfi:
	mov	r1, #0x10
loop_wfi:
	dsb
	isb
	wfi
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	ldr	r4, =hi_sc_virtbase
	ldr	r4, [r4]
	add	r1, #0x1
	str	r1, [r4, #REG_SC_GEN8]
	b	loop_wfi

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

resume_code:
_serial_init:
	mov	r3, #0
	movt	r3, #0xf8b0
	mov	r2, #0
	str	r2, [r3, #48]
#ifdef CONFIG_S40_FPGA
	add	r2, r2, #29
	str	r2, [r3, #36]
	mov	r2, #19
	str	r2, [r3, #40]
#else
	add	r2, r2, #46
	str	r2, [r3, #36]
	mov	r2, #32
	str	r2, [r3, #40]
#endif
	movw	a3, #112
	str	r2, [r3, #44]
	movw	r2, #769
	str	r2, [r3, #48]

	@ write domain access to get the domain access right
	ldr	r7, =0xFFFFFFFF
	mcr	p15, 0, r7, c3, c0, 0

	@ write control register to disable the mmu
	mrc	p15, 0, r1, c1, c0, 0
	bic	r1, #1
	mcr	p15, 0, r1, c1, c0, 0

	@ get end addr of context
	ldr	r1, =(__virt_to_phys(hi_pm_cpu0_len))
	ldr	r0, [r1]
	ldr	r1, =(__virt_to_phys(hi_pm_phybase))
	ldr	r1, [r1]
	add	r0, r1, r0


	@ scu restore
	ldr	r1, =(REG_BASE_A9_PERI + REG_A9_PERI_SCU)
	scu_restore r0

	cp15_restore r0
	mode_restore r0

	ldr	r9,  =resume_virt

	/*
	 * Invalidate L1 I/D
	 */
	mov	r0, #0                   /* set up for MCR */
	mcr	p15, 0, r0, c8, c7, 0    /* invalidate TLBs */
	mcr	p15, 0, r0, c7, c5, 0    /* invalidate icache */

	/* Invalidate L1 D-cache */
	mcr	p15, 2, r0, c0, c0, 0    /* select L1 data cache */
	mrc	p15, 1, r3, c0, c0, 0    /* Read Current Cache Size Identification Register */
	movw	r1, #0x1ff               /* replace ldr r1, =0x1ff */
	and	r3, r1, r3, LSR #13      /* r3 = (number of sets -1) */
	mov	r0, #0
l1_way_loop:
	mov	r1, #0                   /* r1->set counter */
l1_line_loop:
	mov	r2, r0, LSL #30
	orr	r2, r1, LSL #5           /* r2->set/way cache-op format */
	mcr	p15, 0, r2, c7, c6, 2    /* Invalidate line described by r2 */
	add	r1, r1, #1               /* Increment set counter */
	cmp	r1, r3                   /* Check if the last set is reached... */
	ble	l1_line_loop             /* if not, continue the set_loop */
	add	r0, r0, #1               /* else, Increment way counter */
	cmp	r0, #4                   /* Check if the last way is reached... */
	blt	l1_way_loop              /* if not, continue the way_loop */

	@ enable mmu and C
	mrc	p15, 0, r1, c1, c0, 0
	orr	r1, #5
	mcr	p15, 0, r1, c1, c0, 0

	bx	r9

	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP
	NOP

resume_virt:
	mrc	p15, 0, r0, c1, c1, 2       @Read NSACR data
	tst	r0, #(1 << 18)
	mrcne	p15, 0, r0, c1, c0, 1
	orrne	r0, r0, #(1 << 6)            @AMP-->SMP
	mcrne	p15, 0, r0, c1, c0, 1
	isb

	@resume current mode registers
	ldmfd	sp!, {r0-r12, lr}

	/* go back to the call point */
	mov	pc, lr
	nop
	nop
	nop
	b .
ENDPROC(hi_pm_sleep)

.section .data

.global hi_pm_ddrbase
hi_pm_ddrbase:
.word hi_pm_ddrbase

.global hi_pm_phybase
hi_pm_phybase:
.word hi_pm_phybase

.global hi_pm_cpu0_len
hi_pm_cpu0_len:
.word .

.global scureg_base
scureg_base:
.word scureg_base


.global hi_sram_virtbase
hi_sram_virtbase:
.word hi_sram_virtbase

.global _ddr_wakeup_check_code_begin
_ddr_wakeup_check_code_begin:
.word _ddr_wakeup_check_code_begin

.global _ddr_wakeup_check_code_end
_ddr_wakeup_check_code_end:
.word _ddr_wakeup_check_code_end
