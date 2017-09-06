// ----------------------------------------------------------------------------
//          ATMEL Microcontroller Software Support  -  ROUSSET  -
// ----------------------------------------------------------------------------
//  The software is delivered "AS IS" without warranty or condition of any
//  kind, either express, implied or statutory. This includes without
//  limitation any warranty or condition with respect to merchantability or
//  fitness for any particular purpose, or against the infringements of
//  intellectual property rights of others.
// ----------------------------------------------------------------------------
// File Name           : AT91RM9200.h
// Object              : AT91RM9200 definitions
// Generated           : AT91 SW Application Group  11/19/2003 (17:20:51)
// 
// CVS Reference       : /AT91RM9200.pl/1.16/Fri Feb 07 10:29:51 2003//
// CVS Reference       : /SYS_AT91RM9200.pl/1.2/Fri Jan 17 12:44:37 2003//
// CVS Reference       : /MC_1760A.pl/1.1/Fri Aug 23 14:38:22 2002//
// CVS Reference       : /AIC_1796B.pl/1.1.1.1/Fri Jun 28 09:36:47 2002//
// CVS Reference       : /PMC_2636A.pl/1.1.1.1/Fri Jun 28 09:36:48 2002//
// CVS Reference       : /ST_1763B.pl/1.1/Fri Aug 23 14:41:42 2002//
// CVS Reference       : /RTC_1245D.pl/1.2/Fri Jan 31 12:19:06 2003//
// CVS Reference       : /PIO_1725D.pl/1.1.1.1/Fri Jun 28 09:36:47 2002//
// CVS Reference       : /DBGU_1754A.pl/1.4/Fri Jan 31 12:18:24 2003//
// CVS Reference       : /UDP_1765B.pl/1.3/Fri Aug 02 14:45:38 2002//
// CVS Reference       : /MCI_1764A.pl/1.2/Thu Nov 14 17:48:24 2002//
// CVS Reference       : /US_1739C.pl/1.2/Fri Jul 12 07:49:25 2002//
// CVS Reference       : /SPI_AT91RMxxxx.pl/1.3/Tue Nov 26 10:20:29 2002//
// CVS Reference       : /SSC_1762A.pl/1.2/Fri Nov 08 13:26:39 2002//
// CVS Reference       : /TC_1753B.pl/1.2/Fri Jan 31 12:19:55 2003//
// CVS Reference       : /TWI_1761B.pl/1.4/Fri Feb 07 10:30:07 2003//
// CVS Reference       : /PDC_1734B.pl/1.2/Thu Nov 21 16:38:23 2002//
// CVS Reference       : /UHP_xxxxA.pl/1.1/Mon Jul 22 12:21:58 2002//
// CVS Reference       : /EMAC_1794A.pl/1.4/Fri Jan 17 12:11:54 2003//
// CVS Reference       : /EBI_1759B.pl/1.10/Fri Jan 17 12:44:29 2003//
// CVS Reference       : /SMC_1783A.pl/1.3/Thu Oct 31 14:38:17 2002//
// CVS Reference       : /SDRC_1758B.pl/1.2/Thu Oct 03 13:04:41 2002//
// CVS Reference       : /BFC_1757B.pl/1.3/Thu Oct 31 14:38:00 2002//
// ----------------------------------------------------------------------------

// Hardware register definition

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR System Peripherals
// *****************************************************************************

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Memory Controller Interface
// *****************************************************************************
// *** Register offset in AT91S_MC structure ***
#define MC_RCR          ( 0) // MC Remap Control Register
#define MC_ASR          ( 4) // MC Abort Status Register
#define MC_AASR         ( 8) // MC Abort Address Status Register
#define MC_PUIA         (16) // MC Protection Unit Area
#define MC_PUP          (80) // MC Protection Unit Peripherals
#define MC_PUER         (84) // MC Protection Unit Enable Register
// -------- MC_RCR : (MC Offset: 0x0) MC Remap Control Register -------- 
#define AT91C_MC_RCB              (0x1 <<  0) // (MC) Remap Command Bit
// -------- MC_ASR : (MC Offset: 0x4) MC Abort Status Register -------- 
#define AT91C_MC_UNDADD           (0x1 <<  0) // (MC) Undefined Addess Abort Status
#define AT91C_MC_MISADD           (0x1 <<  1) // (MC) Misaligned Addess Abort Status
#define AT91C_MC_MPU              (0x1 <<  2) // (MC) Memory protection Unit Abort Status
#define AT91C_MC_ABTSZ            (0x3 <<  8) // (MC) Abort Size Status
#define 	AT91C_MC_ABTSZ_BYTE                 (0x0 <<  8) // (MC) Byte
#define 	AT91C_MC_ABTSZ_HWORD                (0x1 <<  8) // (MC) Half-word
#define 	AT91C_MC_ABTSZ_WORD                 (0x2 <<  8) // (MC) Word
#define AT91C_MC_ABTTYP           (0x3 << 10) // (MC) Abort Type Status
#define 	AT91C_MC_ABTTYP_DATAR                (0x0 << 10) // (MC) Data Read
#define 	AT91C_MC_ABTTYP_DATAW                (0x1 << 10) // (MC) Data Write
#define 	AT91C_MC_ABTTYP_FETCH                (0x2 << 10) // (MC) Code Fetch
#define AT91C_MC_MST0             (0x1 << 16) // (MC) Master 0 Abort Source
#define AT91C_MC_MST1             (0x1 << 17) // (MC) Master 1 Abort Source
#define AT91C_MC_SVMST0           (0x1 << 24) // (MC) Saved Master 0 Abort Source
#define AT91C_MC_SVMST1           (0x1 << 25) // (MC) Saved Master 1 Abort Source
// -------- MC_PUIA : (MC Offset: 0x10) MC Protection Unit Area -------- 
#define AT91C_MC_PROT             (0x3 <<  0) // (MC) Protection
#define 	AT91C_MC_PROT_PNAUNA               (0x0) // (MC) Privilege: No Access, User: No Access
#define 	AT91C_MC_PROT_PRWUNA               (0x1) // (MC) Privilege: Read/Write, User: No Access
#define 	AT91C_MC_PROT_PRWURO               (0x2) // (MC) Privilege: Read/Write, User: Read Only
#define 	AT91C_MC_PROT_PRWURW               (0x3) // (MC) Privilege: Read/Write, User: Read/Write
#define AT91C_MC_SIZE             (0xF <<  4) // (MC) Internal Area Size
#define 	AT91C_MC_SIZE_1KB                  (0x0 <<  4) // (MC) Area size 1KByte
#define 	AT91C_MC_SIZE_2KB                  (0x1 <<  4) // (MC) Area size 2KByte
#define 	AT91C_MC_SIZE_4KB                  (0x2 <<  4) // (MC) Area size 4KByte
#define 	AT91C_MC_SIZE_8KB                  (0x3 <<  4) // (MC) Area size 8KByte
#define 	AT91C_MC_SIZE_16KB                 (0x4 <<  4) // (MC) Area size 16KByte
#define 	AT91C_MC_SIZE_32KB                 (0x5 <<  4) // (MC) Area size 32KByte
#define 	AT91C_MC_SIZE_64KB                 (0x6 <<  4) // (MC) Area size 64KByte
#define 	AT91C_MC_SIZE_128KB                (0x7 <<  4) // (MC) Area size 128KByte
#define 	AT91C_MC_SIZE_256KB                (0x8 <<  4) // (MC) Area size 256KByte
#define 	AT91C_MC_SIZE_512KB                (0x9 <<  4) // (MC) Area size 512KByte
#define 	AT91C_MC_SIZE_1MB                  (0xA <<  4) // (MC) Area size 1MByte
#define 	AT91C_MC_SIZE_2MB                  (0xB <<  4) // (MC) Area size 2MByte
#define 	AT91C_MC_SIZE_4MB                  (0xC <<  4) // (MC) Area size 4MByte
#define 	AT91C_MC_SIZE_8MB                  (0xD <<  4) // (MC) Area size 8MByte
#define 	AT91C_MC_SIZE_16MB                 (0xE <<  4) // (MC) Area size 16MByte
#define 	AT91C_MC_SIZE_64MB                 (0xF <<  4) // (MC) Area size 64MByte
#define AT91C_MC_BA               (0x3FFFF << 10) // (MC) Internal Area Base Address
// -------- MC_PUP : (MC Offset: 0x50) MC Protection Unit Peripheral -------- 
// -------- MC_PUER : (MC Offset: 0x54) MC Protection Unit Area -------- 
#define AT91C_MC_PUEB             (0x1 <<  0) // (MC) Protection Unit enable Bit

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Real-time Clock Alarm and Parallel Load Interface
// *****************************************************************************
// *** Register offset in AT91S_RTC structure ***
#define RTC_CR          ( 0) // Control Register
#define RTC_MR          ( 4) // Mode Register
#define RTC_TIMR        ( 8) // Time Register
#define RTC_CALR        (12) // Calendar Register
#define RTC_TIMALR      (16) // Time Alarm Register
#define RTC_CALALR      (20) // Calendar Alarm Register
#define RTC_SR          (24) // Status Register
#define RTC_SCCR        (28) // Status Clear Command Register
#define RTC_IER         (32) // Interrupt Enable Register
#define RTC_IDR         (36) // Interrupt Disable Register
#define RTC_IMR         (40) // Interrupt Mask Register
#define RTC_VER         (44) // Valid Entry Register
// -------- RTC_CR : (RTC Offset: 0x0) RTC Control Register -------- 
#define AT91C_RTC_UPDTIM          (0x1 <<  0) // (RTC) Update Request Time Register
#define AT91C_RTC_UPDCAL          (0x1 <<  1) // (RTC) Update Request Calendar Register
#define AT91C_RTC_TIMEVSEL        (0x3 <<  8) // (RTC) Time Event Selection
#define 	AT91C_RTC_TIMEVSEL_MINUTE               (0x0 <<  8) // (RTC) Minute change.
#define 	AT91C_RTC_TIMEVSEL_HOUR                 (0x1 <<  8) // (RTC) Hour change.
#define 	AT91C_RTC_TIMEVSEL_DAY24                (0x2 <<  8) // (RTC) Every day at midnight.
#define 	AT91C_RTC_TIMEVSEL_DAY12                (0x3 <<  8) // (RTC) Every day at noon.
#define AT91C_RTC_CALEVSEL        (0x3 << 16) // (RTC) Calendar Event Selection
#define 	AT91C_RTC_CALEVSEL_WEEK                 (0x0 << 16) // (RTC) Week change (every Monday at time 00:00:00).
#define 	AT91C_RTC_CALEVSEL_MONTH                (0x1 << 16) // (RTC) Month change (every 01 of each month at time 00:00:00).
#define 	AT91C_RTC_CALEVSEL_YEAR                 (0x2 << 16) // (RTC) Year change (every January 1 at time 00:00:00).
// -------- RTC_MR : (RTC Offset: 0x4) RTC Mode Register -------- 
#define AT91C_RTC_HRMOD           (0x1 <<  0) // (RTC) 12-24 hour Mode
// -------- RTC_TIMR : (RTC Offset: 0x8) RTC Time Register -------- 
#define AT91C_RTC_SEC             (0x7F <<  0) // (RTC) Current Second
#define AT91C_RTC_MIN             (0x7F <<  8) // (RTC) Current Minute
#define AT91C_RTC_HOUR            (0x1F << 16) // (RTC) Current Hour
#define AT91C_RTC_AMPM            (0x1 << 22) // (RTC) Ante Meridiem, Post Meridiem Indicator
// -------- RTC_CALR : (RTC Offset: 0xc) RTC Calendar Register -------- 
#define AT91C_RTC_CENT            (0x3F <<  0) // (RTC) Current Century
#define AT91C_RTC_YEAR            (0xFF <<  8) // (RTC) Current Year
#define AT91C_RTC_MONTH           (0x1F << 16) // (RTC) Current Month
#define AT91C_RTC_DAY             (0x7 << 21) // (RTC) Current Day
#define AT91C_RTC_DATE            (0x3F << 24) // (RTC) Current Date
// -------- RTC_TIMALR : (RTC Offset: 0x10) RTC Time Alarm Register -------- 
#define AT91C_RTC_SECEN           (0x1 <<  7) // (RTC) Second Alarm Enable
#define AT91C_RTC_MINEN           (0x1 << 15) // (RTC) Minute Alarm
#define AT91C_RTC_HOUREN          (0x1 << 23) // (RTC) Current Hour
// -------- RTC_CALALR : (RTC Offset: 0x14) RTC Calendar Alarm Register -------- 
#define AT91C_RTC_MONTHEN         (0x1 << 23) // (RTC) Month Alarm Enable
#define AT91C_RTC_DATEEN          (0x1 << 31) // (RTC) Date Alarm Enable
// -------- RTC_SR : (RTC Offset: 0x18) RTC Status Register -------- 
#define AT91C_RTC_ACKUPD          (0x1 <<  0) // (RTC) Acknowledge for Update
#define AT91C_RTC_ALARM           (0x1 <<  1) // (RTC) Alarm Flag
#define AT91C_RTC_SECEV           (0x1 <<  2) // (RTC) Second Event
#define AT91C_RTC_TIMEV           (0x1 <<  3) // (RTC) Time Event
#define AT91C_RTC_CALEV           (0x1 <<  4) // (RTC) Calendar event
// -------- RTC_SCCR : (RTC Offset: 0x1c) RTC Status Clear Command Register -------- 
// -------- RTC_IER : (RTC Offset: 0x20) RTC Interrupt Enable Register -------- 
// -------- RTC_IDR : (RTC Offset: 0x24) RTC Interrupt Disable Register -------- 
// -------- RTC_IMR : (RTC Offset: 0x28) RTC Interrupt Mask Register -------- 
// -------- RTC_VER : (RTC Offset: 0x2c) RTC Valid Entry Register -------- 
#define AT91C_RTC_NVTIM           (0x1 <<  0) // (RTC) Non valid Time
#define AT91C_RTC_NVCAL           (0x1 <<  1) // (RTC) Non valid Calendar
#define AT91C_RTC_NVTIMALR        (0x1 <<  2) // (RTC) Non valid time Alarm
#define AT91C_RTC_NVCALALR        (0x1 <<  3) // (RTC) Nonvalid Calendar Alarm

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR System Timer Interface
// *****************************************************************************
// *** Register offset in AT91S_ST structure ***
#define ST_CR           ( 0) // Control Register
#define ST_PIMR         ( 4) // Period Interval Mode Register
#define ST_WDMR         ( 8) // Watchdog Mode Register
#define ST_RTMR         (12) // Real-time Mode Register
#define ST_SR           (16) // Status Register
#define ST_IER          (20) // Interrupt Enable Register
#define ST_IDR          (24) // Interrupt Disable Register
#define ST_IMR          (28) // Interrupt Mask Register
#define ST_RTAR         (32) // Real-time Alarm Register
#define ST_CRTR         (36) // Current Real-time Register
// -------- ST_CR : (ST Offset: 0x0) System Timer Control Register -------- 
#define AT91C_ST_WDRST            (0x1 <<  0) // (ST) Watchdog Timer Restart
// -------- ST_PIMR : (ST Offset: 0x4) System Timer Period Interval Mode Register -------- 
#define AT91C_ST_PIV              (0xFFFF <<  0) // (ST) Watchdog Timer Restart
// -------- ST_WDMR : (ST Offset: 0x8) System Timer Watchdog Mode Register -------- 
#define AT91C_ST_WDV              (0xFFFF <<  0) // (ST) Watchdog Timer Restart
#define AT91C_ST_RSTEN            (0x1 << 16) // (ST) Reset Enable
#define AT91C_ST_EXTEN            (0x1 << 17) // (ST) External Signal Assertion Enable
// -------- ST_RTMR : (ST Offset: 0xc) System Timer Real-time Mode Register -------- 
#define AT91C_ST_RTPRES           (0xFFFF <<  0) // (ST) Real-time Timer Prescaler Value
// -------- ST_SR : (ST Offset: 0x10) System Timer Status Register -------- 
#define AT91C_ST_PITS             (0x1 <<  0) // (ST) Period Interval Timer Interrupt
#define AT91C_ST_WDOVF            (0x1 <<  1) // (ST) Watchdog Overflow
#define AT91C_ST_RTTINC           (0x1 <<  2) // (ST) Real-time Timer Increment
#define AT91C_ST_ALMS             (0x1 <<  3) // (ST) Alarm Status
// -------- ST_IER : (ST Offset: 0x14) System Timer Interrupt Enable Register -------- 
// -------- ST_IDR : (ST Offset: 0x18) System Timer Interrupt Disable Register -------- 
// -------- ST_IMR : (ST Offset: 0x1c) System Timer Interrupt Mask Register -------- 
// -------- ST_RTAR : (ST Offset: 0x20) System Timer Real-time Alarm Register -------- 
#define AT91C_ST_ALMV             (0xFFFFF <<  0) // (ST) Alarm Value Value
// -------- ST_CRTR : (ST Offset: 0x24) System Timer Current Real-time Register -------- 
#define AT91C_ST_CRTV             (0xFFFFF <<  0) // (ST) Current Real-time Value

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Power Management Controler
// *****************************************************************************
// *** Register offset in AT91S_PMC structure ***
#define PMC_SCER        ( 0) // System Clock Enable Register
#define PMC_SCDR        ( 4) // System Clock Disable Register
#define PMC_SCSR        ( 8) // System Clock Status Register
#define PMC_PCER        (16) // Peripheral Clock Enable Register
#define PMC_PCDR        (20) // Peripheral Clock Disable Register
#define PMC_PCSR        (24) // Peripheral Clock Status Register
#define PMC_MCKR        (48) // Master Clock Register
#define PMC_PCKR        (64) // Programmable Clock Register
#define PMC_IER         (96) // Interrupt Enable Register
#define PMC_IDR         (100) // Interrupt Disable Register
#define PMC_SR          (104) // Status Register
#define PMC_IMR         (108) // Interrupt Mask Register
// -------- PMC_SCER : (PMC Offset: 0x0) System Clock Enable Register -------- 
#define AT91C_PMC_PCK             (0x1 <<  0) // (PMC) Processor Clock
#define AT91C_PMC_UDP             (0x1 <<  1) // (PMC) USB Device Port Clock
#define AT91C_PMC_MCKUDP          (0x1 <<  2) // (PMC) USB Device Port Master Clock Automatic Disable on Suspend
#define AT91C_PMC_UHP             (0x1 <<  4) // (PMC) USB Host Port Clock
#define AT91C_PMC_PCK0            (0x1 <<  8) // (PMC) Programmable Clock Output
#define AT91C_PMC_PCK1            (0x1 <<  9) // (PMC) Programmable Clock Output
#define AT91C_PMC_PCK2            (0x1 << 10) // (PMC) Programmable Clock Output
#define AT91C_PMC_PCK3            (0x1 << 11) // (PMC) Programmable Clock Output
#define AT91C_PMC_PCK4            (0x1 << 12) // (PMC) Programmable Clock Output
#define AT91C_PMC_PCK5            (0x1 << 13) // (PMC) Programmable Clock Output
#define AT91C_PMC_PCK6            (0x1 << 14) // (PMC) Programmable Clock Output
#define AT91C_PMC_PCK7            (0x1 << 15) // (PMC) Programmable Clock Output
// -------- PMC_SCDR : (PMC Offset: 0x4) System Clock Disable Register -------- 
// -------- PMC_SCSR : (PMC Offset: 0x8) System Clock Status Register -------- 
// -------- PMC_MCKR : (PMC Offset: 0x30) Master Clock Register -------- 
#define AT91C_PMC_CSS             (0x3 <<  0) // (PMC) Programmable Clock Selection
#define 	AT91C_PMC_CSS_SLOW_CLK             (0x0) // (PMC) Slow Clock is selected
#define 	AT91C_PMC_CSS_MAIN_CLK             (0x1) // (PMC) Main Clock is selected
#define 	AT91C_PMC_CSS_PLLA_CLK             (0x2) // (PMC) Clock from PLL A is selected
#define 	AT91C_PMC_CSS_PLLB_CLK             (0x3) // (PMC) Clock from PLL B is selected
#define AT91C_PMC_PRES            (0x7 <<  2) // (PMC) Programmable Clock Prescaler
#define 	AT91C_PMC_PRES_CLK                  (0x0 <<  2) // (PMC) Selected clock
#define 	AT91C_PMC_PRES_CLK_2                (0x1 <<  2) // (PMC) Selected clock divided by 2
#define 	AT91C_PMC_PRES_CLK_4                (0x2 <<  2) // (PMC) Selected clock divided by 4
#define 	AT91C_PMC_PRES_CLK_8                (0x3 <<  2) // (PMC) Selected clock divided by 8
#define 	AT91C_PMC_PRES_CLK_16               (0x4 <<  2) // (PMC) Selected clock divided by 16
#define 	AT91C_PMC_PRES_CLK_32               (0x5 <<  2) // (PMC) Selected clock divided by 32
#define 	AT91C_PMC_PRES_CLK_64               (0x6 <<  2) // (PMC) Selected clock divided by 64
#define AT91C_PMC_MDIV            (0x3 <<  8) // (PMC) Master Clock Division
#define 	AT91C_PMC_MDIV_1                    (0x0 <<  8) // (PMC) The master clock and the processor clock are the same
#define 	AT91C_PMC_MDIV_2                    (0x1 <<  8) // (PMC) The processor clock is twice as fast as the master clock
#define 	AT91C_PMC_MDIV_3                    (0x2 <<  8) // (PMC) The processor clock is three times faster than the master clock
#define 	AT91C_PMC_MDIV_4                    (0x3 <<  8) // (PMC) The processor clock is four times faster than the master clock
// -------- PMC_PCKR : (PMC Offset: 0x40) Programmable Clock Register -------- 
// -------- PMC_IER : (PMC Offset: 0x60) PMC Interrupt Enable Register -------- 
#define AT91C_PMC_MOSCS           (0x1 <<  0) // (PMC) MOSC Status/Enable/Disable/Mask
#define AT91C_PMC_LOCKA           (0x1 <<  1) // (PMC) PLL A Status/Enable/Disable/Mask
#define AT91C_PMC_LOCKB           (0x1 <<  2) // (PMC) PLL B Status/Enable/Disable/Mask
#define AT91C_PMC_MCKRDY          (0x1 <<  3) // (PMC) MCK_RDY Status/Enable/Disable/Mask
#define AT91C_PMC_PCK0RDY         (0x1 <<  8) // (PMC) PCK0_RDY Status/Enable/Disable/Mask
#define AT91C_PMC_PCK1RDY         (0x1 <<  9) // (PMC) PCK1_RDY Status/Enable/Disable/Mask
#define AT91C_PMC_PCK2RDY         (0x1 << 10) // (PMC) PCK2_RDY Status/Enable/Disable/Mask
#define AT91C_PMC_PCK3RDY         (0x1 << 11) // (PMC) PCK3_RDY Status/Enable/Disable/Mask
#define AT91C_PMC_PCK4RDY         (0x1 << 12) // (PMC) PCK4_RDY Status/Enable/Disable/Mask
#define AT91C_PMC_PCK5RDY         (0x1 << 13) // (PMC) PCK5_RDY Status/Enable/Disable/Mask
#define AT91C_PMC_PCK6RDY         (0x1 << 14) // (PMC) PCK6_RDY Status/Enable/Disable/Mask
#define AT91C_PMC_PCK7RDY         (0x1 << 15) // (PMC) PCK7_RDY Status/Enable/Disable/Mask
// -------- PMC_IDR : (PMC Offset: 0x64) PMC Interrupt Disable Register -------- 
// -------- PMC_SR : (PMC Offset: 0x68) PMC Status Register -------- 
// -------- PMC_IMR : (PMC Offset: 0x6c) PMC Interrupt Mask Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Clock Generator Controler
// *****************************************************************************
// *** Register offset in AT91S_CKGR structure ***
#define CKGR_MOR        ( 0) // Main Oscillator Register
#define CKGR_MCFR       ( 4) // Main Clock  Frequency Register
#define CKGR_PLLAR      ( 8) // PLL A Register
#define CKGR_PLLBR      (12) // PLL B Register
// -------- CKGR_MOR : (CKGR Offset: 0x0) Main Oscillator Register -------- 
#define AT91C_CKGR_MOSCEN         (0x1 <<  0) // (CKGR) Main Oscillator Enable
#define AT91C_CKGR_OSCTEST        (0x1 <<  1) // (CKGR) Oscillator Test
#define AT91C_CKGR_OSCOUNT        (0xFF <<  8) // (CKGR) Main Oscillator Start-up Time
// -------- CKGR_MCFR : (CKGR Offset: 0x4) Main Clock Frequency Register -------- 
#define AT91C_CKGR_MAINF          (0xFFFF <<  0) // (CKGR) Main Clock Frequency
#define AT91C_CKGR_MAINRDY        (0x1 << 16) // (CKGR) Main Clock Ready
// -------- CKGR_PLLAR : (CKGR Offset: 0x8) PLL A Register -------- 
#define AT91C_CKGR_DIVA           (0xFF <<  0) // (CKGR) Divider Selected
#define 	AT91C_CKGR_DIVA_0                    (0x0) // (CKGR) Divider output is 0
#define 	AT91C_CKGR_DIVA_BYPASS               (0x1) // (CKGR) Divider is bypassed
#define AT91C_CKGR_PLLACOUNT      (0x3F <<  8) // (CKGR) PLL A Counter
#define AT91C_CKGR_OUTA           (0x3 << 14) // (CKGR) PLL A Output Frequency Range
#define 	AT91C_CKGR_OUTA_0                    (0x0 << 14) // (CKGR) Please refer to the PLLA datasheet
#define 	AT91C_CKGR_OUTA_1                    (0x1 << 14) // (CKGR) Please refer to the PLLA datasheet
#define 	AT91C_CKGR_OUTA_2                    (0x2 << 14) // (CKGR) Please refer to the PLLA datasheet
#define 	AT91C_CKGR_OUTA_3                    (0x3 << 14) // (CKGR) Please refer to the PLLA datasheet
#define AT91C_CKGR_MULA           (0x7FF << 16) // (CKGR) PLL A Multiplier
#define AT91C_CKGR_SRCA           (0x1 << 29) // (CKGR) PLL A Source
// -------- CKGR_PLLBR : (CKGR Offset: 0xc) PLL B Register -------- 
#define AT91C_CKGR_DIVB           (0xFF <<  0) // (CKGR) Divider Selected
#define 	AT91C_CKGR_DIVB_0                    (0x0) // (CKGR) Divider output is 0
#define 	AT91C_CKGR_DIVB_BYPASS               (0x1) // (CKGR) Divider is bypassed
#define AT91C_CKGR_PLLBCOUNT      (0x3F <<  8) // (CKGR) PLL B Counter
#define AT91C_CKGR_OUTB           (0x3 << 14) // (CKGR) PLL B Output Frequency Range
#define 	AT91C_CKGR_OUTB_0                    (0x0 << 14) // (CKGR) Please refer to the PLLB datasheet
#define 	AT91C_CKGR_OUTB_1                    (0x1 << 14) // (CKGR) Please refer to the PLLB datasheet
#define 	AT91C_CKGR_OUTB_2                    (0x2 << 14) // (CKGR) Please refer to the PLLB datasheet
#define 	AT91C_CKGR_OUTB_3                    (0x3 << 14) // (CKGR) Please refer to the PLLB datasheet
#define AT91C_CKGR_MULB           (0x7FF << 16) // (CKGR) PLL B Multiplier
#define AT91C_CKGR_USB_96M        (0x1 << 28) // (CKGR) Divider for USB Ports
#define AT91C_CKGR_USB_PLL        (0x1 << 29) // (CKGR) PLL Use

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Parallel Input Output Controler
// *****************************************************************************
// *** Register offset in AT91S_PIO structure ***
#define PIO_PER         ( 0) // PIO Enable Register
#define PIO_PDR         ( 4) // PIO Disable Register
#define PIO_PSR         ( 8) // PIO Status Register
#define PIO_OER         (16) // Output Enable Register
#define PIO_ODR         (20) // Output Disable Registerr
#define PIO_OSR         (24) // Output Status Register
#define PIO_IFER        (32) // Input Filter Enable Register
#define PIO_IFDR        (36) // Input Filter Disable Register
#define PIO_IFSR        (40) // Input Filter Status Register
#define PIO_SODR        (48) // Set Output Data Register
#define PIO_CODR        (52) // Clear Output Data Register
#define PIO_ODSR        (56) // Output Data Status Register
#define PIO_PDSR        (60) // Pin Data Status Register
#define PIO_IER         (64) // Interrupt Enable Register
#define PIO_IDR         (68) // Interrupt Disable Register
#define PIO_IMR         (72) // Interrupt Mask Register
#define PIO_ISR         (76) // Interrupt Status Register
#define PIO_MDER        (80) // Multi-driver Enable Register
#define PIO_MDDR        (84) // Multi-driver Disable Register
#define PIO_MDSR        (88) // Multi-driver Status Register
#define PIO_PPUDR       (96) // Pull-up Disable Register
#define PIO_PPUER       (100) // Pull-up Enable Register
#define PIO_PPUSR       (104) // Pad Pull-up Status Register
#define PIO_ASR         (112) // Select A Register
#define PIO_BSR         (116) // Select B Register
#define PIO_ABSR        (120) // AB Select Status Register
#define PIO_OWER        (160) // Output Write Enable Register
#define PIO_OWDR        (164) // Output Write Disable Register
#define PIO_OWSR        (168) // Output Write Status Register

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Debug Unit
// *****************************************************************************
// *** Register offset in AT91S_DBGU structure ***
#define DBGU_CR         ( 0) // Control Register
#define DBGU_MR         ( 4) // Mode Register
#define DBGU_IER        ( 8) // Interrupt Enable Register
#define DBGU_IDR        (12) // Interrupt Disable Register
#define DBGU_IMR        (16) // Interrupt Mask Register
#define DBGU_CSR        (20) // Channel Status Register
#define DBGU_RHR        (24) // Receiver Holding Register
#define DBGU_THR        (28) // Transmitter Holding Register
#define DBGU_BRGR       (32) // Baud Rate Generator Register
#define DBGU_C1R        (64) // Chip ID1 Register
#define DBGU_C2R        (68) // Chip ID2 Register
#define DBGU_FNTR       (72) // Force NTRST Register
#define DBGU_RPR        (256) // Receive Pointer Register
#define DBGU_RCR        (260) // Receive Counter Register
#define DBGU_TPR        (264) // Transmit Pointer Register
#define DBGU_TCR        (268) // Transmit Counter Register
#define DBGU_RNPR       (272) // Receive Next Pointer Register
#define DBGU_RNCR       (276) // Receive Next Counter Register
#define DBGU_TNPR       (280) // Transmit Next Pointer Register
#define DBGU_TNCR       (284) // Transmit Next Counter Register
#define DBGU_PTCR       (288) // PDC Transfer Control Register
#define DBGU_PTSR       (292) // PDC Transfer Status Register
// -------- DBGU_CR : (DBGU Offset: 0x0) Debug Unit Control Register -------- 
#define AT91C_US_RSTRX            (0x1 <<  2) // (DBGU) Reset Receiver
#define AT91C_US_RSTTX            (0x1 <<  3) // (DBGU) Reset Transmitter
#define AT91C_US_RXEN             (0x1 <<  4) // (DBGU) Receiver Enable
#define AT91C_US_RXDIS            (0x1 <<  5) // (DBGU) Receiver Disable
#define AT91C_US_TXEN             (0x1 <<  6) // (DBGU) Transmitter Enable
#define AT91C_US_TXDIS            (0x1 <<  7) // (DBGU) Transmitter Disable
// -------- DBGU_MR : (DBGU Offset: 0x4) Debug Unit Mode Register -------- 
#define AT91C_US_PAR              (0x7 <<  9) // (DBGU) Parity type
#define 	AT91C_US_PAR_EVEN                 (0x0 <<  9) // (DBGU) Even Parity
#define 	AT91C_US_PAR_ODD                  (0x1 <<  9) // (DBGU) Odd Parity
#define 	AT91C_US_PAR_SPACE                (0x2 <<  9) // (DBGU) Parity forced to 0 (Space)
#define 	AT91C_US_PAR_MARK                 (0x3 <<  9) // (DBGU) Parity forced to 1 (Mark)
#define 	AT91C_US_PAR_NONE                 (0x4 <<  9) // (DBGU) No Parity
#define 	AT91C_US_PAR_MULTI_DROP           (0x6 <<  9) // (DBGU) Multi-drop mode
#define AT91C_US_CHMODE           (0x3 << 14) // (DBGU) Channel Mode
#define 	AT91C_US_CHMODE_NORMAL               (0x0 << 14) // (DBGU) Normal Mode: The USART channel operates as an RX/TX USART.
#define 	AT91C_US_CHMODE_AUTO                 (0x1 << 14) // (DBGU) Automatic Echo: Receiver Data Input is connected to the TXD pin.
#define 	AT91C_US_CHMODE_LOCAL                (0x2 << 14) // (DBGU) Local Loopback: Transmitter Output Signal is connected to Receiver Input Signal.
#define 	AT91C_US_CHMODE_REMOTE               (0x3 << 14) // (DBGU) Remote Loopback: RXD pin is internally connected to TXD pin.
// -------- DBGU_IER : (DBGU Offset: 0x8) Debug Unit Interrupt Enable Register -------- 
#define AT91C_US_RXRDY            (0x1 <<  0) // (DBGU) RXRDY Interrupt
#define AT91C_US_TXRDY            (0x1 <<  1) // (DBGU) TXRDY Interrupt
#define AT91C_US_ENDRX            (0x1 <<  3) // (DBGU) End of Receive Transfer Interrupt
#define AT91C_US_ENDTX            (0x1 <<  4) // (DBGU) End of Transmit Interrupt
#define AT91C_US_OVRE             (0x1 <<  5) // (DBGU) Overrun Interrupt
#define AT91C_US_FRAME            (0x1 <<  6) // (DBGU) Framing Error Interrupt
#define AT91C_US_PARE             (0x1 <<  7) // (DBGU) Parity Error Interrupt
#define AT91C_US_TXEMPTY          (0x1 <<  9) // (DBGU) TXEMPTY Interrupt
#define AT91C_US_TXBUFE           (0x1 << 11) // (DBGU) TXBUFE Interrupt
#define AT91C_US_RXBUFF           (0x1 << 12) // (DBGU) RXBUFF Interrupt
#define AT91C_US_COMM_TX          (0x1 << 30) // (DBGU) COMM_TX Interrupt
#define AT91C_US_COMM_RX          (0x1 << 31) // (DBGU) COMM_RX Interrupt
// -------- DBGU_IDR : (DBGU Offset: 0xc) Debug Unit Interrupt Disable Register -------- 
// -------- DBGU_IMR : (DBGU Offset: 0x10) Debug Unit Interrupt Mask Register -------- 
// -------- DBGU_CSR : (DBGU Offset: 0x14) Debug Unit Channel Status Register -------- 
// -------- DBGU_FNTR : (DBGU Offset: 0x48) Debug Unit FORCE_NTRST Register -------- 
#define AT91C_US_FORCE_NTRST      (0x1 <<  0) // (DBGU) Force NTRST in JTAG

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Peripheral Data Controller
// *****************************************************************************
// *** Register offset in AT91S_PDC structure ***
#define PDC_RPR         ( 0) // Receive Pointer Register
#define PDC_RCR         ( 4) // Receive Counter Register
#define PDC_TPR         ( 8) // Transmit Pointer Register
#define PDC_TCR         (12) // Transmit Counter Register
#define PDC_RNPR        (16) // Receive Next Pointer Register
#define PDC_RNCR        (20) // Receive Next Counter Register
#define PDC_TNPR        (24) // Transmit Next Pointer Register
#define PDC_TNCR        (28) // Transmit Next Counter Register
#define PDC_PTCR        (32) // PDC Transfer Control Register
#define PDC_PTSR        (36) // PDC Transfer Status Register
// -------- PDC_PTCR : (PDC Offset: 0x20) PDC Transfer Control Register -------- 
#define AT91C_PDC_RXTEN           (0x1 <<  0) // (PDC) Receiver Transfer Enable
#define AT91C_PDC_RXTDIS          (0x1 <<  1) // (PDC) Receiver Transfer Disable
#define AT91C_PDC_TXTEN           (0x1 <<  8) // (PDC) Transmitter Transfer Enable
#define AT91C_PDC_TXTDIS          (0x1 <<  9) // (PDC) Transmitter Transfer Disable
// -------- PDC_PTSR : (PDC Offset: 0x24) PDC Transfer Status Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Advanced Interrupt Controller
// *****************************************************************************
// *** Register offset in AT91S_AIC structure ***
#define AIC_SMR         ( 0) // Source Mode Register
#define AIC_SVR         (128) // Source Vector Register
#define AIC_IVR         (256) // IRQ Vector Register
#define AIC_FVR         (260) // FIQ Vector Register
#define AIC_ISR         (264) // Interrupt Status Register
#define AIC_IPR         (268) // Interrupt Pending Register
#define AIC_IMR         (272) // Interrupt Mask Register
#define AIC_CISR        (276) // Core Interrupt Status Register
#define AIC_IECR        (288) // Interrupt Enable Command Register
#define AIC_IDCR        (292) // Interrupt Disable Command Register
#define AIC_ICCR        (296) // Interrupt Clear Command Register
#define AIC_ISCR        (300) // Interrupt Set Command Register
#define AIC_EOICR       (304) // End of Interrupt Command Register
#define AIC_SPU         (308) // Spurious Vector Register
#define AIC_DCR         (312) // Debug Control Register (Protect)
#define AIC_FFER        (320) // Fast Forcing Enable Register
#define AIC_FFDR        (324) // Fast Forcing Disable Register
#define AIC_FFSR        (328) // Fast Forcing Status Register
// -------- AIC_SMR : (AIC Offset: 0x0) Control Register -------- 
#define AT91C_AIC_PRIOR           (0x7 <<  0) // (AIC) Priority Level
#define 	AT91C_AIC_PRIOR_LOWEST               (0x0) // (AIC) Lowest priority level
#define 	AT91C_AIC_PRIOR_HIGHEST              (0x7) // (AIC) Highest priority level
#define AT91C_AIC_SRCTYPE         (0x3 <<  5) // (AIC) Interrupt Source Type
#define 	AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE  (0x0 <<  5) // (AIC) Internal Sources Code Label Level Sensitive
#define 	AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED   (0x1 <<  5) // (AIC) Internal Sources Code Label Edge triggered
#define 	AT91C_AIC_SRCTYPE_EXT_HIGH_LEVEL       (0x2 <<  5) // (AIC) External Sources Code Label High-level Sensitive
#define 	AT91C_AIC_SRCTYPE_EXT_POSITIVE_EDGE    (0x3 <<  5) // (AIC) External Sources Code Label Positive Edge triggered
// -------- AIC_CISR : (AIC Offset: 0x114) AIC Core Interrupt Status Register -------- 
#define AT91C_AIC_NFIQ            (0x1 <<  0) // (AIC) NFIQ Status
#define AT91C_AIC_NIRQ            (0x1 <<  1) // (AIC) NIRQ Status
// -------- AIC_DCR : (AIC Offset: 0x138) AIC Debug Control Register (Protect) -------- 
#define AT91C_AIC_DCR_PROT        (0x1 <<  0) // (AIC) Protection Mode
#define AT91C_AIC_DCR_GMSK        (0x1 <<  1) // (AIC) General Mask

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Serial Parallel Interface
// *****************************************************************************
// *** Register offset in AT91S_SPI structure ***
#define SPI_CR          ( 0) // Control Register
#define SPI_MR          ( 4) // Mode Register
#define SPI_RDR         ( 8) // Receive Data Register
#define SPI_TDR         (12) // Transmit Data Register
#define SPI_SR          (16) // Status Register
#define SPI_IER         (20) // Interrupt Enable Register
#define SPI_IDR         (24) // Interrupt Disable Register
#define SPI_IMR         (28) // Interrupt Mask Register
#define SPI_CSR         (48) // Chip Select Register
#define SPI_RPR         (256) // Receive Pointer Register
#define SPI_RCR         (260) // Receive Counter Register
#define SPI_TPR         (264) // Transmit Pointer Register
#define SPI_TCR         (268) // Transmit Counter Register
#define SPI_RNPR        (272) // Receive Next Pointer Register
#define SPI_RNCR        (276) // Receive Next Counter Register
#define SPI_TNPR        (280) // Transmit Next Pointer Register
#define SPI_TNCR        (284) // Transmit Next Counter Register
#define SPI_PTCR        (288) // PDC Transfer Control Register
#define SPI_PTSR        (292) // PDC Transfer Status Register
// -------- SPI_CR : (SPI Offset: 0x0) SPI Control Register -------- 
#define AT91C_SPI_SPIEN           (0x1 <<  0) // (SPI) SPI Enable
#define AT91C_SPI_SPIDIS          (0x1 <<  1) // (SPI) SPI Disable
#define AT91C_SPI_SWRST           (0x1 <<  7) // (SPI) SPI Software reset
// -------- SPI_MR : (SPI Offset: 0x4) SPI Mode Register -------- 
#define AT91C_SPI_MSTR            (0x1 <<  0) // (SPI) Master/Slave Mode
#define AT91C_SPI_PS              (0x1 <<  1) // (SPI) Peripheral Select
#define 	AT91C_SPI_PS_FIXED                (0x0 <<  1) // (SPI) Fixed Peripheral Select
#define 	AT91C_SPI_PS_VARIABLE             (0x1 <<  1) // (SPI) Variable Peripheral Select
#define AT91C_SPI_PCSDEC          (0x1 <<  2) // (SPI) Chip Select Decode
#define AT91C_SPI_DIV32           (0x1 <<  3) // (SPI) Clock Selection
#define AT91C_SPI_MODFDIS         (0x1 <<  4) // (SPI) Mode Fault Detection
#define AT91C_SPI_LLB             (0x1 <<  7) // (SPI) Clock Selection
#define AT91C_SPI_PCS             (0xF << 16) // (SPI) Peripheral Chip Select
#define AT91C_SPI_DLYBCS          (0xFF << 24) // (SPI) Delay Between Chip Selects
// -------- SPI_RDR : (SPI Offset: 0x8) Receive Data Register -------- 
#define AT91C_SPI_RD              (0xFFFF <<  0) // (SPI) Receive Data
#define AT91C_SPI_RPCS            (0xF << 16) // (SPI) Peripheral Chip Select Status
// -------- SPI_TDR : (SPI Offset: 0xc) Transmit Data Register -------- 
#define AT91C_SPI_TD              (0xFFFF <<  0) // (SPI) Transmit Data
#define AT91C_SPI_TPCS            (0xF << 16) // (SPI) Peripheral Chip Select Status
// -------- SPI_SR : (SPI Offset: 0x10) Status Register -------- 
#define AT91C_SPI_RDRF            (0x1 <<  0) // (SPI) Receive Data Register Full
#define AT91C_SPI_TDRE            (0x1 <<  1) // (SPI) Transmit Data Register Empty
#define AT91C_SPI_MODF            (0x1 <<  2) // (SPI) Mode Fault Error
#define AT91C_SPI_OVRES           (0x1 <<  3) // (SPI) Overrun Error Status
#define AT91C_SPI_SPENDRX         (0x1 <<  4) // (SPI) End of Receiver Transfer
#define AT91C_SPI_SPENDTX         (0x1 <<  5) // (SPI) End of Receiver Transfer
#define AT91C_SPI_RXBUFF          (0x1 <<  6) // (SPI) RXBUFF Interrupt
#define AT91C_SPI_TXBUFE          (0x1 <<  7) // (SPI) TXBUFE Interrupt
#define AT91C_SPI_SPIENS          (0x1 << 16) // (SPI) Enable Status
// -------- SPI_IER : (SPI Offset: 0x14) Interrupt Enable Register -------- 
// -------- SPI_IDR : (SPI Offset: 0x18) Interrupt Disable Register -------- 
// -------- SPI_IMR : (SPI Offset: 0x1c) Interrupt Mask Register -------- 
// -------- SPI_CSR : (SPI Offset: 0x30) Chip Select Register -------- 
#define AT91C_SPI_CPOL            (0x1 <<  0) // (SPI) Clock Polarity
#define AT91C_SPI_NCPHA           (0x1 <<  1) // (SPI) Clock Phase
#define AT91C_SPI_BITS            (0xF <<  4) // (SPI) Bits Per Transfer
#define 	AT91C_SPI_BITS_8                    (0x0 <<  4) // (SPI) 8 Bits Per transfer
#define 	AT91C_SPI_BITS_9                    (0x1 <<  4) // (SPI) 9 Bits Per transfer
#define 	AT91C_SPI_BITS_10                   (0x2 <<  4) // (SPI) 10 Bits Per transfer
#define 	AT91C_SPI_BITS_11                   (0x3 <<  4) // (SPI) 11 Bits Per transfer
#define 	AT91C_SPI_BITS_12                   (0x4 <<  4) // (SPI) 12 Bits Per transfer
#define 	AT91C_SPI_BITS_13                   (0x5 <<  4) // (SPI) 13 Bits Per transfer
#define 	AT91C_SPI_BITS_14                   (0x6 <<  4) // (SPI) 14 Bits Per transfer
#define 	AT91C_SPI_BITS_15                   (0x7 <<  4) // (SPI) 15 Bits Per transfer
#define 	AT91C_SPI_BITS_16                   (0x8 <<  4) // (SPI) 16 Bits Per transfer
#define AT91C_SPI_SCBR            (0xFF <<  8) // (SPI) Serial Clock Baud Rate
#define AT91C_SPI_DLYBS           (0xFF << 16) // (SPI) Serial Clock Baud Rate
#define AT91C_SPI_DLYBCT          (0xFF << 24) // (SPI) Delay Between Consecutive Transfers

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Synchronous Serial Controller Interface
// *****************************************************************************
// *** Register offset in AT91S_SSC structure ***
#define SSC_CR          ( 0) // Control Register
#define SSC_CMR         ( 4) // Clock Mode Register
#define SSC_RCMR        (16) // Receive Clock ModeRegister
#define SSC_RFMR        (20) // Receive Frame Mode Register
#define SSC_TCMR        (24) // Transmit Clock Mode Register
#define SSC_TFMR        (28) // Transmit Frame Mode Register
#define SSC_RHR         (32) // Receive Holding Register
#define SSC_THR         (36) // Transmit Holding Register
#define SSC_RSHR        (48) // Receive Sync Holding Register
#define SSC_TSHR        (52) // Transmit Sync Holding Register
#define SSC_RC0R        (56) // Receive Compare 0 Register
#define SSC_RC1R        (60) // Receive Compare 1 Register
#define SSC_SR          (64) // Status Register
#define SSC_IER         (68) // Interrupt Enable Register
#define SSC_IDR         (72) // Interrupt Disable Register
#define SSC_IMR         (76) // Interrupt Mask Register
#define SSC_RPR         (256) // Receive Pointer Register
#define SSC_RCR         (260) // Receive Counter Register
#define SSC_TPR         (264) // Transmit Pointer Register
#define SSC_TCR         (268) // Transmit Counter Register
#define SSC_RNPR        (272) // Receive Next Pointer Register
#define SSC_RNCR        (276) // Receive Next Counter Register
#define SSC_TNPR        (280) // Transmit Next Pointer Register
#define SSC_TNCR        (284) // Transmit Next Counter Register
#define SSC_PTCR        (288) // PDC Transfer Control Register
#define SSC_PTSR        (292) // PDC Transfer Status Register
// -------- SSC_CR : (SSC Offset: 0x0) SSC Control Register -------- 
#define AT91C_SSC_RXEN            (0x1 <<  0) // (SSC) Receive Enable
#define AT91C_SSC_RXDIS           (0x1 <<  1) // (SSC) Receive Disable
#define AT91C_SSC_TXEN            (0x1 <<  8) // (SSC) Transmit Enable
#define AT91C_SSC_TXDIS           (0x1 <<  9) // (SSC) Transmit Disable
#define AT91C_SSC_SWRST           (0x1 << 15) // (SSC) Software Reset
// -------- SSC_RCMR : (SSC Offset: 0x10) SSC Receive Clock Mode Register -------- 
#define AT91C_SSC_CKS             (0x3 <<  0) // (SSC) Receive/Transmit Clock Selection
#define 	AT91C_SSC_CKS_DIV                  (0x0) // (SSC) Divided Clock
#define 	AT91C_SSC_CKS_TK                   (0x1) // (SSC) TK Clock signal
#define 	AT91C_SSC_CKS_RK                   (0x2) // (SSC) RK pin
#define AT91C_SSC_CKO             (0x7 <<  2) // (SSC) Receive/Transmit Clock Output Mode Selection
#define 	AT91C_SSC_CKO_NONE                 (0x0 <<  2) // (SSC) Receive/Transmit Clock Output Mode: None RK pin: Input-only
#define 	AT91C_SSC_CKO_CONTINOUS            (0x1 <<  2) // (SSC) Continuous Receive/Transmit Clock RK pin: Output
#define 	AT91C_SSC_CKO_DATA_TX              (0x2 <<  2) // (SSC) Receive/Transmit Clock only during data transfers RK pin: Output
#define AT91C_SSC_CKI             (0x1 <<  5) // (SSC) Receive/Transmit Clock Inversion
#define AT91C_SSC_CKG             (0x3 <<  6) // (SSC) Receive/Transmit Clock Gating Selection
#define 	AT91C_SSC_CKG_NONE                 (0x0 <<  6) // (SSC) Receive/Transmit Clock Gating: None, continuous clock
#define 	AT91C_SSC_CKG_LOW                  (0x1 <<  6) // (SSC) Receive/Transmit Clock enabled only if RF Low
#define 	AT91C_SSC_CKG_HIGH                 (0x2 <<  6) // (SSC) Receive/Transmit Clock enabled only if RF High
#define AT91C_SSC_START           (0xF <<  8) // (SSC) Receive/Transmit Start Selection
#define 	AT91C_SSC_START_CONTINOUS            (0x0 <<  8) // (SSC) Continuous, as soon as the receiver is enabled, and immediately after the end of transfer of the previous data.
#define 	AT91C_SSC_START_TX                   (0x1 <<  8) // (SSC) Transmit/Receive start
#define 	AT91C_SSC_START_LOW_RF               (0x2 <<  8) // (SSC) Detection of a low level on RF input
#define 	AT91C_SSC_START_HIGH_RF              (0x3 <<  8) // (SSC) Detection of a high level on RF input
#define 	AT91C_SSC_START_FALL_RF              (0x4 <<  8) // (SSC) Detection of a falling edge on RF input
#define 	AT91C_SSC_START_RISE_RF              (0x5 <<  8) // (SSC) Detection of a rising edge on RF input
#define 	AT91C_SSC_START_LEVEL_RF             (0x6 <<  8) // (SSC) Detection of any level change on RF input
#define 	AT91C_SSC_START_EDGE_RF              (0x7 <<  8) // (SSC) Detection of any edge on RF input
#define 	AT91C_SSC_START_0                    (0x8 <<  8) // (SSC) Compare 0
#define AT91C_SSC_STOP            (0x1 << 12) // (SSC) Receive Stop Selection
#define AT91C_SSC_STTOUT          (0x1 << 15) // (SSC) Receive/Transmit Start Output Selection
#define AT91C_SSC_STTDLY          (0xFF << 16) // (SSC) Receive/Transmit Start Delay
#define AT91C_SSC_PERIOD          (0xFF << 24) // (SSC) Receive/Transmit Period Divider Selection
// -------- SSC_RFMR : (SSC Offset: 0x14) SSC Receive Frame Mode Register -------- 
#define AT91C_SSC_DATLEN          (0x1F <<  0) // (SSC) Data Length
#define AT91C_SSC_LOOP            (0x1 <<  5) // (SSC) Loop Mode
#define AT91C_SSC_MSBF            (0x1 <<  7) // (SSC) Most Significant Bit First
#define AT91C_SSC_DATNB           (0xF <<  8) // (SSC) Data Number per Frame
#define AT91C_SSC_FSLEN           (0xF << 16) // (SSC) Receive/Transmit Frame Sync length
#define AT91C_SSC_FSOS            (0x7 << 20) // (SSC) Receive/Transmit Frame Sync Output Selection
#define 	AT91C_SSC_FSOS_NONE                 (0x0 << 20) // (SSC) Selected Receive/Transmit Frame Sync Signal: None RK pin Input-only
#define 	AT91C_SSC_FSOS_NEGATIVE             (0x1 << 20) // (SSC) Selected Receive/Transmit Frame Sync Signal: Negative Pulse
#define 	AT91C_SSC_FSOS_POSITIVE             (0x2 << 20) // (SSC) Selected Receive/Transmit Frame Sync Signal: Positive Pulse
#define 	AT91C_SSC_FSOS_LOW                  (0x3 << 20) // (SSC) Selected Receive/Transmit Frame Sync Signal: Driver Low during data transfer
#define 	AT91C_SSC_FSOS_HIGH                 (0x4 << 20) // (SSC) Selected Receive/Transmit Frame Sync Signal: Driver High during data transfer
#define 	AT91C_SSC_FSOS_TOGGLE               (0x5 << 20) // (SSC) Selected Receive/Transmit Frame Sync Signal: Toggling at each start of data transfer
#define AT91C_SSC_FSEDGE          (0x1 << 24) // (SSC) Frame Sync Edge Detection
// -------- SSC_TCMR : (SSC Offset: 0x18) SSC Transmit Clock Mode Register -------- 
// -------- SSC_TFMR : (SSC Offset: 0x1c) SSC Transmit Frame Mode Register -------- 
#define AT91C_SSC_DATDEF          (0x1 <<  5) // (SSC) Data Default Value
#define AT91C_SSC_FSDEN           (0x1 << 23) // (SSC) Frame Sync Data Enable
// -------- SSC_SR : (SSC Offset: 0x40) SSC Status Register -------- 
#define AT91C_SSC_TXRDY           (0x1 <<  0) // (SSC) Transmit Ready
#define AT91C_SSC_TXEMPTY         (0x1 <<  1) // (SSC) Transmit Empty
#define AT91C_SSC_ENDTX           (0x1 <<  2) // (SSC) End Of Transmission
#define AT91C_SSC_TXBUFE          (0x1 <<  3) // (SSC) Transmit Buffer Empty
#define AT91C_SSC_RXRDY           (0x1 <<  4) // (SSC) Receive Ready
#define AT91C_SSC_OVRUN           (0x1 <<  5) // (SSC) Receive Overrun
#define AT91C_SSC_ENDRX           (0x1 <<  6) // (SSC) End of Reception
#define AT91C_SSC_RXBUFF          (0x1 <<  7) // (SSC) Receive Buffer Full
#define AT91C_SSC_CP0             (0x1 <<  8) // (SSC) Compare 0
#define AT91C_SSC_CP1             (0x1 <<  9) // (SSC) Compare 1
#define AT91C_SSC_TXSYN           (0x1 << 10) // (SSC) Transmit Sync
#define AT91C_SSC_RXSYN           (0x1 << 11) // (SSC) Receive Sync
#define AT91C_SSC_TXENA           (0x1 << 16) // (SSC) Transmit Enable
#define AT91C_SSC_RXENA           (0x1 << 17) // (SSC) Receive Enable
// -------- SSC_IER : (SSC Offset: 0x44) SSC Interrupt Enable Register -------- 
// -------- SSC_IDR : (SSC Offset: 0x48) SSC Interrupt Disable Register -------- 
// -------- SSC_IMR : (SSC Offset: 0x4c) SSC Interrupt Mask Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Usart
// *****************************************************************************
// *** Register offset in AT91S_USART structure ***
#define US_CR           ( 0) // Control Register
#define US_MR           ( 4) // Mode Register
#define US_IER          ( 8) // Interrupt Enable Register
#define US_IDR          (12) // Interrupt Disable Register
#define US_IMR          (16) // Interrupt Mask Register
#define US_CSR          (20) // Channel Status Register
#define US_RHR          (24) // Receiver Holding Register
#define US_THR          (28) // Transmitter Holding Register
#define US_BRGR         (32) // Baud Rate Generator Register
#define US_RTOR         (36) // Receiver Time-out Register
#define US_TTGR         (40) // Transmitter Time-guard Register
#define US_FIDI         (64) // FI_DI_Ratio Register
#define US_NER          (68) // Nb Errors Register
#define US_XXR          (72) // XON_XOFF Register
#define US_IF           (76) // IRDA_FILTER Register
#define US_RPR          (256) // Receive Pointer Register
#define US_RCR          (260) // Receive Counter Register
#define US_TPR          (264) // Transmit Pointer Register
#define US_TCR          (268) // Transmit Counter Register
#define US_RNPR         (272) // Receive Next Pointer Register
#define US_RNCR         (276) // Receive Next Counter Register
#define US_TNPR         (280) // Transmit Next Pointer Register
#define US_TNCR         (284) // Transmit Next Counter Register
#define US_PTCR         (288) // PDC Transfer Control Register
#define US_PTSR         (292) // PDC Transfer Status Register
// -------- US_CR : (USART Offset: 0x0) Debug Unit Control Register -------- 
#define AT91C_US_RSTSTA           (0x1 <<  8) // (USART) Reset Status Bits
#define AT91C_US_STTBRK           (0x1 <<  9) // (USART) Start Break
#define AT91C_US_STPBRK           (0x1 << 10) // (USART) Stop Break
#define AT91C_US_STTTO            (0x1 << 11) // (USART) Start Time-out
#define AT91C_US_SENDA            (0x1 << 12) // (USART) Send Address
#define AT91C_US_RSTIT            (0x1 << 13) // (USART) Reset Iterations
#define AT91C_US_RSTNACK          (0x1 << 14) // (USART) Reset Non Acknowledge
#define AT91C_US_RETTO            (0x1 << 15) // (USART) Rearm Time-out
#define AT91C_US_DTREN            (0x1 << 16) // (USART) Data Terminal ready Enable
#define AT91C_US_DTRDIS           (0x1 << 17) // (USART) Data Terminal ready Disable
#define AT91C_US_RTSEN            (0x1 << 18) // (USART) Request to Send enable
#define AT91C_US_RTSDIS           (0x1 << 19) // (USART) Request to Send Disable
// -------- US_MR : (USART Offset: 0x4) Debug Unit Mode Register -------- 
#define AT91C_US_USMODE           (0xF <<  0) // (USART) Usart mode
#define 	AT91C_US_USMODE_NORMAL               (0x0) // (USART) Normal
#define 	AT91C_US_USMODE_RS485                (0x1) // (USART) RS485
#define 	AT91C_US_USMODE_HWHSH                (0x2) // (USART) Hardware Handshaking
#define 	AT91C_US_USMODE_MODEM                (0x3) // (USART) Modem
#define 	AT91C_US_USMODE_ISO7816_0            (0x4) // (USART) ISO7816 protocol: T = 0
#define 	AT91C_US_USMODE_ISO7816_1            (0x6) // (USART) ISO7816 protocol: T = 1
#define 	AT91C_US_USMODE_IRDA                 (0x8) // (USART) IrDA
#define 	AT91C_US_USMODE_SWHSH                (0xC) // (USART) Software Handshaking
#define AT91C_US_CLKS             (0x3 <<  4) // (USART) Clock Selection (Baud Rate generator Input Clock
#define 	AT91C_US_CLKS_CLOCK                (0x0 <<  4) // (USART) Clock
#define 	AT91C_US_CLKS_FDIV1                (0x1 <<  4) // (USART) fdiv1
#define 	AT91C_US_CLKS_SLOW                 (0x2 <<  4) // (USART) slow_clock (ARM)
#define 	AT91C_US_CLKS_EXT                  (0x3 <<  4) // (USART) External (SCK)
#define AT91C_US_CHRL             (0x3 <<  6) // (USART) Clock Selection (Baud Rate generator Input Clock
#define 	AT91C_US_CHRL_5_BITS               (0x0 <<  6) // (USART) Character Length: 5 bits
#define 	AT91C_US_CHRL_6_BITS               (0x1 <<  6) // (USART) Character Length: 6 bits
#define 	AT91C_US_CHRL_7_BITS               (0x2 <<  6) // (USART) Character Length: 7 bits
#define 	AT91C_US_CHRL_8_BITS               (0x3 <<  6) // (USART) Character Length: 8 bits
#define AT91C_US_SYNC             (0x1 <<  8) // (USART) Synchronous Mode Select
#define AT91C_US_NBSTOP           (0x3 << 12) // (USART) Number of Stop bits
#define 	AT91C_US_NBSTOP_1_BIT                (0x0 << 12) // (USART) 1 stop bit
#define 	AT91C_US_NBSTOP_15_BIT               (0x1 << 12) // (USART) Asynchronous (SYNC=0) 2 stop bits Synchronous (SYNC=1) 2 stop bits
#define 	AT91C_US_NBSTOP_2_BIT                (0x2 << 12) // (USART) 2 stop bits
#define AT91C_US_MSBF             (0x1 << 16) // (USART) Bit Order
#define AT91C_US_MODE9            (0x1 << 17) // (USART) 9-bit Character length
#define AT91C_US_CKLO             (0x1 << 18) // (USART) Clock Output Select
#define AT91C_US_OVER             (0x1 << 19) // (USART) Over Sampling Mode
#define AT91C_US_INACK            (0x1 << 20) // (USART) Inhibit Non Acknowledge
#define AT91C_US_DSNACK           (0x1 << 21) // (USART) Disable Successive NACK
#define AT91C_US_MAX_ITER         (0x1 << 24) // (USART) Number of Repetitions
#define AT91C_US_FILTER           (0x1 << 28) // (USART) Receive Line Filter
// -------- US_IER : (USART Offset: 0x8) Debug Unit Interrupt Enable Register -------- 
#define AT91C_US_RXBRK            (0x1 <<  2) // (USART) Break Received/End of Break
#define AT91C_US_TIMEOUT          (0x1 <<  8) // (USART) Receiver Time-out
#define AT91C_US_ITERATION        (0x1 << 10) // (USART) Max number of Repetitions Reached
#define AT91C_US_NACK             (0x1 << 13) // (USART) Non Acknowledge
#define AT91C_US_RIIC             (0x1 << 16) // (USART) Ring INdicator Input Change Flag
#define AT91C_US_DSRIC            (0x1 << 17) // (USART) Data Set Ready Input Change Flag
#define AT91C_US_DCDIC            (0x1 << 18) // (USART) Data Carrier Flag
#define AT91C_US_CTSIC            (0x1 << 19) // (USART) Clear To Send Input Change Flag
// -------- US_IDR : (USART Offset: 0xc) Debug Unit Interrupt Disable Register -------- 
// -------- US_IMR : (USART Offset: 0x10) Debug Unit Interrupt Mask Register -------- 
// -------- US_CSR : (USART Offset: 0x14) Debug Unit Channel Status Register -------- 
#define AT91C_US_RI               (0x1 << 20) // (USART) Image of RI Input
#define AT91C_US_DSR              (0x1 << 21) // (USART) Image of DSR Input
#define AT91C_US_DCD              (0x1 << 22) // (USART) Image of DCD Input
#define AT91C_US_CTS              (0x1 << 23) // (USART) Image of CTS Input

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Two-wire Interface
// *****************************************************************************
// *** Register offset in AT91S_TWI structure ***
#define TWI_CR          ( 0) // Control Register
#define TWI_MMR         ( 4) // Master Mode Register
#define TWI_SMR         ( 8) // Slave Mode Register
#define TWI_IADR        (12) // Internal Address Register
#define TWI_CWGR        (16) // Clock Waveform Generator Register
#define TWI_SR          (32) // Status Register
#define TWI_IER         (36) // Interrupt Enable Register
#define TWI_IDR         (40) // Interrupt Disable Register
#define TWI_IMR         (44) // Interrupt Mask Register
#define TWI_RHR         (48) // Receive Holding Register
#define TWI_THR         (52) // Transmit Holding Register
// -------- TWI_CR : (TWI Offset: 0x0) TWI Control Register -------- 
#define AT91C_TWI_START           (0x1 <<  0) // (TWI) Send a START Condition
#define AT91C_TWI_STOP            (0x1 <<  1) // (TWI) Send a STOP Condition
#define AT91C_TWI_MSEN            (0x1 <<  2) // (TWI) TWI Master Transfer Enabled
#define AT91C_TWI_MSDIS           (0x1 <<  3) // (TWI) TWI Master Transfer Disabled
#define AT91C_TWI_SVEN            (0x1 <<  4) // (TWI) TWI Slave Transfer Enabled
#define AT91C_TWI_SVDIS           (0x1 <<  5) // (TWI) TWI Slave Transfer Disabled
#define AT91C_TWI_SWRST           (0x1 <<  7) // (TWI) Software Reset
// -------- TWI_MMR : (TWI Offset: 0x4) TWI Master Mode Register -------- 
#define AT91C_TWI_IADRSZ          (0x3 <<  8) // (TWI) Internal Device Address Size
#define 	AT91C_TWI_IADRSZ_NO                   (0x0 <<  8) // (TWI) No internal device address
#define 	AT91C_TWI_IADRSZ_1_BYTE               (0x1 <<  8) // (TWI) One-byte internal device address
#define 	AT91C_TWI_IADRSZ_2_BYTE               (0x2 <<  8) // (TWI) Two-byte internal device address
#define 	AT91C_TWI_IADRSZ_3_BYTE               (0x3 <<  8) // (TWI) Three-byte internal device address
#define AT91C_TWI_MREAD           (0x1 << 12) // (TWI) Master Read Direction
#define AT91C_TWI_DADR            (0x7F << 16) // (TWI) Device Address
// -------- TWI_SMR : (TWI Offset: 0x8) TWI Slave Mode Register -------- 
#define AT91C_TWI_SADR            (0x7F << 16) // (TWI) Slave Device Address
// -------- TWI_CWGR : (TWI Offset: 0x10) TWI Clock Waveform Generator Register -------- 
#define AT91C_TWI_CLDIV           (0xFF <<  0) // (TWI) Clock Low Divider
#define AT91C_TWI_CHDIV           (0xFF <<  8) // (TWI) Clock High Divider
#define AT91C_TWI_CKDIV           (0x7 << 16) // (TWI) Clock Divider
// -------- TWI_SR : (TWI Offset: 0x20) TWI Status Register -------- 
#define AT91C_TWI_TXCOMP          (0x1 <<  0) // (TWI) Transmission Completed
#define AT91C_TWI_RXRDY           (0x1 <<  1) // (TWI) Receive holding register ReaDY
#define AT91C_TWI_TXRDY           (0x1 <<  2) // (TWI) Transmit holding register ReaDY
#define AT91C_TWI_SVREAD          (0x1 <<  3) // (TWI) Slave Read
#define AT91C_TWI_SVACC           (0x1 <<  4) // (TWI) Slave Access
#define AT91C_TWI_GCACC           (0x1 <<  5) // (TWI) General Call Access
#define AT91C_TWI_OVRE            (0x1 <<  6) // (TWI) Overrun Error
#define AT91C_TWI_UNRE            (0x1 <<  7) // (TWI) Underrun Error
#define AT91C_TWI_NACK            (0x1 <<  8) // (TWI) Not Acknowledged
#define AT91C_TWI_ARBLST          (0x1 <<  9) // (TWI) Arbitration Lost
// -------- TWI_IER : (TWI Offset: 0x24) TWI Interrupt Enable Register -------- 
// -------- TWI_IDR : (TWI Offset: 0x28) TWI Interrupt Disable Register -------- 
// -------- TWI_IMR : (TWI Offset: 0x2c) TWI Interrupt Mask Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Multimedia Card Interface
// *****************************************************************************
// *** Register offset in AT91S_MCI structure ***
#define MCI_CR          ( 0) // MCI Control Register
#define MCI_MR          ( 4) // MCI Mode Register
#define MCI_DTOR        ( 8) // MCI Data Timeout Register
#define MCI_SDCR        (12) // MCI SD Card Register
#define MCI_ARGR        (16) // MCI Argument Register
#define MCI_CMDR        (20) // MCI Command Register
#define MCI_RSPR        (32) // MCI Response Register
#define MCI_RDR         (48) // MCI Receive Data Register
#define MCI_TDR         (52) // MCI Transmit Data Register
#define MCI_SR          (64) // MCI Status Register
#define MCI_IER         (68) // MCI Interrupt Enable Register
#define MCI_IDR         (72) // MCI Interrupt Disable Register
#define MCI_IMR         (76) // MCI Interrupt Mask Register
#define MCI_RPR         (256) // Receive Pointer Register
#define MCI_RCR         (260) // Receive Counter Register
#define MCI_TPR         (264) // Transmit Pointer Register
#define MCI_TCR         (268) // Transmit Counter Register
#define MCI_RNPR        (272) // Receive Next Pointer Register
#define MCI_RNCR        (276) // Receive Next Counter Register
#define MCI_TNPR        (280) // Transmit Next Pointer Register
#define MCI_TNCR        (284) // Transmit Next Counter Register
#define MCI_PTCR        (288) // PDC Transfer Control Register
#define MCI_PTSR        (292) // PDC Transfer Status Register
// -------- MCI_CR : (MCI Offset: 0x0) MCI Control Register -------- 
#define AT91C_MCI_MCIEN           (0x1 <<  0) // (MCI) Multimedia Interface Enable
#define AT91C_MCI_MCIDIS          (0x1 <<  1) // (MCI) Multimedia Interface Disable
#define AT91C_MCI_PWSEN           (0x1 <<  2) // (MCI) Power Save Mode Enable
#define AT91C_MCI_PWSDIS          (0x1 <<  3) // (MCI) Power Save Mode Disable
// -------- MCI_MR : (MCI Offset: 0x4) MCI Mode Register -------- 
#define AT91C_MCI_CLKDIV          (0x1 <<  0) // (MCI) Clock Divider
#define AT91C_MCI_PWSDIV          (0x1 <<  8) // (MCI) Power Saving Divider
#define AT91C_MCI_PDCPADV         (0x1 << 14) // (MCI) PDC Padding Value
#define AT91C_MCI_PDCMODE         (0x1 << 15) // (MCI) PDC Oriented Mode
#define AT91C_MCI_BLKLEN          (0x1 << 18) // (MCI) Data Block Length
// -------- MCI_DTOR : (MCI Offset: 0x8) MCI Data Timeout Register -------- 
#define AT91C_MCI_DTOCYC          (0x1 <<  0) // (MCI) Data Timeout Cycle Number
#define AT91C_MCI_DTOMUL          (0x7 <<  4) // (MCI) Data Timeout Multiplier
#define 	AT91C_MCI_DTOMUL_1                    (0x0 <<  4) // (MCI) DTOCYC x 1
#define 	AT91C_MCI_DTOMUL_16                   (0x1 <<  4) // (MCI) DTOCYC x 16
#define 	AT91C_MCI_DTOMUL_128                  (0x2 <<  4) // (MCI) DTOCYC x 128
#define 	AT91C_MCI_DTOMUL_256                  (0x3 <<  4) // (MCI) DTOCYC x 256
#define 	AT91C_MCI_DTOMUL_1024                 (0x4 <<  4) // (MCI) DTOCYC x 1024
#define 	AT91C_MCI_DTOMUL_4096                 (0x5 <<  4) // (MCI) DTOCYC x 4096
#define 	AT91C_MCI_DTOMUL_65536                (0x6 <<  4) // (MCI) DTOCYC x 65536
#define 	AT91C_MCI_DTOMUL_1048576              (0x7 <<  4) // (MCI) DTOCYC x 1048576
// -------- MCI_SDCR : (MCI Offset: 0xc) MCI SD Card Register -------- 
#define AT91C_MCI_SCDSEL          (0x1 <<  0) // (MCI) SD Card Selector
#define AT91C_MCI_SCDBUS          (0x1 <<  7) // (MCI) SD Card Bus Width
// -------- MCI_CMDR : (MCI Offset: 0x14) MCI Command Register -------- 
#define AT91C_MCI_CMDNB           (0x1F <<  0) // (MCI) Command Number
#define AT91C_MCI_RSPTYP          (0x3 <<  6) // (MCI) Response Type
#define 	AT91C_MCI_RSPTYP_NO                   (0x0 <<  6) // (MCI) No response
#define 	AT91C_MCI_RSPTYP_48                   (0x1 <<  6) // (MCI) 48-bit response
#define 	AT91C_MCI_RSPTYP_136                  (0x2 <<  6) // (MCI) 136-bit response
#define AT91C_MCI_SPCMD           (0x7 <<  8) // (MCI) Special CMD
#define 	AT91C_MCI_SPCMD_NONE                 (0x0 <<  8) // (MCI) Not a special CMD
#define 	AT91C_MCI_SPCMD_INIT                 (0x1 <<  8) // (MCI) Initialization CMD
#define 	AT91C_MCI_SPCMD_SYNC                 (0x2 <<  8) // (MCI) Synchronized CMD
#define 	AT91C_MCI_SPCMD_IT_CMD               (0x4 <<  8) // (MCI) Interrupt command
#define 	AT91C_MCI_SPCMD_IT_REP               (0x5 <<  8) // (MCI) Interrupt response
#define AT91C_MCI_OPDCMD          (0x1 << 11) // (MCI) Open Drain Command
#define AT91C_MCI_MAXLAT          (0x1 << 12) // (MCI) Maximum Latency for Command to respond
#define AT91C_MCI_TRCMD           (0x3 << 16) // (MCI) Transfer CMD
#define 	AT91C_MCI_TRCMD_NO                   (0x0 << 16) // (MCI) No transfer
#define 	AT91C_MCI_TRCMD_START                (0x1 << 16) // (MCI) Start transfer
#define 	AT91C_MCI_TRCMD_STOP                 (0x2 << 16) // (MCI) Stop transfer
#define AT91C_MCI_TRDIR           (0x1 << 18) // (MCI) Transfer Direction
#define AT91C_MCI_TRTYP           (0x3 << 19) // (MCI) Transfer Type
#define 	AT91C_MCI_TRTYP_BLOCK                (0x0 << 19) // (MCI) Block Transfer type
#define 	AT91C_MCI_TRTYP_MULTIPLE             (0x1 << 19) // (MCI) Multiple Block transfer type
#define 	AT91C_MCI_TRTYP_STREAM               (0x2 << 19) // (MCI) Stream transfer type
// -------- MCI_SR : (MCI Offset: 0x40) MCI Status Register -------- 
#define AT91C_MCI_CMDRDY          (0x1 <<  0) // (MCI) Command Ready flag
#define AT91C_MCI_RXRDY           (0x1 <<  1) // (MCI) RX Ready flag
#define AT91C_MCI_TXRDY           (0x1 <<  2) // (MCI) TX Ready flag
#define AT91C_MCI_BLKE            (0x1 <<  3) // (MCI) Data Block Transfer Ended flag
#define AT91C_MCI_DTIP            (0x1 <<  4) // (MCI) Data Transfer in Progress flag
#define AT91C_MCI_NOTBUSY         (0x1 <<  5) // (MCI) Data Line Not Busy flag
#define AT91C_MCI_ENDRX           (0x1 <<  6) // (MCI) End of RX Buffer flag
#define AT91C_MCI_ENDTX           (0x1 <<  7) // (MCI) End of TX Buffer flag
#define AT91C_MCI_RXBUFF          (0x1 << 14) // (MCI) RX Buffer Full flag
#define AT91C_MCI_TXBUFE          (0x1 << 15) // (MCI) TX Buffer Empty flag
#define AT91C_MCI_RINDE           (0x1 << 16) // (MCI) Response Index Error flag
#define AT91C_MCI_RDIRE           (0x1 << 17) // (MCI) Response Direction Error flag
#define AT91C_MCI_RCRCE           (0x1 << 18) // (MCI) Response CRC Error flag
#define AT91C_MCI_RENDE           (0x1 << 19) // (MCI) Response End Bit Error flag
#define AT91C_MCI_RTOE            (0x1 << 20) // (MCI) Response Time-out Error flag
#define AT91C_MCI_DCRCE           (0x1 << 21) // (MCI) data CRC Error flag
#define AT91C_MCI_DTOE            (0x1 << 22) // (MCI) Data timeout Error flag
#define AT91C_MCI_OVRE            (0x1 << 30) // (MCI) Overrun flag
#define AT91C_MCI_UNRE            (0x1 << 31) // (MCI) Underrun flag
// -------- MCI_IER : (MCI Offset: 0x44) MCI Interrupt Enable Register -------- 
// -------- MCI_IDR : (MCI Offset: 0x48) MCI Interrupt Disable Register -------- 
// -------- MCI_IMR : (MCI Offset: 0x4c) MCI Interrupt Mask Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR USB Device Interface
// *****************************************************************************
// *** Register offset in AT91S_UDP structure ***
#define UDP_NUM         ( 0) // Frame Number Register
#define UDP_GLBSTATE    ( 4) // Global State Register
#define UDP_FADDR       ( 8) // Function Address Register
#define UDP_IER         (16) // Interrupt Enable Register
#define UDP_IDR         (20) // Interrupt Disable Register
#define UDP_IMR         (24) // Interrupt Mask Register
#define UDP_ISR         (28) // Interrupt Status Register
#define UDP_ICR         (32) // Interrupt Clear Register
#define UDP_RSTEP       (40) // Reset Endpoint Register
#define UDP_CSR         (48) // Endpoint Control and Status Register
#define UDP_FDR         (80) // Endpoint FIFO Data Register
// -------- UDP_FRM_NUM : (UDP Offset: 0x0) USB Frame Number Register -------- 
#define AT91C_UDP_FRM_NUM         (0x7FF <<  0) // (UDP) Frame Number as Defined in the Packet Field Formats
#define AT91C_UDP_FRM_ERR         (0x1 << 16) // (UDP) Frame Error
#define AT91C_UDP_FRM_OK          (0x1 << 17) // (UDP) Frame OK
// -------- UDP_GLB_STATE : (UDP Offset: 0x4) USB Global State Register -------- 
#define AT91C_UDP_FADDEN          (0x1 <<  0) // (UDP) Function Address Enable
#define AT91C_UDP_CONFG           (0x1 <<  1) // (UDP) Configured
#define AT91C_UDP_RMWUPE          (0x1 <<  2) // (UDP) Remote Wake Up Enable
#define AT91C_UDP_RSMINPR         (0x1 <<  3) // (UDP) A Resume Has Been Sent to the Host
// -------- UDP_FADDR : (UDP Offset: 0x8) USB Function Address Register -------- 
#define AT91C_UDP_FADD            (0xFF <<  0) // (UDP) Function Address Value
#define AT91C_UDP_FEN             (0x1 <<  8) // (UDP) Function Enable
// -------- UDP_IER : (UDP Offset: 0x10) USB Interrupt Enable Register -------- 
#define AT91C_UDP_EPINT0          (0x1 <<  0) // (UDP) Endpoint 0 Interrupt
#define AT91C_UDP_EPINT1          (0x1 <<  1) // (UDP) Endpoint 0 Interrupt
#define AT91C_UDP_EPINT2          (0x1 <<  2) // (UDP) Endpoint 2 Interrupt
#define AT91C_UDP_EPINT3          (0x1 <<  3) // (UDP) Endpoint 3 Interrupt
#define AT91C_UDP_EPINT4          (0x1 <<  4) // (UDP) Endpoint 4 Interrupt
#define AT91C_UDP_EPINT5          (0x1 <<  5) // (UDP) Endpoint 5 Interrupt
#define AT91C_UDP_EPINT6          (0x1 <<  6) // (UDP) Endpoint 6 Interrupt
#define AT91C_UDP_EPINT7          (0x1 <<  7) // (UDP) Endpoint 7 Interrupt
#define AT91C_UDP_RXSUSP          (0x1 <<  8) // (UDP) USB Suspend Interrupt
#define AT91C_UDP_RXRSM           (0x1 <<  9) // (UDP) USB Resume Interrupt
#define AT91C_UDP_EXTRSM          (0x1 << 10) // (UDP) USB External Resume Interrupt
#define AT91C_UDP_SOFINT          (0x1 << 11) // (UDP) USB Start Of frame Interrupt
#define AT91C_UDP_WAKEUP          (0x1 << 13) // (UDP) USB Resume Interrupt
// -------- UDP_IDR : (UDP Offset: 0x14) USB Interrupt Disable Register -------- 
// -------- UDP_IMR : (UDP Offset: 0x18) USB Interrupt Mask Register -------- 
// -------- UDP_ISR : (UDP Offset: 0x1c) USB Interrupt Status Register -------- 
#define AT91C_UDP_ENDBUSRES       (0x1 << 12) // (UDP) USB End Of Bus Reset Interrupt
// -------- UDP_ICR : (UDP Offset: 0x20) USB Interrupt Clear Register -------- 
// -------- UDP_RST_EP : (UDP Offset: 0x28) USB Reset Endpoint Register -------- 
#define AT91C_UDP_EP0             (0x1 <<  0) // (UDP) Reset Endpoint 0
#define AT91C_UDP_EP1             (0x1 <<  1) // (UDP) Reset Endpoint 1
#define AT91C_UDP_EP2             (0x1 <<  2) // (UDP) Reset Endpoint 2
#define AT91C_UDP_EP3             (0x1 <<  3) // (UDP) Reset Endpoint 3
#define AT91C_UDP_EP4             (0x1 <<  4) // (UDP) Reset Endpoint 4
#define AT91C_UDP_EP5             (0x1 <<  5) // (UDP) Reset Endpoint 5
#define AT91C_UDP_EP6             (0x1 <<  6) // (UDP) Reset Endpoint 6
#define AT91C_UDP_EP7             (0x1 <<  7) // (UDP) Reset Endpoint 7
// -------- UDP_CSR : (UDP Offset: 0x30) USB Endpoint Control and Status Register -------- 
#define AT91C_UDP_TXCOMP          (0x1 <<  0) // (UDP) Generates an IN packet with data previously written in the DPR
#define AT91C_UDP_RX_DATA_BK0     (0x1 <<  1) // (UDP) Receive Data Bank 0
#define AT91C_UDP_RXSETUP         (0x1 <<  2) // (UDP) Sends STALL to the Host (Control endpoints)
#define AT91C_UDP_ISOERROR        (0x1 <<  3) // (UDP) Isochronous error (Isochronous endpoints)
#define AT91C_UDP_TXPKTRDY        (0x1 <<  4) // (UDP) Transmit Packet Ready
#define AT91C_UDP_FORCESTALL      (0x1 <<  5) // (UDP) Force Stall (used by Control, Bulk and Isochronous endpoints).
#define AT91C_UDP_RX_DATA_BK1     (0x1 <<  6) // (UDP) Receive Data Bank 1 (only used by endpoints with ping-pong attributes).
#define AT91C_UDP_DIR             (0x1 <<  7) // (UDP) Transfer Direction
#define AT91C_UDP_EPTYPE          (0x7 <<  8) // (UDP) Endpoint type
#define 	AT91C_UDP_EPTYPE_CTRL                 (0x0 <<  8) // (UDP) Control
#define 	AT91C_UDP_EPTYPE_ISO_OUT              (0x1 <<  8) // (UDP) Isochronous OUT
#define 	AT91C_UDP_EPTYPE_BULK_OUT             (0x2 <<  8) // (UDP) Bulk OUT
#define 	AT91C_UDP_EPTYPE_INT_OUT              (0x3 <<  8) // (UDP) Interrupt OUT
#define 	AT91C_UDP_EPTYPE_ISO_IN               (0x5 <<  8) // (UDP) Isochronous IN
#define 	AT91C_UDP_EPTYPE_BULK_IN              (0x6 <<  8) // (UDP) Bulk IN
#define 	AT91C_UDP_EPTYPE_INT_IN               (0x7 <<  8) // (UDP) Interrupt IN
#define AT91C_UDP_DTGLE           (0x1 << 11) // (UDP) Data Toggle
#define AT91C_UDP_EPEDS           (0x1 << 15) // (UDP) Endpoint Enable Disable
#define AT91C_UDP_RXBYTECNT       (0x7FF << 16) // (UDP) Number Of Bytes Available in the FIFO

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Timer Counter Channel Interface
// *****************************************************************************
// *** Register offset in AT91S_TC structure ***
#define TC_CCR          ( 0) // Channel Control Register
#define TC_CMR          ( 4) // Channel Mode Register
#define TC_CV           (16) // Counter Value
#define TC_RA           (20) // Register A
#define TC_RB           (24) // Register B
#define TC_RC           (28) // Register C
#define TC_SR           (32) // Status Register
#define TC_IER          (36) // Interrupt Enable Register
#define TC_IDR          (40) // Interrupt Disable Register
#define TC_IMR          (44) // Interrupt Mask Register
// -------- TC_CCR : (TC Offset: 0x0) TC Channel Control Register -------- 
#define AT91C_TC_CLKEN            (0x1 <<  0) // (TC) Counter Clock Enable Command
#define AT91C_TC_CLKDIS           (0x1 <<  1) // (TC) Counter Clock Disable Command
#define AT91C_TC_SWTRG            (0x1 <<  2) // (TC) Software Trigger Command
// -------- TC_CMR : (TC Offset: 0x4) TC Channel Mode Register: Capture Mode / Waveform Mode -------- 
#define AT91C_TC_CPCSTOP          (0x1 <<  6) // (TC) Counter Clock Stopped with RC Compare
#define AT91C_TC_CPCDIS           (0x1 <<  7) // (TC) Counter Clock Disable with RC Compare
#define AT91C_TC_EEVTEDG          (0x3 <<  8) // (TC) External Event Edge Selection
#define 	AT91C_TC_EEVTEDG_NONE                 (0x0 <<  8) // (TC) Edge: None
#define 	AT91C_TC_EEVTEDG_RISING               (0x1 <<  8) // (TC) Edge: rising edge
#define 	AT91C_TC_EEVTEDG_FALLING              (0x2 <<  8) // (TC) Edge: falling edge
#define 	AT91C_TC_EEVTEDG_BOTH                 (0x3 <<  8) // (TC) Edge: each edge
#define AT91C_TC_EEVT             (0x3 << 10) // (TC) External Event  Selection
#define 	AT91C_TC_EEVT_NONE                 (0x0 << 10) // (TC) Signal selected as external event: TIOB TIOB direction: input
#define 	AT91C_TC_EEVT_RISING               (0x1 << 10) // (TC) Signal selected as external event: XC0 TIOB direction: output
#define 	AT91C_TC_EEVT_FALLING              (0x2 << 10) // (TC) Signal selected as external event: XC1 TIOB direction: output
#define 	AT91C_TC_EEVT_BOTH                 (0x3 << 10) // (TC) Signal selected as external event: XC2 TIOB direction: output
#define AT91C_TC_ENETRG           (0x1 << 12) // (TC) External Event Trigger enable
#define AT91C_TC_WAVESEL          (0x3 << 13) // (TC) Waveform  Selection
#define 	AT91C_TC_WAVESEL_UP                   (0x0 << 13) // (TC) UP mode without atomatic trigger on RC Compare
#define 	AT91C_TC_WAVESEL_UPDOWN               (0x1 << 13) // (TC) UPDOWN mode without automatic trigger on RC Compare
#define 	AT91C_TC_WAVESEL_UP_AUTO              (0x2 << 13) // (TC) UP mode with automatic trigger on RC Compare
#define 	AT91C_TC_WAVESEL_UPDOWN_AUTO          (0x3 << 13) // (TC) UPDOWN mode with automatic trigger on RC Compare
#define AT91C_TC_CPCTRG           (0x1 << 14) // (TC) RC Compare Trigger Enable
#define AT91C_TC_WAVE             (0x1 << 15) // (TC) 
#define AT91C_TC_ACPA             (0x3 << 16) // (TC) RA Compare Effect on TIOA
#define 	AT91C_TC_ACPA_NONE                 (0x0 << 16) // (TC) Effect: none
#define 	AT91C_TC_ACPA_SET                  (0x1 << 16) // (TC) Effect: set
#define 	AT91C_TC_ACPA_CLEAR                (0x2 << 16) // (TC) Effect: clear
#define 	AT91C_TC_ACPA_TOGGLE               (0x3 << 16) // (TC) Effect: toggle
#define AT91C_TC_ACPC             (0x3 << 18) // (TC) RC Compare Effect on TIOA
#define 	AT91C_TC_ACPC_NONE                 (0x0 << 18) // (TC) Effect: none
#define 	AT91C_TC_ACPC_SET                  (0x1 << 18) // (TC) Effect: set
#define 	AT91C_TC_ACPC_CLEAR                (0x2 << 18) // (TC) Effect: clear
#define 	AT91C_TC_ACPC_TOGGLE               (0x3 << 18) // (TC) Effect: toggle
#define AT91C_TC_AEEVT            (0x3 << 20) // (TC) External Event Effect on TIOA
#define 	AT91C_TC_AEEVT_NONE                 (0x0 << 20) // (TC) Effect: none
#define 	AT91C_TC_AEEVT_SET                  (0x1 << 20) // (TC) Effect: set
#define 	AT91C_TC_AEEVT_CLEAR                (0x2 << 20) // (TC) Effect: clear
#define 	AT91C_TC_AEEVT_TOGGLE               (0x3 << 20) // (TC) Effect: toggle
#define AT91C_TC_ASWTRG           (0x3 << 22) // (TC) Software Trigger Effect on TIOA
#define 	AT91C_TC_ASWTRG_NONE                 (0x0 << 22) // (TC) Effect: none
#define 	AT91C_TC_ASWTRG_SET                  (0x1 << 22) // (TC) Effect: set
#define 	AT91C_TC_ASWTRG_CLEAR                (0x2 << 22) // (TC) Effect: clear
#define 	AT91C_TC_ASWTRG_TOGGLE               (0x3 << 22) // (TC) Effect: toggle
#define AT91C_TC_BCPB             (0x3 << 24) // (TC) RB Compare Effect on TIOB
#define 	AT91C_TC_BCPB_NONE                 (0x0 << 24) // (TC) Effect: none
#define 	AT91C_TC_BCPB_SET                  (0x1 << 24) // (TC) Effect: set
#define 	AT91C_TC_BCPB_CLEAR                (0x2 << 24) // (TC) Effect: clear
#define 	AT91C_TC_BCPB_TOGGLE               (0x3 << 24) // (TC) Effect: toggle
#define AT91C_TC_BCPC             (0x3 << 26) // (TC) RC Compare Effect on TIOB
#define 	AT91C_TC_BCPC_NONE                 (0x0 << 26) // (TC) Effect: none
#define 	AT91C_TC_BCPC_SET                  (0x1 << 26) // (TC) Effect: set
#define 	AT91C_TC_BCPC_CLEAR                (0x2 << 26) // (TC) Effect: clear
#define 	AT91C_TC_BCPC_TOGGLE               (0x3 << 26) // (TC) Effect: toggle
#define AT91C_TC_BEEVT            (0x3 << 28) // (TC) External Event Effect on TIOB
#define 	AT91C_TC_BEEVT_NONE                 (0x0 << 28) // (TC) Effect: none
#define 	AT91C_TC_BEEVT_SET                  (0x1 << 28) // (TC) Effect: set
#define 	AT91C_TC_BEEVT_CLEAR                (0x2 << 28) // (TC) Effect: clear
#define 	AT91C_TC_BEEVT_TOGGLE               (0x3 << 28) // (TC) Effect: toggle
#define AT91C_TC_BSWTRG           (0x3 << 30) // (TC) Software Trigger Effect on TIOB
#define 	AT91C_TC_BSWTRG_NONE                 (0x0 << 30) // (TC) Effect: none
#define 	AT91C_TC_BSWTRG_SET                  (0x1 << 30) // (TC) Effect: set
#define 	AT91C_TC_BSWTRG_CLEAR                (0x2 << 30) // (TC) Effect: clear
#define 	AT91C_TC_BSWTRG_TOGGLE               (0x3 << 30) // (TC) Effect: toggle
// -------- TC_SR : (TC Offset: 0x20) TC Channel Status Register -------- 
#define AT91C_TC_COVFS            (0x1 <<  0) // (TC) Counter Overflow
#define AT91C_TC_LOVRS            (0x1 <<  1) // (TC) Load Overrun
#define AT91C_TC_CPAS             (0x1 <<  2) // (TC) RA Compare
#define AT91C_TC_CPBS             (0x1 <<  3) // (TC) RB Compare
#define AT91C_TC_CPCS             (0x1 <<  4) // (TC) RC Compare
#define AT91C_TC_LDRAS            (0x1 <<  5) // (TC) RA Loading
#define AT91C_TC_LDRBS            (0x1 <<  6) // (TC) RB Loading
#define AT91C_TC_ETRCS            (0x1 <<  7) // (TC) External Trigger
#define AT91C_TC_ETRGS            (0x1 << 16) // (TC) Clock Enabling
#define AT91C_TC_MTIOA            (0x1 << 17) // (TC) TIOA Mirror
#define AT91C_TC_MTIOB            (0x1 << 18) // (TC) TIOA Mirror
// -------- TC_IER : (TC Offset: 0x24) TC Channel Interrupt Enable Register -------- 
// -------- TC_IDR : (TC Offset: 0x28) TC Channel Interrupt Disable Register -------- 
// -------- TC_IMR : (TC Offset: 0x2c) TC Channel Interrupt Mask Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Timer Counter Interface
// *****************************************************************************
// *** Register offset in AT91S_TCB structure ***
#define TCB_TC0         ( 0) // TC Channel 0
#define TCB_TC1         (64) // TC Channel 1
#define TCB_TC2         (128) // TC Channel 2
#define TCB_BCR         (192) // TC Block Control Register
#define TCB_BMR         (196) // TC Block Mode Register
// -------- TCB_BCR : (TCB Offset: 0xc0) TC Block Control Register -------- 
#define AT91C_TCB_SYNC            (0x1 <<  0) // (TCB) Synchro Command
// -------- TCB_BMR : (TCB Offset: 0xc4) TC Block Mode Register -------- 
#define AT91C_TCB_TC0XC0S         (0x1 <<  0) // (TCB) External Clock Signal 0 Selection
#define 	AT91C_TCB_TC0XC0S_TCLK0                (0x0) // (TCB) TCLK0 connected to XC0
#define 	AT91C_TCB_TC0XC0S_NONE                 (0x1) // (TCB) None signal connected to XC0
#define 	AT91C_TCB_TC0XC0S_TIOA1                (0x2) // (TCB) TIOA1 connected to XC0
#define 	AT91C_TCB_TC0XC0S_TIOA2                (0x3) // (TCB) TIOA2 connected to XC0
#define AT91C_TCB_TC1XC1S         (0x1 <<  2) // (TCB) External Clock Signal 1 Selection
#define 	AT91C_TCB_TC1XC1S_TCLK1                (0x0 <<  2) // (TCB) TCLK1 connected to XC1
#define 	AT91C_TCB_TC1XC1S_NONE                 (0x1 <<  2) // (TCB) None signal connected to XC1
#define 	AT91C_TCB_TC1XC1S_TIOA0                (0x2 <<  2) // (TCB) TIOA0 connected to XC1
#define 	AT91C_TCB_TC1XC1S_TIOA2                (0x3 <<  2) // (TCB) TIOA2 connected to XC1
#define AT91C_TCB_TC2XC2S         (0x1 <<  4) // (TCB) External Clock Signal 2 Selection
#define 	AT91C_TCB_TC2XC2S_TCLK2                (0x0 <<  4) // (TCB) TCLK2 connected to XC2
#define 	AT91C_TCB_TC2XC2S_NONE                 (0x1 <<  4) // (TCB) None signal connected to XC2
#define 	AT91C_TCB_TC2XC2S_TIOA0                (0x2 <<  4) // (TCB) TIOA0 connected to XC2
#define 	AT91C_TCB_TC2XC2S_TIOA2                (0x3 <<  4) // (TCB) TIOA2 connected to XC2

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR USB Host Interface
// *****************************************************************************
// *** Register offset in AT91S_UHP structure ***
#define UHP_HcRevision  ( 0) // Revision
#define UHP_HcControl   ( 4) // Operating modes for the Host Controller
#define UHP_HcCommandStatus ( 8) // Command & status Register
#define UHP_HcInterruptStatus (12) // Interrupt Status Register
#define UHP_HcInterruptEnable (16) // Interrupt Enable Register
#define UHP_HcInterruptDisable (20) // Interrupt Disable Register
#define UHP_HcHCCA      (24) // Pointer to the Host Controller Communication Area
#define UHP_HcPeriodCurrentED (28) // Current Isochronous or Interrupt Endpoint Descriptor
#define UHP_HcControlHeadED (32) // First Endpoint Descriptor of the Control list
#define UHP_HcControlCurrentED (36) // Endpoint Control and Status Register
#define UHP_HcBulkHeadED (40) // First endpoint register of the Bulk list
#define UHP_HcBulkCurrentED (44) // Current endpoint of the Bulk list
#define UHP_HcBulkDoneHead (48) // Last completed transfer descriptor
#define UHP_HcFmInterval (52) // Bit time between 2 consecutive SOFs
#define UHP_HcFmRemaining (56) // Bit time remaining in the current Frame
#define UHP_HcFmNumber  (60) // Frame number
#define UHP_HcPeriodicStart (64) // Periodic Start
#define UHP_HcLSThreshold (68) // LS Threshold
#define UHP_HcRhDescriptorA (72) // Root Hub characteristics A
#define UHP_HcRhDescriptorB (76) // Root Hub characteristics B
#define UHP_HcRhStatus  (80) // Root Hub Status register
#define UHP_HcRhPortStatus (84) // Root Hub Port Status Register

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Ethernet MAC
// *****************************************************************************
// *** Register offset in AT91S_EMAC structure ***
#define EMAC_CTL        ( 0) // Network Control Register
#define EMAC_CFG        ( 4) // Network Configuration Register
#define EMAC_SR         ( 8) // Network Status Register
#define EMAC_TAR        (12) // Transmit Address Register
#define EMAC_TCR        (16) // Transmit Control Register
#define EMAC_TSR        (20) // Transmit Status Register
#define EMAC_RBQP       (24) // Receive Buffer Queue Pointer
#define EMAC_RSR        (32) // Receive Status Register
#define EMAC_ISR        (36) // Interrupt Status Register
#define EMAC_IER        (40) // Interrupt Enable Register
#define EMAC_IDR        (44) // Interrupt Disable Register
#define EMAC_IMR        (48) // Interrupt Mask Register
#define EMAC_MAN        (52) // PHY Maintenance Register
#define EMAC_FRA        (64) // Frames Transmitted OK Register
#define EMAC_SCOL       (68) // Single Collision Frame Register
#define EMAC_MCOL       (72) // Multiple Collision Frame Register
#define EMAC_OK         (76) // Frames Received OK Register
#define EMAC_SEQE       (80) // Frame Check Sequence Error Register
#define EMAC_ALE        (84) // Alignment Error Register
#define EMAC_DTE        (88) // Deferred Transmission Frame Register
#define EMAC_LCOL       (92) // Late Collision Register
#define EMAC_ECOL       (96) // Excessive Collision Register
#define EMAC_CSE        (100) // Carrier Sense Error Register
#define EMAC_TUE        (104) // Transmit Underrun Error Register
#define EMAC_CDE        (108) // Code Error Register
#define EMAC_ELR        (112) // Excessive Length Error Register
#define EMAC_RJB        (116) // Receive Jabber Register
#define EMAC_USF        (120) // Undersize Frame Register
#define EMAC_SQEE       (124) // SQE Test Error Register
#define EMAC_DRFC       (128) // Discarded RX Frame Register
#define EMAC_HSH        (144) // Hash Address High[63:32]
#define EMAC_HSL        (148) // Hash Address Low[31:0]
#define EMAC_SA1L       (152) // Specific Address 1 Low, First 4 bytes
#define EMAC_SA1H       (156) // Specific Address 1 High, Last 2 bytes
#define EMAC_SA2L       (160) // Specific Address 2 Low, First 4 bytes
#define EMAC_SA2H       (164) // Specific Address 2 High, Last 2 bytes
#define EMAC_SA3L       (168) // Specific Address 3 Low, First 4 bytes
#define EMAC_SA3H       (172) // Specific Address 3 High, Last 2 bytes
#define EMAC_SA4L       (176) // Specific Address 4 Low, First 4 bytes
#define EMAC_SA4H       (180) // Specific Address 4 High, Last 2 bytesr
// -------- EMAC_CTL : (EMAC Offset: 0x0)  -------- 
#define AT91C_EMAC_LB             (0x1 <<  0) // (EMAC) Loopback. Optional. When set, loopback signal is at high level.
#define AT91C_EMAC_LBL            (0x1 <<  1) // (EMAC) Loopback local. 
#define AT91C_EMAC_RE             (0x1 <<  2) // (EMAC) Receive enable. 
#define AT91C_EMAC_TE             (0x1 <<  3) // (EMAC) Transmit enable. 
#define AT91C_EMAC_MPE            (0x1 <<  4) // (EMAC) Management port enable. 
#define AT91C_EMAC_CSR            (0x1 <<  5) // (EMAC) Clear statistics registers. 
#define AT91C_EMAC_ISR            (0x1 <<  6) // (EMAC) Increment statistics registers. 
#define AT91C_EMAC_WES            (0x1 <<  7) // (EMAC) Write enable for statistics registers. 
#define AT91C_EMAC_BP             (0x1 <<  8) // (EMAC) Back pressure. 
// -------- EMAC_CFG : (EMAC Offset: 0x4) Network Configuration Register -------- 
#define AT91C_EMAC_SPD            (0x1 <<  0) // (EMAC) Speed. 
#define AT91C_EMAC_FD             (0x1 <<  1) // (EMAC) Full duplex. 
#define AT91C_EMAC_BR             (0x1 <<  2) // (EMAC) Bit rate. 
#define AT91C_EMAC_CAF            (0x1 <<  4) // (EMAC) Copy all frames. 
#define AT91C_EMAC_NBC            (0x1 <<  5) // (EMAC) No broadcast. 
#define AT91C_EMAC_MTI            (0x1 <<  6) // (EMAC) Multicast hash enable
#define AT91C_EMAC_UNI            (0x1 <<  7) // (EMAC) Unicast hash enable. 
#define AT91C_EMAC_BIG            (0x1 <<  8) // (EMAC) Receive 1522 bytes. 
#define AT91C_EMAC_EAE            (0x1 <<  9) // (EMAC) External address match enable. 
#define AT91C_EMAC_CLK            (0x3 << 10) // (EMAC) 
#define 	AT91C_EMAC_CLK_HCLK_8               (0x0 << 10) // (EMAC) HCLK divided by 8
#define 	AT91C_EMAC_CLK_HCLK_16              (0x1 << 10) // (EMAC) HCLK divided by 16
#define 	AT91C_EMAC_CLK_HCLK_32              (0x2 << 10) // (EMAC) HCLK divided by 32
#define 	AT91C_EMAC_CLK_HCLK_64              (0x3 << 10) // (EMAC) HCLK divided by 64
#define AT91C_EMAC_RTY            (0x1 << 12) // (EMAC) 
#define AT91C_EMAC_RMII           (0x1 << 13) // (EMAC) 
// -------- EMAC_SR : (EMAC Offset: 0x8) Network Status Register -------- 
#define AT91C_EMAC_MDIO           (0x1 <<  1) // (EMAC) 
#define AT91C_EMAC_IDLE           (0x1 <<  2) // (EMAC) 
// -------- EMAC_TCR : (EMAC Offset: 0x10) Transmit Control Register -------- 
#define AT91C_EMAC_LEN            (0x7FF <<  0) // (EMAC) 
#define AT91C_EMAC_NCRC           (0x1 << 15) // (EMAC) 
// -------- EMAC_TSR : (EMAC Offset: 0x14) Transmit Control Register -------- 
#define AT91C_EMAC_OVR            (0x1 <<  0) // (EMAC) 
#define AT91C_EMAC_COL            (0x1 <<  1) // (EMAC) 
#define AT91C_EMAC_RLE            (0x1 <<  2) // (EMAC) 
#define AT91C_EMAC_TXIDLE         (0x1 <<  3) // (EMAC) 
#define AT91C_EMAC_BNQ            (0x1 <<  4) // (EMAC) 
#define AT91C_EMAC_COMP           (0x1 <<  5) // (EMAC) 
#define AT91C_EMAC_UND            (0x1 <<  6) // (EMAC) 
// -------- EMAC_RSR : (EMAC Offset: 0x20) Receive Status Register -------- 
#define AT91C_EMAC_BNA            (0x1 <<  0) // (EMAC) 
#define AT91C_EMAC_REC            (0x1 <<  1) // (EMAC) 
// -------- EMAC_ISR : (EMAC Offset: 0x24) Interrupt Status Register -------- 
#define AT91C_EMAC_DONE           (0x1 <<  0) // (EMAC) 
#define AT91C_EMAC_RCOM           (0x1 <<  1) // (EMAC) 
#define AT91C_EMAC_RBNA           (0x1 <<  2) // (EMAC) 
#define AT91C_EMAC_TOVR           (0x1 <<  3) // (EMAC) 
#define AT91C_EMAC_TUND           (0x1 <<  4) // (EMAC) 
#define AT91C_EMAC_RTRY           (0x1 <<  5) // (EMAC) 
#define AT91C_EMAC_TBRE           (0x1 <<  6) // (EMAC) 
#define AT91C_EMAC_TCOM           (0x1 <<  7) // (EMAC) 
#define AT91C_EMAC_TIDLE          (0x1 <<  8) // (EMAC) 
#define AT91C_EMAC_LINK           (0x1 <<  9) // (EMAC) 
#define AT91C_EMAC_ROVR           (0x1 << 10) // (EMAC) 
#define AT91C_EMAC_HRESP          (0x1 << 11) // (EMAC) 
// -------- EMAC_IER : (EMAC Offset: 0x28) Interrupt Enable Register -------- 
// -------- EMAC_IDR : (EMAC Offset: 0x2c) Interrupt Disable Register -------- 
// -------- EMAC_IMR : (EMAC Offset: 0x30) Interrupt Mask Register -------- 
// -------- EMAC_MAN : (EMAC Offset: 0x34) PHY Maintenance Register -------- 
#define AT91C_EMAC_DATA           (0xFFFF <<  0) // (EMAC) 
#define AT91C_EMAC_CODE           (0x3 << 16) // (EMAC) 
#define AT91C_EMAC_REGA           (0x1F << 18) // (EMAC) 
#define AT91C_EMAC_PHYA           (0x1F << 23) // (EMAC) 
#define AT91C_EMAC_RW             (0x3 << 28) // (EMAC) 
#define AT91C_EMAC_HIGH           (0x1 << 30) // (EMAC) 
#define AT91C_EMAC_LOW            (0x1 << 31) // (EMAC) 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR External Bus Interface
// *****************************************************************************
// *** Register offset in AT91S_EBI structure ***
#define EBI_CSA         ( 0) // Chip Select Assignment Register
#define EBI_CFGR        ( 4) // Configuration Register
// -------- EBI_CSA : (EBI Offset: 0x0) Chip Select Assignment Register -------- 
#define AT91C_EBI_CS0A            (0x1 <<  0) // (EBI) Chip Select 0 Assignment
#define 	AT91C_EBI_CS0A_SMC                  (0x0) // (EBI) Chip Select 0 is assigned to the Static Memory Controller.
#define 	AT91C_EBI_CS0A_BFC                  (0x1) // (EBI) Chip Select 0 is assigned to the Burst Flash Controller.
#define AT91C_EBI_CS1A            (0x1 <<  1) // (EBI) Chip Select 1 Assignment
#define 	AT91C_EBI_CS1A_SMC                  (0x0 <<  1) // (EBI) Chip Select 1 is assigned to the Static Memory Controller.
#define 	AT91C_EBI_CS1A_SDRAMC               (0x1 <<  1) // (EBI) Chip Select 1 is assigned to the SDRAM Controller.
#define AT91C_EBI_CS3A            (0x1 <<  3) // (EBI) Chip Select 3 Assignment
#define 	AT91C_EBI_CS3A_SMC                  (0x0 <<  3) // (EBI) Chip Select 3 is only assigned to the Static Memory Controller and NCS3 behaves as defined by the SMC2.
#define 	AT91C_EBI_CS3A_SMC_SmartMedia       (0x1 <<  3) // (EBI) Chip Select 3 is assigned to the Static Memory Controller and the SmartMedia Logic is activated.
#define AT91C_EBI_CS4A            (0x1 <<  4) // (EBI) Chip Select 4 Assignment
#define 	AT91C_EBI_CS4A_SMC                  (0x0 <<  4) // (EBI) Chip Select 4 is assigned to the Static Memory Controller and NCS4,NCS5 and NCS6 behave as defined by the SMC2.
#define 	AT91C_EBI_CS4A_SMC_CompactFlash     (0x1 <<  4) // (EBI) Chip Select 4 is assigned to the Static Memory Controller and the CompactFlash Logic is activated.
// -------- EBI_CFGR : (EBI Offset: 0x4) Configuration Register -------- 
#define AT91C_EBI_DBPUC           (0x1 <<  0) // (EBI) Data Bus Pull-Up Configuration
#define AT91C_EBI_EBSEN           (0x1 <<  1) // (EBI) Bus Sharing Enable

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Static Memory Controller 2 Interface
// *****************************************************************************
// *** Register offset in AT91S_SMC2 structure ***
#define SMC2_CSR        ( 0) // SMC2 Chip Select Register
// -------- SMC2_CSR : (SMC2 Offset: 0x0) SMC2 Chip Select Register -------- 
#define AT91C_SMC2_NWS            (0x7F <<  0) // (SMC2) Number of Wait States
#define AT91C_SMC2_WSEN           (0x1 <<  7) // (SMC2) Wait State Enable
#define AT91C_SMC2_TDF            (0xF <<  8) // (SMC2) Data Float Time
#define AT91C_SMC2_BAT            (0x1 << 12) // (SMC2) Byte Access Type
#define AT91C_SMC2_DBW            (0x1 << 13) // (SMC2) Data Bus Width
#define 	AT91C_SMC2_DBW_16                   (0x1 << 13) // (SMC2) 16-bit.
#define 	AT91C_SMC2_DBW_8                    (0x2 << 13) // (SMC2) 8-bit.
#define AT91C_SMC2_DRP            (0x1 << 15) // (SMC2) Data Read Protocol
#define AT91C_SMC2_ACSS           (0x3 << 16) // (SMC2) Address to Chip Select Setup
#define 	AT91C_SMC2_ACSS_STANDARD             (0x0 << 16) // (SMC2) Standard, asserted at the beginning of the access and deasserted at the end.
#define 	AT91C_SMC2_ACSS_1_CYCLE              (0x1 << 16) // (SMC2) One cycle less at the beginning and the end of the access.
#define 	AT91C_SMC2_ACSS_2_CYCLES             (0x2 << 16) // (SMC2) Two cycles less at the beginning and the end of the access.
#define 	AT91C_SMC2_ACSS_3_CYCLES             (0x3 << 16) // (SMC2) Three cycles less at the beginning and the end of the access.
#define AT91C_SMC2_RWSETUP        (0x7 << 24) // (SMC2) Read and Write Signal Setup Time
#define AT91C_SMC2_RWHOLD         (0x7 << 29) // (SMC2) Read and Write Signal Hold Time

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR SDRAM Controller Interface
// *****************************************************************************
// *** Register offset in AT91S_SDRC structure ***
#define SDRC_MR         ( 0) // SDRAM Controller Mode Register
#define SDRC_TR         ( 4) // SDRAM Controller Refresh Timer Register
#define SDRC_CR         ( 8) // SDRAM Controller Configuration Register
#define SDRC_SRR        (12) // SDRAM Controller Self Refresh Register
#define SDRC_LPR        (16) // SDRAM Controller Low Power Register
#define SDRC_IER        (20) // SDRAM Controller Interrupt Enable Register
#define SDRC_IDR        (24) // SDRAM Controller Interrupt Disable Register
#define SDRC_IMR        (28) // SDRAM Controller Interrupt Mask Register
#define SDRC_ISR        (32) // SDRAM Controller Interrupt Mask Register
// -------- SDRC_MR : (SDRC Offset: 0x0) SDRAM Controller Mode Register -------- 
#define AT91C_SDRC_MODE           (0xF <<  0) // (SDRC) Mode
#define 	AT91C_SDRC_MODE_NORMAL_CMD           (0x0) // (SDRC) Normal Mode
#define 	AT91C_SDRC_MODE_NOP_CMD              (0x1) // (SDRC) NOP Command
#define 	AT91C_SDRC_MODE_PRCGALL_CMD          (0x2) // (SDRC) All Banks Precharge Command
#define 	AT91C_SDRC_MODE_LMR_CMD              (0x3) // (SDRC) Load Mode Register Command
#define 	AT91C_SDRC_MODE_RFSH_CMD             (0x4) // (SDRC) Refresh Command
#define AT91C_SDRC_DBW            (0x1 <<  4) // (SDRC) Data Bus Width
#define 	AT91C_SDRC_DBW_32_BITS              (0x0 <<  4) // (SDRC) 32 Bits datas bus
#define 	AT91C_SDRC_DBW_16_BITS              (0x1 <<  4) // (SDRC) 16 Bits datas bus
// -------- SDRC_TR : (SDRC Offset: 0x4) SDRC Refresh Timer Register -------- 
#define AT91C_SDRC_COUNT          (0xFFF <<  0) // (SDRC) Refresh Counter
// -------- SDRC_CR : (SDRC Offset: 0x8) SDRAM Configuration Register -------- 
#define AT91C_SDRC_NC             (0x3 <<  0) // (SDRC) Number of Column Bits
#define 	AT91C_SDRC_NC_8                    (0x0) // (SDRC) 8 Bits
#define 	AT91C_SDRC_NC_9                    (0x1) // (SDRC) 9 Bits
#define 	AT91C_SDRC_NC_10                   (0x2) // (SDRC) 10 Bits
#define 	AT91C_SDRC_NC_11                   (0x3) // (SDRC) 11 Bits
#define AT91C_SDRC_NR             (0x3 <<  2) // (SDRC) Number of Row Bits
#define 	AT91C_SDRC_NR_11                   (0x0 <<  2) // (SDRC) 11 Bits
#define 	AT91C_SDRC_NR_12                   (0x1 <<  2) // (SDRC) 12 Bits
#define 	AT91C_SDRC_NR_13                   (0x2 <<  2) // (SDRC) 13 Bits
#define AT91C_SDRC_NB             (0x1 <<  4) // (SDRC) Number of Banks
#define 	AT91C_SDRC_NB_2_BANKS              (0x0 <<  4) // (SDRC) 2 banks
#define 	AT91C_SDRC_NB_4_BANKS              (0x1 <<  4) // (SDRC) 4 banks
#define AT91C_SDRC_CAS            (0x3 <<  5) // (SDRC) CAS Latency
#define 	AT91C_SDRC_CAS_2                    (0x2 <<  5) // (SDRC) 2 cycles
#define AT91C_SDRC_TWR            (0xF <<  7) // (SDRC) Number of Write Recovery Time Cycles
#define AT91C_SDRC_TRC            (0xF << 11) // (SDRC) Number of RAS Cycle Time Cycles
#define AT91C_SDRC_TRP            (0xF << 15) // (SDRC) Number of RAS Precharge Time Cycles
#define AT91C_SDRC_TRCD           (0xF << 19) // (SDRC) Number of RAS to CAS Delay Cycles
#define AT91C_SDRC_TRAS           (0xF << 23) // (SDRC) Number of RAS Active Time Cycles
#define AT91C_SDRC_TXSR           (0xF << 27) // (SDRC) Number of Command Recovery Time Cycles
// -------- SDRC_SRR : (SDRC Offset: 0xc) SDRAM Controller Self-refresh Register -------- 
#define AT91C_SDRC_SRCB           (0x1 <<  0) // (SDRC) Self-refresh Command Bit
// -------- SDRC_LPR : (SDRC Offset: 0x10) SDRAM Controller Low-power Register -------- 
#define AT91C_SDRC_LPCB           (0x1 <<  0) // (SDRC) Low-power Command Bit
// -------- SDRC_IER : (SDRC Offset: 0x14) SDRAM Controller Interrupt Enable Register -------- 
#define AT91C_SDRC_RES            (0x1 <<  0) // (SDRC) Refresh Error Status
// -------- SDRC_IDR : (SDRC Offset: 0x18) SDRAM Controller Interrupt Disable Register -------- 
// -------- SDRC_IMR : (SDRC Offset: 0x1c) SDRAM Controller Interrupt Mask Register -------- 
// -------- SDRC_ISR : (SDRC Offset: 0x20) SDRAM Controller Interrupt Status Register -------- 

// *****************************************************************************
//              SOFTWARE API DEFINITION  FOR Burst Flash Controller Interface
// *****************************************************************************
// *** Register offset in AT91S_BFC structure ***
#define BFC_MR          ( 0) // BFC Mode Register
// -------- BFC_MR : (BFC Offset: 0x0) BFC Mode Register -------- 
#define AT91C_BFC_BFCOM           (0x3 <<  0) // (BFC) Burst Flash Controller Operating Mode
#define 	AT91C_BFC_BFCOM_DISABLED             (0x0) // (BFC) NPCS0 is driven by the SMC or remains high.
#define 	AT91C_BFC_BFCOM_ASYNC                (0x1) // (BFC) Asynchronous
#define 	AT91C_BFC_BFCOM_BURST_READ           (0x2) // (BFC) Burst Read
#define AT91C_BFC_BFCC            (0x3 <<  2) // (BFC) Burst Flash Controller Operating Mode
#define 	AT91C_BFC_BFCC_MCK                  (0x1 <<  2) // (BFC) Master Clock.
#define 	AT91C_BFC_BFCC_MCK_DIV_2            (0x2 <<  2) // (BFC) Master Clock divided by 2.
#define 	AT91C_BFC_BFCC_MCK_DIV_4            (0x3 <<  2) // (BFC) Master Clock divided by 4.
#define AT91C_BFC_AVL             (0xF <<  4) // (BFC) Address Valid Latency
#define AT91C_BFC_PAGES           (0x7 <<  8) // (BFC) Page Size
#define 	AT91C_BFC_PAGES_NO_PAGE              (0x0 <<  8) // (BFC) No page handling.
#define 	AT91C_BFC_PAGES_16                   (0x1 <<  8) // (BFC) 16 bytes page size.
#define 	AT91C_BFC_PAGES_32                   (0x2 <<  8) // (BFC) 32 bytes page size.
#define 	AT91C_BFC_PAGES_64                   (0x3 <<  8) // (BFC) 64 bytes page size.
#define 	AT91C_BFC_PAGES_128                  (0x4 <<  8) // (BFC) 128 bytes page size.
#define 	AT91C_BFC_PAGES_256                  (0x5 <<  8) // (BFC) 256 bytes page size.
#define 	AT91C_BFC_PAGES_512                  (0x6 <<  8) // (BFC) 512 bytes page size.
#define 	AT91C_BFC_PAGES_1024                 (0x7 <<  8) // (BFC) 1024 bytes page size.
#define AT91C_BFC_OEL             (0x3 << 12) // (BFC) Output Enable Latency
#define AT91C_BFC_BAAEN           (0x1 << 16) // (BFC) Burst Address Advance Enable
#define AT91C_BFC_BFOEH           (0x1 << 17) // (BFC) Burst Flash Output Enable Handling
#define AT91C_BFC_MUXEN           (0x1 << 18) // (BFC) Multiplexed Bus Enable
#define AT91C_BFC_RDYEN           (0x1 << 19) // (BFC) Ready Enable Mode

// *****************************************************************************
//               REGISTER ADDRESS DEFINITION FOR AT91RM9200
// *****************************************************************************
// ========== Register definition for SYS peripheral ========== 
// ========== Register definition for MC peripheral ========== 
#define AT91C_MC_PUER             (0xFFFFFF54) // (MC) MC Protection Unit Enable Register
#define AT91C_MC_ASR              (0xFFFFFF04) // (MC) MC Abort Status Register
#define AT91C_MC_PUP              (0xFFFFFF50) // (MC) MC Protection Unit Peripherals
#define AT91C_MC_PUIA             (0xFFFFFF10) // (MC) MC Protection Unit Area
#define AT91C_MC_AASR             (0xFFFFFF08) // (MC) MC Abort Address Status Register
#define AT91C_MC_RCR              (0xFFFFFF00) // (MC) MC Remap Control Register
// ========== Register definition for RTC peripheral ========== 
#define AT91C_RTC_IMR             (0xFFFFFE28) // (RTC) Interrupt Mask Register
#define AT91C_RTC_IER             (0xFFFFFE20) // (RTC) Interrupt Enable Register
#define AT91C_RTC_SR              (0xFFFFFE18) // (RTC) Status Register
#define AT91C_RTC_TIMALR          (0xFFFFFE10) // (RTC) Time Alarm Register
#define AT91C_RTC_TIMR            (0xFFFFFE08) // (RTC) Time Register
#define AT91C_RTC_CR              (0xFFFFFE00) // (RTC) Control Register
#define AT91C_RTC_VER             (0xFFFFFE2C) // (RTC) Valid Entry Register
#define AT91C_RTC_IDR             (0xFFFFFE24) // (RTC) Interrupt Disable Register
#define AT91C_RTC_SCCR            (0xFFFFFE1C) // (RTC) Status Clear Command Register
#define AT91C_RTC_CALALR          (0xFFFFFE14) // (RTC) Calendar Alarm Register
#define AT91C_RTC_CALR            (0xFFFFFE0C) // (RTC) Calendar Register
#define AT91C_RTC_MR              (0xFFFFFE04) // (RTC) Mode Register
// ========== Register definition for ST peripheral ========== 
#define AT91C_ST_CRTR             (0xFFFFFD24) // (ST) Current Real-time Register
#define AT91C_ST_IMR              (0xFFFFFD1C) // (ST) Interrupt Mask Register
#define AT91C_ST_IER              (0xFFFFFD14) // (ST) Interrupt Enable Register
#define AT91C_ST_RTMR             (0xFFFFFD0C) // (ST) Real-time Mode Register
#define AT91C_ST_PIMR             (0xFFFFFD04) // (ST) Period Interval Mode Register
#define AT91C_ST_RTAR             (0xFFFFFD20) // (ST) Real-time Alarm Register
#define AT91C_ST_IDR              (0xFFFFFD18) // (ST) Interrupt Disable Register
#define AT91C_ST_SR               (0xFFFFFD10) // (ST) Status Register
#define AT91C_ST_WDMR             (0xFFFFFD08) // (ST) Watchdog Mode Register
#define AT91C_ST_CR               (0xFFFFFD00) // (ST) Control Register
// ========== Register definition for PMC peripheral ========== 
#define AT91C_PMC_SCSR            (0xFFFFFC08) // (PMC) System Clock Status Register
#define AT91C_PMC_SCER            (0xFFFFFC00) // (PMC) System Clock Enable Register
#define AT91C_PMC_IMR             (0xFFFFFC6C) // (PMC) Interrupt Mask Register
#define AT91C_PMC_IDR             (0xFFFFFC64) // (PMC) Interrupt Disable Register
#define AT91C_PMC_PCDR            (0xFFFFFC14) // (PMC) Peripheral Clock Disable Register
#define AT91C_PMC_SCDR            (0xFFFFFC04) // (PMC) System Clock Disable Register
#define AT91C_PMC_SR              (0xFFFFFC68) // (PMC) Status Register
#define AT91C_PMC_IER             (0xFFFFFC60) // (PMC) Interrupt Enable Register
#define AT91C_PMC_MCKR            (0xFFFFFC30) // (PMC) Master Clock Register
#define AT91C_PMC_PCER            (0xFFFFFC10) // (PMC) Peripheral Clock Enable Register
#define AT91C_PMC_PCSR            (0xFFFFFC18) // (PMC) Peripheral Clock Status Register
#define AT91C_PMC_PCKR            (0xFFFFFC40) // (PMC) Programmable Clock Register
// ========== Register definition for CKGR peripheral ========== 
#define AT91C_CKGR_PLLBR          (0xFFFFFC2C) // (CKGR) PLL B Register
#define AT91C_CKGR_MCFR           (0xFFFFFC24) // (CKGR) Main Clock  Frequency Register
#define AT91C_CKGR_PLLAR          (0xFFFFFC28) // (CKGR) PLL A Register
#define AT91C_CKGR_MOR            (0xFFFFFC20) // (CKGR) Main Oscillator Register
// ========== Register definition for PIOD peripheral ========== 
#define AT91C_PIOD_PDSR           (0xFFFFFA3C) // (PIOD) Pin Data Status Register
#define AT91C_PIOD_CODR           (0xFFFFFA34) // (PIOD) Clear Output Data Register
#define AT91C_PIOD_OWER           (0xFFFFFAA0) // (PIOD) Output Write Enable Register
#define AT91C_PIOD_MDER           (0xFFFFFA50) // (PIOD) Multi-driver Enable Register
#define AT91C_PIOD_IMR            (0xFFFFFA48) // (PIOD) Interrupt Mask Register
#define AT91C_PIOD_IER            (0xFFFFFA40) // (PIOD) Interrupt Enable Register
#define AT91C_PIOD_ODSR           (0xFFFFFA38) // (PIOD) Output Data Status Register
#define AT91C_PIOD_SODR           (0xFFFFFA30) // (PIOD) Set Output Data Register
#define AT91C_PIOD_PER            (0xFFFFFA00) // (PIOD) PIO Enable Register
#define AT91C_PIOD_OWDR           (0xFFFFFAA4) // (PIOD) Output Write Disable Register
#define AT91C_PIOD_PPUER          (0xFFFFFA64) // (PIOD) Pull-up Enable Register
#define AT91C_PIOD_MDDR           (0xFFFFFA54) // (PIOD) Multi-driver Disable Register
#define AT91C_PIOD_ISR            (0xFFFFFA4C) // (PIOD) Interrupt Status Register
#define AT91C_PIOD_IDR            (0xFFFFFA44) // (PIOD) Interrupt Disable Register
#define AT91C_PIOD_PDR            (0xFFFFFA04) // (PIOD) PIO Disable Register
#define AT91C_PIOD_ODR            (0xFFFFFA14) // (PIOD) Output Disable Registerr
#define AT91C_PIOD_OWSR           (0xFFFFFAA8) // (PIOD) Output Write Status Register
#define AT91C_PIOD_ABSR           (0xFFFFFA78) // (PIOD) AB Select Status Register
#define AT91C_PIOD_ASR            (0xFFFFFA70) // (PIOD) Select A Register
#define AT91C_PIOD_PPUSR          (0xFFFFFA68) // (PIOD) Pad Pull-up Status Register
#define AT91C_PIOD_PPUDR          (0xFFFFFA60) // (PIOD) Pull-up Disable Register
#define AT91C_PIOD_MDSR           (0xFFFFFA58) // (PIOD) Multi-driver Status Register
#define AT91C_PIOD_PSR            (0xFFFFFA08) // (PIOD) PIO Status Register
#define AT91C_PIOD_OER            (0xFFFFFA10) // (PIOD) Output Enable Register
#define AT91C_PIOD_OSR            (0xFFFFFA18) // (PIOD) Output Status Register
#define AT91C_PIOD_IFER           (0xFFFFFA20) // (PIOD) Input Filter Enable Register
#define AT91C_PIOD_BSR            (0xFFFFFA74) // (PIOD) Select B Register
#define AT91C_PIOD_IFDR           (0xFFFFFA24) // (PIOD) Input Filter Disable Register
#define AT91C_PIOD_IFSR           (0xFFFFFA28) // (PIOD) Input Filter Status Register
// ========== Register definition for PIOC peripheral ========== 
#define AT91C_PIOC_IFDR           (0xFFFFF824) // (PIOC) Input Filter Disable Register
#define AT91C_PIOC_ODR            (0xFFFFF814) // (PIOC) Output Disable Registerr
#define AT91C_PIOC_ABSR           (0xFFFFF878) // (PIOC) AB Select Status Register
#define AT91C_PIOC_SODR           (0xFFFFF830) // (PIOC) Set Output Data Register
#define AT91C_PIOC_IFSR           (0xFFFFF828) // (PIOC) Input Filter Status Register
#define AT91C_PIOC_CODR           (0xFFFFF834) // (PIOC) Clear Output Data Register
#define AT91C_PIOC_ODSR           (0xFFFFF838) // (PIOC) Output Data Status Register
#define AT91C_PIOC_IER            (0xFFFFF840) // (PIOC) Interrupt Enable Register
#define AT91C_PIOC_IMR            (0xFFFFF848) // (PIOC) Interrupt Mask Register
#define AT91C_PIOC_OWDR           (0xFFFFF8A4) // (PIOC) Output Write Disable Register
#define AT91C_PIOC_MDDR           (0xFFFFF854) // (PIOC) Multi-driver Disable Register
#define AT91C_PIOC_PDSR           (0xFFFFF83C) // (PIOC) Pin Data Status Register
#define AT91C_PIOC_IDR            (0xFFFFF844) // (PIOC) Interrupt Disable Register
#define AT91C_PIOC_ISR            (0xFFFFF84C) // (PIOC) Interrupt Status Register
#define AT91C_PIOC_PDR            (0xFFFFF804) // (PIOC) PIO Disable Register
#define AT91C_PIOC_OWSR           (0xFFFFF8A8) // (PIOC) Output Write Status Register
#define AT91C_PIOC_OWER           (0xFFFFF8A0) // (PIOC) Output Write Enable Register
#define AT91C_PIOC_ASR            (0xFFFFF870) // (PIOC) Select A Register
#define AT91C_PIOC_PPUSR          (0xFFFFF868) // (PIOC) Pad Pull-up Status Register
#define AT91C_PIOC_PPUDR          (0xFFFFF860) // (PIOC) Pull-up Disable Register
#define AT91C_PIOC_MDSR           (0xFFFFF858) // (PIOC) Multi-driver Status Register
#define AT91C_PIOC_MDER           (0xFFFFF850) // (PIOC) Multi-driver Enable Register
#define AT91C_PIOC_IFER           (0xFFFFF820) // (PIOC) Input Filter Enable Register
#define AT91C_PIOC_OSR            (0xFFFFF818) // (PIOC) Output Status Register
#define AT91C_PIOC_OER            (0xFFFFF810) // (PIOC) Output Enable Register
#define AT91C_PIOC_PSR            (0xFFFFF808) // (PIOC) PIO Status Register
#define AT91C_PIOC_PER            (0xFFFFF800) // (PIOC) PIO Enable Register
#define AT91C_PIOC_BSR            (0xFFFFF874) // (PIOC) Select B Register
#define AT91C_PIOC_PPUER          (0xFFFFF864) // (PIOC) Pull-up Enable Register
// ========== Register definition for PIOB peripheral ========== 
#define AT91C_PIOB_OWSR           (0xFFFFF6A8) // (PIOB) Output Write Status Register
#define AT91C_PIOB_PPUSR          (0xFFFFF668) // (PIOB) Pad Pull-up Status Register
#define AT91C_PIOB_PPUDR          (0xFFFFF660) // (PIOB) Pull-up Disable Register
#define AT91C_PIOB_MDSR           (0xFFFFF658) // (PIOB) Multi-driver Status Register
#define AT91C_PIOB_MDER           (0xFFFFF650) // (PIOB) Multi-driver Enable Register
#define AT91C_PIOB_IMR            (0xFFFFF648) // (PIOB) Interrupt Mask Register
#define AT91C_PIOB_OSR            (0xFFFFF618) // (PIOB) Output Status Register
#define AT91C_PIOB_OER            (0xFFFFF610) // (PIOB) Output Enable Register
#define AT91C_PIOB_PSR            (0xFFFFF608) // (PIOB) PIO Status Register
#define AT91C_PIOB_PER            (0xFFFFF600) // (PIOB) PIO Enable Register
#define AT91C_PIOB_BSR            (0xFFFFF674) // (PIOB) Select B Register
#define AT91C_PIOB_PPUER          (0xFFFFF664) // (PIOB) Pull-up Enable Register
#define AT91C_PIOB_IFDR           (0xFFFFF624) // (PIOB) Input Filter Disable Register
#define AT91C_PIOB_ODR            (0xFFFFF614) // (PIOB) Output Disable Registerr
#define AT91C_PIOB_ABSR           (0xFFFFF678) // (PIOB) AB Select Status Register
#define AT91C_PIOB_ASR            (0xFFFFF670) // (PIOB) Select A Register
#define AT91C_PIOB_IFER           (0xFFFFF620) // (PIOB) Input Filter Enable Register
#define AT91C_PIOB_IFSR           (0xFFFFF628) // (PIOB) Input Filter Status Register
#define AT91C_PIOB_SODR           (0xFFFFF630) // (PIOB) Set Output Data Register
#define AT91C_PIOB_ODSR           (0xFFFFF638) // (PIOB) Output Data Status Register
#define AT91C_PIOB_CODR           (0xFFFFF634) // (PIOB) Clear Output Data Register
#define AT91C_PIOB_PDSR           (0xFFFFF63C) // (PIOB) Pin Data Status Register
#define AT91C_PIOB_OWER           (0xFFFFF6A0) // (PIOB) Output Write Enable Register
#define AT91C_PIOB_IER            (0xFFFFF640) // (PIOB) Interrupt Enable Register
#define AT91C_PIOB_OWDR           (0xFFFFF6A4) // (PIOB) Output Write Disable Register
#define AT91C_PIOB_MDDR           (0xFFFFF654) // (PIOB) Multi-driver Disable Register
#define AT91C_PIOB_ISR            (0xFFFFF64C) // (PIOB) Interrupt Status Register
#define AT91C_PIOB_IDR            (0xFFFFF644) // (PIOB) Interrupt Disable Register
#define AT91C_PIOB_PDR            (0xFFFFF604) // (PIOB) PIO Disable Register
// ========== Register definition for PIOA peripheral ========== 
#define AT91C_PIOA_IMR            (0xFFFFF448) // (PIOA) Interrupt Mask Register
#define AT91C_PIOA_IER            (0xFFFFF440) // (PIOA) Interrupt Enable Register
#define AT91C_PIOA_OWDR           (0xFFFFF4A4) // (PIOA) Output Write Disable Register
#define AT91C_PIOA_ISR            (0xFFFFF44C) // (PIOA) Interrupt Status Register
#define AT91C_PIOA_PPUDR          (0xFFFFF460) // (PIOA) Pull-up Disable Register
#define AT91C_PIOA_MDSR           (0xFFFFF458) // (PIOA) Multi-driver Status Register
#define AT91C_PIOA_MDER           (0xFFFFF450) // (PIOA) Multi-driver Enable Register
#define AT91C_PIOA_PER            (0xFFFFF400) // (PIOA) PIO Enable Register
#define AT91C_PIOA_PSR            (0xFFFFF408) // (PIOA) PIO Status Register
#define AT91C_PIOA_OER            (0xFFFFF410) // (PIOA) Output Enable Register
#define AT91C_PIOA_BSR            (0xFFFFF474) // (PIOA) Select B Register
#define AT91C_PIOA_PPUER          (0xFFFFF464) // (PIOA) Pull-up Enable Register
#define AT91C_PIOA_MDDR           (0xFFFFF454) // (PIOA) Multi-driver Disable Register
#define AT91C_PIOA_PDR            (0xFFFFF404) // (PIOA) PIO Disable Register
#define AT91C_PIOA_ODR            (0xFFFFF414) // (PIOA) Output Disable Registerr
#define AT91C_PIOA_IFDR           (0xFFFFF424) // (PIOA) Input Filter Disable Register
#define AT91C_PIOA_ABSR           (0xFFFFF478) // (PIOA) AB Select Status Register
#define AT91C_PIOA_ASR            (0xFFFFF470) // (PIOA) Select A Register
#define AT91C_PIOA_PPUSR          (0xFFFFF468) // (PIOA) Pad Pull-up Status Register
#define AT91C_PIOA_ODSR           (0xFFFFF438) // (PIOA) Output Data Status Register
#define AT91C_PIOA_SODR           (0xFFFFF430) // (PIOA) Set Output Data Register
#define AT91C_PIOA_IFSR           (0xFFFFF428) // (PIOA) Input Filter Status Register
#define AT91C_PIOA_IFER           (0xFFFFF420) // (PIOA) Input Filter Enable Register
#define AT91C_PIOA_OSR            (0xFFFFF418) // (PIOA) Output Status Register
#define AT91C_PIOA_IDR            (0xFFFFF444) // (PIOA) Interrupt Disable Register
#define AT91C_PIOA_PDSR           (0xFFFFF43C) // (PIOA) Pin Data Status Register
#define AT91C_PIOA_CODR           (0xFFFFF434) // (PIOA) Clear Output Data Register
#define AT91C_PIOA_OWSR           (0xFFFFF4A8) // (PIOA) Output Write Status Register
#define AT91C_PIOA_OWER           (0xFFFFF4A0) // (PIOA) Output Write Enable Register
// ========== Register definition for DBGU peripheral ========== 
#define AT91C_DBGU_C2R            (0xFFFFF244) // (DBGU) Chip ID2 Register
#define AT91C_DBGU_THR            (0xFFFFF21C) // (DBGU) Transmitter Holding Register
#define AT91C_DBGU_CSR            (0xFFFFF214) // (DBGU) Channel Status Register
#define AT91C_DBGU_IDR            (0xFFFFF20C) // (DBGU) Interrupt Disable Register
#define AT91C_DBGU_MR             (0xFFFFF204) // (DBGU) Mode Register
#define AT91C_DBGU_FNTR           (0xFFFFF248) // (DBGU) Force NTRST Register
#define AT91C_DBGU_C1R            (0xFFFFF240) // (DBGU) Chip ID1 Register
#define AT91C_DBGU_BRGR           (0xFFFFF220) // (DBGU) Baud Rate Generator Register
#define AT91C_DBGU_RHR            (0xFFFFF218) // (DBGU) Receiver Holding Register
#define AT91C_DBGU_IMR            (0xFFFFF210) // (DBGU) Interrupt Mask Register
#define AT91C_DBGU_IER            (0xFFFFF208) // (DBGU) Interrupt Enable Register
#define AT91C_DBGU_CR             (0xFFFFF200) // (DBGU) Control Register
// ========== Register definition for PDC_DBGU peripheral ========== 
#define AT91C_DBGU_TNCR           (0xFFFFF31C) // (PDC_DBGU) Transmit Next Counter Register
#define AT91C_DBGU_RNCR           (0xFFFFF314) // (PDC_DBGU) Receive Next Counter Register
#define AT91C_DBGU_PTCR           (0xFFFFF320) // (PDC_DBGU) PDC Transfer Control Register
#define AT91C_DBGU_PTSR           (0xFFFFF324) // (PDC_DBGU) PDC Transfer Status Register
#define AT91C_DBGU_RCR            (0xFFFFF304) // (PDC_DBGU) Receive Counter Register
#define AT91C_DBGU_TCR            (0xFFFFF30C) // (PDC_DBGU) Transmit Counter Register
#define AT91C_DBGU_RPR            (0xFFFFF300) // (PDC_DBGU) Receive Pointer Register
#define AT91C_DBGU_TPR            (0xFFFFF308) // (PDC_DBGU) Transmit Pointer Register
#define AT91C_DBGU_RNPR           (0xFFFFF310) // (PDC_DBGU) Receive Next Pointer Register
#define AT91C_DBGU_TNPR           (0xFFFFF318) // (PDC_DBGU) Transmit Next Pointer Register
// ========== Register definition for AIC peripheral ========== 
#define AT91C_AIC_ICCR            (0xFFFFF128) // (AIC) Interrupt Clear Command Register
#define AT91C_AIC_IECR            (0xFFFFF120) // (AIC) Interrupt Enable Command Register
#define AT91C_AIC_SMR             (0xFFFFF000) // (AIC) Source Mode Register
#define AT91C_AIC_ISCR            (0xFFFFF12C) // (AIC) Interrupt Set Command Register
#define AT91C_AIC_EOICR           (0xFFFFF130) // (AIC) End of Interrupt Command Register
#define AT91C_AIC_DCR             (0xFFFFF138) // (AIC) Debug Control Register (Protect)
#define AT91C_AIC_FFER            (0xFFFFF140) // (AIC) Fast Forcing Enable Register
#define AT91C_AIC_SVR             (0xFFFFF080) // (AIC) Source Vector Register
#define AT91C_AIC_SPU             (0xFFFFF134) // (AIC) Spurious Vector Register
#define AT91C_AIC_FFDR            (0xFFFFF144) // (AIC) Fast Forcing Disable Register
#define AT91C_AIC_FVR             (0xFFFFF104) // (AIC) FIQ Vector Register
#define AT91C_AIC_FFSR            (0xFFFFF148) // (AIC) Fast Forcing Status Register
#define AT91C_AIC_IMR             (0xFFFFF110) // (AIC) Interrupt Mask Register
#define AT91C_AIC_ISR             (0xFFFFF108) // (AIC) Interrupt Status Register
#define AT91C_AIC_IVR             (0xFFFFF100) // (AIC) IRQ Vector Register
#define AT91C_AIC_IDCR            (0xFFFFF124) // (AIC) Interrupt Disable Command Register
#define AT91C_AIC_CISR            (0xFFFFF114) // (AIC) Core Interrupt Status Register
#define AT91C_AIC_IPR             (0xFFFFF10C) // (AIC) Interrupt Pending Register
// ========== Register definition for PDC_SPI peripheral ========== 
#define AT91C_SPI_PTCR            (0xFFFE0120) // (PDC_SPI) PDC Transfer Control Register
#define AT91C_SPI_TNPR            (0xFFFE0118) // (PDC_SPI) Transmit Next Pointer Register
#define AT91C_SPI_RNPR            (0xFFFE0110) // (PDC_SPI) Receive Next Pointer Register
#define AT91C_SPI_TPR             (0xFFFE0108) // (PDC_SPI) Transmit Pointer Register
#define AT91C_SPI_RPR             (0xFFFE0100) // (PDC_SPI) Receive Pointer Register
#define AT91C_SPI_PTSR            (0xFFFE0124) // (PDC_SPI) PDC Transfer Status Register
#define AT91C_SPI_TNCR            (0xFFFE011C) // (PDC_SPI) Transmit Next Counter Register
#define AT91C_SPI_RNCR            (0xFFFE0114) // (PDC_SPI) Receive Next Counter Register
#define AT91C_SPI_TCR             (0xFFFE010C) // (PDC_SPI) Transmit Counter Register
#define AT91C_SPI_RCR             (0xFFFE0104) // (PDC_SPI) Receive Counter Register
// ========== Register definition for SPI peripheral ========== 
#define AT91C_SPI_CSR             (0xFFFE0030) // (SPI) Chip Select Register
#define AT91C_SPI_IDR             (0xFFFE0018) // (SPI) Interrupt Disable Register
#define AT91C_SPI_SR              (0xFFFE0010) // (SPI) Status Register
#define AT91C_SPI_RDR             (0xFFFE0008) // (SPI) Receive Data Register
#define AT91C_SPI_CR              (0xFFFE0000) // (SPI) Control Register
#define AT91C_SPI_IMR             (0xFFFE001C) // (SPI) Interrupt Mask Register
#define AT91C_SPI_IER             (0xFFFE0014) // (SPI) Interrupt Enable Register
#define AT91C_SPI_TDR             (0xFFFE000C) // (SPI) Transmit Data Register
#define AT91C_SPI_MR              (0xFFFE0004) // (SPI) Mode Register
// ========== Register definition for PDC_SSC2 peripheral ========== 
#define AT91C_SSC2_PTCR           (0xFFFD8120) // (PDC_SSC2) PDC Transfer Control Register
#define AT91C_SSC2_TNPR           (0xFFFD8118) // (PDC_SSC2) Transmit Next Pointer Register
#define AT91C_SSC2_RNPR           (0xFFFD8110) // (PDC_SSC2) Receive Next Pointer Register
#define AT91C_SSC2_TPR            (0xFFFD8108) // (PDC_SSC2) Transmit Pointer Register
#define AT91C_SSC2_RPR            (0xFFFD8100) // (PDC_SSC2) Receive Pointer Register
#define AT91C_SSC2_PTSR           (0xFFFD8124) // (PDC_SSC2) PDC Transfer Status Register
#define AT91C_SSC2_TNCR           (0xFFFD811C) // (PDC_SSC2) Transmit Next Counter Register
#define AT91C_SSC2_RNCR           (0xFFFD8114) // (PDC_SSC2) Receive Next Counter Register
#define AT91C_SSC2_TCR            (0xFFFD810C) // (PDC_SSC2) Transmit Counter Register
#define AT91C_SSC2_RCR            (0xFFFD8104) // (PDC_SSC2) Receive Counter Register
// ========== Register definition for SSC2 peripheral ========== 
#define AT91C_SSC2_IMR            (0xFFFD804C) // (SSC2) Interrupt Mask Register
#define AT91C_SSC2_IER            (0xFFFD8044) // (SSC2) Interrupt Enable Register
#define AT91C_SSC2_RC1R           (0xFFFD803C) // (SSC2) Receive Compare 1 Register
#define AT91C_SSC2_TSHR           (0xFFFD8034) // (SSC2) Transmit Sync Holding Register
#define AT91C_SSC2_CMR            (0xFFFD8004) // (SSC2) Clock Mode Register
#define AT91C_SSC2_IDR            (0xFFFD8048) // (SSC2) Interrupt Disable Register
#define AT91C_SSC2_TCMR           (0xFFFD8018) // (SSC2) Transmit Clock Mode Register
#define AT91C_SSC2_RCMR           (0xFFFD8010) // (SSC2) Receive Clock ModeRegister
#define AT91C_SSC2_CR             (0xFFFD8000) // (SSC2) Control Register
#define AT91C_SSC2_RFMR           (0xFFFD8014) // (SSC2) Receive Frame Mode Register
#define AT91C_SSC2_TFMR           (0xFFFD801C) // (SSC2) Transmit Frame Mode Register
#define AT91C_SSC2_THR            (0xFFFD8024) // (SSC2) Transmit Holding Register
#define AT91C_SSC2_SR             (0xFFFD8040) // (SSC2) Status Register
#define AT91C_SSC2_RC0R           (0xFFFD8038) // (SSC2) Receive Compare 0 Register
#define AT91C_SSC2_RSHR           (0xFFFD8030) // (SSC2) Receive Sync Holding Register
#define AT91C_SSC2_RHR            (0xFFFD8020) // (SSC2) Receive Holding Register
// ========== Register definition for PDC_SSC1 peripheral ========== 
#define AT91C_SSC1_PTCR           (0xFFFD4120) // (PDC_SSC1) PDC Transfer Control Register
#define AT91C_SSC1_TNPR           (0xFFFD4118) // (PDC_SSC1) Transmit Next Pointer Register
#define AT91C_SSC1_RNPR           (0xFFFD4110) // (PDC_SSC1) Receive Next Pointer Register
#define AT91C_SSC1_TPR            (0xFFFD4108) // (PDC_SSC1) Transmit Pointer Register
#define AT91C_SSC1_RPR            (0xFFFD4100) // (PDC_SSC1) Receive Pointer Register
#define AT91C_SSC1_PTSR           (0xFFFD4124) // (PDC_SSC1) PDC Transfer Status Register
#define AT91C_SSC1_TNCR           (0xFFFD411C) // (PDC_SSC1) Transmit Next Counter Register
#define AT91C_SSC1_RNCR           (0xFFFD4114) // (PDC_SSC1) Receive Next Counter Register
#define AT91C_SSC1_TCR            (0xFFFD410C) // (PDC_SSC1) Transmit Counter Register
#define AT91C_SSC1_RCR            (0xFFFD4104) // (PDC_SSC1) Receive Counter Register
// ========== Register definition for SSC1 peripheral ========== 
#define AT91C_SSC1_RFMR           (0xFFFD4014) // (SSC1) Receive Frame Mode Register
#define AT91C_SSC1_CMR            (0xFFFD4004) // (SSC1) Clock Mode Register
#define AT91C_SSC1_IDR            (0xFFFD4048) // (SSC1) Interrupt Disable Register
#define AT91C_SSC1_SR             (0xFFFD4040) // (SSC1) Status Register
#define AT91C_SSC1_RC0R           (0xFFFD4038) // (SSC1) Receive Compare 0 Register
#define AT91C_SSC1_RSHR           (0xFFFD4030) // (SSC1) Receive Sync Holding Register
#define AT91C_SSC1_RHR            (0xFFFD4020) // (SSC1) Receive Holding Register
#define AT91C_SSC1_TCMR           (0xFFFD4018) // (SSC1) Transmit Clock Mode Register
#define AT91C_SSC1_RCMR           (0xFFFD4010) // (SSC1) Receive Clock ModeRegister
#define AT91C_SSC1_CR             (0xFFFD4000) // (SSC1) Control Register
#define AT91C_SSC1_IMR            (0xFFFD404C) // (SSC1) Interrupt Mask Register
#define AT91C_SSC1_IER            (0xFFFD4044) // (SSC1) Interrupt Enable Register
#define AT91C_SSC1_RC1R           (0xFFFD403C) // (SSC1) Receive Compare 1 Register
#define AT91C_SSC1_TSHR           (0xFFFD4034) // (SSC1) Transmit Sync Holding Register
#define AT91C_SSC1_THR            (0xFFFD4024) // (SSC1) Transmit Holding Register
#define AT91C_SSC1_TFMR           (0xFFFD401C) // (SSC1) Transmit Frame Mode Register
// ========== Register definition for PDC_SSC0 peripheral ========== 
#define AT91C_SSC0_PTCR           (0xFFFD0120) // (PDC_SSC0) PDC Transfer Control Register
#define AT91C_SSC0_TNPR           (0xFFFD0118) // (PDC_SSC0) Transmit Next Pointer Register
#define AT91C_SSC0_RNPR           (0xFFFD0110) // (PDC_SSC0) Receive Next Pointer Register
#define AT91C_SSC0_TPR            (0xFFFD0108) // (PDC_SSC0) Transmit Pointer Register
#define AT91C_SSC0_RPR            (0xFFFD0100) // (PDC_SSC0) Receive Pointer Register
#define AT91C_SSC0_PTSR           (0xFFFD0124) // (PDC_SSC0) PDC Transfer Status Register
#define AT91C_SSC0_TNCR           (0xFFFD011C) // (PDC_SSC0) Transmit Next Counter Register
#define AT91C_SSC0_RNCR           (0xFFFD0114) // (PDC_SSC0) Receive Next Counter Register
#define AT91C_SSC0_TCR            (0xFFFD010C) // (PDC_SSC0) Transmit Counter Register
#define AT91C_SSC0_RCR            (0xFFFD0104) // (PDC_SSC0) Receive Counter Register
// ========== Register definition for SSC0 peripheral ========== 
#define AT91C_SSC0_IMR            (0xFFFD004C) // (SSC0) Interrupt Mask Register
#define AT91C_SSC0_IER            (0xFFFD0044) // (SSC0) Interrupt Enable Register
#define AT91C_SSC0_RC1R           (0xFFFD003C) // (SSC0) Receive Compare 1 Register
#define AT91C_SSC0_TSHR           (0xFFFD0034) // (SSC0) Transmit Sync Holding Register
#define AT91C_SSC0_THR            (0xFFFD0024) // (SSC0) Transmit Holding Register
#define AT91C_SSC0_TFMR           (0xFFFD001C) // (SSC0) Transmit Frame Mode Register
#define AT91C_SSC0_RFMR           (0xFFFD0014) // (SSC0) Receive Frame Mode Register
#define AT91C_SSC0_CMR            (0xFFFD0004) // (SSC0) Clock Mode Register
#define AT91C_SSC0_IDR            (0xFFFD0048) // (SSC0) Interrupt Disable Register
#define AT91C_SSC0_SR             (0xFFFD0040) // (SSC0) Status Register
#define AT91C_SSC0_RC0R           (0xFFFD0038) // (SSC0) Receive Compare 0 Register
#define AT91C_SSC0_RSHR           (0xFFFD0030) // (SSC0) Receive Sync Holding Register
#define AT91C_SSC0_RHR            (0xFFFD0020) // (SSC0) Receive Holding Register
#define AT91C_SSC0_TCMR           (0xFFFD0018) // (SSC0) Transmit Clock Mode Register
#define AT91C_SSC0_RCMR           (0xFFFD0010) // (SSC0) Receive Clock ModeRegister
#define AT91C_SSC0_CR             (0xFFFD0000) // (SSC0) Control Register
// ========== Register definition for PDC_US3 peripheral ========== 
#define AT91C_US3_PTSR            (0xFFFCC124) // (PDC_US3) PDC Transfer Status Register
#define AT91C_US3_TNCR            (0xFFFCC11C) // (PDC_US3) Transmit Next Counter Register
#define AT91C_US3_RNCR            (0xFFFCC114) // (PDC_US3) Receive Next Counter Register
#define AT91C_US3_TCR             (0xFFFCC10C) // (PDC_US3) Transmit Counter Register
#define AT91C_US3_RCR             (0xFFFCC104) // (PDC_US3) Receive Counter Register
#define AT91C_US3_PTCR            (0xFFFCC120) // (PDC_US3) PDC Transfer Control Register
#define AT91C_US3_TNPR            (0xFFFCC118) // (PDC_US3) Transmit Next Pointer Register
#define AT91C_US3_RNPR            (0xFFFCC110) // (PDC_US3) Receive Next Pointer Register
#define AT91C_US3_TPR             (0xFFFCC108) // (PDC_US3) Transmit Pointer Register
#define AT91C_US3_RPR             (0xFFFCC100) // (PDC_US3) Receive Pointer Register
// ========== Register definition for US3 peripheral ========== 
#define AT91C_US3_IF              (0xFFFCC04C) // (US3) IRDA_FILTER Register
#define AT91C_US3_NER             (0xFFFCC044) // (US3) Nb Errors Register
#define AT91C_US3_RTOR            (0xFFFCC024) // (US3) Receiver Time-out Register
#define AT91C_US3_THR             (0xFFFCC01C) // (US3) Transmitter Holding Register
#define AT91C_US3_CSR             (0xFFFCC014) // (US3) Channel Status Register
#define AT91C_US3_IDR             (0xFFFCC00C) // (US3) Interrupt Disable Register
#define AT91C_US3_MR              (0xFFFCC004) // (US3) Mode Register
#define AT91C_US3_XXR             (0xFFFCC048) // (US3) XON_XOFF Register
#define AT91C_US3_FIDI            (0xFFFCC040) // (US3) FI_DI_Ratio Register
#define AT91C_US3_TTGR            (0xFFFCC028) // (US3) Transmitter Time-guard Register
#define AT91C_US3_BRGR            (0xFFFCC020) // (US3) Baud Rate Generator Register
#define AT91C_US3_RHR             (0xFFFCC018) // (US3) Receiver Holding Register
#define AT91C_US3_IMR             (0xFFFCC010) // (US3) Interrupt Mask Register
#define AT91C_US3_IER             (0xFFFCC008) // (US3) Interrupt Enable Register
#define AT91C_US3_CR              (0xFFFCC000) // (US3) Control Register
// ========== Register definition for PDC_US2 peripheral ========== 
#define AT91C_US2_PTSR            (0xFFFC8124) // (PDC_US2) PDC Transfer Status Register
#define AT91C_US2_TNCR            (0xFFFC811C) // (PDC_US2) Transmit Next Counter Register
#define AT91C_US2_RNCR            (0xFFFC8114) // (PDC_US2) Receive Next Counter Register
#define AT91C_US2_TCR             (0xFFFC810C) // (PDC_US2) Transmit Counter Register
#define AT91C_US2_PTCR            (0xFFFC8120) // (PDC_US2) PDC Transfer Control Register
#define AT91C_US2_RCR             (0xFFFC8104) // (PDC_US2) Receive Counter Register
#define AT91C_US2_TNPR            (0xFFFC8118) // (PDC_US2) Transmit Next Pointer Register
#define AT91C_US2_RPR             (0xFFFC8100) // (PDC_US2) Receive Pointer Register
#define AT91C_US2_TPR             (0xFFFC8108) // (PDC_US2) Transmit Pointer Register
#define AT91C_US2_RNPR            (0xFFFC8110) // (PDC_US2) Receive Next Pointer Register
// ========== Register definition for US2 peripheral ========== 
#define AT91C_US2_XXR             (0xFFFC8048) // (US2) XON_XOFF Register
#define AT91C_US2_FIDI            (0xFFFC8040) // (US2) FI_DI_Ratio Register
#define AT91C_US2_TTGR            (0xFFFC8028) // (US2) Transmitter Time-guard Register
#define AT91C_US2_BRGR            (0xFFFC8020) // (US2) Baud Rate Generator Register
#define AT91C_US2_RHR             (0xFFFC8018) // (US2) Receiver Holding Register
#define AT91C_US2_IMR             (0xFFFC8010) // (US2) Interrupt Mask Register
#define AT91C_US2_IER             (0xFFFC8008) // (US2) Interrupt Enable Register
#define AT91C_US2_CR              (0xFFFC8000) // (US2) Control Register
#define AT91C_US2_IF              (0xFFFC804C) // (US2) IRDA_FILTER Register
#define AT91C_US2_NER             (0xFFFC8044) // (US2) Nb Errors Register
#define AT91C_US2_RTOR            (0xFFFC8024) // (US2) Receiver Time-out Register
#define AT91C_US2_THR             (0xFFFC801C) // (US2) Transmitter Holding Register
#define AT91C_US2_CSR             (0xFFFC8014) // (US2) Channel Status Register
#define AT91C_US2_IDR             (0xFFFC800C) // (US2) Interrupt Disable Register
#define AT91C_US2_MR              (0xFFFC8004) // (US2) Mode Register
// ========== Register definition for PDC_US1 peripheral ========== 
#define AT91C_US1_PTSR            (0xFFFC4124) // (PDC_US1) PDC Transfer Status Register
#define AT91C_US1_TNCR            (0xFFFC411C) // (PDC_US1) Transmit Next Counter Register
#define AT91C_US1_RNCR            (0xFFFC4114) // (PDC_US1) Receive Next Counter Register
#define AT91C_US1_TCR             (0xFFFC410C) // (PDC_US1) Transmit Counter Register
#define AT91C_US1_RCR             (0xFFFC4104) // (PDC_US1) Receive Counter Register
#define AT91C_US1_PTCR            (0xFFFC4120) // (PDC_US1) PDC Transfer Control Register
#define AT91C_US1_TNPR            (0xFFFC4118) // (PDC_US1) Transmit Next Pointer Register
#define AT91C_US1_RNPR            (0xFFFC4110) // (PDC_US1) Receive Next Pointer Register
#define AT91C_US1_TPR             (0xFFFC4108) // (PDC_US1) Transmit Pointer Register
#define AT91C_US1_RPR             (0xFFFC4100) // (PDC_US1) Receive Pointer Register
// ========== Register definition for US1 peripheral ========== 
#define AT91C_US1_XXR             (0xFFFC4048) // (US1) XON_XOFF Register
#define AT91C_US1_RHR             (0xFFFC4018) // (US1) Receiver Holding Register
#define AT91C_US1_IMR             (0xFFFC4010) // (US1) Interrupt Mask Register
#define AT91C_US1_IER             (0xFFFC4008) // (US1) Interrupt Enable Register
#define AT91C_US1_CR              (0xFFFC4000) // (US1) Control Register
#define AT91C_US1_RTOR            (0xFFFC4024) // (US1) Receiver Time-out Register
#define AT91C_US1_THR             (0xFFFC401C) // (US1) Transmitter Holding Register
#define AT91C_US1_CSR             (0xFFFC4014) // (US1) Channel Status Register
#define AT91C_US1_IDR             (0xFFFC400C) // (US1) Interrupt Disable Register
#define AT91C_US1_FIDI            (0xFFFC4040) // (US1) FI_DI_Ratio Register
#define AT91C_US1_BRGR            (0xFFFC4020) // (US1) Baud Rate Generator Register
#define AT91C_US1_TTGR            (0xFFFC4028) // (US1) Transmitter Time-guard Register
#define AT91C_US1_IF              (0xFFFC404C) // (US1) IRDA_FILTER Register
#define AT91C_US1_NER             (0xFFFC4044) // (US1) Nb Errors Register
#define AT91C_US1_MR              (0xFFFC4004) // (US1) Mode Register
// ========== Register definition for PDC_US0 peripheral ========== 
#define AT91C_US0_PTCR            (0xFFFC0120) // (PDC_US0) PDC Transfer Control Register
#define AT91C_US0_TNPR            (0xFFFC0118) // (PDC_US0) Transmit Next Pointer Register
#define AT91C_US0_RNPR            (0xFFFC0110) // (PDC_US0) Receive Next Pointer Register
#define AT91C_US0_TPR             (0xFFFC0108) // (PDC_US0) Transmit Pointer Register
#define AT91C_US0_RPR             (0xFFFC0100) // (PDC_US0) Receive Pointer Register
#define AT91C_US0_PTSR            (0xFFFC0124) // (PDC_US0) PDC Transfer Status Register
#define AT91C_US0_TNCR            (0xFFFC011C) // (PDC_US0) Transmit Next Counter Register
#define AT91C_US0_RNCR            (0xFFFC0114) // (PDC_US0) Receive Next Counter Register
#define AT91C_US0_TCR             (0xFFFC010C) // (PDC_US0) Transmit Counter Register
#define AT91C_US0_RCR             (0xFFFC0104) // (PDC_US0) Receive Counter Register
// ========== Register definition for US0 peripheral ========== 
#define AT91C_US0_TTGR            (0xFFFC0028) // (US0) Transmitter Time-guard Register
#define AT91C_US0_BRGR            (0xFFFC0020) // (US0) Baud Rate Generator Register
#define AT91C_US0_RHR             (0xFFFC0018) // (US0) Receiver Holding Register
#define AT91C_US0_IMR             (0xFFFC0010) // (US0) Interrupt Mask Register
#define AT91C_US0_NER             (0xFFFC0044) // (US0) Nb Errors Register
#define AT91C_US0_RTOR            (0xFFFC0024) // (US0) Receiver Time-out Register
#define AT91C_US0_XXR             (0xFFFC0048) // (US0) XON_XOFF Register
#define AT91C_US0_FIDI            (0xFFFC0040) // (US0) FI_DI_Ratio Register
#define AT91C_US0_CR              (0xFFFC0000) // (US0) Control Register
#define AT91C_US0_IER             (0xFFFC0008) // (US0) Interrupt Enable Register
#define AT91C_US0_IF              (0xFFFC004C) // (US0) IRDA_FILTER Register
#define AT91C_US0_MR              (0xFFFC0004) // (US0) Mode Register
#define AT91C_US0_IDR             (0xFFFC000C) // (US0) Interrupt Disable Register
#define AT91C_US0_CSR             (0xFFFC0014) // (US0) Channel Status Register
#define AT91C_US0_THR             (0xFFFC001C) // (US0) Transmitter Holding Register
// ========== Register definition for TWI peripheral ========== 
#define AT91C_TWI_RHR             (0xFFFB8030) // (TWI) Receive Holding Register
#define AT91C_TWI_IDR             (0xFFFB8028) // (TWI) Interrupt Disable Register
#define AT91C_TWI_SR              (0xFFFB8020) // (TWI) Status Register
#define AT91C_TWI_CWGR            (0xFFFB8010) // (TWI) Clock Waveform Generator Register
#define AT91C_TWI_SMR             (0xFFFB8008) // (TWI) Slave Mode Register
#define AT91C_TWI_CR              (0xFFFB8000) // (TWI) Control Register
#define AT91C_TWI_THR             (0xFFFB8034) // (TWI) Transmit Holding Register
#define AT91C_TWI_IMR             (0xFFFB802C) // (TWI) Interrupt Mask Register
#define AT91C_TWI_IER             (0xFFFB8024) // (TWI) Interrupt Enable Register
#define AT91C_TWI_IADR            (0xFFFB800C) // (TWI) Internal Address Register
#define AT91C_TWI_MMR             (0xFFFB8004) // (TWI) Master Mode Register
// ========== Register definition for PDC_MCI peripheral ========== 
#define AT91C_MCI_PTCR            (0xFFFB4120) // (PDC_MCI) PDC Transfer Control Register
#define AT91C_MCI_TNPR            (0xFFFB4118) // (PDC_MCI) Transmit Next Pointer Register
#define AT91C_MCI_RNPR            (0xFFFB4110) // (PDC_MCI) Receive Next Pointer Register
#define AT91C_MCI_TPR             (0xFFFB4108) // (PDC_MCI) Transmit Pointer Register
#define AT91C_MCI_RPR             (0xFFFB4100) // (PDC_MCI) Receive Pointer Register
#define AT91C_MCI_PTSR            (0xFFFB4124) // (PDC_MCI) PDC Transfer Status Register
#define AT91C_MCI_TNCR            (0xFFFB411C) // (PDC_MCI) Transmit Next Counter Register
#define AT91C_MCI_RNCR            (0xFFFB4114) // (PDC_MCI) Receive Next Counter Register
#define AT91C_MCI_TCR             (0xFFFB410C) // (PDC_MCI) Transmit Counter Register
#define AT91C_MCI_RCR             (0xFFFB4104) // (PDC_MCI) Receive Counter Register
// ========== Register definition for MCI peripheral ========== 
#define AT91C_MCI_IDR             (0xFFFB4048) // (MCI) MCI Interrupt Disable Register
#define AT91C_MCI_SR              (0xFFFB4040) // (MCI) MCI Status Register
#define AT91C_MCI_RDR             (0xFFFB4030) // (MCI) MCI Receive Data Register
#define AT91C_MCI_RSPR            (0xFFFB4020) // (MCI) MCI Response Register
#define AT91C_MCI_ARGR            (0xFFFB4010) // (MCI) MCI Argument Register
#define AT91C_MCI_DTOR            (0xFFFB4008) // (MCI) MCI Data Timeout Register
#define AT91C_MCI_CR              (0xFFFB4000) // (MCI) MCI Control Register
#define AT91C_MCI_IMR             (0xFFFB404C) // (MCI) MCI Interrupt Mask Register
#define AT91C_MCI_IER             (0xFFFB4044) // (MCI) MCI Interrupt Enable Register
#define AT91C_MCI_TDR             (0xFFFB4034) // (MCI) MCI Transmit Data Register
#define AT91C_MCI_CMDR            (0xFFFB4014) // (MCI) MCI Command Register
#define AT91C_MCI_SDCR            (0xFFFB400C) // (MCI) MCI SD Card Register
#define AT91C_MCI_MR              (0xFFFB4004) // (MCI) MCI Mode Register
// ========== Register definition for UDP peripheral ========== 
#define AT91C_UDP_ISR             (0xFFFB001C) // (UDP) Interrupt Status Register
#define AT91C_UDP_IDR             (0xFFFB0014) // (UDP) Interrupt Disable Register
#define AT91C_UDP_GLBSTATE        (0xFFFB0004) // (UDP) Global State Register
#define AT91C_UDP_FDR             (0xFFFB0050) // (UDP) Endpoint FIFO Data Register
#define AT91C_UDP_CSR             (0xFFFB0030) // (UDP) Endpoint Control and Status Register
#define AT91C_UDP_RSTEP           (0xFFFB0028) // (UDP) Reset Endpoint Register
#define AT91C_UDP_ICR             (0xFFFB0020) // (UDP) Interrupt Clear Register
#define AT91C_UDP_IMR             (0xFFFB0018) // (UDP) Interrupt Mask Register
#define AT91C_UDP_IER             (0xFFFB0010) // (UDP) Interrupt Enable Register
#define AT91C_UDP_FADDR           (0xFFFB0008) // (UDP) Function Address Register
#define AT91C_UDP_NUM             (0xFFFB0000) // (UDP) Frame Number Register
// ========== Register definition for TC5 peripheral ========== 
#define AT91C_TC5_CMR             (0xFFFA4084) // (TC5) Channel Mode Register
#define AT91C_TC5_IDR             (0xFFFA40A8) // (TC5) Interrupt Disable Register
#define AT91C_TC5_SR              (0xFFFA40A0) // (TC5) Status Register
#define AT91C_TC5_RB              (0xFFFA4098) // (TC5) Register B
#define AT91C_TC5_CV              (0xFFFA4090) // (TC5) Counter Value
#define AT91C_TC5_CCR             (0xFFFA4080) // (TC5) Channel Control Register
#define AT91C_TC5_IMR             (0xFFFA40AC) // (TC5) Interrupt Mask Register
#define AT91C_TC5_IER             (0xFFFA40A4) // (TC5) Interrupt Enable Register
#define AT91C_TC5_RC              (0xFFFA409C) // (TC5) Register C
#define AT91C_TC5_RA              (0xFFFA4094) // (TC5) Register A
// ========== Register definition for TC4 peripheral ========== 
#define AT91C_TC4_IMR             (0xFFFA406C) // (TC4) Interrupt Mask Register
#define AT91C_TC4_IER             (0xFFFA4064) // (TC4) Interrupt Enable Register
#define AT91C_TC4_RC              (0xFFFA405C) // (TC4) Register C
#define AT91C_TC4_RA              (0xFFFA4054) // (TC4) Register A
#define AT91C_TC4_CMR             (0xFFFA4044) // (TC4) Channel Mode Register
#define AT91C_TC4_IDR             (0xFFFA4068) // (TC4) Interrupt Disable Register
#define AT91C_TC4_SR              (0xFFFA4060) // (TC4) Status Register
#define AT91C_TC4_RB              (0xFFFA4058) // (TC4) Register B
#define AT91C_TC4_CV              (0xFFFA4050) // (TC4) Counter Value
#define AT91C_TC4_CCR             (0xFFFA4040) // (TC4) Channel Control Register
// ========== Register definition for TC3 peripheral ========== 
#define AT91C_TC3_IMR             (0xFFFA402C) // (TC3) Interrupt Mask Register
#define AT91C_TC3_CV              (0xFFFA4010) // (TC3) Counter Value
#define AT91C_TC3_CCR             (0xFFFA4000) // (TC3) Channel Control Register
#define AT91C_TC3_IER             (0xFFFA4024) // (TC3) Interrupt Enable Register
#define AT91C_TC3_CMR             (0xFFFA4004) // (TC3) Channel Mode Register
#define AT91C_TC3_RA              (0xFFFA4014) // (TC3) Register A
#define AT91C_TC3_RC              (0xFFFA401C) // (TC3) Register C
#define AT91C_TC3_IDR             (0xFFFA4028) // (TC3) Interrupt Disable Register
#define AT91C_TC3_RB              (0xFFFA4018) // (TC3) Register B
#define AT91C_TC3_SR              (0xFFFA4020) // (TC3) Status Register
// ========== Register definition for TCB1 peripheral ========== 
#define AT91C_TCB1_BCR            (0xFFFA4140) // (TCB1) TC Block Control Register
#define AT91C_TCB1_BMR            (0xFFFA4144) // (TCB1) TC Block Mode Register
// ========== Register definition for TC2 peripheral ========== 
#define AT91C_TC2_IMR             (0xFFFA00AC) // (TC2) Interrupt Mask Register
#define AT91C_TC2_IER             (0xFFFA00A4) // (TC2) Interrupt Enable Register
#define AT91C_TC2_RC              (0xFFFA009C) // (TC2) Register C
#define AT91C_TC2_RA              (0xFFFA0094) // (TC2) Register A
#define AT91C_TC2_CMR             (0xFFFA0084) // (TC2) Channel Mode Register
#define AT91C_TC2_IDR             (0xFFFA00A8) // (TC2) Interrupt Disable Register
#define AT91C_TC2_SR              (0xFFFA00A0) // (TC2) Status Register
#define AT91C_TC2_RB              (0xFFFA0098) // (TC2) Register B
#define AT91C_TC2_CV              (0xFFFA0090) // (TC2) Counter Value
#define AT91C_TC2_CCR             (0xFFFA0080) // (TC2) Channel Control Register
// ========== Register definition for TC1 peripheral ========== 
#define AT91C_TC1_IMR             (0xFFFA006C) // (TC1) Interrupt Mask Register
#define AT91C_TC1_IER             (0xFFFA0064) // (TC1) Interrupt Enable Register
#define AT91C_TC1_RC              (0xFFFA005C) // (TC1) Register C
#define AT91C_TC1_RA              (0xFFFA0054) // (TC1) Register A
#define AT91C_TC1_CMR             (0xFFFA0044) // (TC1) Channel Mode Register
#define AT91C_TC1_IDR             (0xFFFA0068) // (TC1) Interrupt Disable Register
#define AT91C_TC1_SR              (0xFFFA0060) // (TC1) Status Register
#define AT91C_TC1_RB              (0xFFFA0058) // (TC1) Register B
#define AT91C_TC1_CV              (0xFFFA0050) // (TC1) Counter Value
#define AT91C_TC1_CCR             (0xFFFA0040) // (TC1) Channel Control Register
// ========== Register definition for TC0 peripheral ========== 
#define AT91C_TC0_IMR             (0xFFFA002C) // (TC0) Interrupt Mask Register
#define AT91C_TC0_IER             (0xFFFA0024) // (TC0) Interrupt Enable Register
#define AT91C_TC0_RC              (0xFFFA001C) // (TC0) Register C
#define AT91C_TC0_RA              (0xFFFA0014) // (TC0) Register A
#define AT91C_TC0_CMR             (0xFFFA0004) // (TC0) Channel Mode Register
#define AT91C_TC0_IDR             (0xFFFA0028) // (TC0) Interrupt Disable Register
#define AT91C_TC0_SR              (0xFFFA0020) // (TC0) Status Register
#define AT91C_TC0_RB              (0xFFFA0018) // (TC0) Register B
#define AT91C_TC0_CV              (0xFFFA0010) // (TC0) Counter Value
#define AT91C_TC0_CCR             (0xFFFA0000) // (TC0) Channel Control Register
// ========== Register definition for TCB0 peripheral ========== 
#define AT91C_TCB0_BMR            (0xFFFA00C4) // (TCB0) TC Block Mode Register
#define AT91C_TCB0_BCR            (0xFFFA00C0) // (TCB0) TC Block Control Register
// ========== Register definition for UHP peripheral ========== 
#define AT91C_UHP_HcRhDescriptorA (0x00300048) // (UHP) Root Hub characteristics A
#define AT91C_UHP_HcRhPortStatus  (0x00300054) // (UHP) Root Hub Port Status Register
#define AT91C_UHP_HcRhDescriptorB (0x0030004C) // (UHP) Root Hub characteristics B
#define AT91C_UHP_HcControl       (0x00300004) // (UHP) Operating modes for the Host Controller
#define AT91C_UHP_HcInterruptStatus (0x0030000C) // (UHP) Interrupt Status Register
#define AT91C_UHP_HcRhStatus      (0x00300050) // (UHP) Root Hub Status register
#define AT91C_UHP_HcRevision      (0x00300000) // (UHP) Revision
#define AT91C_UHP_HcCommandStatus (0x00300008) // (UHP) Command & status Register
#define AT91C_UHP_HcInterruptEnable (0x00300010) // (UHP) Interrupt Enable Register
#define AT91C_UHP_HcHCCA          (0x00300018) // (UHP) Pointer to the Host Controller Communication Area
#define AT91C_UHP_HcControlHeadED (0x00300020) // (UHP) First Endpoint Descriptor of the Control list
#define AT91C_UHP_HcInterruptDisable (0x00300014) // (UHP) Interrupt Disable Register
#define AT91C_UHP_HcPeriodCurrentED (0x0030001C) // (UHP) Current Isochronous or Interrupt Endpoint Descriptor
#define AT91C_UHP_HcControlCurrentED (0x00300024) // (UHP) Endpoint Control and Status Register
#define AT91C_UHP_HcBulkCurrentED (0x0030002C) // (UHP) Current endpoint of the Bulk list
#define AT91C_UHP_HcFmInterval    (0x00300034) // (UHP) Bit time between 2 consecutive SOFs
#define AT91C_UHP_HcBulkHeadED    (0x00300028) // (UHP) First endpoint register of the Bulk list
#define AT91C_UHP_HcBulkDoneHead  (0x00300030) // (UHP) Last completed transfer descriptor
#define AT91C_UHP_HcFmRemaining   (0x00300038) // (UHP) Bit time remaining in the current Frame
#define AT91C_UHP_HcPeriodicStart (0x00300040) // (UHP) Periodic Start
#define AT91C_UHP_HcLSThreshold   (0x00300044) // (UHP) LS Threshold
#define AT91C_UHP_HcFmNumber      (0x0030003C) // (UHP) Frame number
// ========== Register definition for EMAC peripheral ========== 
#define AT91C_EMAC_RSR            (0xFFFBC020) // (EMAC) Receive Status Register
#define AT91C_EMAC_MAN            (0xFFFBC034) // (EMAC) PHY Maintenance Register
#define AT91C_EMAC_HSH            (0xFFFBC090) // (EMAC) Hash Address High[63:32]
#define AT91C_EMAC_MCOL           (0xFFFBC048) // (EMAC) Multiple Collision Frame Register
#define AT91C_EMAC_IER            (0xFFFBC028) // (EMAC) Interrupt Enable Register
#define AT91C_EMAC_SA2H           (0xFFFBC0A4) // (EMAC) Specific Address 2 High, Last 2 bytes
#define AT91C_EMAC_HSL            (0xFFFBC094) // (EMAC) Hash Address Low[31:0]
#define AT91C_EMAC_LCOL           (0xFFFBC05C) // (EMAC) Late Collision Register
#define AT91C_EMAC_OK             (0xFFFBC04C) // (EMAC) Frames Received OK Register
#define AT91C_EMAC_CFG            (0xFFFBC004) // (EMAC) Network Configuration Register
#define AT91C_EMAC_SA3L           (0xFFFBC0A8) // (EMAC) Specific Address 3 Low, First 4 bytes
#define AT91C_EMAC_SEQE           (0xFFFBC050) // (EMAC) Frame Check Sequence Error Register
#define AT91C_EMAC_ECOL           (0xFFFBC060) // (EMAC) Excessive Collision Register
#define AT91C_EMAC_ELR            (0xFFFBC070) // (EMAC) Excessive Length Error Register
#define AT91C_EMAC_SR             (0xFFFBC008) // (EMAC) Network Status Register
#define AT91C_EMAC_RBQP           (0xFFFBC018) // (EMAC) Receive Buffer Queue Pointer
#define AT91C_EMAC_CSE            (0xFFFBC064) // (EMAC) Carrier Sense Error Register
#define AT91C_EMAC_RJB            (0xFFFBC074) // (EMAC) Receive Jabber Register
#define AT91C_EMAC_USF            (0xFFFBC078) // (EMAC) Undersize Frame Register
#define AT91C_EMAC_IDR            (0xFFFBC02C) // (EMAC) Interrupt Disable Register
#define AT91C_EMAC_SA1L           (0xFFFBC098) // (EMAC) Specific Address 1 Low, First 4 bytes
#define AT91C_EMAC_IMR            (0xFFFBC030) // (EMAC) Interrupt Mask Register
#define AT91C_EMAC_FRA            (0xFFFBC040) // (EMAC) Frames Transmitted OK Register
#define AT91C_EMAC_SA3H           (0xFFFBC0AC) // (EMAC) Specific Address 3 High, Last 2 bytes
#define AT91C_EMAC_SA1H           (0xFFFBC09C) // (EMAC) Specific Address 1 High, Last 2 bytes
#define AT91C_EMAC_SCOL           (0xFFFBC044) // (EMAC) Single Collision Frame Register
#define AT91C_EMAC_ALE            (0xFFFBC054) // (EMAC) Alignment Error Register
#define AT91C_EMAC_TAR            (0xFFFBC00C) // (EMAC) Transmit Address Register
#define AT91C_EMAC_SA4L           (0xFFFBC0B0) // (EMAC) Specific Address 4 Low, First 4 bytes
#define AT91C_EMAC_SA2L           (0xFFFBC0A0) // (EMAC) Specific Address 2 Low, First 4 bytes
#define AT91C_EMAC_TUE            (0xFFFBC068) // (EMAC) Transmit Underrun Error Register
#define AT91C_EMAC_DTE            (0xFFFBC058) // (EMAC) Deferred Transmission Frame Register
#define AT91C_EMAC_TCR            (0xFFFBC010) // (EMAC) Transmit Control Register
#define AT91C_EMAC_CTL            (0xFFFBC000) // (EMAC) Network Control Register
#define AT91C_EMAC_SA4H           (0xFFFBC0B4) // (EMAC) Specific Address 4 High, Last 2 bytesr
#define AT91C_EMAC_CDE            (0xFFFBC06C) // (EMAC) Code Error Register
#define AT91C_EMAC_SQEE           (0xFFFBC07C) // (EMAC) SQE Test Error Register
#define AT91C_EMAC_TSR            (0xFFFBC014) // (EMAC) Transmit Status Register
#define AT91C_EMAC_DRFC           (0xFFFBC080) // (EMAC) Discarded RX Frame Register
// ========== Register definition for EBI peripheral ========== 
#define AT91C_EBI_CFGR            (0xFFFFFF64) // (EBI) Configuration Register
#define AT91C_EBI_CSA             (0xFFFFFF60) // (EBI) Chip Select Assignment Register
// ========== Register definition for SMC2 peripheral ========== 
#define AT91C_SMC2_CSR            (0xFFFFFF70) // (SMC2) SMC2 Chip Select Register
// ========== Register definition for SDRC peripheral ========== 
#define AT91C_SDRC_IMR            (0xFFFFFFAC) // (SDRC) SDRAM Controller Interrupt Mask Register
#define AT91C_SDRC_IER            (0xFFFFFFA4) // (SDRC) SDRAM Controller Interrupt Enable Register
#define AT91C_SDRC_SRR            (0xFFFFFF9C) // (SDRC) SDRAM Controller Self Refresh Register
#define AT91C_SDRC_TR             (0xFFFFFF94) // (SDRC) SDRAM Controller Refresh Timer Register
#define AT91C_SDRC_ISR            (0xFFFFFFB0) // (SDRC) SDRAM Controller Interrupt Mask Register
#define AT91C_SDRC_IDR            (0xFFFFFFA8) // (SDRC) SDRAM Controller Interrupt Disable Register
#define AT91C_SDRC_LPR            (0xFFFFFFA0) // (SDRC) SDRAM Controller Low Power Register
#define AT91C_SDRC_CR             (0xFFFFFF98) // (SDRC) SDRAM Controller Configuration Register
#define AT91C_SDRC_MR             (0xFFFFFF90) // (SDRC) SDRAM Controller Mode Register
// ========== Register definition for BFC peripheral ========== 
#define AT91C_BFC_MR              (0xFFFFFFC0) // (BFC) BFC Mode Register

// *****************************************************************************
//               PIO DEFINITIONS FOR AT91RM9200
// *****************************************************************************
#define AT91C_PIO_PA0             (1 <<  0) // Pin Controlled by PA0
#define AT91C_PA0_MISO            (AT91C_PIO_PA0) //  SPI Master In Slave
#define AT91C_PA0_PCK3            (AT91C_PIO_PA0) //  PMC Programmable Clock Output 3
#define AT91C_PIO_PA1             (1 <<  1) // Pin Controlled by PA1
#define AT91C_PA1_MOSI            (AT91C_PIO_PA1) //  SPI Master Out Slave
#define AT91C_PA1_PCK0            (AT91C_PIO_PA1) //  PMC Programmable Clock Output 0
#define AT91C_PIO_PA10            (1 << 10) // Pin Controlled by PA10
#define AT91C_PA10_ETX1           (AT91C_PIO_PA10) //  Ethernet MAC Transmit Data 1
#define AT91C_PA10_MCDB1          (AT91C_PIO_PA10) //  Multimedia Card B Data 1
#define AT91C_PIO_PA11            (1 << 11) // Pin Controlled by PA11
#define AT91C_PA11_ECRS_ECRSDV    (AT91C_PIO_PA11) //  Ethernet MAC Carrier Sense/Carrier Sense and Data Valid
#define AT91C_PA11_MCDB2          (AT91C_PIO_PA11) //  Multimedia Card B Data 2
#define AT91C_PIO_PA12            (1 << 12) // Pin Controlled by PA12
#define AT91C_PA12_ERX0           (AT91C_PIO_PA12) //  Ethernet MAC Receive Data 0
#define AT91C_PA12_MCDB3          (AT91C_PIO_PA12) //  Multimedia Card B Data 3
#define AT91C_PIO_PA13            (1 << 13) // Pin Controlled by PA13
#define AT91C_PA13_ERX1           (AT91C_PIO_PA13) //  Ethernet MAC Receive Data 1
#define AT91C_PA13_TCLK0          (AT91C_PIO_PA13) //  Timer Counter 0 external clock input
#define AT91C_PIO_PA14            (1 << 14) // Pin Controlled by PA14
#define AT91C_PA14_ERXER          (AT91C_PIO_PA14) //  Ethernet MAC Receive Error
#define AT91C_PA14_TCLK1          (AT91C_PIO_PA14) //  Timer Counter 1 external clock input
#define AT91C_PIO_PA15            (1 << 15) // Pin Controlled by PA15
#define AT91C_PA15_EMDC           (AT91C_PIO_PA15) //  Ethernet MAC Management Data Clock
#define AT91C_PA15_TCLK2          (AT91C_PIO_PA15) //  Timer Counter 2 external clock input
#define AT91C_PIO_PA16            (1 << 16) // Pin Controlled by PA16
#define AT91C_PA16_EMDIO          (AT91C_PIO_PA16) //  Ethernet MAC Management Data Input/Output
#define AT91C_PA16_IRQ6           (AT91C_PIO_PA16) //  AIC Interrupt input 6
#define AT91C_PIO_PA17            (1 << 17) // Pin Controlled by PA17
#define AT91C_PA17_TXD0           (AT91C_PIO_PA17) //  USART 0 Transmit Data
#define AT91C_PA17_TIOA0          (AT91C_PIO_PA17) //  Timer Counter 0 Multipurpose Timer I/O Pin A
#define AT91C_PIO_PA18            (1 << 18) // Pin Controlled by PA18
#define AT91C_PA18_RXD0           (AT91C_PIO_PA18) //  USART 0 Receive Data
#define AT91C_PA18_TIOB0          (AT91C_PIO_PA18) //  Timer Counter 0 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PA19            (1 << 19) // Pin Controlled by PA19
#define AT91C_PA19_SCK0           (AT91C_PIO_PA19) //  USART 0 Serial Clock
#define AT91C_PA19_TIOA1          (AT91C_PIO_PA19) //  Timer Counter 1 Multipurpose Timer I/O Pin A
#define AT91C_PIO_PA2             (1 <<  2) // Pin Controlled by PA2
#define AT91C_PA2_SPCK            (AT91C_PIO_PA2) //  SPI Serial Clock
#define AT91C_PA2_IRQ4            (AT91C_PIO_PA2) //  AIC Interrupt Input 4
#define AT91C_PIO_PA20            (1 << 20) // Pin Controlled by PA20
#define AT91C_PA20_CTS0           (AT91C_PIO_PA20) //  USART 0 Clear To Send
#define AT91C_PA20_TIOB1          (AT91C_PIO_PA20) //  Timer Counter 1 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PA21            (1 << 21) // Pin Controlled by PA21
#define AT91C_PA21_RTS0           (AT91C_PIO_PA21) //  Usart 0 Ready To Send
#define AT91C_PA21_TIOA2          (AT91C_PIO_PA21) //  Timer Counter 2 Multipurpose Timer I/O Pin A
#define AT91C_PIO_PA22            (1 << 22) // Pin Controlled by PA22
#define AT91C_PA22_RXD2           (AT91C_PIO_PA22) //  USART 2 Receive Data
#define AT91C_PA22_TIOB2          (AT91C_PIO_PA22) //  Timer Counter 2 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PA23            (1 << 23) // Pin Controlled by PA23
#define AT91C_PA23_TXD2           (AT91C_PIO_PA23) //  USART 2 Transmit Data
#define AT91C_PA23_IRQ3           (AT91C_PIO_PA23) //  Interrupt input 3
#define AT91C_PIO_PA24            (1 << 24) // Pin Controlled by PA24
#define AT91C_PA24_SCK2           (AT91C_PIO_PA24) //  USART2 Serial Clock
#define AT91C_PA24_PCK1           (AT91C_PIO_PA24) //  PMC Programmable Clock Output 1
#define AT91C_PIO_PA25            (1 << 25) // Pin Controlled by PA25
#define AT91C_PA25_TWD            (AT91C_PIO_PA25) //  TWI Two-wire Serial Data
#define AT91C_PA25_IRQ2           (AT91C_PIO_PA25) //  Interrupt input 2
#define AT91C_PIO_PA26            (1 << 26) // Pin Controlled by PA26
#define AT91C_PA26_TWCK           (AT91C_PIO_PA26) //  TWI Two-wire Serial Clock
#define AT91C_PA26_IRQ1           (AT91C_PIO_PA26) //  Interrupt input 1
#define AT91C_PIO_PA27            (1 << 27) // Pin Controlled by PA27
#define AT91C_PA27_MCCK           (AT91C_PIO_PA27) //  Multimedia Card Clock
#define AT91C_PA27_TCLK3          (AT91C_PIO_PA27) //  Timer Counter 3 External Clock Input
#define AT91C_PIO_PA28            (1 << 28) // Pin Controlled by PA28
#define AT91C_PA28_MCCDA          (AT91C_PIO_PA28) //  Multimedia Card A Command
#define AT91C_PA28_TCLK4          (AT91C_PIO_PA28) //  Timer Counter 4 external Clock Input
#define AT91C_PIO_PA29            (1 << 29) // Pin Controlled by PA29
#define AT91C_PA29_MCDA0          (AT91C_PIO_PA29) //  Multimedia Card A Data 0
#define AT91C_PA29_TCLK5          (AT91C_PIO_PA29) //  Timer Counter 5 external clock input
#define AT91C_PIO_PA3             (1 <<  3) // Pin Controlled by PA3
#define AT91C_PA3_NPCS0           (AT91C_PIO_PA3) //  SPI Peripheral Chip Select 0
#define AT91C_PA3_IRQ5            (AT91C_PIO_PA3) //  AIC Interrupt Input 5
#define AT91C_PIO_PA30            (1 << 30) // Pin Controlled by PA30
#define AT91C_PA30_DRXD           (AT91C_PIO_PA30) //  DBGU Debug Receive Data
#define AT91C_PA30_CTS2           (AT91C_PIO_PA30) //  Usart 2 Clear To Send
#define AT91C_PIO_PA31            (1 << 31) // Pin Controlled by PA31
#define AT91C_PA31_DTXD           (AT91C_PIO_PA31) //  DBGU Debug Transmit Data
#define AT91C_PA31_RTS2           (AT91C_PIO_PA31) //  USART 2 Ready To Send
#define AT91C_PIO_PA4             (1 <<  4) // Pin Controlled by PA4
#define AT91C_PA4_NPCS1           (AT91C_PIO_PA4) //  SPI Peripheral Chip Select 1
#define AT91C_PA4_PCK1            (AT91C_PIO_PA4) //  PMC Programmable Clock Output 1
#define AT91C_PIO_PA5             (1 <<  5) // Pin Controlled by PA5
#define AT91C_PA5_NPCS2           (AT91C_PIO_PA5) //  SPI Peripheral Chip Select 2
#define AT91C_PA5_TXD3            (AT91C_PIO_PA5) //  USART 3 Transmit Data
#define AT91C_PIO_PA6             (1 <<  6) // Pin Controlled by PA6
#define AT91C_PA6_NPCS3           (AT91C_PIO_PA6) //  SPI Peripheral Chip Select 3
#define AT91C_PA6_RXD3            (AT91C_PIO_PA6) //  USART 3 Receive Data
#define AT91C_PIO_PA7             (1 <<  7) // Pin Controlled by PA7
#define AT91C_PA7_ETXCK_EREFCK    (AT91C_PIO_PA7) //  Ethernet MAC Transmit Clock/Reference Clock
#define AT91C_PA7_PCK2            (AT91C_PIO_PA7) //  PMC Programmable Clock 2
#define AT91C_PIO_PA8             (1 <<  8) // Pin Controlled by PA8
#define AT91C_PA8_ETXEN           (AT91C_PIO_PA8) //  Ethernet MAC Transmit Enable
#define AT91C_PA8_MCCDB           (AT91C_PIO_PA8) //  Multimedia Card B Command
#define AT91C_PIO_PA9             (1 <<  9) // Pin Controlled by PA9
#define AT91C_PA9_ETX0            (AT91C_PIO_PA9) //  Ethernet MAC Transmit Data 0
#define AT91C_PA9_MCDB0           (AT91C_PIO_PA9) //  Multimedia Card B Data 0
#define AT91C_PIO_PB0             (1 <<  0) // Pin Controlled by PB0
#define AT91C_PB0_TF0             (AT91C_PIO_PB0) //  SSC Transmit Frame Sync 0
#define AT91C_PB0_TIOB3           (AT91C_PIO_PB0) //  Timer Counter 3 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PB1             (1 <<  1) // Pin Controlled by PB1
#define AT91C_PB1_TK0             (AT91C_PIO_PB1) //  SSC Transmit Clock 0
#define AT91C_PB1_CTS3            (AT91C_PIO_PB1) //  USART 3 Clear To Send
#define AT91C_PIO_PB10            (1 << 10) // Pin Controlled by PB10
#define AT91C_PB10_RK1            (AT91C_PIO_PB10) //  SSC Receive Clock 1
#define AT91C_PB10_TIOA5          (AT91C_PIO_PB10) //  Timer Counter 5 Multipurpose Timer I/O Pin A
#define AT91C_PIO_PB11            (1 << 11) // Pin Controlled by PB11
#define AT91C_PB11_RF1            (AT91C_PIO_PB11) //  SSC Receive Frame Sync 1
#define AT91C_PB11_TIOB5          (AT91C_PIO_PB11) //  Timer Counter 5 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PB12            (1 << 12) // Pin Controlled by PB12
#define AT91C_PB12_TF2            (AT91C_PIO_PB12) //  SSC Transmit Frame Sync 2
#define AT91C_PB12_ETX2           (AT91C_PIO_PB12) //  Ethernet MAC Transmit Data 2
#define AT91C_PIO_PB13            (1 << 13) // Pin Controlled by PB13
#define AT91C_PB13_TK2            (AT91C_PIO_PB13) //  SSC Transmit Clock 2
#define AT91C_PB13_ETX3           (AT91C_PIO_PB13) //  Ethernet MAC Transmit Data 3
#define AT91C_PIO_PB14            (1 << 14) // Pin Controlled by PB14
#define AT91C_PB14_TD2            (AT91C_PIO_PB14) //  SSC Transmit Data 2
#define AT91C_PB14_ETXER          (AT91C_PIO_PB14) //  Ethernet MAC Transmikt Coding Error
#define AT91C_PIO_PB15            (1 << 15) // Pin Controlled by PB15
#define AT91C_PB15_RD2            (AT91C_PIO_PB15) //  SSC Receive Data 2
#define AT91C_PB15_ERX2           (AT91C_PIO_PB15) //  Ethernet MAC Receive Data 2
#define AT91C_PIO_PB16            (1 << 16) // Pin Controlled by PB16
#define AT91C_PB16_RK2            (AT91C_PIO_PB16) //  SSC Receive Clock 2
#define AT91C_PB16_ERX3           (AT91C_PIO_PB16) //  Ethernet MAC Receive Data 3
#define AT91C_PIO_PB17            (1 << 17) // Pin Controlled by PB17
#define AT91C_PB17_RF2            (AT91C_PIO_PB17) //  SSC Receive Frame Sync 2
#define AT91C_PB17_ERXDV          (AT91C_PIO_PB17) //  Ethernet MAC Receive Data Valid
#define AT91C_PIO_PB18            (1 << 18) // Pin Controlled by PB18
#define AT91C_PB18_RI1            (AT91C_PIO_PB18) //  USART 1 Ring Indicator
#define AT91C_PB18_ECOL           (AT91C_PIO_PB18) //  Ethernet MAC Collision Detected
#define AT91C_PIO_PB19            (1 << 19) // Pin Controlled by PB19
#define AT91C_PB19_DTR1           (AT91C_PIO_PB19) //  USART 1 Data Terminal ready
#define AT91C_PB19_ERXCK          (AT91C_PIO_PB19) //  Ethernet MAC Receive Clock
#define AT91C_PIO_PB2             (1 <<  2) // Pin Controlled by PB2
#define AT91C_PB2_TD0             (AT91C_PIO_PB2) //  SSC Transmit data
#define AT91C_PB2_SCK3            (AT91C_PIO_PB2) //  USART 3 Serial Clock
#define AT91C_PIO_PB20            (1 << 20) // Pin Controlled by PB20
#define AT91C_PB20_TXD1           (AT91C_PIO_PB20) //  USART 1 Transmit Data
#define AT91C_PIO_PB21            (1 << 21) // Pin Controlled by PB21
#define AT91C_PB21_RXD1           (AT91C_PIO_PB21) //  USART 1 Receive Data
#define AT91C_PIO_PB22            (1 << 22) // Pin Controlled by PB22
#define AT91C_PB22_SCK1           (AT91C_PIO_PB22) //  USART1 Serial Clock
#define AT91C_PIO_PB23            (1 << 23) // Pin Controlled by PB23
#define AT91C_PB23_DCD1           (AT91C_PIO_PB23) //  USART 1 Data Carrier Detect
#define AT91C_PIO_PB24            (1 << 24) // Pin Controlled by PB24
#define AT91C_PB24_CTS1           (AT91C_PIO_PB24) //  USART 1 Clear To Send
#define AT91C_PIO_PB25            (1 << 25) // Pin Controlled by PB25
#define AT91C_PB25_DSR1           (AT91C_PIO_PB25) //  USART 1 Data Set ready
#define AT91C_PB25_EF100          (AT91C_PIO_PB25) //  Ethernet MAC Force 100 Mbits/sec
#define AT91C_PIO_PB26            (1 << 26) // Pin Controlled by PB26
#define AT91C_PB26_RTS1           (AT91C_PIO_PB26) //  Usart 0 Ready To Send
#define AT91C_PIO_PB27            (1 << 27) // Pin Controlled by PB27
#define AT91C_PB27_PCK0           (AT91C_PIO_PB27) //  PMC Programmable Clock Output 0
#define AT91C_PIO_PB28            (1 << 28) // Pin Controlled by PB28
#define AT91C_PB28_FIQ            (AT91C_PIO_PB28) //  AIC Fast Interrupt Input
#define AT91C_PIO_PB29            (1 << 29) // Pin Controlled by PB29
#define AT91C_PB29_IRQ0           (AT91C_PIO_PB29) //  Interrupt input 0
#define AT91C_PIO_PB3             (1 <<  3) // Pin Controlled by PB3
#define AT91C_PB3_RD0             (AT91C_PIO_PB3) //  SSC Receive Data
#define AT91C_PB3_MCDA1           (AT91C_PIO_PB3) //  Multimedia Card A Data 1
#define AT91C_PIO_PB4             (1 <<  4) // Pin Controlled by PB4
#define AT91C_PB4_RK0             (AT91C_PIO_PB4) //  SSC Receive Clock
#define AT91C_PB4_MCDA2           (AT91C_PIO_PB4) //  Multimedia Card A Data 2
#define AT91C_PIO_PB5             (1 <<  5) // Pin Controlled by PB5
#define AT91C_PB5_RF0             (AT91C_PIO_PB5) //  SSC Receive Frame Sync 0
#define AT91C_PB5_MCDA3           (AT91C_PIO_PB5) //  Multimedia Card A Data 3
#define AT91C_PIO_PB6             (1 <<  6) // Pin Controlled by PB6
#define AT91C_PB6_TF1             (AT91C_PIO_PB6) //  SSC Transmit Frame Sync 1
#define AT91C_PB6_TIOA3           (AT91C_PIO_PB6) //  Timer Counter 4 Multipurpose Timer I/O Pin A
#define AT91C_PIO_PB7             (1 <<  7) // Pin Controlled by PB7
#define AT91C_PB7_TK1             (AT91C_PIO_PB7) //  SSC Transmit Clock 1
#define AT91C_PB7_TIOB3           (AT91C_PIO_PB7) //  Timer Counter 3 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PB8             (1 <<  8) // Pin Controlled by PB8
#define AT91C_PB8_TD1             (AT91C_PIO_PB8) //  SSC Transmit Data 1
#define AT91C_PB8_TIOA4           (AT91C_PIO_PB8) //  Timer Counter 4 Multipurpose Timer I/O Pin A
#define AT91C_PIO_PB9             (1 <<  9) // Pin Controlled by PB9
#define AT91C_PB9_RD1             (AT91C_PIO_PB9) //  SSC Receive Data 1
#define AT91C_PB9_TIOB4           (AT91C_PIO_PB9) //  Timer Counter 4 Multipurpose Timer I/O Pin B
#define AT91C_PIO_PC0             (1 <<  0) // Pin Controlled by PC0
#define AT91C_PC0_BFCK            (AT91C_PIO_PC0) //  Burst Flash Clock
#define AT91C_PIO_PC1             (1 <<  1) // Pin Controlled by PC1
#define AT91C_PC1_BFRDY_SMOE      (AT91C_PIO_PC1) //  Burst Flash Ready
#define AT91C_PIO_PC10            (1 << 10) // Pin Controlled by PC10
#define AT91C_PC10_NCS4_CFCS      (AT91C_PIO_PC10) //  Compact Flash Chip Select
#define AT91C_PIO_PC11            (1 << 11) // Pin Controlled by PC11
#define AT91C_PC11_NCS5_CFCE1     (AT91C_PIO_PC11) //  Chip Select 5 / Compact Flash Chip Enable 1
#define AT91C_PIO_PC12            (1 << 12) // Pin Controlled by PC12
#define AT91C_PC12_NCS6_CFCE2     (AT91C_PIO_PC12) //  Chip Select 6 / Compact Flash Chip Enable 2
#define AT91C_PIO_PC13            (1 << 13) // Pin Controlled by PC13
#define AT91C_PC13_NCS7           (AT91C_PIO_PC13) //  Chip Select 7
#define AT91C_PIO_PC14            (1 << 14) // Pin Controlled by PC14
#define AT91C_PIO_PC15            (1 << 15) // Pin Controlled by PC15
#define AT91C_PIO_PC16            (1 << 16) // Pin Controlled by PC16
#define AT91C_PC16_D16            (AT91C_PIO_PC16) //  Data Bus [16]
#define AT91C_PIO_PC17            (1 << 17) // Pin Controlled by PC17
#define AT91C_PC17_D17            (AT91C_PIO_PC17) //  Data Bus [17]
#define AT91C_PIO_PC18            (1 << 18) // Pin Controlled by PC18
#define AT91C_PC18_D18            (AT91C_PIO_PC18) //  Data Bus [18]
#define AT91C_PIO_PC19            (1 << 19) // Pin Controlled by PC19
#define AT91C_PC19_D19            (AT91C_PIO_PC19) //  Data Bus [19]
#define AT91C_PIO_PC2             (1 <<  2) // Pin Controlled by PC2
#define AT91C_PC2_BFAVD           (AT91C_PIO_PC2) //  Burst Flash Address Valid
#define AT91C_PIO_PC20            (1 << 20) // Pin Controlled by PC20
#define AT91C_PC20_D20            (AT91C_PIO_PC20) //  Data Bus [20]
#define AT91C_PIO_PC21            (1 << 21) // Pin Controlled by PC21
#define AT91C_PC21_D21            (AT91C_PIO_PC21) //  Data Bus [21]
#define AT91C_PIO_PC22            (1 << 22) // Pin Controlled by PC22
#define AT91C_PC22_D22            (AT91C_PIO_PC22) //  Data Bus [22]
#define AT91C_PIO_PC23            (1 << 23) // Pin Controlled by PC23
#define AT91C_PC23_D23            (AT91C_PIO_PC23) //  Data Bus [23]
#define AT91C_PIO_PC24            (1 << 24) // Pin Controlled by PC24
#define AT91C_PC24_D24            (AT91C_PIO_PC24) //  Data Bus [24]
#define AT91C_PIO_PC25            (1 << 25) // Pin Controlled by PC25
#define AT91C_PC25_D25            (AT91C_PIO_PC25) //  Data Bus [25]
#define AT91C_PIO_PC26            (1 << 26) // Pin Controlled by PC26
#define AT91C_PC26_D26            (AT91C_PIO_PC26) //  Data Bus [26]
#define AT91C_PIO_PC27            (1 << 27) // Pin Controlled by PC27
#define AT91C_PC27_D27            (AT91C_PIO_PC27) //  Data Bus [27]
#define AT91C_PIO_PC28            (1 << 28) // Pin Controlled by PC28
#define AT91C_PC28_D28            (AT91C_PIO_PC28) //  Data Bus [28]
#define AT91C_PIO_PC29            (1 << 29) // Pin Controlled by PC29
#define AT91C_PC29_D29            (AT91C_PIO_PC29) //  Data Bus [29]
#define AT91C_PIO_PC3             (1 <<  3) // Pin Controlled by PC3
#define AT91C_PC3_BFBAA_SMWE      (AT91C_PIO_PC3) //  Burst Flash Address Advance / SmartMedia Write Enable
#define AT91C_PIO_PC30            (1 << 30) // Pin Controlled by PC30
#define AT91C_PC30_D30            (AT91C_PIO_PC30) //  Data Bus [30]
#define AT91C_PIO_PC31            (1 << 31) // Pin Controlled by PC31
#define AT91C_PC31_D31            (AT91C_PIO_PC31) //  Data Bus [31]
#define AT91C_PIO_PC4             (1 <<  4) // Pin Controlled by PC4
#define AT91C_PC4_BFOE            (AT91C_PIO_PC4) //  Burst Flash Output Enable
#define AT91C_PIO_PC5             (1 <<  5) // Pin Controlled by PC5
#define AT91C_PC5_BFWE            (AT91C_PIO_PC5) //  Burst Flash Write Enable
#define AT91C_PIO_PC6             (1 <<  6) // Pin Controlled by PC6
#define AT91C_PC6_NWAIT           (AT91C_PIO_PC6) //  NWAIT
#define AT91C_PIO_PC7             (1 <<  7) // Pin Controlled by PC7
#define AT91C_PC7_A23             (AT91C_PIO_PC7) //  Address Bus[23]
#define AT91C_PIO_PC8             (1 <<  8) // Pin Controlled by PC8
#define AT91C_PC8_A24             (AT91C_PIO_PC8) //  Address Bus[24]
#define AT91C_PIO_PC9             (1 <<  9) // Pin Controlled by PC9
#define AT91C_PC9_A25_CFRNW       (AT91C_PIO_PC9) //  Address Bus[25] /  Compact Flash Read Not Write
#define AT91C_PIO_PD0             (1 <<  0) // Pin Controlled by PD0
#define AT91C_PD0_ETX0            (AT91C_PIO_PD0) //  Ethernet MAC Transmit Data 0
#define AT91C_PIO_PD1             (1 <<  1) // Pin Controlled by PD1
#define AT91C_PD1_ETX1            (AT91C_PIO_PD1) //  Ethernet MAC Transmit Data 1
#define AT91C_PIO_PD10            (1 << 10) // Pin Controlled by PD10
#define AT91C_PD10_PCK3           (AT91C_PIO_PD10) //  PMC Programmable Clock Output 3
#define AT91C_PD10_TPS1           (AT91C_PIO_PD10) //  ETM ARM9 pipeline status 1
#define AT91C_PIO_PD11            (1 << 11) // Pin Controlled by PD11
#define AT91C_PD11_               (AT91C_PIO_PD11) //  
#define AT91C_PD11_TPS2           (AT91C_PIO_PD11) //  ETM ARM9 pipeline status 2
#define AT91C_PIO_PD12            (1 << 12) // Pin Controlled by PD12
#define AT91C_PD12_               (AT91C_PIO_PD12) //  
#define AT91C_PD12_TPK0           (AT91C_PIO_PD12) //  ETM Trace Packet 0
#define AT91C_PIO_PD13            (1 << 13) // Pin Controlled by PD13
#define AT91C_PD13_               (AT91C_PIO_PD13) //  
#define AT91C_PD13_TPK1           (AT91C_PIO_PD13) //  ETM Trace Packet 1
#define AT91C_PIO_PD14            (1 << 14) // Pin Controlled by PD14
#define AT91C_PD14_               (AT91C_PIO_PD14) //  
#define AT91C_PD14_TPK2           (AT91C_PIO_PD14) //  ETM Trace Packet 2
#define AT91C_PIO_PD15            (1 << 15) // Pin Controlled by PD15
#define AT91C_PD15_TD0            (AT91C_PIO_PD15) //  SSC Transmit data
#define AT91C_PD15_TPK3           (AT91C_PIO_PD15) //  ETM Trace Packet 3
#define AT91C_PIO_PD16            (1 << 16) // Pin Controlled by PD16
#define AT91C_PD16_TD1            (AT91C_PIO_PD16) //  SSC Transmit Data 1
#define AT91C_PD16_TPK4           (AT91C_PIO_PD16) //  ETM Trace Packet 4
#define AT91C_PIO_PD17            (1 << 17) // Pin Controlled by PD17
#define AT91C_PD17_TD2            (AT91C_PIO_PD17) //  SSC Transmit Data 2
#define AT91C_PD17_TPK5           (AT91C_PIO_PD17) //  ETM Trace Packet 5
#define AT91C_PIO_PD18            (1 << 18) // Pin Controlled by PD18
#define AT91C_PD18_NPCS1          (AT91C_PIO_PD18) //  SPI Peripheral Chip Select 1
#define AT91C_PD18_TPK6           (AT91C_PIO_PD18) //  ETM Trace Packet 6
#define AT91C_PIO_PD19            (1 << 19) // Pin Controlled by PD19
#define AT91C_PD19_NPCS2          (AT91C_PIO_PD19) //  SPI Peripheral Chip Select 2
#define AT91C_PD19_TPK7           (AT91C_PIO_PD19) //  ETM Trace Packet 7
#define AT91C_PIO_PD2             (1 <<  2) // Pin Controlled by PD2
#define AT91C_PD2_ETX2            (AT91C_PIO_PD2) //  Ethernet MAC Transmit Data 2
#define AT91C_PIO_PD20            (1 << 20) // Pin Controlled by PD20
#define AT91C_PD20_NPCS3          (AT91C_PIO_PD20) //  SPI Peripheral Chip Select 3
#define AT91C_PD20_TPK8           (AT91C_PIO_PD20) //  ETM Trace Packet 8
#define AT91C_PIO_PD21            (1 << 21) // Pin Controlled by PD21
#define AT91C_PD21_RTS0           (AT91C_PIO_PD21) //  Usart 0 Ready To Send
#define AT91C_PD21_TPK9           (AT91C_PIO_PD21) //  ETM Trace Packet 9
#define AT91C_PIO_PD22            (1 << 22) // Pin Controlled by PD22
#define AT91C_PD22_RTS1           (AT91C_PIO_PD22) //  Usart 0 Ready To Send
#define AT91C_PD22_TPK10          (AT91C_PIO_PD22) //  ETM Trace Packet 10
#define AT91C_PIO_PD23            (1 << 23) // Pin Controlled by PD23
#define AT91C_PD23_RTS2           (AT91C_PIO_PD23) //  USART 2 Ready To Send
#define AT91C_PD23_TPK11          (AT91C_PIO_PD23) //  ETM Trace Packet 11
#define AT91C_PIO_PD24            (1 << 24) // Pin Controlled by PD24
#define AT91C_PD24_RTS3           (AT91C_PIO_PD24) //  USART 3 Ready To Send
#define AT91C_PD24_TPK12          (AT91C_PIO_PD24) //  ETM Trace Packet 12
#define AT91C_PIO_PD25            (1 << 25) // Pin Controlled by PD25
#define AT91C_PD25_DTR1           (AT91C_PIO_PD25) //  USART 1 Data Terminal ready
#define AT91C_PD25_TPK13          (AT91C_PIO_PD25) //  ETM Trace Packet 13
#define AT91C_PIO_PD26            (1 << 26) // Pin Controlled by PD26
#define AT91C_PD26_TPK14          (AT91C_PIO_PD26) //  ETM Trace Packet 14
#define AT91C_PIO_PD27            (1 << 27) // Pin Controlled by PD27
#define AT91C_PD27_TPK15          (AT91C_PIO_PD27) //  ETM Trace Packet 15
#define AT91C_PIO_PD3             (1 <<  3) // Pin Controlled by PD3
#define AT91C_PD3_ETX3            (AT91C_PIO_PD3) //  Ethernet MAC Transmit Data 3
#define AT91C_PIO_PD4             (1 <<  4) // Pin Controlled by PD4
#define AT91C_PD4_ETXEN           (AT91C_PIO_PD4) //  Ethernet MAC Transmit Enable
#define AT91C_PIO_PD5             (1 <<  5) // Pin Controlled by PD5
#define AT91C_PD5_ETXER           (AT91C_PIO_PD5) //  Ethernet MAC Transmikt Coding Error
#define AT91C_PIO_PD6             (1 <<  6) // Pin Controlled by PD6
#define AT91C_PD6_DTXD            (AT91C_PIO_PD6) //  DBGU Debug Transmit Data
#define AT91C_PIO_PD7             (1 <<  7) // Pin Controlled by PD7
#define AT91C_PD7_PCK0            (AT91C_PIO_PD7) //  PMC Programmable Clock Output 0
#define AT91C_PD7_TSYNC           (AT91C_PIO_PD7) //  ETM Synchronization signal
#define AT91C_PIO_PD8             (1 <<  8) // Pin Controlled by PD8
#define AT91C_PD8_PCK1            (AT91C_PIO_PD8) //  PMC Programmable Clock Output 1
#define AT91C_PD8_TCLK            (AT91C_PIO_PD8) //  ETM Trace Clock signal
#define AT91C_PIO_PD9             (1 <<  9) // Pin Controlled by PD9
#define AT91C_PD9_PCK2            (AT91C_PIO_PD9) //  PMC Programmable Clock 2
#define AT91C_PD9_TPS0            (AT91C_PIO_PD9) //  ETM ARM9 pipeline status 0

// *****************************************************************************
//               PERIPHERAL ID DEFINITIONS FOR AT91RM9200
// *****************************************************************************
#define AT91C_ID_FIQ              ( 0) // Advanced Interrupt Controller (FIQ)
#define AT91C_ID_SYS              ( 1) // System Peripheral
#define AT91C_ID_PIOA             ( 2) // Parallel IO Controller A 
#define AT91C_ID_PIOB             ( 3) // Parallel IO Controller B
#define AT91C_ID_PIOC             ( 4) // Parallel IO Controller C
#define AT91C_ID_PIOD             ( 5) // Parallel IO Controller D
#define AT91C_ID_US0              ( 6) // USART 0
#define AT91C_ID_US1              ( 7) // USART 1
#define AT91C_ID_US2              ( 8) // USART 2
#define AT91C_ID_US3              ( 9) // USART 3
#define AT91C_ID_MCI              (10) // Multimedia Card Interface
#define AT91C_ID_UDP              (11) // USB Device Port
#define AT91C_ID_TWI              (12) // Two-Wire Interface
#define AT91C_ID_SPI              (13) // Serial Peripheral Interface
#define AT91C_ID_SSC0             (14) // Serial Synchronous Controller 0
#define AT91C_ID_SSC1             (15) // Serial Synchronous Controller 1
#define AT91C_ID_SSC2             (16) // Serial Synchronous Controller 2
#define AT91C_ID_TC0              (17) // Timer Counter 0
#define AT91C_ID_TC1              (18) // Timer Counter 1
#define AT91C_ID_TC2              (19) // Timer Counter 2
#define AT91C_ID_TC3              (20) // Timer Counter 3
#define AT91C_ID_TC4              (21) // Timer Counter 4
#define AT91C_ID_TC5              (22) // Timer Counter 5
#define AT91C_ID_UHP              (23) // USB Host port
#define AT91C_ID_EMAC             (24) // Ethernet MAC
#define AT91C_ID_IRQ0             (25) // Advanced Interrupt Controller (IRQ0)
#define AT91C_ID_IRQ1             (26) // Advanced Interrupt Controller (IRQ1)
#define AT91C_ID_IRQ2             (27) // Advanced Interrupt Controller (IRQ2)
#define AT91C_ID_IRQ3             (28) // Advanced Interrupt Controller (IRQ3)
#define AT91C_ID_IRQ4             (29) // Advanced Interrupt Controller (IRQ4)
#define AT91C_ID_IRQ5             (30) // Advanced Interrupt Controller (IRQ5)
#define AT91C_ID_IRQ6             (31) // Advanced Interrupt Controller (IRQ6)

// *****************************************************************************
//               BASE ADDRESS DEFINITIONS FOR AT91RM9200
// *****************************************************************************
#define AT91C_BASE_SYS            (0xFFFFF000) // (SYS) Base Address
#define AT91C_BASE_MC             (0xFFFFFF00) // (MC) Base Address
#define AT91C_BASE_RTC            (0xFFFFFE00) // (RTC) Base Address
#define AT91C_BASE_ST             (0xFFFFFD00) // (ST) Base Address
#define AT91C_BASE_PMC            (0xFFFFFC00) // (PMC) Base Address
#define AT91C_BASE_CKGR           (0xFFFFFC20) // (CKGR) Base Address
#define AT91C_BASE_PIOD           (0xFFFFFA00) // (PIOD) Base Address
#define AT91C_BASE_PIOC           (0xFFFFF800) // (PIOC) Base Address
#define AT91C_BASE_PIOB           (0xFFFFF600) // (PIOB) Base Address
#define AT91C_BASE_PIOA           (0xFFFFF400) // (PIOA) Base Address
#define AT91C_BASE_DBGU           (0xFFFFF200) // (DBGU) Base Address
#define AT91C_BASE_PDC_DBGU       (0xFFFFF300) // (PDC_DBGU) Base Address
#define AT91C_BASE_AIC            (0xFFFFF000) // (AIC) Base Address
#define AT91C_BASE_PDC_SPI        (0xFFFE0100) // (PDC_SPI) Base Address
#define AT91C_BASE_SPI            (0xFFFE0000) // (SPI) Base Address
#define AT91C_BASE_PDC_SSC2       (0xFFFD8100) // (PDC_SSC2) Base Address
#define AT91C_BASE_SSC2           (0xFFFD8000) // (SSC2) Base Address
#define AT91C_BASE_PDC_SSC1       (0xFFFD4100) // (PDC_SSC1) Base Address
#define AT91C_BASE_SSC1           (0xFFFD4000) // (SSC1) Base Address
#define AT91C_BASE_PDC_SSC0       (0xFFFD0100) // (PDC_SSC0) Base Address
#define AT91C_BASE_SSC0           (0xFFFD0000) // (SSC0) Base Address
#define AT91C_BASE_PDC_US3        (0xFFFCC100) // (PDC_US3) Base Address
#define AT91C_BASE_US3            (0xFFFCC000) // (US3) Base Address
#define AT91C_BASE_PDC_US2        (0xFFFC8100) // (PDC_US2) Base Address
#define AT91C_BASE_US2            (0xFFFC8000) // (US2) Base Address
#define AT91C_BASE_PDC_US1        (0xFFFC4100) // (PDC_US1) Base Address
#define AT91C_BASE_US1            (0xFFFC4000) // (US1) Base Address
#define AT91C_BASE_PDC_US0        (0xFFFC0100) // (PDC_US0) Base Address
#define AT91C_BASE_US0            (0xFFFC0000) // (US0) Base Address
#define AT91C_BASE_TWI            (0xFFFB8000) // (TWI) Base Address
#define AT91C_BASE_PDC_MCI        (0xFFFB4100) // (PDC_MCI) Base Address
#define AT91C_BASE_MCI            (0xFFFB4000) // (MCI) Base Address
#define AT91C_BASE_UDP            (0xFFFB0000) // (UDP) Base Address
#define AT91C_BASE_TC5            (0xFFFA4080) // (TC5) Base Address
#define AT91C_BASE_TC4            (0xFFFA4040) // (TC4) Base Address
#define AT91C_BASE_TC3            (0xFFFA4000) // (TC3) Base Address
#define AT91C_BASE_TCB1           (0xFFFA4080) // (TCB1) Base Address
#define AT91C_BASE_TC2            (0xFFFA0080) // (TC2) Base Address
#define AT91C_BASE_TC1            (0xFFFA0040) // (TC1) Base Address
#define AT91C_BASE_TC0            (0xFFFA0000) // (TC0) Base Address
#define AT91C_BASE_TCB0           (0xFFFA0000) // (TCB0) Base Address
#define AT91C_BASE_UHP            (0x00300000) // (UHP) Base Address
#define AT91C_BASE_EMAC           (0xFFFBC000) // (EMAC) Base Address
#define AT91C_BASE_EBI            (0xFFFFFF60) // (EBI) Base Address
#define AT91C_BASE_SMC2           (0xFFFFFF70) // (SMC2) Base Address
#define AT91C_BASE_SDRC           (0xFFFFFF90) // (SDRC) Base Address
#define AT91C_BASE_BFC            (0xFFFFFFC0) // (BFC) Base Address

// *****************************************************************************
//               MEMORY MAPPING DEFINITIONS FOR AT91RM9200
// *****************************************************************************
#define AT91C_ISRAM	              (0x00200000) // Internal SRAM base address
#define AT91C_ISRAM_SIZE	         (0x00004000) // Internal SRAM size in byte (16 Kbyte)
#define AT91C_IROM 	              (0x00100000) // Internal ROM base address
#define AT91C_IROM_SIZE	          (0x00020000) // Internal ROM size in byte (128 Kbyte)


