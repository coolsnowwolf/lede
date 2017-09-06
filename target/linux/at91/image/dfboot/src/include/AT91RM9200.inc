;- ----------------------------------------------------------------------------
;-          ATMEL Microcontroller Software Support  -  ROUSSET  -
;- ----------------------------------------------------------------------------
;-  The software is delivered "AS IS" without warranty or condition of any
;-  kind, either express, implied or statutory. This includes without
;-  limitation any warranty or condition with respect to merchantability or
;-  fitness for any particular purpose, or against the infringements of
;-  intellectual property rights of others.
;- ----------------------------------------------------------------------------
;- File Name           : AT91RM9200.h
;- Object              : AT91RM9200 definitions
;- Generated           : AT91 SW Application Group  11/19/2003 (17:20:51)
;- 
;- CVS Reference       : /AT91RM9200.pl/1.16/Fri Feb 07 10:29:51 2003//
;- CVS Reference       : /SYS_AT91RM9200.pl/1.2/Fri Jan 17 12:44:37 2003//
;- CVS Reference       : /MC_1760A.pl/1.1/Fri Aug 23 14:38:22 2002//
;- CVS Reference       : /AIC_1796B.pl/1.1.1.1/Fri Jun 28 09:36:47 2002//
;- CVS Reference       : /PMC_2636A.pl/1.1.1.1/Fri Jun 28 09:36:48 2002//
;- CVS Reference       : /ST_1763B.pl/1.1/Fri Aug 23 14:41:42 2002//
;- CVS Reference       : /RTC_1245D.pl/1.2/Fri Jan 31 12:19:06 2003//
;- CVS Reference       : /PIO_1725D.pl/1.1.1.1/Fri Jun 28 09:36:47 2002//
;- CVS Reference       : /DBGU_1754A.pl/1.4/Fri Jan 31 12:18:24 2003//
;- CVS Reference       : /UDP_1765B.pl/1.3/Fri Aug 02 14:45:38 2002//
;- CVS Reference       : /MCI_1764A.pl/1.2/Thu Nov 14 17:48:24 2002//
;- CVS Reference       : /US_1739C.pl/1.2/Fri Jul 12 07:49:25 2002//
;- CVS Reference       : /SPI_AT91RMxxxx.pl/1.3/Tue Nov 26 10:20:29 2002//
;- CVS Reference       : /SSC_1762A.pl/1.2/Fri Nov 08 13:26:39 2002//
;- CVS Reference       : /TC_1753B.pl/1.2/Fri Jan 31 12:19:55 2003//
;- CVS Reference       : /TWI_1761B.pl/1.4/Fri Feb 07 10:30:07 2003//
;- CVS Reference       : /PDC_1734B.pl/1.2/Thu Nov 21 16:38:23 2002//
;- CVS Reference       : /UHP_xxxxA.pl/1.1/Mon Jul 22 12:21:58 2002//
;- CVS Reference       : /EMAC_1794A.pl/1.4/Fri Jan 17 12:11:54 2003//
;- CVS Reference       : /EBI_1759B.pl/1.10/Fri Jan 17 12:44:29 2003//
;- CVS Reference       : /SMC_1783A.pl/1.3/Thu Oct 31 14:38:17 2002//
;- CVS Reference       : /SDRC_1758B.pl/1.2/Thu Oct 03 13:04:41 2002//
;- CVS Reference       : /BFC_1757B.pl/1.3/Thu Oct 31 14:38:00 2002//
;- ----------------------------------------------------------------------------

;- Hardware register definition

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR System Peripherals
;- *****************************************************************************

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Memory Controller Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_MC
MC_RCR          #  4 ;- MC Remap Control Register
MC_ASR          #  4 ;- MC Abort Status Register
MC_AASR         #  4 ;- MC Abort Address Status Register
                #  4 ;- Reserved
MC_PUIA         # 64 ;- MC Protection Unit Area
MC_PUP          #  4 ;- MC Protection Unit Peripherals
MC_PUER         #  4 ;- MC Protection Unit Enable Register
;- -------- MC_RCR : (MC Offset: 0x0) MC Remap Control Register -------- 
AT91C_MC_RCB              EQU (0x1:SHL:0) ;- (MC) Remap Command Bit
;- -------- MC_ASR : (MC Offset: 0x4) MC Abort Status Register -------- 
AT91C_MC_UNDADD           EQU (0x1:SHL:0) ;- (MC) Undefined Addess Abort Status
AT91C_MC_MISADD           EQU (0x1:SHL:1) ;- (MC) Misaligned Addess Abort Status
AT91C_MC_MPU              EQU (0x1:SHL:2) ;- (MC) Memory protection Unit Abort Status
AT91C_MC_ABTSZ            EQU (0x3:SHL:8) ;- (MC) Abort Size Status
AT91C_MC_ABTSZ_BYTE       EQU (0x0:SHL:8) ;- (MC) Byte
AT91C_MC_ABTSZ_HWORD      EQU (0x1:SHL:8) ;- (MC) Half-word
AT91C_MC_ABTSZ_WORD       EQU (0x2:SHL:8) ;- (MC) Word
AT91C_MC_ABTTYP           EQU (0x3:SHL:10) ;- (MC) Abort Type Status
AT91C_MC_ABTTYP_DATAR     EQU (0x0:SHL:10) ;- (MC) Data Read
AT91C_MC_ABTTYP_DATAW     EQU (0x1:SHL:10) ;- (MC) Data Write
AT91C_MC_ABTTYP_FETCH     EQU (0x2:SHL:10) ;- (MC) Code Fetch
AT91C_MC_MST0             EQU (0x1:SHL:16) ;- (MC) Master 0 Abort Source
AT91C_MC_MST1             EQU (0x1:SHL:17) ;- (MC) Master 1 Abort Source
AT91C_MC_SVMST0           EQU (0x1:SHL:24) ;- (MC) Saved Master 0 Abort Source
AT91C_MC_SVMST1           EQU (0x1:SHL:25) ;- (MC) Saved Master 1 Abort Source
;- -------- MC_PUIA : (MC Offset: 0x10) MC Protection Unit Area -------- 
AT91C_MC_PROT             EQU (0x3:SHL:0) ;- (MC) Protection
AT91C_MC_PROT_PNAUNA      EQU (0x0) ;- (MC) Privilege: No Access, User: No Access
AT91C_MC_PROT_PRWUNA      EQU (0x1) ;- (MC) Privilege: Read/Write, User: No Access
AT91C_MC_PROT_PRWURO      EQU (0x2) ;- (MC) Privilege: Read/Write, User: Read Only
AT91C_MC_PROT_PRWURW      EQU (0x3) ;- (MC) Privilege: Read/Write, User: Read/Write
AT91C_MC_SIZE             EQU (0xF:SHL:4) ;- (MC) Internal Area Size
AT91C_MC_SIZE_1KB         EQU (0x0:SHL:4) ;- (MC) Area size 1KByte
AT91C_MC_SIZE_2KB         EQU (0x1:SHL:4) ;- (MC) Area size 2KByte
AT91C_MC_SIZE_4KB         EQU (0x2:SHL:4) ;- (MC) Area size 4KByte
AT91C_MC_SIZE_8KB         EQU (0x3:SHL:4) ;- (MC) Area size 8KByte
AT91C_MC_SIZE_16KB        EQU (0x4:SHL:4) ;- (MC) Area size 16KByte
AT91C_MC_SIZE_32KB        EQU (0x5:SHL:4) ;- (MC) Area size 32KByte
AT91C_MC_SIZE_64KB        EQU (0x6:SHL:4) ;- (MC) Area size 64KByte
AT91C_MC_SIZE_128KB       EQU (0x7:SHL:4) ;- (MC) Area size 128KByte
AT91C_MC_SIZE_256KB       EQU (0x8:SHL:4) ;- (MC) Area size 256KByte
AT91C_MC_SIZE_512KB       EQU (0x9:SHL:4) ;- (MC) Area size 512KByte
AT91C_MC_SIZE_1MB         EQU (0xA:SHL:4) ;- (MC) Area size 1MByte
AT91C_MC_SIZE_2MB         EQU (0xB:SHL:4) ;- (MC) Area size 2MByte
AT91C_MC_SIZE_4MB         EQU (0xC:SHL:4) ;- (MC) Area size 4MByte
AT91C_MC_SIZE_8MB         EQU (0xD:SHL:4) ;- (MC) Area size 8MByte
AT91C_MC_SIZE_16MB        EQU (0xE:SHL:4) ;- (MC) Area size 16MByte
AT91C_MC_SIZE_64MB        EQU (0xF:SHL:4) ;- (MC) Area size 64MByte
AT91C_MC_BA               EQU (0x3FFFF:SHL:10) ;- (MC) Internal Area Base Address
;- -------- MC_PUP : (MC Offset: 0x50) MC Protection Unit Peripheral -------- 
;- -------- MC_PUER : (MC Offset: 0x54) MC Protection Unit Area -------- 
AT91C_MC_PUEB             EQU (0x1:SHL:0) ;- (MC) Protection Unit enable Bit

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Real-time Clock Alarm and Parallel Load Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_RTC
RTC_CR          #  4 ;- Control Register
RTC_MR          #  4 ;- Mode Register
RTC_TIMR        #  4 ;- Time Register
RTC_CALR        #  4 ;- Calendar Register
RTC_TIMALR      #  4 ;- Time Alarm Register
RTC_CALALR      #  4 ;- Calendar Alarm Register
RTC_SR          #  4 ;- Status Register
RTC_SCCR        #  4 ;- Status Clear Command Register
RTC_IER         #  4 ;- Interrupt Enable Register
RTC_IDR         #  4 ;- Interrupt Disable Register
RTC_IMR         #  4 ;- Interrupt Mask Register
RTC_VER         #  4 ;- Valid Entry Register
;- -------- RTC_CR : (RTC Offset: 0x0) RTC Control Register -------- 
AT91C_RTC_UPDTIM          EQU (0x1:SHL:0) ;- (RTC) Update Request Time Register
AT91C_RTC_UPDCAL          EQU (0x1:SHL:1) ;- (RTC) Update Request Calendar Register
AT91C_RTC_TIMEVSEL        EQU (0x3:SHL:8) ;- (RTC) Time Event Selection
AT91C_RTC_TIMEVSEL_MINUTE EQU (0x0:SHL:8) ;- (RTC) Minute change.
AT91C_RTC_TIMEVSEL_HOUR   EQU (0x1:SHL:8) ;- (RTC) Hour change.
AT91C_RTC_TIMEVSEL_DAY24  EQU (0x2:SHL:8) ;- (RTC) Every day at midnight.
AT91C_RTC_TIMEVSEL_DAY12  EQU (0x3:SHL:8) ;- (RTC) Every day at noon.
AT91C_RTC_CALEVSEL        EQU (0x3:SHL:16) ;- (RTC) Calendar Event Selection
AT91C_RTC_CALEVSEL_WEEK   EQU (0x0:SHL:16) ;- (RTC) Week change (every Monday at time 00:00:00).
AT91C_RTC_CALEVSEL_MONTH  EQU (0x1:SHL:16) ;- (RTC) Month change (every 01 of each month at time 00:00:00).
AT91C_RTC_CALEVSEL_YEAR   EQU (0x2:SHL:16) ;- (RTC) Year change (every January 1 at time 00:00:00).
;- -------- RTC_MR : (RTC Offset: 0x4) RTC Mode Register -------- 
AT91C_RTC_HRMOD           EQU (0x1:SHL:0) ;- (RTC) 12-24 hour Mode
;- -------- RTC_TIMR : (RTC Offset: 0x8) RTC Time Register -------- 
AT91C_RTC_SEC             EQU (0x7F:SHL:0) ;- (RTC) Current Second
AT91C_RTC_MIN             EQU (0x7F:SHL:8) ;- (RTC) Current Minute
AT91C_RTC_HOUR            EQU (0x1F:SHL:16) ;- (RTC) Current Hour
AT91C_RTC_AMPM            EQU (0x1:SHL:22) ;- (RTC) Ante Meridiem, Post Meridiem Indicator
;- -------- RTC_CALR : (RTC Offset: 0xc) RTC Calendar Register -------- 
AT91C_RTC_CENT            EQU (0x3F:SHL:0) ;- (RTC) Current Century
AT91C_RTC_YEAR            EQU (0xFF:SHL:8) ;- (RTC) Current Year
AT91C_RTC_MONTH           EQU (0x1F:SHL:16) ;- (RTC) Current Month
AT91C_RTC_DAY             EQU (0x7:SHL:21) ;- (RTC) Current Day
AT91C_RTC_DATE            EQU (0x3F:SHL:24) ;- (RTC) Current Date
;- -------- RTC_TIMALR : (RTC Offset: 0x10) RTC Time Alarm Register -------- 
AT91C_RTC_SECEN           EQU (0x1:SHL:7) ;- (RTC) Second Alarm Enable
AT91C_RTC_MINEN           EQU (0x1:SHL:15) ;- (RTC) Minute Alarm
AT91C_RTC_HOUREN          EQU (0x1:SHL:23) ;- (RTC) Current Hour
;- -------- RTC_CALALR : (RTC Offset: 0x14) RTC Calendar Alarm Register -------- 
AT91C_RTC_MONTHEN         EQU (0x1:SHL:23) ;- (RTC) Month Alarm Enable
AT91C_RTC_DATEEN          EQU (0x1:SHL:31) ;- (RTC) Date Alarm Enable
;- -------- RTC_SR : (RTC Offset: 0x18) RTC Status Register -------- 
AT91C_RTC_ACKUPD          EQU (0x1:SHL:0) ;- (RTC) Acknowledge for Update
AT91C_RTC_ALARM           EQU (0x1:SHL:1) ;- (RTC) Alarm Flag
AT91C_RTC_SECEV           EQU (0x1:SHL:2) ;- (RTC) Second Event
AT91C_RTC_TIMEV           EQU (0x1:SHL:3) ;- (RTC) Time Event
AT91C_RTC_CALEV           EQU (0x1:SHL:4) ;- (RTC) Calendar event
;- -------- RTC_SCCR : (RTC Offset: 0x1c) RTC Status Clear Command Register -------- 
;- -------- RTC_IER : (RTC Offset: 0x20) RTC Interrupt Enable Register -------- 
;- -------- RTC_IDR : (RTC Offset: 0x24) RTC Interrupt Disable Register -------- 
;- -------- RTC_IMR : (RTC Offset: 0x28) RTC Interrupt Mask Register -------- 
;- -------- RTC_VER : (RTC Offset: 0x2c) RTC Valid Entry Register -------- 
AT91C_RTC_NVTIM           EQU (0x1:SHL:0) ;- (RTC) Non valid Time
AT91C_RTC_NVCAL           EQU (0x1:SHL:1) ;- (RTC) Non valid Calendar
AT91C_RTC_NVTIMALR        EQU (0x1:SHL:2) ;- (RTC) Non valid time Alarm
AT91C_RTC_NVCALALR        EQU (0x1:SHL:3) ;- (RTC) Nonvalid Calendar Alarm

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR System Timer Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_ST
ST_CR           #  4 ;- Control Register
ST_PIMR         #  4 ;- Period Interval Mode Register
ST_WDMR         #  4 ;- Watchdog Mode Register
ST_RTMR         #  4 ;- Real-time Mode Register
ST_SR           #  4 ;- Status Register
ST_IER          #  4 ;- Interrupt Enable Register
ST_IDR          #  4 ;- Interrupt Disable Register
ST_IMR          #  4 ;- Interrupt Mask Register
ST_RTAR         #  4 ;- Real-time Alarm Register
ST_CRTR         #  4 ;- Current Real-time Register
;- -------- ST_CR : (ST Offset: 0x0) System Timer Control Register -------- 
AT91C_ST_WDRST            EQU (0x1:SHL:0) ;- (ST) Watchdog Timer Restart
;- -------- ST_PIMR : (ST Offset: 0x4) System Timer Period Interval Mode Register -------- 
AT91C_ST_PIV              EQU (0xFFFF:SHL:0) ;- (ST) Watchdog Timer Restart
;- -------- ST_WDMR : (ST Offset: 0x8) System Timer Watchdog Mode Register -------- 
AT91C_ST_WDV              EQU (0xFFFF:SHL:0) ;- (ST) Watchdog Timer Restart
AT91C_ST_RSTEN            EQU (0x1:SHL:16) ;- (ST) Reset Enable
AT91C_ST_EXTEN            EQU (0x1:SHL:17) ;- (ST) External Signal Assertion Enable
;- -------- ST_RTMR : (ST Offset: 0xc) System Timer Real-time Mode Register -------- 
AT91C_ST_RTPRES           EQU (0xFFFF:SHL:0) ;- (ST) Real-time Timer Prescaler Value
;- -------- ST_SR : (ST Offset: 0x10) System Timer Status Register -------- 
AT91C_ST_PITS             EQU (0x1:SHL:0) ;- (ST) Period Interval Timer Interrupt
AT91C_ST_WDOVF            EQU (0x1:SHL:1) ;- (ST) Watchdog Overflow
AT91C_ST_RTTINC           EQU (0x1:SHL:2) ;- (ST) Real-time Timer Increment
AT91C_ST_ALMS             EQU (0x1:SHL:3) ;- (ST) Alarm Status
;- -------- ST_IER : (ST Offset: 0x14) System Timer Interrupt Enable Register -------- 
;- -------- ST_IDR : (ST Offset: 0x18) System Timer Interrupt Disable Register -------- 
;- -------- ST_IMR : (ST Offset: 0x1c) System Timer Interrupt Mask Register -------- 
;- -------- ST_RTAR : (ST Offset: 0x20) System Timer Real-time Alarm Register -------- 
AT91C_ST_ALMV             EQU (0xFFFFF:SHL:0) ;- (ST) Alarm Value Value
;- -------- ST_CRTR : (ST Offset: 0x24) System Timer Current Real-time Register -------- 
AT91C_ST_CRTV             EQU (0xFFFFF:SHL:0) ;- (ST) Current Real-time Value

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Power Management Controler
;- *****************************************************************************
                ^ 0 ;- AT91S_PMC
PMC_SCER        #  4 ;- System Clock Enable Register
PMC_SCDR        #  4 ;- System Clock Disable Register
PMC_SCSR        #  4 ;- System Clock Status Register
                #  4 ;- Reserved
PMC_PCER        #  4 ;- Peripheral Clock Enable Register
PMC_PCDR        #  4 ;- Peripheral Clock Disable Register
PMC_PCSR        #  4 ;- Peripheral Clock Status Register
                # 20 ;- Reserved
PMC_MCKR        #  4 ;- Master Clock Register
                # 12 ;- Reserved
PMC_PCKR        # 32 ;- Programmable Clock Register
PMC_IER         #  4 ;- Interrupt Enable Register
PMC_IDR         #  4 ;- Interrupt Disable Register
PMC_SR          #  4 ;- Status Register
PMC_IMR         #  4 ;- Interrupt Mask Register
;- -------- PMC_SCER : (PMC Offset: 0x0) System Clock Enable Register -------- 
AT91C_PMC_PCK             EQU (0x1:SHL:0) ;- (PMC) Processor Clock
AT91C_PMC_UDP             EQU (0x1:SHL:1) ;- (PMC) USB Device Port Clock
AT91C_PMC_MCKUDP          EQU (0x1:SHL:2) ;- (PMC) USB Device Port Master Clock Automatic Disable on Suspend
AT91C_PMC_UHP             EQU (0x1:SHL:4) ;- (PMC) USB Host Port Clock
AT91C_PMC_PCK0            EQU (0x1:SHL:8) ;- (PMC) Programmable Clock Output
AT91C_PMC_PCK1            EQU (0x1:SHL:9) ;- (PMC) Programmable Clock Output
AT91C_PMC_PCK2            EQU (0x1:SHL:10) ;- (PMC) Programmable Clock Output
AT91C_PMC_PCK3            EQU (0x1:SHL:11) ;- (PMC) Programmable Clock Output
AT91C_PMC_PCK4            EQU (0x1:SHL:12) ;- (PMC) Programmable Clock Output
AT91C_PMC_PCK5            EQU (0x1:SHL:13) ;- (PMC) Programmable Clock Output
AT91C_PMC_PCK6            EQU (0x1:SHL:14) ;- (PMC) Programmable Clock Output
AT91C_PMC_PCK7            EQU (0x1:SHL:15) ;- (PMC) Programmable Clock Output
;- -------- PMC_SCDR : (PMC Offset: 0x4) System Clock Disable Register -------- 
;- -------- PMC_SCSR : (PMC Offset: 0x8) System Clock Status Register -------- 
;- -------- PMC_MCKR : (PMC Offset: 0x30) Master Clock Register -------- 
AT91C_PMC_CSS             EQU (0x3:SHL:0) ;- (PMC) Programmable Clock Selection
AT91C_PMC_CSS_SLOW_CLK    EQU (0x0) ;- (PMC) Slow Clock is selected
AT91C_PMC_CSS_MAIN_CLK    EQU (0x1) ;- (PMC) Main Clock is selected
AT91C_PMC_CSS_PLLA_CLK    EQU (0x2) ;- (PMC) Clock from PLL A is selected
AT91C_PMC_CSS_PLLB_CLK    EQU (0x3) ;- (PMC) Clock from PLL B is selected
AT91C_PMC_PRES            EQU (0x7:SHL:2) ;- (PMC) Programmable Clock Prescaler
AT91C_PMC_PRES_CLK        EQU (0x0:SHL:2) ;- (PMC) Selected clock
AT91C_PMC_PRES_CLK_2      EQU (0x1:SHL:2) ;- (PMC) Selected clock divided by 2
AT91C_PMC_PRES_CLK_4      EQU (0x2:SHL:2) ;- (PMC) Selected clock divided by 4
AT91C_PMC_PRES_CLK_8      EQU (0x3:SHL:2) ;- (PMC) Selected clock divided by 8
AT91C_PMC_PRES_CLK_16     EQU (0x4:SHL:2) ;- (PMC) Selected clock divided by 16
AT91C_PMC_PRES_CLK_32     EQU (0x5:SHL:2) ;- (PMC) Selected clock divided by 32
AT91C_PMC_PRES_CLK_64     EQU (0x6:SHL:2) ;- (PMC) Selected clock divided by 64
AT91C_PMC_MDIV            EQU (0x3:SHL:8) ;- (PMC) Master Clock Division
AT91C_PMC_MDIV_1          EQU (0x0:SHL:8) ;- (PMC) The master clock and the processor clock are the same
AT91C_PMC_MDIV_2          EQU (0x1:SHL:8) ;- (PMC) The processor clock is twice as fast as the master clock
AT91C_PMC_MDIV_3          EQU (0x2:SHL:8) ;- (PMC) The processor clock is three times faster than the master clock
AT91C_PMC_MDIV_4          EQU (0x3:SHL:8) ;- (PMC) The processor clock is four times faster than the master clock
;- -------- PMC_PCKR : (PMC Offset: 0x40) Programmable Clock Register -------- 
;- -------- PMC_IER : (PMC Offset: 0x60) PMC Interrupt Enable Register -------- 
AT91C_PMC_MOSCS           EQU (0x1:SHL:0) ;- (PMC) MOSC Status/Enable/Disable/Mask
AT91C_PMC_LOCKA           EQU (0x1:SHL:1) ;- (PMC) PLL A Status/Enable/Disable/Mask
AT91C_PMC_LOCKB           EQU (0x1:SHL:2) ;- (PMC) PLL B Status/Enable/Disable/Mask
AT91C_PMC_MCKRDY          EQU (0x1:SHL:3) ;- (PMC) MCK_RDY Status/Enable/Disable/Mask
AT91C_PMC_PCK0RDY         EQU (0x1:SHL:8) ;- (PMC) PCK0_RDY Status/Enable/Disable/Mask
AT91C_PMC_PCK1RDY         EQU (0x1:SHL:9) ;- (PMC) PCK1_RDY Status/Enable/Disable/Mask
AT91C_PMC_PCK2RDY         EQU (0x1:SHL:10) ;- (PMC) PCK2_RDY Status/Enable/Disable/Mask
AT91C_PMC_PCK3RDY         EQU (0x1:SHL:11) ;- (PMC) PCK3_RDY Status/Enable/Disable/Mask
AT91C_PMC_PCK4RDY         EQU (0x1:SHL:12) ;- (PMC) PCK4_RDY Status/Enable/Disable/Mask
AT91C_PMC_PCK5RDY         EQU (0x1:SHL:13) ;- (PMC) PCK5_RDY Status/Enable/Disable/Mask
AT91C_PMC_PCK6RDY         EQU (0x1:SHL:14) ;- (PMC) PCK6_RDY Status/Enable/Disable/Mask
AT91C_PMC_PCK7RDY         EQU (0x1:SHL:15) ;- (PMC) PCK7_RDY Status/Enable/Disable/Mask
;- -------- PMC_IDR : (PMC Offset: 0x64) PMC Interrupt Disable Register -------- 
;- -------- PMC_SR : (PMC Offset: 0x68) PMC Status Register -------- 
;- -------- PMC_IMR : (PMC Offset: 0x6c) PMC Interrupt Mask Register -------- 

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Clock Generator Controler
;- *****************************************************************************
                ^ 0 ;- AT91S_CKGR
CKGR_MOR        #  4 ;- Main Oscillator Register
CKGR_MCFR       #  4 ;- Main Clock  Frequency Register
CKGR_PLLAR      #  4 ;- PLL A Register
CKGR_PLLBR      #  4 ;- PLL B Register
;- -------- CKGR_MOR : (CKGR Offset: 0x0) Main Oscillator Register -------- 
AT91C_CKGR_MOSCEN         EQU (0x1:SHL:0) ;- (CKGR) Main Oscillator Enable
AT91C_CKGR_OSCTEST        EQU (0x1:SHL:1) ;- (CKGR) Oscillator Test
AT91C_CKGR_OSCOUNT        EQU (0xFF:SHL:8) ;- (CKGR) Main Oscillator Start-up Time
;- -------- CKGR_MCFR : (CKGR Offset: 0x4) Main Clock Frequency Register -------- 
AT91C_CKGR_MAINF          EQU (0xFFFF:SHL:0) ;- (CKGR) Main Clock Frequency
AT91C_CKGR_MAINRDY        EQU (0x1:SHL:16) ;- (CKGR) Main Clock Ready
;- -------- CKGR_PLLAR : (CKGR Offset: 0x8) PLL A Register -------- 
AT91C_CKGR_DIVA           EQU (0xFF:SHL:0) ;- (CKGR) Divider Selected
AT91C_CKGR_DIVA_0         EQU (0x0) ;- (CKGR) Divider output is 0
AT91C_CKGR_DIVA_BYPASS    EQU (0x1) ;- (CKGR) Divider is bypassed
AT91C_CKGR_PLLACOUNT      EQU (0x3F:SHL:8) ;- (CKGR) PLL A Counter
AT91C_CKGR_OUTA           EQU (0x3:SHL:14) ;- (CKGR) PLL A Output Frequency Range
AT91C_CKGR_OUTA_0         EQU (0x0:SHL:14) ;- (CKGR) Please refer to the PLLA datasheet
AT91C_CKGR_OUTA_1         EQU (0x1:SHL:14) ;- (CKGR) Please refer to the PLLA datasheet
AT91C_CKGR_OUTA_2         EQU (0x2:SHL:14) ;- (CKGR) Please refer to the PLLA datasheet
AT91C_CKGR_OUTA_3         EQU (0x3:SHL:14) ;- (CKGR) Please refer to the PLLA datasheet
AT91C_CKGR_MULA           EQU (0x7FF:SHL:16) ;- (CKGR) PLL A Multiplier
AT91C_CKGR_SRCA           EQU (0x1:SHL:29) ;- (CKGR) PLL A Source
;- -------- CKGR_PLLBR : (CKGR Offset: 0xc) PLL B Register -------- 
AT91C_CKGR_DIVB           EQU (0xFF:SHL:0) ;- (CKGR) Divider Selected
AT91C_CKGR_DIVB_0         EQU (0x0) ;- (CKGR) Divider output is 0
AT91C_CKGR_DIVB_BYPASS    EQU (0x1) ;- (CKGR) Divider is bypassed
AT91C_CKGR_PLLBCOUNT      EQU (0x3F:SHL:8) ;- (CKGR) PLL B Counter
AT91C_CKGR_OUTB           EQU (0x3:SHL:14) ;- (CKGR) PLL B Output Frequency Range
AT91C_CKGR_OUTB_0         EQU (0x0:SHL:14) ;- (CKGR) Please refer to the PLLB datasheet
AT91C_CKGR_OUTB_1         EQU (0x1:SHL:14) ;- (CKGR) Please refer to the PLLB datasheet
AT91C_CKGR_OUTB_2         EQU (0x2:SHL:14) ;- (CKGR) Please refer to the PLLB datasheet
AT91C_CKGR_OUTB_3         EQU (0x3:SHL:14) ;- (CKGR) Please refer to the PLLB datasheet
AT91C_CKGR_MULB           EQU (0x7FF:SHL:16) ;- (CKGR) PLL B Multiplier
AT91C_CKGR_USB_96M        EQU (0x1:SHL:28) ;- (CKGR) Divider for USB Ports
AT91C_CKGR_USB_PLL        EQU (0x1:SHL:29) ;- (CKGR) PLL Use

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Parallel Input Output Controler
;- *****************************************************************************
                ^ 0 ;- AT91S_PIO
PIO_PER         #  4 ;- PIO Enable Register
PIO_PDR         #  4 ;- PIO Disable Register
PIO_PSR         #  4 ;- PIO Status Register
                #  4 ;- Reserved
PIO_OER         #  4 ;- Output Enable Register
PIO_ODR         #  4 ;- Output Disable Registerr
PIO_OSR         #  4 ;- Output Status Register
                #  4 ;- Reserved
PIO_IFER        #  4 ;- Input Filter Enable Register
PIO_IFDR        #  4 ;- Input Filter Disable Register
PIO_IFSR        #  4 ;- Input Filter Status Register
                #  4 ;- Reserved
PIO_SODR        #  4 ;- Set Output Data Register
PIO_CODR        #  4 ;- Clear Output Data Register
PIO_ODSR        #  4 ;- Output Data Status Register
PIO_PDSR        #  4 ;- Pin Data Status Register
PIO_IER         #  4 ;- Interrupt Enable Register
PIO_IDR         #  4 ;- Interrupt Disable Register
PIO_IMR         #  4 ;- Interrupt Mask Register
PIO_ISR         #  4 ;- Interrupt Status Register
PIO_MDER        #  4 ;- Multi-driver Enable Register
PIO_MDDR        #  4 ;- Multi-driver Disable Register
PIO_MDSR        #  4 ;- Multi-driver Status Register
                #  4 ;- Reserved
PIO_PPUDR       #  4 ;- Pull-up Disable Register
PIO_PPUER       #  4 ;- Pull-up Enable Register
PIO_PPUSR       #  4 ;- Pad Pull-up Status Register
                #  4 ;- Reserved
PIO_ASR         #  4 ;- Select A Register
PIO_BSR         #  4 ;- Select B Register
PIO_ABSR        #  4 ;- AB Select Status Register
                # 36 ;- Reserved
PIO_OWER        #  4 ;- Output Write Enable Register
PIO_OWDR        #  4 ;- Output Write Disable Register
PIO_OWSR        #  4 ;- Output Write Status Register

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Debug Unit
;- *****************************************************************************
                ^ 0 ;- AT91S_DBGU
DBGU_CR         #  4 ;- Control Register
DBGU_MR         #  4 ;- Mode Register
DBGU_IER        #  4 ;- Interrupt Enable Register
DBGU_IDR        #  4 ;- Interrupt Disable Register
DBGU_IMR        #  4 ;- Interrupt Mask Register
DBGU_CSR        #  4 ;- Channel Status Register
DBGU_RHR        #  4 ;- Receiver Holding Register
DBGU_THR        #  4 ;- Transmitter Holding Register
DBGU_BRGR       #  4 ;- Baud Rate Generator Register
                # 28 ;- Reserved
DBGU_C1R        #  4 ;- Chip ID1 Register
DBGU_C2R        #  4 ;- Chip ID2 Register
DBGU_FNTR       #  4 ;- Force NTRST Register
                # 180 ;- Reserved
DBGU_RPR        #  4 ;- Receive Pointer Register
DBGU_RCR        #  4 ;- Receive Counter Register
DBGU_TPR        #  4 ;- Transmit Pointer Register
DBGU_TCR        #  4 ;- Transmit Counter Register
DBGU_RNPR       #  4 ;- Receive Next Pointer Register
DBGU_RNCR       #  4 ;- Receive Next Counter Register
DBGU_TNPR       #  4 ;- Transmit Next Pointer Register
DBGU_TNCR       #  4 ;- Transmit Next Counter Register
DBGU_PTCR       #  4 ;- PDC Transfer Control Register
DBGU_PTSR       #  4 ;- PDC Transfer Status Register
;- -------- DBGU_CR : (DBGU Offset: 0x0) Debug Unit Control Register -------- 
AT91C_US_RSTRX            EQU (0x1:SHL:2) ;- (DBGU) Reset Receiver
AT91C_US_RSTTX            EQU (0x1:SHL:3) ;- (DBGU) Reset Transmitter
AT91C_US_RXEN             EQU (0x1:SHL:4) ;- (DBGU) Receiver Enable
AT91C_US_RXDIS            EQU (0x1:SHL:5) ;- (DBGU) Receiver Disable
AT91C_US_TXEN             EQU (0x1:SHL:6) ;- (DBGU) Transmitter Enable
AT91C_US_TXDIS            EQU (0x1:SHL:7) ;- (DBGU) Transmitter Disable
;- -------- DBGU_MR : (DBGU Offset: 0x4) Debug Unit Mode Register -------- 
AT91C_US_PAR              EQU (0x7:SHL:9) ;- (DBGU) Parity type
AT91C_US_PAR_EVEN         EQU (0x0:SHL:9) ;- (DBGU) Even Parity
AT91C_US_PAR_ODD          EQU (0x1:SHL:9) ;- (DBGU) Odd Parity
AT91C_US_PAR_SPACE        EQU (0x2:SHL:9) ;- (DBGU) Parity forced to 0 (Space)
AT91C_US_PAR_MARK         EQU (0x3:SHL:9) ;- (DBGU) Parity forced to 1 (Mark)
AT91C_US_PAR_NONE         EQU (0x4:SHL:9) ;- (DBGU) No Parity
AT91C_US_PAR_MULTI_DROP   EQU (0x6:SHL:9) ;- (DBGU) Multi-drop mode
AT91C_US_CHMODE           EQU (0x3:SHL:14) ;- (DBGU) Channel Mode
AT91C_US_CHMODE_NORMAL    EQU (0x0:SHL:14) ;- (DBGU) Normal Mode: The USART channel operates as an RX/TX USART.
AT91C_US_CHMODE_AUTO      EQU (0x1:SHL:14) ;- (DBGU) Automatic Echo: Receiver Data Input is connected to the TXD pin.
AT91C_US_CHMODE_LOCAL     EQU (0x2:SHL:14) ;- (DBGU) Local Loopback: Transmitter Output Signal is connected to Receiver Input Signal.
AT91C_US_CHMODE_REMOTE    EQU (0x3:SHL:14) ;- (DBGU) Remote Loopback: RXD pin is internally connected to TXD pin.
;- -------- DBGU_IER : (DBGU Offset: 0x8) Debug Unit Interrupt Enable Register -------- 
AT91C_US_RXRDY            EQU (0x1:SHL:0) ;- (DBGU) RXRDY Interrupt
AT91C_US_TXRDY            EQU (0x1:SHL:1) ;- (DBGU) TXRDY Interrupt
AT91C_US_ENDRX            EQU (0x1:SHL:3) ;- (DBGU) End of Receive Transfer Interrupt
AT91C_US_ENDTX            EQU (0x1:SHL:4) ;- (DBGU) End of Transmit Interrupt
AT91C_US_OVRE             EQU (0x1:SHL:5) ;- (DBGU) Overrun Interrupt
AT91C_US_FRAME            EQU (0x1:SHL:6) ;- (DBGU) Framing Error Interrupt
AT91C_US_PARE             EQU (0x1:SHL:7) ;- (DBGU) Parity Error Interrupt
AT91C_US_TXEMPTY          EQU (0x1:SHL:9) ;- (DBGU) TXEMPTY Interrupt
AT91C_US_TXBUFE           EQU (0x1:SHL:11) ;- (DBGU) TXBUFE Interrupt
AT91C_US_RXBUFF           EQU (0x1:SHL:12) ;- (DBGU) RXBUFF Interrupt
AT91C_US_COMM_TX          EQU (0x1:SHL:30) ;- (DBGU) COMM_TX Interrupt
AT91C_US_COMM_RX          EQU (0x1:SHL:31) ;- (DBGU) COMM_RX Interrupt
;- -------- DBGU_IDR : (DBGU Offset: 0xc) Debug Unit Interrupt Disable Register -------- 
;- -------- DBGU_IMR : (DBGU Offset: 0x10) Debug Unit Interrupt Mask Register -------- 
;- -------- DBGU_CSR : (DBGU Offset: 0x14) Debug Unit Channel Status Register -------- 
;- -------- DBGU_FNTR : (DBGU Offset: 0x48) Debug Unit FORCE_NTRST Register -------- 
AT91C_US_FORCE_NTRST      EQU (0x1:SHL:0) ;- (DBGU) Force NTRST in JTAG

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Peripheral Data Controller
;- *****************************************************************************
                ^ 0 ;- AT91S_PDC
PDC_RPR         #  4 ;- Receive Pointer Register
PDC_RCR         #  4 ;- Receive Counter Register
PDC_TPR         #  4 ;- Transmit Pointer Register
PDC_TCR         #  4 ;- Transmit Counter Register
PDC_RNPR        #  4 ;- Receive Next Pointer Register
PDC_RNCR        #  4 ;- Receive Next Counter Register
PDC_TNPR        #  4 ;- Transmit Next Pointer Register
PDC_TNCR        #  4 ;- Transmit Next Counter Register
PDC_PTCR        #  4 ;- PDC Transfer Control Register
PDC_PTSR        #  4 ;- PDC Transfer Status Register
;- -------- PDC_PTCR : (PDC Offset: 0x20) PDC Transfer Control Register -------- 
AT91C_PDC_RXTEN           EQU (0x1:SHL:0) ;- (PDC) Receiver Transfer Enable
AT91C_PDC_RXTDIS          EQU (0x1:SHL:1) ;- (PDC) Receiver Transfer Disable
AT91C_PDC_TXTEN           EQU (0x1:SHL:8) ;- (PDC) Transmitter Transfer Enable
AT91C_PDC_TXTDIS          EQU (0x1:SHL:9) ;- (PDC) Transmitter Transfer Disable
;- -------- PDC_PTSR : (PDC Offset: 0x24) PDC Transfer Status Register -------- 

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Advanced Interrupt Controller
;- *****************************************************************************
                ^ 0 ;- AT91S_AIC
AIC_SMR         # 128 ;- Source Mode Register
AIC_SVR         # 128 ;- Source Vector Register
AIC_IVR         #  4 ;- IRQ Vector Register
AIC_FVR         #  4 ;- FIQ Vector Register
AIC_ISR         #  4 ;- Interrupt Status Register
AIC_IPR         #  4 ;- Interrupt Pending Register
AIC_IMR         #  4 ;- Interrupt Mask Register
AIC_CISR        #  4 ;- Core Interrupt Status Register
                #  8 ;- Reserved
AIC_IECR        #  4 ;- Interrupt Enable Command Register
AIC_IDCR        #  4 ;- Interrupt Disable Command Register
AIC_ICCR        #  4 ;- Interrupt Clear Command Register
AIC_ISCR        #  4 ;- Interrupt Set Command Register
AIC_EOICR       #  4 ;- End of Interrupt Command Register
AIC_SPU         #  4 ;- Spurious Vector Register
AIC_DCR         #  4 ;- Debug Control Register (Protect)
                #  4 ;- Reserved
AIC_FFER        #  4 ;- Fast Forcing Enable Register
AIC_FFDR        #  4 ;- Fast Forcing Disable Register
AIC_FFSR        #  4 ;- Fast Forcing Status Register
;- -------- AIC_SMR : (AIC Offset: 0x0) Control Register -------- 
AT91C_AIC_PRIOR           EQU (0x7:SHL:0) ;- (AIC) Priority Level
AT91C_AIC_PRIOR_LOWEST    EQU (0x0) ;- (AIC) Lowest priority level
AT91C_AIC_PRIOR_HIGHEST   EQU (0x7) ;- (AIC) Highest priority level
AT91C_AIC_SRCTYPE         EQU (0x3:SHL:5) ;- (AIC) Interrupt Source Type
AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE EQU (0x0:SHL:5) ;- (AIC) Internal Sources Code Label Level Sensitive
AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED EQU (0x1:SHL:5) ;- (AIC) Internal Sources Code Label Edge triggered
AT91C_AIC_SRCTYPE_EXT_HIGH_LEVEL EQU (0x2:SHL:5) ;- (AIC) External Sources Code Label High-level Sensitive
AT91C_AIC_SRCTYPE_EXT_POSITIVE_EDGE EQU (0x3:SHL:5) ;- (AIC) External Sources Code Label Positive Edge triggered
;- -------- AIC_CISR : (AIC Offset: 0x114) AIC Core Interrupt Status Register -------- 
AT91C_AIC_NFIQ            EQU (0x1:SHL:0) ;- (AIC) NFIQ Status
AT91C_AIC_NIRQ            EQU (0x1:SHL:1) ;- (AIC) NIRQ Status
;- -------- AIC_DCR : (AIC Offset: 0x138) AIC Debug Control Register (Protect) -------- 
AT91C_AIC_DCR_PROT        EQU (0x1:SHL:0) ;- (AIC) Protection Mode
AT91C_AIC_DCR_GMSK        EQU (0x1:SHL:1) ;- (AIC) General Mask

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Serial Parallel Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_SPI
SPI_CR          #  4 ;- Control Register
SPI_MR          #  4 ;- Mode Register
SPI_RDR         #  4 ;- Receive Data Register
SPI_TDR         #  4 ;- Transmit Data Register
SPI_SR          #  4 ;- Status Register
SPI_IER         #  4 ;- Interrupt Enable Register
SPI_IDR         #  4 ;- Interrupt Disable Register
SPI_IMR         #  4 ;- Interrupt Mask Register
                # 16 ;- Reserved
SPI_CSR         # 16 ;- Chip Select Register
                # 192 ;- Reserved
SPI_RPR         #  4 ;- Receive Pointer Register
SPI_RCR         #  4 ;- Receive Counter Register
SPI_TPR         #  4 ;- Transmit Pointer Register
SPI_TCR         #  4 ;- Transmit Counter Register
SPI_RNPR        #  4 ;- Receive Next Pointer Register
SPI_RNCR        #  4 ;- Receive Next Counter Register
SPI_TNPR        #  4 ;- Transmit Next Pointer Register
SPI_TNCR        #  4 ;- Transmit Next Counter Register
SPI_PTCR        #  4 ;- PDC Transfer Control Register
SPI_PTSR        #  4 ;- PDC Transfer Status Register
;- -------- SPI_CR : (SPI Offset: 0x0) SPI Control Register -------- 
AT91C_SPI_SPIEN           EQU (0x1:SHL:0) ;- (SPI) SPI Enable
AT91C_SPI_SPIDIS          EQU (0x1:SHL:1) ;- (SPI) SPI Disable
AT91C_SPI_SWRST           EQU (0x1:SHL:7) ;- (SPI) SPI Software reset
;- -------- SPI_MR : (SPI Offset: 0x4) SPI Mode Register -------- 
AT91C_SPI_MSTR            EQU (0x1:SHL:0) ;- (SPI) Master/Slave Mode
AT91C_SPI_PS              EQU (0x1:SHL:1) ;- (SPI) Peripheral Select
AT91C_SPI_PS_FIXED        EQU (0x0:SHL:1) ;- (SPI) Fixed Peripheral Select
AT91C_SPI_PS_VARIABLE     EQU (0x1:SHL:1) ;- (SPI) Variable Peripheral Select
AT91C_SPI_PCSDEC          EQU (0x1:SHL:2) ;- (SPI) Chip Select Decode
AT91C_SPI_DIV32           EQU (0x1:SHL:3) ;- (SPI) Clock Selection
AT91C_SPI_MODFDIS         EQU (0x1:SHL:4) ;- (SPI) Mode Fault Detection
AT91C_SPI_LLB             EQU (0x1:SHL:7) ;- (SPI) Clock Selection
AT91C_SPI_PCS             EQU (0xF:SHL:16) ;- (SPI) Peripheral Chip Select
AT91C_SPI_DLYBCS          EQU (0xFF:SHL:24) ;- (SPI) Delay Between Chip Selects
;- -------- SPI_RDR : (SPI Offset: 0x8) Receive Data Register -------- 
AT91C_SPI_RD              EQU (0xFFFF:SHL:0) ;- (SPI) Receive Data
AT91C_SPI_RPCS            EQU (0xF:SHL:16) ;- (SPI) Peripheral Chip Select Status
;- -------- SPI_TDR : (SPI Offset: 0xc) Transmit Data Register -------- 
AT91C_SPI_TD              EQU (0xFFFF:SHL:0) ;- (SPI) Transmit Data
AT91C_SPI_TPCS            EQU (0xF:SHL:16) ;- (SPI) Peripheral Chip Select Status
;- -------- SPI_SR : (SPI Offset: 0x10) Status Register -------- 
AT91C_SPI_RDRF            EQU (0x1:SHL:0) ;- (SPI) Receive Data Register Full
AT91C_SPI_TDRE            EQU (0x1:SHL:1) ;- (SPI) Transmit Data Register Empty
AT91C_SPI_MODF            EQU (0x1:SHL:2) ;- (SPI) Mode Fault Error
AT91C_SPI_OVRES           EQU (0x1:SHL:3) ;- (SPI) Overrun Error Status
AT91C_SPI_SPENDRX         EQU (0x1:SHL:4) ;- (SPI) End of Receiver Transfer
AT91C_SPI_SPENDTX         EQU (0x1:SHL:5) ;- (SPI) End of Receiver Transfer
AT91C_SPI_RXBUFF          EQU (0x1:SHL:6) ;- (SPI) RXBUFF Interrupt
AT91C_SPI_TXBUFE          EQU (0x1:SHL:7) ;- (SPI) TXBUFE Interrupt
AT91C_SPI_SPIENS          EQU (0x1:SHL:16) ;- (SPI) Enable Status
;- -------- SPI_IER : (SPI Offset: 0x14) Interrupt Enable Register -------- 
;- -------- SPI_IDR : (SPI Offset: 0x18) Interrupt Disable Register -------- 
;- -------- SPI_IMR : (SPI Offset: 0x1c) Interrupt Mask Register -------- 
;- -------- SPI_CSR : (SPI Offset: 0x30) Chip Select Register -------- 
AT91C_SPI_CPOL            EQU (0x1:SHL:0) ;- (SPI) Clock Polarity
AT91C_SPI_NCPHA           EQU (0x1:SHL:1) ;- (SPI) Clock Phase
AT91C_SPI_BITS            EQU (0xF:SHL:4) ;- (SPI) Bits Per Transfer
AT91C_SPI_BITS_8          EQU (0x0:SHL:4) ;- (SPI) 8 Bits Per transfer
AT91C_SPI_BITS_9          EQU (0x1:SHL:4) ;- (SPI) 9 Bits Per transfer
AT91C_SPI_BITS_10         EQU (0x2:SHL:4) ;- (SPI) 10 Bits Per transfer
AT91C_SPI_BITS_11         EQU (0x3:SHL:4) ;- (SPI) 11 Bits Per transfer
AT91C_SPI_BITS_12         EQU (0x4:SHL:4) ;- (SPI) 12 Bits Per transfer
AT91C_SPI_BITS_13         EQU (0x5:SHL:4) ;- (SPI) 13 Bits Per transfer
AT91C_SPI_BITS_14         EQU (0x6:SHL:4) ;- (SPI) 14 Bits Per transfer
AT91C_SPI_BITS_15         EQU (0x7:SHL:4) ;- (SPI) 15 Bits Per transfer
AT91C_SPI_BITS_16         EQU (0x8:SHL:4) ;- (SPI) 16 Bits Per transfer
AT91C_SPI_SCBR            EQU (0xFF:SHL:8) ;- (SPI) Serial Clock Baud Rate
AT91C_SPI_DLYBS           EQU (0xFF:SHL:16) ;- (SPI) Serial Clock Baud Rate
AT91C_SPI_DLYBCT          EQU (0xFF:SHL:24) ;- (SPI) Delay Between Consecutive Transfers

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Synchronous Serial Controller Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_SSC
SSC_CR          #  4 ;- Control Register
SSC_CMR         #  4 ;- Clock Mode Register
                #  8 ;- Reserved
SSC_RCMR        #  4 ;- Receive Clock ModeRegister
SSC_RFMR        #  4 ;- Receive Frame Mode Register
SSC_TCMR        #  4 ;- Transmit Clock Mode Register
SSC_TFMR        #  4 ;- Transmit Frame Mode Register
SSC_RHR         #  4 ;- Receive Holding Register
SSC_THR         #  4 ;- Transmit Holding Register
                #  8 ;- Reserved
SSC_RSHR        #  4 ;- Receive Sync Holding Register
SSC_TSHR        #  4 ;- Transmit Sync Holding Register
SSC_RC0R        #  4 ;- Receive Compare 0 Register
SSC_RC1R        #  4 ;- Receive Compare 1 Register
SSC_SR          #  4 ;- Status Register
SSC_IER         #  4 ;- Interrupt Enable Register
SSC_IDR         #  4 ;- Interrupt Disable Register
SSC_IMR         #  4 ;- Interrupt Mask Register
                # 176 ;- Reserved
SSC_RPR         #  4 ;- Receive Pointer Register
SSC_RCR         #  4 ;- Receive Counter Register
SSC_TPR         #  4 ;- Transmit Pointer Register
SSC_TCR         #  4 ;- Transmit Counter Register
SSC_RNPR        #  4 ;- Receive Next Pointer Register
SSC_RNCR        #  4 ;- Receive Next Counter Register
SSC_TNPR        #  4 ;- Transmit Next Pointer Register
SSC_TNCR        #  4 ;- Transmit Next Counter Register
SSC_PTCR        #  4 ;- PDC Transfer Control Register
SSC_PTSR        #  4 ;- PDC Transfer Status Register
;- -------- SSC_CR : (SSC Offset: 0x0) SSC Control Register -------- 
AT91C_SSC_RXEN            EQU (0x1:SHL:0) ;- (SSC) Receive Enable
AT91C_SSC_RXDIS           EQU (0x1:SHL:1) ;- (SSC) Receive Disable
AT91C_SSC_TXEN            EQU (0x1:SHL:8) ;- (SSC) Transmit Enable
AT91C_SSC_TXDIS           EQU (0x1:SHL:9) ;- (SSC) Transmit Disable
AT91C_SSC_SWRST           EQU (0x1:SHL:15) ;- (SSC) Software Reset
;- -------- SSC_RCMR : (SSC Offset: 0x10) SSC Receive Clock Mode Register -------- 
AT91C_SSC_CKS             EQU (0x3:SHL:0) ;- (SSC) Receive/Transmit Clock Selection
AT91C_SSC_CKS_DIV         EQU (0x0) ;- (SSC) Divided Clock
AT91C_SSC_CKS_TK          EQU (0x1) ;- (SSC) TK Clock signal
AT91C_SSC_CKS_RK          EQU (0x2) ;- (SSC) RK pin
AT91C_SSC_CKO             EQU (0x7:SHL:2) ;- (SSC) Receive/Transmit Clock Output Mode Selection
AT91C_SSC_CKO_NONE        EQU (0x0:SHL:2) ;- (SSC) Receive/Transmit Clock Output Mode: None RK pin: Input-only
AT91C_SSC_CKO_CONTINOUS   EQU (0x1:SHL:2) ;- (SSC) Continuous Receive/Transmit Clock RK pin: Output
AT91C_SSC_CKO_DATA_TX     EQU (0x2:SHL:2) ;- (SSC) Receive/Transmit Clock only during data transfers RK pin: Output
AT91C_SSC_CKI             EQU (0x1:SHL:5) ;- (SSC) Receive/Transmit Clock Inversion
AT91C_SSC_CKG             EQU (0x3:SHL:6) ;- (SSC) Receive/Transmit Clock Gating Selection
AT91C_SSC_CKG_NONE        EQU (0x0:SHL:6) ;- (SSC) Receive/Transmit Clock Gating: None, continuous clock
AT91C_SSC_CKG_LOW         EQU (0x1:SHL:6) ;- (SSC) Receive/Transmit Clock enabled only if RF Low
AT91C_SSC_CKG_HIGH        EQU (0x2:SHL:6) ;- (SSC) Receive/Transmit Clock enabled only if RF High
AT91C_SSC_START           EQU (0xF:SHL:8) ;- (SSC) Receive/Transmit Start Selection
AT91C_SSC_START_CONTINOUS EQU (0x0:SHL:8) ;- (SSC) Continuous, as soon as the receiver is enabled, and immediately after the end of transfer of the previous data.
AT91C_SSC_START_TX        EQU (0x1:SHL:8) ;- (SSC) Transmit/Receive start
AT91C_SSC_START_LOW_RF    EQU (0x2:SHL:8) ;- (SSC) Detection of a low level on RF input
AT91C_SSC_START_HIGH_RF   EQU (0x3:SHL:8) ;- (SSC) Detection of a high level on RF input
AT91C_SSC_START_FALL_RF   EQU (0x4:SHL:8) ;- (SSC) Detection of a falling edge on RF input
AT91C_SSC_START_RISE_RF   EQU (0x5:SHL:8) ;- (SSC) Detection of a rising edge on RF input
AT91C_SSC_START_LEVEL_RF  EQU (0x6:SHL:8) ;- (SSC) Detection of any level change on RF input
AT91C_SSC_START_EDGE_RF   EQU (0x7:SHL:8) ;- (SSC) Detection of any edge on RF input
AT91C_SSC_START_0         EQU (0x8:SHL:8) ;- (SSC) Compare 0
AT91C_SSC_STOP            EQU (0x1:SHL:12) ;- (SSC) Receive Stop Selection
AT91C_SSC_STTOUT          EQU (0x1:SHL:15) ;- (SSC) Receive/Transmit Start Output Selection
AT91C_SSC_STTDLY          EQU (0xFF:SHL:16) ;- (SSC) Receive/Transmit Start Delay
AT91C_SSC_PERIOD          EQU (0xFF:SHL:24) ;- (SSC) Receive/Transmit Period Divider Selection
;- -------- SSC_RFMR : (SSC Offset: 0x14) SSC Receive Frame Mode Register -------- 
AT91C_SSC_DATLEN          EQU (0x1F:SHL:0) ;- (SSC) Data Length
AT91C_SSC_LOOP            EQU (0x1:SHL:5) ;- (SSC) Loop Mode
AT91C_SSC_MSBF            EQU (0x1:SHL:7) ;- (SSC) Most Significant Bit First
AT91C_SSC_DATNB           EQU (0xF:SHL:8) ;- (SSC) Data Number per Frame
AT91C_SSC_FSLEN           EQU (0xF:SHL:16) ;- (SSC) Receive/Transmit Frame Sync length
AT91C_SSC_FSOS            EQU (0x7:SHL:20) ;- (SSC) Receive/Transmit Frame Sync Output Selection
AT91C_SSC_FSOS_NONE       EQU (0x0:SHL:20) ;- (SSC) Selected Receive/Transmit Frame Sync Signal: None RK pin Input-only
AT91C_SSC_FSOS_NEGATIVE   EQU (0x1:SHL:20) ;- (SSC) Selected Receive/Transmit Frame Sync Signal: Negative Pulse
AT91C_SSC_FSOS_POSITIVE   EQU (0x2:SHL:20) ;- (SSC) Selected Receive/Transmit Frame Sync Signal: Positive Pulse
AT91C_SSC_FSOS_LOW        EQU (0x3:SHL:20) ;- (SSC) Selected Receive/Transmit Frame Sync Signal: Driver Low during data transfer
AT91C_SSC_FSOS_HIGH       EQU (0x4:SHL:20) ;- (SSC) Selected Receive/Transmit Frame Sync Signal: Driver High during data transfer
AT91C_SSC_FSOS_TOGGLE     EQU (0x5:SHL:20) ;- (SSC) Selected Receive/Transmit Frame Sync Signal: Toggling at each start of data transfer
AT91C_SSC_FSEDGE          EQU (0x1:SHL:24) ;- (SSC) Frame Sync Edge Detection
;- -------- SSC_TCMR : (SSC Offset: 0x18) SSC Transmit Clock Mode Register -------- 
;- -------- SSC_TFMR : (SSC Offset: 0x1c) SSC Transmit Frame Mode Register -------- 
AT91C_SSC_DATDEF          EQU (0x1:SHL:5) ;- (SSC) Data Default Value
AT91C_SSC_FSDEN           EQU (0x1:SHL:23) ;- (SSC) Frame Sync Data Enable
;- -------- SSC_SR : (SSC Offset: 0x40) SSC Status Register -------- 
AT91C_SSC_TXRDY           EQU (0x1:SHL:0) ;- (SSC) Transmit Ready
AT91C_SSC_TXEMPTY         EQU (0x1:SHL:1) ;- (SSC) Transmit Empty
AT91C_SSC_ENDTX           EQU (0x1:SHL:2) ;- (SSC) End Of Transmission
AT91C_SSC_TXBUFE          EQU (0x1:SHL:3) ;- (SSC) Transmit Buffer Empty
AT91C_SSC_RXRDY           EQU (0x1:SHL:4) ;- (SSC) Receive Ready
AT91C_SSC_OVRUN           EQU (0x1:SHL:5) ;- (SSC) Receive Overrun
AT91C_SSC_ENDRX           EQU (0x1:SHL:6) ;- (SSC) End of Reception
AT91C_SSC_RXBUFF          EQU (0x1:SHL:7) ;- (SSC) Receive Buffer Full
AT91C_SSC_CP0             EQU (0x1:SHL:8) ;- (SSC) Compare 0
AT91C_SSC_CP1             EQU (0x1:SHL:9) ;- (SSC) Compare 1
AT91C_SSC_TXSYN           EQU (0x1:SHL:10) ;- (SSC) Transmit Sync
AT91C_SSC_RXSYN           EQU (0x1:SHL:11) ;- (SSC) Receive Sync
AT91C_SSC_TXENA           EQU (0x1:SHL:16) ;- (SSC) Transmit Enable
AT91C_SSC_RXENA           EQU (0x1:SHL:17) ;- (SSC) Receive Enable
;- -------- SSC_IER : (SSC Offset: 0x44) SSC Interrupt Enable Register -------- 
;- -------- SSC_IDR : (SSC Offset: 0x48) SSC Interrupt Disable Register -------- 
;- -------- SSC_IMR : (SSC Offset: 0x4c) SSC Interrupt Mask Register -------- 

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Usart
;- *****************************************************************************
                ^ 0 ;- AT91S_USART
US_CR           #  4 ;- Control Register
US_MR           #  4 ;- Mode Register
US_IER          #  4 ;- Interrupt Enable Register
US_IDR          #  4 ;- Interrupt Disable Register
US_IMR          #  4 ;- Interrupt Mask Register
US_CSR          #  4 ;- Channel Status Register
US_RHR          #  4 ;- Receiver Holding Register
US_THR          #  4 ;- Transmitter Holding Register
US_BRGR         #  4 ;- Baud Rate Generator Register
US_RTOR         #  4 ;- Receiver Time-out Register
US_TTGR         #  4 ;- Transmitter Time-guard Register
                # 20 ;- Reserved
US_FIDI         #  4 ;- FI_DI_Ratio Register
US_NER          #  4 ;- Nb Errors Register
US_XXR          #  4 ;- XON_XOFF Register
US_IF           #  4 ;- IRDA_FILTER Register
                # 176 ;- Reserved
US_RPR          #  4 ;- Receive Pointer Register
US_RCR          #  4 ;- Receive Counter Register
US_TPR          #  4 ;- Transmit Pointer Register
US_TCR          #  4 ;- Transmit Counter Register
US_RNPR         #  4 ;- Receive Next Pointer Register
US_RNCR         #  4 ;- Receive Next Counter Register
US_TNPR         #  4 ;- Transmit Next Pointer Register
US_TNCR         #  4 ;- Transmit Next Counter Register
US_PTCR         #  4 ;- PDC Transfer Control Register
US_PTSR         #  4 ;- PDC Transfer Status Register
;- -------- US_CR : (USART Offset: 0x0) Debug Unit Control Register -------- 
AT91C_US_RSTSTA           EQU (0x1:SHL:8) ;- (USART) Reset Status Bits
AT91C_US_STTBRK           EQU (0x1:SHL:9) ;- (USART) Start Break
AT91C_US_STPBRK           EQU (0x1:SHL:10) ;- (USART) Stop Break
AT91C_US_STTTO            EQU (0x1:SHL:11) ;- (USART) Start Time-out
AT91C_US_SENDA            EQU (0x1:SHL:12) ;- (USART) Send Address
AT91C_US_RSTIT            EQU (0x1:SHL:13) ;- (USART) Reset Iterations
AT91C_US_RSTNACK          EQU (0x1:SHL:14) ;- (USART) Reset Non Acknowledge
AT91C_US_RETTO            EQU (0x1:SHL:15) ;- (USART) Rearm Time-out
AT91C_US_DTREN            EQU (0x1:SHL:16) ;- (USART) Data Terminal ready Enable
AT91C_US_DTRDIS           EQU (0x1:SHL:17) ;- (USART) Data Terminal ready Disable
AT91C_US_RTSEN            EQU (0x1:SHL:18) ;- (USART) Request to Send enable
AT91C_US_RTSDIS           EQU (0x1:SHL:19) ;- (USART) Request to Send Disable
;- -------- US_MR : (USART Offset: 0x4) Debug Unit Mode Register -------- 
AT91C_US_USMODE           EQU (0xF:SHL:0) ;- (USART) Usart mode
AT91C_US_USMODE_NORMAL    EQU (0x0) ;- (USART) Normal
AT91C_US_USMODE_RS485     EQU (0x1) ;- (USART) RS485
AT91C_US_USMODE_HWHSH     EQU (0x2) ;- (USART) Hardware Handshaking
AT91C_US_USMODE_MODEM     EQU (0x3) ;- (USART) Modem
AT91C_US_USMODE_ISO7816_0 EQU (0x4) ;- (USART) ISO7816 protocol: T = 0
AT91C_US_USMODE_ISO7816_1 EQU (0x6) ;- (USART) ISO7816 protocol: T = 1
AT91C_US_USMODE_IRDA      EQU (0x8) ;- (USART) IrDA
AT91C_US_USMODE_SWHSH     EQU (0xC) ;- (USART) Software Handshaking
AT91C_US_CLKS             EQU (0x3:SHL:4) ;- (USART) Clock Selection (Baud Rate generator Input Clock
AT91C_US_CLKS_CLOCK       EQU (0x0:SHL:4) ;- (USART) Clock
AT91C_US_CLKS_FDIV1       EQU (0x1:SHL:4) ;- (USART) fdiv1
AT91C_US_CLKS_SLOW        EQU (0x2:SHL:4) ;- (USART) slow_clock (ARM)
AT91C_US_CLKS_EXT         EQU (0x3:SHL:4) ;- (USART) External (SCK)
AT91C_US_CHRL             EQU (0x3:SHL:6) ;- (USART) Clock Selection (Baud Rate generator Input Clock
AT91C_US_CHRL_5_BITS      EQU (0x0:SHL:6) ;- (USART) Character Length: 5 bits
AT91C_US_CHRL_6_BITS      EQU (0x1:SHL:6) ;- (USART) Character Length: 6 bits
AT91C_US_CHRL_7_BITS      EQU (0x2:SHL:6) ;- (USART) Character Length: 7 bits
AT91C_US_CHRL_8_BITS      EQU (0x3:SHL:6) ;- (USART) Character Length: 8 bits
AT91C_US_SYNC             EQU (0x1:SHL:8) ;- (USART) Synchronous Mode Select
AT91C_US_NBSTOP           EQU (0x3:SHL:12) ;- (USART) Number of Stop bits
AT91C_US_NBSTOP_1_BIT     EQU (0x0:SHL:12) ;- (USART) 1 stop bit
AT91C_US_NBSTOP_15_BIT    EQU (0x1:SHL:12) ;- (USART) Asynchronous (SYNC=0) 2 stop bits Synchronous (SYNC=1) 2 stop bits
AT91C_US_NBSTOP_2_BIT     EQU (0x2:SHL:12) ;- (USART) 2 stop bits
AT91C_US_MSBF             EQU (0x1:SHL:16) ;- (USART) Bit Order
AT91C_US_MODE9            EQU (0x1:SHL:17) ;- (USART) 9-bit Character length
AT91C_US_CKLO             EQU (0x1:SHL:18) ;- (USART) Clock Output Select
AT91C_US_OVER             EQU (0x1:SHL:19) ;- (USART) Over Sampling Mode
AT91C_US_INACK            EQU (0x1:SHL:20) ;- (USART) Inhibit Non Acknowledge
AT91C_US_DSNACK           EQU (0x1:SHL:21) ;- (USART) Disable Successive NACK
AT91C_US_MAX_ITER         EQU (0x1:SHL:24) ;- (USART) Number of Repetitions
AT91C_US_FILTER           EQU (0x1:SHL:28) ;- (USART) Receive Line Filter
;- -------- US_IER : (USART Offset: 0x8) Debug Unit Interrupt Enable Register -------- 
AT91C_US_RXBRK            EQU (0x1:SHL:2) ;- (USART) Break Received/End of Break
AT91C_US_TIMEOUT          EQU (0x1:SHL:8) ;- (USART) Receiver Time-out
AT91C_US_ITERATION        EQU (0x1:SHL:10) ;- (USART) Max number of Repetitions Reached
AT91C_US_NACK             EQU (0x1:SHL:13) ;- (USART) Non Acknowledge
AT91C_US_RIIC             EQU (0x1:SHL:16) ;- (USART) Ring INdicator Input Change Flag
AT91C_US_DSRIC            EQU (0x1:SHL:17) ;- (USART) Data Set Ready Input Change Flag
AT91C_US_DCDIC            EQU (0x1:SHL:18) ;- (USART) Data Carrier Flag
AT91C_US_CTSIC            EQU (0x1:SHL:19) ;- (USART) Clear To Send Input Change Flag
;- -------- US_IDR : (USART Offset: 0xc) Debug Unit Interrupt Disable Register -------- 
;- -------- US_IMR : (USART Offset: 0x10) Debug Unit Interrupt Mask Register -------- 
;- -------- US_CSR : (USART Offset: 0x14) Debug Unit Channel Status Register -------- 
AT91C_US_RI               EQU (0x1:SHL:20) ;- (USART) Image of RI Input
AT91C_US_DSR              EQU (0x1:SHL:21) ;- (USART) Image of DSR Input
AT91C_US_DCD              EQU (0x1:SHL:22) ;- (USART) Image of DCD Input
AT91C_US_CTS              EQU (0x1:SHL:23) ;- (USART) Image of CTS Input

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Two-wire Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_TWI
TWI_CR          #  4 ;- Control Register
TWI_MMR         #  4 ;- Master Mode Register
TWI_SMR         #  4 ;- Slave Mode Register
TWI_IADR        #  4 ;- Internal Address Register
TWI_CWGR        #  4 ;- Clock Waveform Generator Register
                # 12 ;- Reserved
TWI_SR          #  4 ;- Status Register
TWI_IER         #  4 ;- Interrupt Enable Register
TWI_IDR         #  4 ;- Interrupt Disable Register
TWI_IMR         #  4 ;- Interrupt Mask Register
TWI_RHR         #  4 ;- Receive Holding Register
TWI_THR         #  4 ;- Transmit Holding Register
;- -------- TWI_CR : (TWI Offset: 0x0) TWI Control Register -------- 
AT91C_TWI_START           EQU (0x1:SHL:0) ;- (TWI) Send a START Condition
AT91C_TWI_STOP            EQU (0x1:SHL:1) ;- (TWI) Send a STOP Condition
AT91C_TWI_MSEN            EQU (0x1:SHL:2) ;- (TWI) TWI Master Transfer Enabled
AT91C_TWI_MSDIS           EQU (0x1:SHL:3) ;- (TWI) TWI Master Transfer Disabled
AT91C_TWI_SVEN            EQU (0x1:SHL:4) ;- (TWI) TWI Slave Transfer Enabled
AT91C_TWI_SVDIS           EQU (0x1:SHL:5) ;- (TWI) TWI Slave Transfer Disabled
AT91C_TWI_SWRST           EQU (0x1:SHL:7) ;- (TWI) Software Reset
;- -------- TWI_MMR : (TWI Offset: 0x4) TWI Master Mode Register -------- 
AT91C_TWI_IADRSZ          EQU (0x3:SHL:8) ;- (TWI) Internal Device Address Size
AT91C_TWI_IADRSZ_NO       EQU (0x0:SHL:8) ;- (TWI) No internal device address
AT91C_TWI_IADRSZ_1_BYTE   EQU (0x1:SHL:8) ;- (TWI) One-byte internal device address
AT91C_TWI_IADRSZ_2_BYTE   EQU (0x2:SHL:8) ;- (TWI) Two-byte internal device address
AT91C_TWI_IADRSZ_3_BYTE   EQU (0x3:SHL:8) ;- (TWI) Three-byte internal device address
AT91C_TWI_MREAD           EQU (0x1:SHL:12) ;- (TWI) Master Read Direction
AT91C_TWI_DADR            EQU (0x7F:SHL:16) ;- (TWI) Device Address
;- -------- TWI_SMR : (TWI Offset: 0x8) TWI Slave Mode Register -------- 
AT91C_TWI_SADR            EQU (0x7F:SHL:16) ;- (TWI) Slave Device Address
;- -------- TWI_CWGR : (TWI Offset: 0x10) TWI Clock Waveform Generator Register -------- 
AT91C_TWI_CLDIV           EQU (0xFF:SHL:0) ;- (TWI) Clock Low Divider
AT91C_TWI_CHDIV           EQU (0xFF:SHL:8) ;- (TWI) Clock High Divider
AT91C_TWI_CKDIV           EQU (0x7:SHL:16) ;- (TWI) Clock Divider
;- -------- TWI_SR : (TWI Offset: 0x20) TWI Status Register -------- 
AT91C_TWI_TXCOMP          EQU (0x1:SHL:0) ;- (TWI) Transmission Completed
AT91C_TWI_RXRDY           EQU (0x1:SHL:1) ;- (TWI) Receive holding register ReaDY
AT91C_TWI_TXRDY           EQU (0x1:SHL:2) ;- (TWI) Transmit holding register ReaDY
AT91C_TWI_SVREAD          EQU (0x1:SHL:3) ;- (TWI) Slave Read
AT91C_TWI_SVACC           EQU (0x1:SHL:4) ;- (TWI) Slave Access
AT91C_TWI_GCACC           EQU (0x1:SHL:5) ;- (TWI) General Call Access
AT91C_TWI_OVRE            EQU (0x1:SHL:6) ;- (TWI) Overrun Error
AT91C_TWI_UNRE            EQU (0x1:SHL:7) ;- (TWI) Underrun Error
AT91C_TWI_NACK            EQU (0x1:SHL:8) ;- (TWI) Not Acknowledged
AT91C_TWI_ARBLST          EQU (0x1:SHL:9) ;- (TWI) Arbitration Lost
;- -------- TWI_IER : (TWI Offset: 0x24) TWI Interrupt Enable Register -------- 
;- -------- TWI_IDR : (TWI Offset: 0x28) TWI Interrupt Disable Register -------- 
;- -------- TWI_IMR : (TWI Offset: 0x2c) TWI Interrupt Mask Register -------- 

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Multimedia Card Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_MCI
MCI_CR          #  4 ;- MCI Control Register
MCI_MR          #  4 ;- MCI Mode Register
MCI_DTOR        #  4 ;- MCI Data Timeout Register
MCI_SDCR        #  4 ;- MCI SD Card Register
MCI_ARGR        #  4 ;- MCI Argument Register
MCI_CMDR        #  4 ;- MCI Command Register
                #  8 ;- Reserved
MCI_RSPR        # 16 ;- MCI Response Register
MCI_RDR         #  4 ;- MCI Receive Data Register
MCI_TDR         #  4 ;- MCI Transmit Data Register
                #  8 ;- Reserved
MCI_SR          #  4 ;- MCI Status Register
MCI_IER         #  4 ;- MCI Interrupt Enable Register
MCI_IDR         #  4 ;- MCI Interrupt Disable Register
MCI_IMR         #  4 ;- MCI Interrupt Mask Register
                # 176 ;- Reserved
MCI_RPR         #  4 ;- Receive Pointer Register
MCI_RCR         #  4 ;- Receive Counter Register
MCI_TPR         #  4 ;- Transmit Pointer Register
MCI_TCR         #  4 ;- Transmit Counter Register
MCI_RNPR        #  4 ;- Receive Next Pointer Register
MCI_RNCR        #  4 ;- Receive Next Counter Register
MCI_TNPR        #  4 ;- Transmit Next Pointer Register
MCI_TNCR        #  4 ;- Transmit Next Counter Register
MCI_PTCR        #  4 ;- PDC Transfer Control Register
MCI_PTSR        #  4 ;- PDC Transfer Status Register
;- -------- MCI_CR : (MCI Offset: 0x0) MCI Control Register -------- 
AT91C_MCI_MCIEN           EQU (0x1:SHL:0) ;- (MCI) Multimedia Interface Enable
AT91C_MCI_MCIDIS          EQU (0x1:SHL:1) ;- (MCI) Multimedia Interface Disable
AT91C_MCI_PWSEN           EQU (0x1:SHL:2) ;- (MCI) Power Save Mode Enable
AT91C_MCI_PWSDIS          EQU (0x1:SHL:3) ;- (MCI) Power Save Mode Disable
;- -------- MCI_MR : (MCI Offset: 0x4) MCI Mode Register -------- 
AT91C_MCI_CLKDIV          EQU (0x1:SHL:0) ;- (MCI) Clock Divider
AT91C_MCI_PWSDIV          EQU (0x1:SHL:8) ;- (MCI) Power Saving Divider
AT91C_MCI_PDCPADV         EQU (0x1:SHL:14) ;- (MCI) PDC Padding Value
AT91C_MCI_PDCMODE         EQU (0x1:SHL:15) ;- (MCI) PDC Oriented Mode
AT91C_MCI_BLKLEN          EQU (0x1:SHL:18) ;- (MCI) Data Block Length
;- -------- MCI_DTOR : (MCI Offset: 0x8) MCI Data Timeout Register -------- 
AT91C_MCI_DTOCYC          EQU (0x1:SHL:0) ;- (MCI) Data Timeout Cycle Number
AT91C_MCI_DTOMUL          EQU (0x7:SHL:4) ;- (MCI) Data Timeout Multiplier
AT91C_MCI_DTOMUL_1        EQU (0x0:SHL:4) ;- (MCI) DTOCYC x 1
AT91C_MCI_DTOMUL_16       EQU (0x1:SHL:4) ;- (MCI) DTOCYC x 16
AT91C_MCI_DTOMUL_128      EQU (0x2:SHL:4) ;- (MCI) DTOCYC x 128
AT91C_MCI_DTOMUL_256      EQU (0x3:SHL:4) ;- (MCI) DTOCYC x 256
AT91C_MCI_DTOMUL_1024     EQU (0x4:SHL:4) ;- (MCI) DTOCYC x 1024
AT91C_MCI_DTOMUL_4096     EQU (0x5:SHL:4) ;- (MCI) DTOCYC x 4096
AT91C_MCI_DTOMUL_65536    EQU (0x6:SHL:4) ;- (MCI) DTOCYC x 65536
AT91C_MCI_DTOMUL_1048576  EQU (0x7:SHL:4) ;- (MCI) DTOCYC x 1048576
;- -------- MCI_SDCR : (MCI Offset: 0xc) MCI SD Card Register -------- 
AT91C_MCI_SCDSEL          EQU (0x1:SHL:0) ;- (MCI) SD Card Selector
AT91C_MCI_SCDBUS          EQU (0x1:SHL:7) ;- (MCI) SD Card Bus Width
;- -------- MCI_CMDR : (MCI Offset: 0x14) MCI Command Register -------- 
AT91C_MCI_CMDNB           EQU (0x1F:SHL:0) ;- (MCI) Command Number
AT91C_MCI_RSPTYP          EQU (0x3:SHL:6) ;- (MCI) Response Type
AT91C_MCI_RSPTYP_NO       EQU (0x0:SHL:6) ;- (MCI) No response
AT91C_MCI_RSPTYP_48       EQU (0x1:SHL:6) ;- (MCI) 48-bit response
AT91C_MCI_RSPTYP_136      EQU (0x2:SHL:6) ;- (MCI) 136-bit response
AT91C_MCI_SPCMD           EQU (0x7:SHL:8) ;- (MCI) Special CMD
AT91C_MCI_SPCMD_NONE      EQU (0x0:SHL:8) ;- (MCI) Not a special CMD
AT91C_MCI_SPCMD_INIT      EQU (0x1:SHL:8) ;- (MCI) Initialization CMD
AT91C_MCI_SPCMD_SYNC      EQU (0x2:SHL:8) ;- (MCI) Synchronized CMD
AT91C_MCI_SPCMD_IT_CMD    EQU (0x4:SHL:8) ;- (MCI) Interrupt command
AT91C_MCI_SPCMD_IT_REP    EQU (0x5:SHL:8) ;- (MCI) Interrupt response
AT91C_MCI_OPDCMD          EQU (0x1:SHL:11) ;- (MCI) Open Drain Command
AT91C_MCI_MAXLAT          EQU (0x1:SHL:12) ;- (MCI) Maximum Latency for Command to respond
AT91C_MCI_TRCMD           EQU (0x3:SHL:16) ;- (MCI) Transfer CMD
AT91C_MCI_TRCMD_NO        EQU (0x0:SHL:16) ;- (MCI) No transfer
AT91C_MCI_TRCMD_START     EQU (0x1:SHL:16) ;- (MCI) Start transfer
AT91C_MCI_TRCMD_STOP      EQU (0x2:SHL:16) ;- (MCI) Stop transfer
AT91C_MCI_TRDIR           EQU (0x1:SHL:18) ;- (MCI) Transfer Direction
AT91C_MCI_TRTYP           EQU (0x3:SHL:19) ;- (MCI) Transfer Type
AT91C_MCI_TRTYP_BLOCK     EQU (0x0:SHL:19) ;- (MCI) Block Transfer type
AT91C_MCI_TRTYP_MULTIPLE  EQU (0x1:SHL:19) ;- (MCI) Multiple Block transfer type
AT91C_MCI_TRTYP_STREAM    EQU (0x2:SHL:19) ;- (MCI) Stream transfer type
;- -------- MCI_SR : (MCI Offset: 0x40) MCI Status Register -------- 
AT91C_MCI_CMDRDY          EQU (0x1:SHL:0) ;- (MCI) Command Ready flag
AT91C_MCI_RXRDY           EQU (0x1:SHL:1) ;- (MCI) RX Ready flag
AT91C_MCI_TXRDY           EQU (0x1:SHL:2) ;- (MCI) TX Ready flag
AT91C_MCI_BLKE            EQU (0x1:SHL:3) ;- (MCI) Data Block Transfer Ended flag
AT91C_MCI_DTIP            EQU (0x1:SHL:4) ;- (MCI) Data Transfer in Progress flag
AT91C_MCI_NOTBUSY         EQU (0x1:SHL:5) ;- (MCI) Data Line Not Busy flag
AT91C_MCI_ENDRX           EQU (0x1:SHL:6) ;- (MCI) End of RX Buffer flag
AT91C_MCI_ENDTX           EQU (0x1:SHL:7) ;- (MCI) End of TX Buffer flag
AT91C_MCI_RXBUFF          EQU (0x1:SHL:14) ;- (MCI) RX Buffer Full flag
AT91C_MCI_TXBUFE          EQU (0x1:SHL:15) ;- (MCI) TX Buffer Empty flag
AT91C_MCI_RINDE           EQU (0x1:SHL:16) ;- (MCI) Response Index Error flag
AT91C_MCI_RDIRE           EQU (0x1:SHL:17) ;- (MCI) Response Direction Error flag
AT91C_MCI_RCRCE           EQU (0x1:SHL:18) ;- (MCI) Response CRC Error flag
AT91C_MCI_RENDE           EQU (0x1:SHL:19) ;- (MCI) Response End Bit Error flag
AT91C_MCI_RTOE            EQU (0x1:SHL:20) ;- (MCI) Response Time-out Error flag
AT91C_MCI_DCRCE           EQU (0x1:SHL:21) ;- (MCI) data CRC Error flag
AT91C_MCI_DTOE            EQU (0x1:SHL:22) ;- (MCI) Data timeout Error flag
AT91C_MCI_OVRE            EQU (0x1:SHL:30) ;- (MCI) Overrun flag
AT91C_MCI_UNRE            EQU (0x1:SHL:31) ;- (MCI) Underrun flag
;- -------- MCI_IER : (MCI Offset: 0x44) MCI Interrupt Enable Register -------- 
;- -------- MCI_IDR : (MCI Offset: 0x48) MCI Interrupt Disable Register -------- 
;- -------- MCI_IMR : (MCI Offset: 0x4c) MCI Interrupt Mask Register -------- 

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR USB Device Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_UDP
UDP_NUM         #  4 ;- Frame Number Register
UDP_GLBSTATE    #  4 ;- Global State Register
UDP_FADDR       #  4 ;- Function Address Register
                #  4 ;- Reserved
UDP_IER         #  4 ;- Interrupt Enable Register
UDP_IDR         #  4 ;- Interrupt Disable Register
UDP_IMR         #  4 ;- Interrupt Mask Register
UDP_ISR         #  4 ;- Interrupt Status Register
UDP_ICR         #  4 ;- Interrupt Clear Register
                #  4 ;- Reserved
UDP_RSTEP       #  4 ;- Reset Endpoint Register
                #  4 ;- Reserved
UDP_CSR         # 32 ;- Endpoint Control and Status Register
UDP_FDR         # 32 ;- Endpoint FIFO Data Register
;- -------- UDP_FRM_NUM : (UDP Offset: 0x0) USB Frame Number Register -------- 
AT91C_UDP_FRM_NUM         EQU (0x7FF:SHL:0) ;- (UDP) Frame Number as Defined in the Packet Field Formats
AT91C_UDP_FRM_ERR         EQU (0x1:SHL:16) ;- (UDP) Frame Error
AT91C_UDP_FRM_OK          EQU (0x1:SHL:17) ;- (UDP) Frame OK
;- -------- UDP_GLB_STATE : (UDP Offset: 0x4) USB Global State Register -------- 
AT91C_UDP_FADDEN          EQU (0x1:SHL:0) ;- (UDP) Function Address Enable
AT91C_UDP_CONFG           EQU (0x1:SHL:1) ;- (UDP) Configured
AT91C_UDP_RMWUPE          EQU (0x1:SHL:2) ;- (UDP) Remote Wake Up Enable
AT91C_UDP_RSMINPR         EQU (0x1:SHL:3) ;- (UDP) A Resume Has Been Sent to the Host
;- -------- UDP_FADDR : (UDP Offset: 0x8) USB Function Address Register -------- 
AT91C_UDP_FADD            EQU (0xFF:SHL:0) ;- (UDP) Function Address Value
AT91C_UDP_FEN             EQU (0x1:SHL:8) ;- (UDP) Function Enable
;- -------- UDP_IER : (UDP Offset: 0x10) USB Interrupt Enable Register -------- 
AT91C_UDP_EPINT0          EQU (0x1:SHL:0) ;- (UDP) Endpoint 0 Interrupt
AT91C_UDP_EPINT1          EQU (0x1:SHL:1) ;- (UDP) Endpoint 0 Interrupt
AT91C_UDP_EPINT2          EQU (0x1:SHL:2) ;- (UDP) Endpoint 2 Interrupt
AT91C_UDP_EPINT3          EQU (0x1:SHL:3) ;- (UDP) Endpoint 3 Interrupt
AT91C_UDP_EPINT4          EQU (0x1:SHL:4) ;- (UDP) Endpoint 4 Interrupt
AT91C_UDP_EPINT5          EQU (0x1:SHL:5) ;- (UDP) Endpoint 5 Interrupt
AT91C_UDP_EPINT6          EQU (0x1:SHL:6) ;- (UDP) Endpoint 6 Interrupt
AT91C_UDP_EPINT7          EQU (0x1:SHL:7) ;- (UDP) Endpoint 7 Interrupt
AT91C_UDP_RXSUSP          EQU (0x1:SHL:8) ;- (UDP) USB Suspend Interrupt
AT91C_UDP_RXRSM           EQU (0x1:SHL:9) ;- (UDP) USB Resume Interrupt
AT91C_UDP_EXTRSM          EQU (0x1:SHL:10) ;- (UDP) USB External Resume Interrupt
AT91C_UDP_SOFINT          EQU (0x1:SHL:11) ;- (UDP) USB Start Of frame Interrupt
AT91C_UDP_WAKEUP          EQU (0x1:SHL:13) ;- (UDP) USB Resume Interrupt
;- -------- UDP_IDR : (UDP Offset: 0x14) USB Interrupt Disable Register -------- 
;- -------- UDP_IMR : (UDP Offset: 0x18) USB Interrupt Mask Register -------- 
;- -------- UDP_ISR : (UDP Offset: 0x1c) USB Interrupt Status Register -------- 
AT91C_UDP_ENDBUSRES       EQU (0x1:SHL:12) ;- (UDP) USB End Of Bus Reset Interrupt
;- -------- UDP_ICR : (UDP Offset: 0x20) USB Interrupt Clear Register -------- 
;- -------- UDP_RST_EP : (UDP Offset: 0x28) USB Reset Endpoint Register -------- 
AT91C_UDP_EP0             EQU (0x1:SHL:0) ;- (UDP) Reset Endpoint 0
AT91C_UDP_EP1             EQU (0x1:SHL:1) ;- (UDP) Reset Endpoint 1
AT91C_UDP_EP2             EQU (0x1:SHL:2) ;- (UDP) Reset Endpoint 2
AT91C_UDP_EP3             EQU (0x1:SHL:3) ;- (UDP) Reset Endpoint 3
AT91C_UDP_EP4             EQU (0x1:SHL:4) ;- (UDP) Reset Endpoint 4
AT91C_UDP_EP5             EQU (0x1:SHL:5) ;- (UDP) Reset Endpoint 5
AT91C_UDP_EP6             EQU (0x1:SHL:6) ;- (UDP) Reset Endpoint 6
AT91C_UDP_EP7             EQU (0x1:SHL:7) ;- (UDP) Reset Endpoint 7
;- -------- UDP_CSR : (UDP Offset: 0x30) USB Endpoint Control and Status Register -------- 
AT91C_UDP_TXCOMP          EQU (0x1:SHL:0) ;- (UDP) Generates an IN packet with data previously written in the DPR
AT91C_UDP_RX_DATA_BK0     EQU (0x1:SHL:1) ;- (UDP) Receive Data Bank 0
AT91C_UDP_RXSETUP         EQU (0x1:SHL:2) ;- (UDP) Sends STALL to the Host (Control endpoints)
AT91C_UDP_ISOERROR        EQU (0x1:SHL:3) ;- (UDP) Isochronous error (Isochronous endpoints)
AT91C_UDP_TXPKTRDY        EQU (0x1:SHL:4) ;- (UDP) Transmit Packet Ready
AT91C_UDP_FORCESTALL      EQU (0x1:SHL:5) ;- (UDP) Force Stall (used by Control, Bulk and Isochronous endpoints).
AT91C_UDP_RX_DATA_BK1     EQU (0x1:SHL:6) ;- (UDP) Receive Data Bank 1 (only used by endpoints with ping-pong attributes).
AT91C_UDP_DIR             EQU (0x1:SHL:7) ;- (UDP) Transfer Direction
AT91C_UDP_EPTYPE          EQU (0x7:SHL:8) ;- (UDP) Endpoint type
AT91C_UDP_EPTYPE_CTRL     EQU (0x0:SHL:8) ;- (UDP) Control
AT91C_UDP_EPTYPE_ISO_OUT  EQU (0x1:SHL:8) ;- (UDP) Isochronous OUT
AT91C_UDP_EPTYPE_BULK_OUT EQU (0x2:SHL:8) ;- (UDP) Bulk OUT
AT91C_UDP_EPTYPE_INT_OUT  EQU (0x3:SHL:8) ;- (UDP) Interrupt OUT
AT91C_UDP_EPTYPE_ISO_IN   EQU (0x5:SHL:8) ;- (UDP) Isochronous IN
AT91C_UDP_EPTYPE_BULK_IN  EQU (0x6:SHL:8) ;- (UDP) Bulk IN
AT91C_UDP_EPTYPE_INT_IN   EQU (0x7:SHL:8) ;- (UDP) Interrupt IN
AT91C_UDP_DTGLE           EQU (0x1:SHL:11) ;- (UDP) Data Toggle
AT91C_UDP_EPEDS           EQU (0x1:SHL:15) ;- (UDP) Endpoint Enable Disable
AT91C_UDP_RXBYTECNT       EQU (0x7FF:SHL:16) ;- (UDP) Number Of Bytes Available in the FIFO

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Timer Counter Channel Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_TC
TC_CCR          #  4 ;- Channel Control Register
TC_CMR          #  4 ;- Channel Mode Register
                #  8 ;- Reserved
TC_CV           #  4 ;- Counter Value
TC_RA           #  4 ;- Register A
TC_RB           #  4 ;- Register B
TC_RC           #  4 ;- Register C
TC_SR           #  4 ;- Status Register
TC_IER          #  4 ;- Interrupt Enable Register
TC_IDR          #  4 ;- Interrupt Disable Register
TC_IMR          #  4 ;- Interrupt Mask Register
;- -------- TC_CCR : (TC Offset: 0x0) TC Channel Control Register -------- 
AT91C_TC_CLKEN            EQU (0x1:SHL:0) ;- (TC) Counter Clock Enable Command
AT91C_TC_CLKDIS           EQU (0x1:SHL:1) ;- (TC) Counter Clock Disable Command
AT91C_TC_SWTRG            EQU (0x1:SHL:2) ;- (TC) Software Trigger Command
;- -------- TC_CMR : (TC Offset: 0x4) TC Channel Mode Register: Capture Mode / Waveform Mode -------- 
AT91C_TC_CPCSTOP          EQU (0x1:SHL:6) ;- (TC) Counter Clock Stopped with RC Compare
AT91C_TC_CPCDIS           EQU (0x1:SHL:7) ;- (TC) Counter Clock Disable with RC Compare
AT91C_TC_EEVTEDG          EQU (0x3:SHL:8) ;- (TC) External Event Edge Selection
AT91C_TC_EEVTEDG_NONE     EQU (0x0:SHL:8) ;- (TC) Edge: None
AT91C_TC_EEVTEDG_RISING   EQU (0x1:SHL:8) ;- (TC) Edge: rising edge
AT91C_TC_EEVTEDG_FALLING  EQU (0x2:SHL:8) ;- (TC) Edge: falling edge
AT91C_TC_EEVTEDG_BOTH     EQU (0x3:SHL:8) ;- (TC) Edge: each edge
AT91C_TC_EEVT             EQU (0x3:SHL:10) ;- (TC) External Event  Selection
AT91C_TC_EEVT_NONE        EQU (0x0:SHL:10) ;- (TC) Signal selected as external event: TIOB TIOB direction: input
AT91C_TC_EEVT_RISING      EQU (0x1:SHL:10) ;- (TC) Signal selected as external event: XC0 TIOB direction: output
AT91C_TC_EEVT_FALLING     EQU (0x2:SHL:10) ;- (TC) Signal selected as external event: XC1 TIOB direction: output
AT91C_TC_EEVT_BOTH        EQU (0x3:SHL:10) ;- (TC) Signal selected as external event: XC2 TIOB direction: output
AT91C_TC_ENETRG           EQU (0x1:SHL:12) ;- (TC) External Event Trigger enable
AT91C_TC_WAVESEL          EQU (0x3:SHL:13) ;- (TC) Waveform  Selection
AT91C_TC_WAVESEL_UP       EQU (0x0:SHL:13) ;- (TC) UP mode without atomatic trigger on RC Compare
AT91C_TC_WAVESEL_UPDOWN   EQU (0x1:SHL:13) ;- (TC) UPDOWN mode without automatic trigger on RC Compare
AT91C_TC_WAVESEL_UP_AUTO  EQU (0x2:SHL:13) ;- (TC) UP mode with automatic trigger on RC Compare
AT91C_TC_WAVESEL_UPDOWN_AUTO EQU (0x3:SHL:13) ;- (TC) UPDOWN mode with automatic trigger on RC Compare
AT91C_TC_CPCTRG           EQU (0x1:SHL:14) ;- (TC) RC Compare Trigger Enable
AT91C_TC_WAVE             EQU (0x1:SHL:15) ;- (TC) 
AT91C_TC_ACPA             EQU (0x3:SHL:16) ;- (TC) RA Compare Effect on TIOA
AT91C_TC_ACPA_NONE        EQU (0x0:SHL:16) ;- (TC) Effect: none
AT91C_TC_ACPA_SET         EQU (0x1:SHL:16) ;- (TC) Effect: set
AT91C_TC_ACPA_CLEAR       EQU (0x2:SHL:16) ;- (TC) Effect: clear
AT91C_TC_ACPA_TOGGLE      EQU (0x3:SHL:16) ;- (TC) Effect: toggle
AT91C_TC_ACPC             EQU (0x3:SHL:18) ;- (TC) RC Compare Effect on TIOA
AT91C_TC_ACPC_NONE        EQU (0x0:SHL:18) ;- (TC) Effect: none
AT91C_TC_ACPC_SET         EQU (0x1:SHL:18) ;- (TC) Effect: set
AT91C_TC_ACPC_CLEAR       EQU (0x2:SHL:18) ;- (TC) Effect: clear
AT91C_TC_ACPC_TOGGLE      EQU (0x3:SHL:18) ;- (TC) Effect: toggle
AT91C_TC_AEEVT            EQU (0x3:SHL:20) ;- (TC) External Event Effect on TIOA
AT91C_TC_AEEVT_NONE       EQU (0x0:SHL:20) ;- (TC) Effect: none
AT91C_TC_AEEVT_SET        EQU (0x1:SHL:20) ;- (TC) Effect: set
AT91C_TC_AEEVT_CLEAR      EQU (0x2:SHL:20) ;- (TC) Effect: clear
AT91C_TC_AEEVT_TOGGLE     EQU (0x3:SHL:20) ;- (TC) Effect: toggle
AT91C_TC_ASWTRG           EQU (0x3:SHL:22) ;- (TC) Software Trigger Effect on TIOA
AT91C_TC_ASWTRG_NONE      EQU (0x0:SHL:22) ;- (TC) Effect: none
AT91C_TC_ASWTRG_SET       EQU (0x1:SHL:22) ;- (TC) Effect: set
AT91C_TC_ASWTRG_CLEAR     EQU (0x2:SHL:22) ;- (TC) Effect: clear
AT91C_TC_ASWTRG_TOGGLE    EQU (0x3:SHL:22) ;- (TC) Effect: toggle
AT91C_TC_BCPB             EQU (0x3:SHL:24) ;- (TC) RB Compare Effect on TIOB
AT91C_TC_BCPB_NONE        EQU (0x0:SHL:24) ;- (TC) Effect: none
AT91C_TC_BCPB_SET         EQU (0x1:SHL:24) ;- (TC) Effect: set
AT91C_TC_BCPB_CLEAR       EQU (0x2:SHL:24) ;- (TC) Effect: clear
AT91C_TC_BCPB_TOGGLE      EQU (0x3:SHL:24) ;- (TC) Effect: toggle
AT91C_TC_BCPC             EQU (0x3:SHL:26) ;- (TC) RC Compare Effect on TIOB
AT91C_TC_BCPC_NONE        EQU (0x0:SHL:26) ;- (TC) Effect: none
AT91C_TC_BCPC_SET         EQU (0x1:SHL:26) ;- (TC) Effect: set
AT91C_TC_BCPC_CLEAR       EQU (0x2:SHL:26) ;- (TC) Effect: clear
AT91C_TC_BCPC_TOGGLE      EQU (0x3:SHL:26) ;- (TC) Effect: toggle
AT91C_TC_BEEVT            EQU (0x3:SHL:28) ;- (TC) External Event Effect on TIOB
AT91C_TC_BEEVT_NONE       EQU (0x0:SHL:28) ;- (TC) Effect: none
AT91C_TC_BEEVT_SET        EQU (0x1:SHL:28) ;- (TC) Effect: set
AT91C_TC_BEEVT_CLEAR      EQU (0x2:SHL:28) ;- (TC) Effect: clear
AT91C_TC_BEEVT_TOGGLE     EQU (0x3:SHL:28) ;- (TC) Effect: toggle
AT91C_TC_BSWTRG           EQU (0x3:SHL:30) ;- (TC) Software Trigger Effect on TIOB
AT91C_TC_BSWTRG_NONE      EQU (0x0:SHL:30) ;- (TC) Effect: none
AT91C_TC_BSWTRG_SET       EQU (0x1:SHL:30) ;- (TC) Effect: set
AT91C_TC_BSWTRG_CLEAR     EQU (0x2:SHL:30) ;- (TC) Effect: clear
AT91C_TC_BSWTRG_TOGGLE    EQU (0x3:SHL:30) ;- (TC) Effect: toggle
;- -------- TC_SR : (TC Offset: 0x20) TC Channel Status Register -------- 
AT91C_TC_COVFS            EQU (0x1:SHL:0) ;- (TC) Counter Overflow
AT91C_TC_LOVRS            EQU (0x1:SHL:1) ;- (TC) Load Overrun
AT91C_TC_CPAS             EQU (0x1:SHL:2) ;- (TC) RA Compare
AT91C_TC_CPBS             EQU (0x1:SHL:3) ;- (TC) RB Compare
AT91C_TC_CPCS             EQU (0x1:SHL:4) ;- (TC) RC Compare
AT91C_TC_LDRAS            EQU (0x1:SHL:5) ;- (TC) RA Loading
AT91C_TC_LDRBS            EQU (0x1:SHL:6) ;- (TC) RB Loading
AT91C_TC_ETRCS            EQU (0x1:SHL:7) ;- (TC) External Trigger
AT91C_TC_ETRGS            EQU (0x1:SHL:16) ;- (TC) Clock Enabling
AT91C_TC_MTIOA            EQU (0x1:SHL:17) ;- (TC) TIOA Mirror
AT91C_TC_MTIOB            EQU (0x1:SHL:18) ;- (TC) TIOA Mirror
;- -------- TC_IER : (TC Offset: 0x24) TC Channel Interrupt Enable Register -------- 
;- -------- TC_IDR : (TC Offset: 0x28) TC Channel Interrupt Disable Register -------- 
;- -------- TC_IMR : (TC Offset: 0x2c) TC Channel Interrupt Mask Register -------- 

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Timer Counter Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_TCB
TCB_TC0         # 48 ;- TC Channel 0
                # 16 ;- Reserved
TCB_TC1         # 48 ;- TC Channel 1
                # 16 ;- Reserved
TCB_TC2         # 48 ;- TC Channel 2
                # 16 ;- Reserved
TCB_BCR         #  4 ;- TC Block Control Register
TCB_BMR         #  4 ;- TC Block Mode Register
;- -------- TCB_BCR : (TCB Offset: 0xc0) TC Block Control Register -------- 
AT91C_TCB_SYNC            EQU (0x1:SHL:0) ;- (TCB) Synchro Command
;- -------- TCB_BMR : (TCB Offset: 0xc4) TC Block Mode Register -------- 
AT91C_TCB_TC0XC0S         EQU (0x1:SHL:0) ;- (TCB) External Clock Signal 0 Selection
AT91C_TCB_TC0XC0S_TCLK0   EQU (0x0) ;- (TCB) TCLK0 connected to XC0
AT91C_TCB_TC0XC0S_NONE    EQU (0x1) ;- (TCB) None signal connected to XC0
AT91C_TCB_TC0XC0S_TIOA1   EQU (0x2) ;- (TCB) TIOA1 connected to XC0
AT91C_TCB_TC0XC0S_TIOA2   EQU (0x3) ;- (TCB) TIOA2 connected to XC0
AT91C_TCB_TC1XC1S         EQU (0x1:SHL:2) ;- (TCB) External Clock Signal 1 Selection
AT91C_TCB_TC1XC1S_TCLK1   EQU (0x0:SHL:2) ;- (TCB) TCLK1 connected to XC1
AT91C_TCB_TC1XC1S_NONE    EQU (0x1:SHL:2) ;- (TCB) None signal connected to XC1
AT91C_TCB_TC1XC1S_TIOA0   EQU (0x2:SHL:2) ;- (TCB) TIOA0 connected to XC1
AT91C_TCB_TC1XC1S_TIOA2   EQU (0x3:SHL:2) ;- (TCB) TIOA2 connected to XC1
AT91C_TCB_TC2XC2S         EQU (0x1:SHL:4) ;- (TCB) External Clock Signal 2 Selection
AT91C_TCB_TC2XC2S_TCLK2   EQU (0x0:SHL:4) ;- (TCB) TCLK2 connected to XC2
AT91C_TCB_TC2XC2S_NONE    EQU (0x1:SHL:4) ;- (TCB) None signal connected to XC2
AT91C_TCB_TC2XC2S_TIOA0   EQU (0x2:SHL:4) ;- (TCB) TIOA0 connected to XC2
AT91C_TCB_TC2XC2S_TIOA2   EQU (0x3:SHL:4) ;- (TCB) TIOA2 connected to XC2

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR USB Host Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_UHP
UHP_HcRevision  #  4 ;- Revision
UHP_HcControl   #  4 ;- Operating modes for the Host Controller
UHP_HcCommandStatus #  4 ;- Command & status Register
UHP_HcInterruptStatus #  4 ;- Interrupt Status Register
UHP_HcInterruptEnable #  4 ;- Interrupt Enable Register
UHP_HcInterruptDisable #  4 ;- Interrupt Disable Register
UHP_HcHCCA      #  4 ;- Pointer to the Host Controller Communication Area
UHP_HcPeriodCurrentED #  4 ;- Current Isochronous or Interrupt Endpoint Descriptor
UHP_HcControlHeadED #  4 ;- First Endpoint Descriptor of the Control list
UHP_HcControlCurrentED #  4 ;- Endpoint Control and Status Register
UHP_HcBulkHeadED #  4 ;- First endpoint register of the Bulk list
UHP_HcBulkCurrentED #  4 ;- Current endpoint of the Bulk list
UHP_HcBulkDoneHead #  4 ;- Last completed transfer descriptor
UHP_HcFmInterval #  4 ;- Bit time between 2 consecutive SOFs
UHP_HcFmRemaining #  4 ;- Bit time remaining in the current Frame
UHP_HcFmNumber  #  4 ;- Frame number
UHP_HcPeriodicStart #  4 ;- Periodic Start
UHP_HcLSThreshold #  4 ;- LS Threshold
UHP_HcRhDescriptorA #  4 ;- Root Hub characteristics A
UHP_HcRhDescriptorB #  4 ;- Root Hub characteristics B
UHP_HcRhStatus  #  4 ;- Root Hub Status register
UHP_HcRhPortStatus #  8 ;- Root Hub Port Status Register

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Ethernet MAC
;- *****************************************************************************
                ^ 0 ;- AT91S_EMAC
EMAC_CTL        #  4 ;- Network Control Register
EMAC_CFG        #  4 ;- Network Configuration Register
EMAC_SR         #  4 ;- Network Status Register
EMAC_TAR        #  4 ;- Transmit Address Register
EMAC_TCR        #  4 ;- Transmit Control Register
EMAC_TSR        #  4 ;- Transmit Status Register
EMAC_RBQP       #  4 ;- Receive Buffer Queue Pointer
                #  4 ;- Reserved
EMAC_RSR        #  4 ;- Receive Status Register
EMAC_ISR        #  4 ;- Interrupt Status Register
EMAC_IER        #  4 ;- Interrupt Enable Register
EMAC_IDR        #  4 ;- Interrupt Disable Register
EMAC_IMR        #  4 ;- Interrupt Mask Register
EMAC_MAN        #  4 ;- PHY Maintenance Register
                #  8 ;- Reserved
EMAC_FRA        #  4 ;- Frames Transmitted OK Register
EMAC_SCOL       #  4 ;- Single Collision Frame Register
EMAC_MCOL       #  4 ;- Multiple Collision Frame Register
EMAC_OK         #  4 ;- Frames Received OK Register
EMAC_SEQE       #  4 ;- Frame Check Sequence Error Register
EMAC_ALE        #  4 ;- Alignment Error Register
EMAC_DTE        #  4 ;- Deferred Transmission Frame Register
EMAC_LCOL       #  4 ;- Late Collision Register
EMAC_ECOL       #  4 ;- Excessive Collision Register
EMAC_CSE        #  4 ;- Carrier Sense Error Register
EMAC_TUE        #  4 ;- Transmit Underrun Error Register
EMAC_CDE        #  4 ;- Code Error Register
EMAC_ELR        #  4 ;- Excessive Length Error Register
EMAC_RJB        #  4 ;- Receive Jabber Register
EMAC_USF        #  4 ;- Undersize Frame Register
EMAC_SQEE       #  4 ;- SQE Test Error Register
EMAC_DRFC       #  4 ;- Discarded RX Frame Register
                # 12 ;- Reserved
EMAC_HSH        #  4 ;- Hash Address High[63:32]
EMAC_HSL        #  4 ;- Hash Address Low[31:0]
EMAC_SA1L       #  4 ;- Specific Address 1 Low, First 4 bytes
EMAC_SA1H       #  4 ;- Specific Address 1 High, Last 2 bytes
EMAC_SA2L       #  4 ;- Specific Address 2 Low, First 4 bytes
EMAC_SA2H       #  4 ;- Specific Address 2 High, Last 2 bytes
EMAC_SA3L       #  4 ;- Specific Address 3 Low, First 4 bytes
EMAC_SA3H       #  4 ;- Specific Address 3 High, Last 2 bytes
EMAC_SA4L       #  4 ;- Specific Address 4 Low, First 4 bytes
EMAC_SA4H       #  4 ;- Specific Address 4 High, Last 2 bytesr
;- -------- EMAC_CTL : (EMAC Offset: 0x0)  -------- 
AT91C_EMAC_LB             EQU (0x1:SHL:0) ;- (EMAC) Loopback. Optional. When set, loopback signal is at high level.
AT91C_EMAC_LBL            EQU (0x1:SHL:1) ;- (EMAC) Loopback local. 
AT91C_EMAC_RE             EQU (0x1:SHL:2) ;- (EMAC) Receive enable. 
AT91C_EMAC_TE             EQU (0x1:SHL:3) ;- (EMAC) Transmit enable. 
AT91C_EMAC_MPE            EQU (0x1:SHL:4) ;- (EMAC) Management port enable. 
AT91C_EMAC_CSR            EQU (0x1:SHL:5) ;- (EMAC) Clear statistics registers. 
AT91C_EMAC_ISR            EQU (0x1:SHL:6) ;- (EMAC) Increment statistics registers. 
AT91C_EMAC_WES            EQU (0x1:SHL:7) ;- (EMAC) Write enable for statistics registers. 
AT91C_EMAC_BP             EQU (0x1:SHL:8) ;- (EMAC) Back pressure. 
;- -------- EMAC_CFG : (EMAC Offset: 0x4) Network Configuration Register -------- 
AT91C_EMAC_SPD            EQU (0x1:SHL:0) ;- (EMAC) Speed. 
AT91C_EMAC_FD             EQU (0x1:SHL:1) ;- (EMAC) Full duplex. 
AT91C_EMAC_BR             EQU (0x1:SHL:2) ;- (EMAC) Bit rate. 
AT91C_EMAC_CAF            EQU (0x1:SHL:4) ;- (EMAC) Copy all frames. 
AT91C_EMAC_NBC            EQU (0x1:SHL:5) ;- (EMAC) No broadcast. 
AT91C_EMAC_MTI            EQU (0x1:SHL:6) ;- (EMAC) Multicast hash enable
AT91C_EMAC_UNI            EQU (0x1:SHL:7) ;- (EMAC) Unicast hash enable. 
AT91C_EMAC_BIG            EQU (0x1:SHL:8) ;- (EMAC) Receive 1522 bytes. 
AT91C_EMAC_EAE            EQU (0x1:SHL:9) ;- (EMAC) External address match enable. 
AT91C_EMAC_CLK            EQU (0x3:SHL:10) ;- (EMAC) 
AT91C_EMAC_CLK_HCLK_8     EQU (0x0:SHL:10) ;- (EMAC) HCLK divided by 8
AT91C_EMAC_CLK_HCLK_16    EQU (0x1:SHL:10) ;- (EMAC) HCLK divided by 16
AT91C_EMAC_CLK_HCLK_32    EQU (0x2:SHL:10) ;- (EMAC) HCLK divided by 32
AT91C_EMAC_CLK_HCLK_64    EQU (0x3:SHL:10) ;- (EMAC) HCLK divided by 64
AT91C_EMAC_RTY            EQU (0x1:SHL:12) ;- (EMAC) 
AT91C_EMAC_RMII           EQU (0x1:SHL:13) ;- (EMAC) 
;- -------- EMAC_SR : (EMAC Offset: 0x8) Network Status Register -------- 
AT91C_EMAC_MDIO           EQU (0x1:SHL:1) ;- (EMAC) 
AT91C_EMAC_IDLE           EQU (0x1:SHL:2) ;- (EMAC) 
;- -------- EMAC_TCR : (EMAC Offset: 0x10) Transmit Control Register -------- 
AT91C_EMAC_LEN            EQU (0x7FF:SHL:0) ;- (EMAC) 
AT91C_EMAC_NCRC           EQU (0x1:SHL:15) ;- (EMAC) 
;- -------- EMAC_TSR : (EMAC Offset: 0x14) Transmit Control Register -------- 
AT91C_EMAC_OVR            EQU (0x1:SHL:0) ;- (EMAC) 
AT91C_EMAC_COL            EQU (0x1:SHL:1) ;- (EMAC) 
AT91C_EMAC_RLE            EQU (0x1:SHL:2) ;- (EMAC) 
AT91C_EMAC_TXIDLE         EQU (0x1:SHL:3) ;- (EMAC) 
AT91C_EMAC_BNQ            EQU (0x1:SHL:4) ;- (EMAC) 
AT91C_EMAC_COMP           EQU (0x1:SHL:5) ;- (EMAC) 
AT91C_EMAC_UND            EQU (0x1:SHL:6) ;- (EMAC) 
;- -------- EMAC_RSR : (EMAC Offset: 0x20) Receive Status Register -------- 
AT91C_EMAC_BNA            EQU (0x1:SHL:0) ;- (EMAC) 
AT91C_EMAC_REC            EQU (0x1:SHL:1) ;- (EMAC) 
;- -------- EMAC_ISR : (EMAC Offset: 0x24) Interrupt Status Register -------- 
AT91C_EMAC_DONE           EQU (0x1:SHL:0) ;- (EMAC) 
AT91C_EMAC_RCOM           EQU (0x1:SHL:1) ;- (EMAC) 
AT91C_EMAC_RBNA           EQU (0x1:SHL:2) ;- (EMAC) 
AT91C_EMAC_TOVR           EQU (0x1:SHL:3) ;- (EMAC) 
AT91C_EMAC_TUND           EQU (0x1:SHL:4) ;- (EMAC) 
AT91C_EMAC_RTRY           EQU (0x1:SHL:5) ;- (EMAC) 
AT91C_EMAC_TBRE           EQU (0x1:SHL:6) ;- (EMAC) 
AT91C_EMAC_TCOM           EQU (0x1:SHL:7) ;- (EMAC) 
AT91C_EMAC_TIDLE          EQU (0x1:SHL:8) ;- (EMAC) 
AT91C_EMAC_LINK           EQU (0x1:SHL:9) ;- (EMAC) 
AT91C_EMAC_ROVR           EQU (0x1:SHL:10) ;- (EMAC) 
AT91C_EMAC_HRESP          EQU (0x1:SHL:11) ;- (EMAC) 
;- -------- EMAC_IER : (EMAC Offset: 0x28) Interrupt Enable Register -------- 
;- -------- EMAC_IDR : (EMAC Offset: 0x2c) Interrupt Disable Register -------- 
;- -------- EMAC_IMR : (EMAC Offset: 0x30) Interrupt Mask Register -------- 
;- -------- EMAC_MAN : (EMAC Offset: 0x34) PHY Maintenance Register -------- 
AT91C_EMAC_DATA           EQU (0xFFFF:SHL:0) ;- (EMAC) 
AT91C_EMAC_CODE           EQU (0x3:SHL:16) ;- (EMAC) 
AT91C_EMAC_REGA           EQU (0x1F:SHL:18) ;- (EMAC) 
AT91C_EMAC_PHYA           EQU (0x1F:SHL:23) ;- (EMAC) 
AT91C_EMAC_RW             EQU (0x3:SHL:28) ;- (EMAC) 
AT91C_EMAC_HIGH           EQU (0x1:SHL:30) ;- (EMAC) 
AT91C_EMAC_LOW            EQU (0x1:SHL:31) ;- (EMAC) 

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR External Bus Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_EBI
EBI_CSA         #  4 ;- Chip Select Assignment Register
EBI_CFGR        #  4 ;- Configuration Register
;- -------- EBI_CSA : (EBI Offset: 0x0) Chip Select Assignment Register -------- 
AT91C_EBI_CS0A            EQU (0x1:SHL:0) ;- (EBI) Chip Select 0 Assignment
AT91C_EBI_CS0A_SMC        EQU (0x0) ;- (EBI) Chip Select 0 is assigned to the Static Memory Controller.
AT91C_EBI_CS0A_BFC        EQU (0x1) ;- (EBI) Chip Select 0 is assigned to the Burst Flash Controller.
AT91C_EBI_CS1A            EQU (0x1:SHL:1) ;- (EBI) Chip Select 1 Assignment
AT91C_EBI_CS1A_SMC        EQU (0x0:SHL:1) ;- (EBI) Chip Select 1 is assigned to the Static Memory Controller.
AT91C_EBI_CS1A_SDRAMC     EQU (0x1:SHL:1) ;- (EBI) Chip Select 1 is assigned to the SDRAM Controller.
AT91C_EBI_CS3A            EQU (0x1:SHL:3) ;- (EBI) Chip Select 3 Assignment
AT91C_EBI_CS3A_SMC        EQU (0x0:SHL:3) ;- (EBI) Chip Select 3 is only assigned to the Static Memory Controller and NCS3 behaves as defined by the SMC2.
AT91C_EBI_CS3A_SMC_SmartMedia EQU (0x1:SHL:3) ;- (EBI) Chip Select 3 is assigned to the Static Memory Controller and the SmartMedia Logic is activated.
AT91C_EBI_CS4A            EQU (0x1:SHL:4) ;- (EBI) Chip Select 4 Assignment
AT91C_EBI_CS4A_SMC        EQU (0x0:SHL:4) ;- (EBI) Chip Select 4 is assigned to the Static Memory Controller and NCS4,NCS5 and NCS6 behave as defined by the SMC2.
AT91C_EBI_CS4A_SMC_CompactFlash EQU (0x1:SHL:4) ;- (EBI) Chip Select 4 is assigned to the Static Memory Controller and the CompactFlash Logic is activated.
;- -------- EBI_CFGR : (EBI Offset: 0x4) Configuration Register -------- 
AT91C_EBI_DBPUC           EQU (0x1:SHL:0) ;- (EBI) Data Bus Pull-Up Configuration
AT91C_EBI_EBSEN           EQU (0x1:SHL:1) ;- (EBI) Bus Sharing Enable

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Static Memory Controller 2 Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_SMC2
SMC2_CSR        # 32 ;- SMC2 Chip Select Register
;- -------- SMC2_CSR : (SMC2 Offset: 0x0) SMC2 Chip Select Register -------- 
AT91C_SMC2_NWS            EQU (0x7F:SHL:0) ;- (SMC2) Number of Wait States
AT91C_SMC2_WSEN           EQU (0x1:SHL:7) ;- (SMC2) Wait State Enable
AT91C_SMC2_TDF            EQU (0xF:SHL:8) ;- (SMC2) Data Float Time
AT91C_SMC2_BAT            EQU (0x1:SHL:12) ;- (SMC2) Byte Access Type
AT91C_SMC2_DBW            EQU (0x1:SHL:13) ;- (SMC2) Data Bus Width
AT91C_SMC2_DBW_16         EQU (0x1:SHL:13) ;- (SMC2) 16-bit.
AT91C_SMC2_DBW_8          EQU (0x2:SHL:13) ;- (SMC2) 8-bit.
AT91C_SMC2_DRP            EQU (0x1:SHL:15) ;- (SMC2) Data Read Protocol
AT91C_SMC2_ACSS           EQU (0x3:SHL:16) ;- (SMC2) Address to Chip Select Setup
AT91C_SMC2_ACSS_STANDARD  EQU (0x0:SHL:16) ;- (SMC2) Standard, asserted at the beginning of the access and deasserted at the end.
AT91C_SMC2_ACSS_1_CYCLE   EQU (0x1:SHL:16) ;- (SMC2) One cycle less at the beginning and the end of the access.
AT91C_SMC2_ACSS_2_CYCLES  EQU (0x2:SHL:16) ;- (SMC2) Two cycles less at the beginning and the end of the access.
AT91C_SMC2_ACSS_3_CYCLES  EQU (0x3:SHL:16) ;- (SMC2) Three cycles less at the beginning and the end of the access.
AT91C_SMC2_RWSETUP        EQU (0x7:SHL:24) ;- (SMC2) Read and Write Signal Setup Time
AT91C_SMC2_RWHOLD         EQU (0x7:SHL:29) ;- (SMC2) Read and Write Signal Hold Time

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR SDRAM Controller Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_SDRC
SDRC_MR         #  4 ;- SDRAM Controller Mode Register
SDRC_TR         #  4 ;- SDRAM Controller Refresh Timer Register
SDRC_CR         #  4 ;- SDRAM Controller Configuration Register
SDRC_SRR        #  4 ;- SDRAM Controller Self Refresh Register
SDRC_LPR        #  4 ;- SDRAM Controller Low Power Register
SDRC_IER        #  4 ;- SDRAM Controller Interrupt Enable Register
SDRC_IDR        #  4 ;- SDRAM Controller Interrupt Disable Register
SDRC_IMR        #  4 ;- SDRAM Controller Interrupt Mask Register
SDRC_ISR        #  4 ;- SDRAM Controller Interrupt Mask Register
;- -------- SDRC_MR : (SDRC Offset: 0x0) SDRAM Controller Mode Register -------- 
AT91C_SDRC_MODE           EQU (0xF:SHL:0) ;- (SDRC) Mode
AT91C_SDRC_MODE_NORMAL_CMD EQU (0x0) ;- (SDRC) Normal Mode
AT91C_SDRC_MODE_NOP_CMD   EQU (0x1) ;- (SDRC) NOP Command
AT91C_SDRC_MODE_PRCGALL_CMD EQU (0x2) ;- (SDRC) All Banks Precharge Command
AT91C_SDRC_MODE_LMR_CMD   EQU (0x3) ;- (SDRC) Load Mode Register Command
AT91C_SDRC_MODE_RFSH_CMD  EQU (0x4) ;- (SDRC) Refresh Command
AT91C_SDRC_DBW            EQU (0x1:SHL:4) ;- (SDRC) Data Bus Width
AT91C_SDRC_DBW_32_BITS    EQU (0x0:SHL:4) ;- (SDRC) 32 Bits datas bus
AT91C_SDRC_DBW_16_BITS    EQU (0x1:SHL:4) ;- (SDRC) 16 Bits datas bus
;- -------- SDRC_TR : (SDRC Offset: 0x4) SDRC Refresh Timer Register -------- 
AT91C_SDRC_COUNT          EQU (0xFFF:SHL:0) ;- (SDRC) Refresh Counter
;- -------- SDRC_CR : (SDRC Offset: 0x8) SDRAM Configuration Register -------- 
AT91C_SDRC_NC             EQU (0x3:SHL:0) ;- (SDRC) Number of Column Bits
AT91C_SDRC_NC_8           EQU (0x0) ;- (SDRC) 8 Bits
AT91C_SDRC_NC_9           EQU (0x1) ;- (SDRC) 9 Bits
AT91C_SDRC_NC_10          EQU (0x2) ;- (SDRC) 10 Bits
AT91C_SDRC_NC_11          EQU (0x3) ;- (SDRC) 11 Bits
AT91C_SDRC_NR             EQU (0x3:SHL:2) ;- (SDRC) Number of Row Bits
AT91C_SDRC_NR_11          EQU (0x0:SHL:2) ;- (SDRC) 11 Bits
AT91C_SDRC_NR_12          EQU (0x1:SHL:2) ;- (SDRC) 12 Bits
AT91C_SDRC_NR_13          EQU (0x2:SHL:2) ;- (SDRC) 13 Bits
AT91C_SDRC_NB             EQU (0x1:SHL:4) ;- (SDRC) Number of Banks
AT91C_SDRC_NB_2_BANKS     EQU (0x0:SHL:4) ;- (SDRC) 2 banks
AT91C_SDRC_NB_4_BANKS     EQU (0x1:SHL:4) ;- (SDRC) 4 banks
AT91C_SDRC_CAS            EQU (0x3:SHL:5) ;- (SDRC) CAS Latency
AT91C_SDRC_CAS_2          EQU (0x2:SHL:5) ;- (SDRC) 2 cycles
AT91C_SDRC_TWR            EQU (0xF:SHL:7) ;- (SDRC) Number of Write Recovery Time Cycles
AT91C_SDRC_TRC            EQU (0xF:SHL:11) ;- (SDRC) Number of RAS Cycle Time Cycles
AT91C_SDRC_TRP            EQU (0xF:SHL:15) ;- (SDRC) Number of RAS Precharge Time Cycles
AT91C_SDRC_TRCD           EQU (0xF:SHL:19) ;- (SDRC) Number of RAS to CAS Delay Cycles
AT91C_SDRC_TRAS           EQU (0xF:SHL:23) ;- (SDRC) Number of RAS Active Time Cycles
AT91C_SDRC_TXSR           EQU (0xF:SHL:27) ;- (SDRC) Number of Command Recovery Time Cycles
;- -------- SDRC_SRR : (SDRC Offset: 0xc) SDRAM Controller Self-refresh Register -------- 
AT91C_SDRC_SRCB           EQU (0x1:SHL:0) ;- (SDRC) Self-refresh Command Bit
;- -------- SDRC_LPR : (SDRC Offset: 0x10) SDRAM Controller Low-power Register -------- 
AT91C_SDRC_LPCB           EQU (0x1:SHL:0) ;- (SDRC) Low-power Command Bit
;- -------- SDRC_IER : (SDRC Offset: 0x14) SDRAM Controller Interrupt Enable Register -------- 
AT91C_SDRC_RES            EQU (0x1:SHL:0) ;- (SDRC) Refresh Error Status
;- -------- SDRC_IDR : (SDRC Offset: 0x18) SDRAM Controller Interrupt Disable Register -------- 
;- -------- SDRC_IMR : (SDRC Offset: 0x1c) SDRAM Controller Interrupt Mask Register -------- 
;- -------- SDRC_ISR : (SDRC Offset: 0x20) SDRAM Controller Interrupt Status Register -------- 

;- *****************************************************************************
;-              SOFTWARE API DEFINITION  FOR Burst Flash Controller Interface
;- *****************************************************************************
                ^ 0 ;- AT91S_BFC
BFC_MR          #  4 ;- BFC Mode Register
;- -------- BFC_MR : (BFC Offset: 0x0) BFC Mode Register -------- 
AT91C_BFC_BFCOM           EQU (0x3:SHL:0) ;- (BFC) Burst Flash Controller Operating Mode
AT91C_BFC_BFCOM_DISABLED  EQU (0x0) ;- (BFC) NPCS0 is driven by the SMC or remains high.
AT91C_BFC_BFCOM_ASYNC     EQU (0x1) ;- (BFC) Asynchronous
AT91C_BFC_BFCOM_BURST_READ EQU (0x2) ;- (BFC) Burst Read
AT91C_BFC_BFCC            EQU (0x3:SHL:2) ;- (BFC) Burst Flash Controller Operating Mode
AT91C_BFC_BFCC_MCK        EQU (0x1:SHL:2) ;- (BFC) Master Clock.
AT91C_BFC_BFCC_MCK_DIV_2  EQU (0x2:SHL:2) ;- (BFC) Master Clock divided by 2.
AT91C_BFC_BFCC_MCK_DIV_4  EQU (0x3:SHL:2) ;- (BFC) Master Clock divided by 4.
AT91C_BFC_AVL             EQU (0xF:SHL:4) ;- (BFC) Address Valid Latency
AT91C_BFC_PAGES           EQU (0x7:SHL:8) ;- (BFC) Page Size
AT91C_BFC_PAGES_NO_PAGE   EQU (0x0:SHL:8) ;- (BFC) No page handling.
AT91C_BFC_PAGES_16        EQU (0x1:SHL:8) ;- (BFC) 16 bytes page size.
AT91C_BFC_PAGES_32        EQU (0x2:SHL:8) ;- (BFC) 32 bytes page size.
AT91C_BFC_PAGES_64        EQU (0x3:SHL:8) ;- (BFC) 64 bytes page size.
AT91C_BFC_PAGES_128       EQU (0x4:SHL:8) ;- (BFC) 128 bytes page size.
AT91C_BFC_PAGES_256       EQU (0x5:SHL:8) ;- (BFC) 256 bytes page size.
AT91C_BFC_PAGES_512       EQU (0x6:SHL:8) ;- (BFC) 512 bytes page size.
AT91C_BFC_PAGES_1024      EQU (0x7:SHL:8) ;- (BFC) 1024 bytes page size.
AT91C_BFC_OEL             EQU (0x3:SHL:12) ;- (BFC) Output Enable Latency
AT91C_BFC_BAAEN           EQU (0x1:SHL:16) ;- (BFC) Burst Address Advance Enable
AT91C_BFC_BFOEH           EQU (0x1:SHL:17) ;- (BFC) Burst Flash Output Enable Handling
AT91C_BFC_MUXEN           EQU (0x1:SHL:18) ;- (BFC) Multiplexed Bus Enable
AT91C_BFC_RDYEN           EQU (0x1:SHL:19) ;- (BFC) Ready Enable Mode

;- *****************************************************************************
;-               REGISTER ADDRESS DEFINITION FOR AT91RM9200
;- *****************************************************************************
;- ========== Register definition for SYS peripheral ========== 
;- ========== Register definition for MC peripheral ========== 
AT91C_MC_PUER             EQU (0xFFFFFF54) ;- (MC) MC Protection Unit Enable Register
AT91C_MC_ASR              EQU (0xFFFFFF04) ;- (MC) MC Abort Status Register
AT91C_MC_PUP              EQU (0xFFFFFF50) ;- (MC) MC Protection Unit Peripherals
AT91C_MC_PUIA             EQU (0xFFFFFF10) ;- (MC) MC Protection Unit Area
AT91C_MC_AASR             EQU (0xFFFFFF08) ;- (MC) MC Abort Address Status Register
AT91C_MC_RCR              EQU (0xFFFFFF00) ;- (MC) MC Remap Control Register
;- ========== Register definition for RTC peripheral ========== 
AT91C_RTC_IMR             EQU (0xFFFFFE28) ;- (RTC) Interrupt Mask Register
AT91C_RTC_IER             EQU (0xFFFFFE20) ;- (RTC) Interrupt Enable Register
AT91C_RTC_SR              EQU (0xFFFFFE18) ;- (RTC) Status Register
AT91C_RTC_TIMALR          EQU (0xFFFFFE10) ;- (RTC) Time Alarm Register
AT91C_RTC_TIMR            EQU (0xFFFFFE08) ;- (RTC) Time Register
AT91C_RTC_CR              EQU (0xFFFFFE00) ;- (RTC) Control Register
AT91C_RTC_VER             EQU (0xFFFFFE2C) ;- (RTC) Valid Entry Register
AT91C_RTC_IDR             EQU (0xFFFFFE24) ;- (RTC) Interrupt Disable Register
AT91C_RTC_SCCR            EQU (0xFFFFFE1C) ;- (RTC) Status Clear Command Register
AT91C_RTC_CALALR          EQU (0xFFFFFE14) ;- (RTC) Calendar Alarm Register
AT91C_RTC_CALR            EQU (0xFFFFFE0C) ;- (RTC) Calendar Register
AT91C_RTC_MR              EQU (0xFFFFFE04) ;- (RTC) Mode Register
;- ========== Register definition for ST peripheral ========== 
AT91C_ST_CRTR             EQU (0xFFFFFD24) ;- (ST) Current Real-time Register
AT91C_ST_IMR              EQU (0xFFFFFD1C) ;- (ST) Interrupt Mask Register
AT91C_ST_IER              EQU (0xFFFFFD14) ;- (ST) Interrupt Enable Register
AT91C_ST_RTMR             EQU (0xFFFFFD0C) ;- (ST) Real-time Mode Register
AT91C_ST_PIMR             EQU (0xFFFFFD04) ;- (ST) Period Interval Mode Register
AT91C_ST_RTAR             EQU (0xFFFFFD20) ;- (ST) Real-time Alarm Register
AT91C_ST_IDR              EQU (0xFFFFFD18) ;- (ST) Interrupt Disable Register
AT91C_ST_SR               EQU (0xFFFFFD10) ;- (ST) Status Register
AT91C_ST_WDMR             EQU (0xFFFFFD08) ;- (ST) Watchdog Mode Register
AT91C_ST_CR               EQU (0xFFFFFD00) ;- (ST) Control Register
;- ========== Register definition for PMC peripheral ========== 
AT91C_PMC_SCSR            EQU (0xFFFFFC08) ;- (PMC) System Clock Status Register
AT91C_PMC_SCER            EQU (0xFFFFFC00) ;- (PMC) System Clock Enable Register
AT91C_PMC_IMR             EQU (0xFFFFFC6C) ;- (PMC) Interrupt Mask Register
AT91C_PMC_IDR             EQU (0xFFFFFC64) ;- (PMC) Interrupt Disable Register
AT91C_PMC_PCDR            EQU (0xFFFFFC14) ;- (PMC) Peripheral Clock Disable Register
AT91C_PMC_SCDR            EQU (0xFFFFFC04) ;- (PMC) System Clock Disable Register
AT91C_PMC_SR              EQU (0xFFFFFC68) ;- (PMC) Status Register
AT91C_PMC_IER             EQU (0xFFFFFC60) ;- (PMC) Interrupt Enable Register
AT91C_PMC_MCKR            EQU (0xFFFFFC30) ;- (PMC) Master Clock Register
AT91C_PMC_PCER            EQU (0xFFFFFC10) ;- (PMC) Peripheral Clock Enable Register
AT91C_PMC_PCSR            EQU (0xFFFFFC18) ;- (PMC) Peripheral Clock Status Register
AT91C_PMC_PCKR            EQU (0xFFFFFC40) ;- (PMC) Programmable Clock Register
;- ========== Register definition for CKGR peripheral ========== 
AT91C_CKGR_PLLBR          EQU (0xFFFFFC2C) ;- (CKGR) PLL B Register
AT91C_CKGR_MCFR           EQU (0xFFFFFC24) ;- (CKGR) Main Clock  Frequency Register
AT91C_CKGR_PLLAR          EQU (0xFFFFFC28) ;- (CKGR) PLL A Register
AT91C_CKGR_MOR            EQU (0xFFFFFC20) ;- (CKGR) Main Oscillator Register
;- ========== Register definition for PIOD peripheral ========== 
AT91C_PIOD_PDSR           EQU (0xFFFFFA3C) ;- (PIOD) Pin Data Status Register
AT91C_PIOD_CODR           EQU (0xFFFFFA34) ;- (PIOD) Clear Output Data Register
AT91C_PIOD_OWER           EQU (0xFFFFFAA0) ;- (PIOD) Output Write Enable Register
AT91C_PIOD_MDER           EQU (0xFFFFFA50) ;- (PIOD) Multi-driver Enable Register
AT91C_PIOD_IMR            EQU (0xFFFFFA48) ;- (PIOD) Interrupt Mask Register
AT91C_PIOD_IER            EQU (0xFFFFFA40) ;- (PIOD) Interrupt Enable Register
AT91C_PIOD_ODSR           EQU (0xFFFFFA38) ;- (PIOD) Output Data Status Register
AT91C_PIOD_SODR           EQU (0xFFFFFA30) ;- (PIOD) Set Output Data Register
AT91C_PIOD_PER            EQU (0xFFFFFA00) ;- (PIOD) PIO Enable Register
AT91C_PIOD_OWDR           EQU (0xFFFFFAA4) ;- (PIOD) Output Write Disable Register
AT91C_PIOD_PPUER          EQU (0xFFFFFA64) ;- (PIOD) Pull-up Enable Register
AT91C_PIOD_MDDR           EQU (0xFFFFFA54) ;- (PIOD) Multi-driver Disable Register
AT91C_PIOD_ISR            EQU (0xFFFFFA4C) ;- (PIOD) Interrupt Status Register
AT91C_PIOD_IDR            EQU (0xFFFFFA44) ;- (PIOD) Interrupt Disable Register
AT91C_PIOD_PDR            EQU (0xFFFFFA04) ;- (PIOD) PIO Disable Register
AT91C_PIOD_ODR            EQU (0xFFFFFA14) ;- (PIOD) Output Disable Registerr
AT91C_PIOD_OWSR           EQU (0xFFFFFAA8) ;- (PIOD) Output Write Status Register
AT91C_PIOD_ABSR           EQU (0xFFFFFA78) ;- (PIOD) AB Select Status Register
AT91C_PIOD_ASR            EQU (0xFFFFFA70) ;- (PIOD) Select A Register
AT91C_PIOD_PPUSR          EQU (0xFFFFFA68) ;- (PIOD) Pad Pull-up Status Register
AT91C_PIOD_PPUDR          EQU (0xFFFFFA60) ;- (PIOD) Pull-up Disable Register
AT91C_PIOD_MDSR           EQU (0xFFFFFA58) ;- (PIOD) Multi-driver Status Register
AT91C_PIOD_PSR            EQU (0xFFFFFA08) ;- (PIOD) PIO Status Register
AT91C_PIOD_OER            EQU (0xFFFFFA10) ;- (PIOD) Output Enable Register
AT91C_PIOD_OSR            EQU (0xFFFFFA18) ;- (PIOD) Output Status Register
AT91C_PIOD_IFER           EQU (0xFFFFFA20) ;- (PIOD) Input Filter Enable Register
AT91C_PIOD_BSR            EQU (0xFFFFFA74) ;- (PIOD) Select B Register
AT91C_PIOD_IFDR           EQU (0xFFFFFA24) ;- (PIOD) Input Filter Disable Register
AT91C_PIOD_IFSR           EQU (0xFFFFFA28) ;- (PIOD) Input Filter Status Register
;- ========== Register definition for PIOC peripheral ========== 
AT91C_PIOC_IFDR           EQU (0xFFFFF824) ;- (PIOC) Input Filter Disable Register
AT91C_PIOC_ODR            EQU (0xFFFFF814) ;- (PIOC) Output Disable Registerr
AT91C_PIOC_ABSR           EQU (0xFFFFF878) ;- (PIOC) AB Select Status Register
AT91C_PIOC_SODR           EQU (0xFFFFF830) ;- (PIOC) Set Output Data Register
AT91C_PIOC_IFSR           EQU (0xFFFFF828) ;- (PIOC) Input Filter Status Register
AT91C_PIOC_CODR           EQU (0xFFFFF834) ;- (PIOC) Clear Output Data Register
AT91C_PIOC_ODSR           EQU (0xFFFFF838) ;- (PIOC) Output Data Status Register
AT91C_PIOC_IER            EQU (0xFFFFF840) ;- (PIOC) Interrupt Enable Register
AT91C_PIOC_IMR            EQU (0xFFFFF848) ;- (PIOC) Interrupt Mask Register
AT91C_PIOC_OWDR           EQU (0xFFFFF8A4) ;- (PIOC) Output Write Disable Register
AT91C_PIOC_MDDR           EQU (0xFFFFF854) ;- (PIOC) Multi-driver Disable Register
AT91C_PIOC_PDSR           EQU (0xFFFFF83C) ;- (PIOC) Pin Data Status Register
AT91C_PIOC_IDR            EQU (0xFFFFF844) ;- (PIOC) Interrupt Disable Register
AT91C_PIOC_ISR            EQU (0xFFFFF84C) ;- (PIOC) Interrupt Status Register
AT91C_PIOC_PDR            EQU (0xFFFFF804) ;- (PIOC) PIO Disable Register
AT91C_PIOC_OWSR           EQU (0xFFFFF8A8) ;- (PIOC) Output Write Status Register
AT91C_PIOC_OWER           EQU (0xFFFFF8A0) ;- (PIOC) Output Write Enable Register
AT91C_PIOC_ASR            EQU (0xFFFFF870) ;- (PIOC) Select A Register
AT91C_PIOC_PPUSR          EQU (0xFFFFF868) ;- (PIOC) Pad Pull-up Status Register
AT91C_PIOC_PPUDR          EQU (0xFFFFF860) ;- (PIOC) Pull-up Disable Register
AT91C_PIOC_MDSR           EQU (0xFFFFF858) ;- (PIOC) Multi-driver Status Register
AT91C_PIOC_MDER           EQU (0xFFFFF850) ;- (PIOC) Multi-driver Enable Register
AT91C_PIOC_IFER           EQU (0xFFFFF820) ;- (PIOC) Input Filter Enable Register
AT91C_PIOC_OSR            EQU (0xFFFFF818) ;- (PIOC) Output Status Register
AT91C_PIOC_OER            EQU (0xFFFFF810) ;- (PIOC) Output Enable Register
AT91C_PIOC_PSR            EQU (0xFFFFF808) ;- (PIOC) PIO Status Register
AT91C_PIOC_PER            EQU (0xFFFFF800) ;- (PIOC) PIO Enable Register
AT91C_PIOC_BSR            EQU (0xFFFFF874) ;- (PIOC) Select B Register
AT91C_PIOC_PPUER          EQU (0xFFFFF864) ;- (PIOC) Pull-up Enable Register
;- ========== Register definition for PIOB peripheral ========== 
AT91C_PIOB_OWSR           EQU (0xFFFFF6A8) ;- (PIOB) Output Write Status Register
AT91C_PIOB_PPUSR          EQU (0xFFFFF668) ;- (PIOB) Pad Pull-up Status Register
AT91C_PIOB_PPUDR          EQU (0xFFFFF660) ;- (PIOB) Pull-up Disable Register
AT91C_PIOB_MDSR           EQU (0xFFFFF658) ;- (PIOB) Multi-driver Status Register
AT91C_PIOB_MDER           EQU (0xFFFFF650) ;- (PIOB) Multi-driver Enable Register
AT91C_PIOB_IMR            EQU (0xFFFFF648) ;- (PIOB) Interrupt Mask Register
AT91C_PIOB_OSR            EQU (0xFFFFF618) ;- (PIOB) Output Status Register
AT91C_PIOB_OER            EQU (0xFFFFF610) ;- (PIOB) Output Enable Register
AT91C_PIOB_PSR            EQU (0xFFFFF608) ;- (PIOB) PIO Status Register
AT91C_PIOB_PER            EQU (0xFFFFF600) ;- (PIOB) PIO Enable Register
AT91C_PIOB_BSR            EQU (0xFFFFF674) ;- (PIOB) Select B Register
AT91C_PIOB_PPUER          EQU (0xFFFFF664) ;- (PIOB) Pull-up Enable Register
AT91C_PIOB_IFDR           EQU (0xFFFFF624) ;- (PIOB) Input Filter Disable Register
AT91C_PIOB_ODR            EQU (0xFFFFF614) ;- (PIOB) Output Disable Registerr
AT91C_PIOB_ABSR           EQU (0xFFFFF678) ;- (PIOB) AB Select Status Register
AT91C_PIOB_ASR            EQU (0xFFFFF670) ;- (PIOB) Select A Register
AT91C_PIOB_IFER           EQU (0xFFFFF620) ;- (PIOB) Input Filter Enable Register
AT91C_PIOB_IFSR           EQU (0xFFFFF628) ;- (PIOB) Input Filter Status Register
AT91C_PIOB_SODR           EQU (0xFFFFF630) ;- (PIOB) Set Output Data Register
AT91C_PIOB_ODSR           EQU (0xFFFFF638) ;- (PIOB) Output Data Status Register
AT91C_PIOB_CODR           EQU (0xFFFFF634) ;- (PIOB) Clear Output Data Register
AT91C_PIOB_PDSR           EQU (0xFFFFF63C) ;- (PIOB) Pin Data Status Register
AT91C_PIOB_OWER           EQU (0xFFFFF6A0) ;- (PIOB) Output Write Enable Register
AT91C_PIOB_IER            EQU (0xFFFFF640) ;- (PIOB) Interrupt Enable Register
AT91C_PIOB_OWDR           EQU (0xFFFFF6A4) ;- (PIOB) Output Write Disable Register
AT91C_PIOB_MDDR           EQU (0xFFFFF654) ;- (PIOB) Multi-driver Disable Register
AT91C_PIOB_ISR            EQU (0xFFFFF64C) ;- (PIOB) Interrupt Status Register
AT91C_PIOB_IDR            EQU (0xFFFFF644) ;- (PIOB) Interrupt Disable Register
AT91C_PIOB_PDR            EQU (0xFFFFF604) ;- (PIOB) PIO Disable Register
;- ========== Register definition for PIOA peripheral ========== 
AT91C_PIOA_IMR            EQU (0xFFFFF448) ;- (PIOA) Interrupt Mask Register
AT91C_PIOA_IER            EQU (0xFFFFF440) ;- (PIOA) Interrupt Enable Register
AT91C_PIOA_OWDR           EQU (0xFFFFF4A4) ;- (PIOA) Output Write Disable Register
AT91C_PIOA_ISR            EQU (0xFFFFF44C) ;- (PIOA) Interrupt Status Register
AT91C_PIOA_PPUDR          EQU (0xFFFFF460) ;- (PIOA) Pull-up Disable Register
AT91C_PIOA_MDSR           EQU (0xFFFFF458) ;- (PIOA) Multi-driver Status Register
AT91C_PIOA_MDER           EQU (0xFFFFF450) ;- (PIOA) Multi-driver Enable Register
AT91C_PIOA_PER            EQU (0xFFFFF400) ;- (PIOA) PIO Enable Register
AT91C_PIOA_PSR            EQU (0xFFFFF408) ;- (PIOA) PIO Status Register
AT91C_PIOA_OER            EQU (0xFFFFF410) ;- (PIOA) Output Enable Register
AT91C_PIOA_BSR            EQU (0xFFFFF474) ;- (PIOA) Select B Register
AT91C_PIOA_PPUER          EQU (0xFFFFF464) ;- (PIOA) Pull-up Enable Register
AT91C_PIOA_MDDR           EQU (0xFFFFF454) ;- (PIOA) Multi-driver Disable Register
AT91C_PIOA_PDR            EQU (0xFFFFF404) ;- (PIOA) PIO Disable Register
AT91C_PIOA_ODR            EQU (0xFFFFF414) ;- (PIOA) Output Disable Registerr
AT91C_PIOA_IFDR           EQU (0xFFFFF424) ;- (PIOA) Input Filter Disable Register
AT91C_PIOA_ABSR           EQU (0xFFFFF478) ;- (PIOA) AB Select Status Register
AT91C_PIOA_ASR            EQU (0xFFFFF470) ;- (PIOA) Select A Register
AT91C_PIOA_PPUSR          EQU (0xFFFFF468) ;- (PIOA) Pad Pull-up Status Register
AT91C_PIOA_ODSR           EQU (0xFFFFF438) ;- (PIOA) Output Data Status Register
AT91C_PIOA_SODR           EQU (0xFFFFF430) ;- (PIOA) Set Output Data Register
AT91C_PIOA_IFSR           EQU (0xFFFFF428) ;- (PIOA) Input Filter Status Register
AT91C_PIOA_IFER           EQU (0xFFFFF420) ;- (PIOA) Input Filter Enable Register
AT91C_PIOA_OSR            EQU (0xFFFFF418) ;- (PIOA) Output Status Register
AT91C_PIOA_IDR            EQU (0xFFFFF444) ;- (PIOA) Interrupt Disable Register
AT91C_PIOA_PDSR           EQU (0xFFFFF43C) ;- (PIOA) Pin Data Status Register
AT91C_PIOA_CODR           EQU (0xFFFFF434) ;- (PIOA) Clear Output Data Register
AT91C_PIOA_OWSR           EQU (0xFFFFF4A8) ;- (PIOA) Output Write Status Register
AT91C_PIOA_OWER           EQU (0xFFFFF4A0) ;- (PIOA) Output Write Enable Register
;- ========== Register definition for DBGU peripheral ========== 
AT91C_DBGU_C2R            EQU (0xFFFFF244) ;- (DBGU) Chip ID2 Register
AT91C_DBGU_THR            EQU (0xFFFFF21C) ;- (DBGU) Transmitter Holding Register
AT91C_DBGU_CSR            EQU (0xFFFFF214) ;- (DBGU) Channel Status Register
AT91C_DBGU_IDR            EQU (0xFFFFF20C) ;- (DBGU) Interrupt Disable Register
AT91C_DBGU_MR             EQU (0xFFFFF204) ;- (DBGU) Mode Register
AT91C_DBGU_FNTR           EQU (0xFFFFF248) ;- (DBGU) Force NTRST Register
AT91C_DBGU_C1R            EQU (0xFFFFF240) ;- (DBGU) Chip ID1 Register
AT91C_DBGU_BRGR           EQU (0xFFFFF220) ;- (DBGU) Baud Rate Generator Register
AT91C_DBGU_RHR            EQU (0xFFFFF218) ;- (DBGU) Receiver Holding Register
AT91C_DBGU_IMR            EQU (0xFFFFF210) ;- (DBGU) Interrupt Mask Register
AT91C_DBGU_IER            EQU (0xFFFFF208) ;- (DBGU) Interrupt Enable Register
AT91C_DBGU_CR             EQU (0xFFFFF200) ;- (DBGU) Control Register
;- ========== Register definition for PDC_DBGU peripheral ========== 
AT91C_DBGU_TNCR           EQU (0xFFFFF31C) ;- (PDC_DBGU) Transmit Next Counter Register
AT91C_DBGU_RNCR           EQU (0xFFFFF314) ;- (PDC_DBGU) Receive Next Counter Register
AT91C_DBGU_PTCR           EQU (0xFFFFF320) ;- (PDC_DBGU) PDC Transfer Control Register
AT91C_DBGU_PTSR           EQU (0xFFFFF324) ;- (PDC_DBGU) PDC Transfer Status Register
AT91C_DBGU_RCR            EQU (0xFFFFF304) ;- (PDC_DBGU) Receive Counter Register
AT91C_DBGU_TCR            EQU (0xFFFFF30C) ;- (PDC_DBGU) Transmit Counter Register
AT91C_DBGU_RPR            EQU (0xFFFFF300) ;- (PDC_DBGU) Receive Pointer Register
AT91C_DBGU_TPR            EQU (0xFFFFF308) ;- (PDC_DBGU) Transmit Pointer Register
AT91C_DBGU_RNPR           EQU (0xFFFFF310) ;- (PDC_DBGU) Receive Next Pointer Register
AT91C_DBGU_TNPR           EQU (0xFFFFF318) ;- (PDC_DBGU) Transmit Next Pointer Register
;- ========== Register definition for AIC peripheral ========== 
AT91C_AIC_ICCR            EQU (0xFFFFF128) ;- (AIC) Interrupt Clear Command Register
AT91C_AIC_IECR            EQU (0xFFFFF120) ;- (AIC) Interrupt Enable Command Register
AT91C_AIC_SMR             EQU (0xFFFFF000) ;- (AIC) Source Mode Register
AT91C_AIC_ISCR            EQU (0xFFFFF12C) ;- (AIC) Interrupt Set Command Register
AT91C_AIC_EOICR           EQU (0xFFFFF130) ;- (AIC) End of Interrupt Command Register
AT91C_AIC_DCR             EQU (0xFFFFF138) ;- (AIC) Debug Control Register (Protect)
AT91C_AIC_FFER            EQU (0xFFFFF140) ;- (AIC) Fast Forcing Enable Register
AT91C_AIC_SVR             EQU (0xFFFFF080) ;- (AIC) Source Vector Register
AT91C_AIC_SPU             EQU (0xFFFFF134) ;- (AIC) Spurious Vector Register
AT91C_AIC_FFDR            EQU (0xFFFFF144) ;- (AIC) Fast Forcing Disable Register
AT91C_AIC_FVR             EQU (0xFFFFF104) ;- (AIC) FIQ Vector Register
AT91C_AIC_FFSR            EQU (0xFFFFF148) ;- (AIC) Fast Forcing Status Register
AT91C_AIC_IMR             EQU (0xFFFFF110) ;- (AIC) Interrupt Mask Register
AT91C_AIC_ISR             EQU (0xFFFFF108) ;- (AIC) Interrupt Status Register
AT91C_AIC_IVR             EQU (0xFFFFF100) ;- (AIC) IRQ Vector Register
AT91C_AIC_IDCR            EQU (0xFFFFF124) ;- (AIC) Interrupt Disable Command Register
AT91C_AIC_CISR            EQU (0xFFFFF114) ;- (AIC) Core Interrupt Status Register
AT91C_AIC_IPR             EQU (0xFFFFF10C) ;- (AIC) Interrupt Pending Register
;- ========== Register definition for PDC_SPI peripheral ========== 
AT91C_SPI_PTCR            EQU (0xFFFE0120) ;- (PDC_SPI) PDC Transfer Control Register
AT91C_SPI_TNPR            EQU (0xFFFE0118) ;- (PDC_SPI) Transmit Next Pointer Register
AT91C_SPI_RNPR            EQU (0xFFFE0110) ;- (PDC_SPI) Receive Next Pointer Register
AT91C_SPI_TPR             EQU (0xFFFE0108) ;- (PDC_SPI) Transmit Pointer Register
AT91C_SPI_RPR             EQU (0xFFFE0100) ;- (PDC_SPI) Receive Pointer Register
AT91C_SPI_PTSR            EQU (0xFFFE0124) ;- (PDC_SPI) PDC Transfer Status Register
AT91C_SPI_TNCR            EQU (0xFFFE011C) ;- (PDC_SPI) Transmit Next Counter Register
AT91C_SPI_RNCR            EQU (0xFFFE0114) ;- (PDC_SPI) Receive Next Counter Register
AT91C_SPI_TCR             EQU (0xFFFE010C) ;- (PDC_SPI) Transmit Counter Register
AT91C_SPI_RCR             EQU (0xFFFE0104) ;- (PDC_SPI) Receive Counter Register
;- ========== Register definition for SPI peripheral ========== 
AT91C_SPI_CSR             EQU (0xFFFE0030) ;- (SPI) Chip Select Register
AT91C_SPI_IDR             EQU (0xFFFE0018) ;- (SPI) Interrupt Disable Register
AT91C_SPI_SR              EQU (0xFFFE0010) ;- (SPI) Status Register
AT91C_SPI_RDR             EQU (0xFFFE0008) ;- (SPI) Receive Data Register
AT91C_SPI_CR              EQU (0xFFFE0000) ;- (SPI) Control Register
AT91C_SPI_IMR             EQU (0xFFFE001C) ;- (SPI) Interrupt Mask Register
AT91C_SPI_IER             EQU (0xFFFE0014) ;- (SPI) Interrupt Enable Register
AT91C_SPI_TDR             EQU (0xFFFE000C) ;- (SPI) Transmit Data Register
AT91C_SPI_MR              EQU (0xFFFE0004) ;- (SPI) Mode Register
;- ========== Register definition for PDC_SSC2 peripheral ========== 
AT91C_SSC2_PTCR           EQU (0xFFFD8120) ;- (PDC_SSC2) PDC Transfer Control Register
AT91C_SSC2_TNPR           EQU (0xFFFD8118) ;- (PDC_SSC2) Transmit Next Pointer Register
AT91C_SSC2_RNPR           EQU (0xFFFD8110) ;- (PDC_SSC2) Receive Next Pointer Register
AT91C_SSC2_TPR            EQU (0xFFFD8108) ;- (PDC_SSC2) Transmit Pointer Register
AT91C_SSC2_RPR            EQU (0xFFFD8100) ;- (PDC_SSC2) Receive Pointer Register
AT91C_SSC2_PTSR           EQU (0xFFFD8124) ;- (PDC_SSC2) PDC Transfer Status Register
AT91C_SSC2_TNCR           EQU (0xFFFD811C) ;- (PDC_SSC2) Transmit Next Counter Register
AT91C_SSC2_RNCR           EQU (0xFFFD8114) ;- (PDC_SSC2) Receive Next Counter Register
AT91C_SSC2_TCR            EQU (0xFFFD810C) ;- (PDC_SSC2) Transmit Counter Register
AT91C_SSC2_RCR            EQU (0xFFFD8104) ;- (PDC_SSC2) Receive Counter Register
;- ========== Register definition for SSC2 peripheral ========== 
AT91C_SSC2_IMR            EQU (0xFFFD804C) ;- (SSC2) Interrupt Mask Register
AT91C_SSC2_IER            EQU (0xFFFD8044) ;- (SSC2) Interrupt Enable Register
AT91C_SSC2_RC1R           EQU (0xFFFD803C) ;- (SSC2) Receive Compare 1 Register
AT91C_SSC2_TSHR           EQU (0xFFFD8034) ;- (SSC2) Transmit Sync Holding Register
AT91C_SSC2_CMR            EQU (0xFFFD8004) ;- (SSC2) Clock Mode Register
AT91C_SSC2_IDR            EQU (0xFFFD8048) ;- (SSC2) Interrupt Disable Register
AT91C_SSC2_TCMR           EQU (0xFFFD8018) ;- (SSC2) Transmit Clock Mode Register
AT91C_SSC2_RCMR           EQU (0xFFFD8010) ;- (SSC2) Receive Clock ModeRegister
AT91C_SSC2_CR             EQU (0xFFFD8000) ;- (SSC2) Control Register
AT91C_SSC2_RFMR           EQU (0xFFFD8014) ;- (SSC2) Receive Frame Mode Register
AT91C_SSC2_TFMR           EQU (0xFFFD801C) ;- (SSC2) Transmit Frame Mode Register
AT91C_SSC2_THR            EQU (0xFFFD8024) ;- (SSC2) Transmit Holding Register
AT91C_SSC2_SR             EQU (0xFFFD8040) ;- (SSC2) Status Register
AT91C_SSC2_RC0R           EQU (0xFFFD8038) ;- (SSC2) Receive Compare 0 Register
AT91C_SSC2_RSHR           EQU (0xFFFD8030) ;- (SSC2) Receive Sync Holding Register
AT91C_SSC2_RHR            EQU (0xFFFD8020) ;- (SSC2) Receive Holding Register
;- ========== Register definition for PDC_SSC1 peripheral ========== 
AT91C_SSC1_PTCR           EQU (0xFFFD4120) ;- (PDC_SSC1) PDC Transfer Control Register
AT91C_SSC1_TNPR           EQU (0xFFFD4118) ;- (PDC_SSC1) Transmit Next Pointer Register
AT91C_SSC1_RNPR           EQU (0xFFFD4110) ;- (PDC_SSC1) Receive Next Pointer Register
AT91C_SSC1_TPR            EQU (0xFFFD4108) ;- (PDC_SSC1) Transmit Pointer Register
AT91C_SSC1_RPR            EQU (0xFFFD4100) ;- (PDC_SSC1) Receive Pointer Register
AT91C_SSC1_PTSR           EQU (0xFFFD4124) ;- (PDC_SSC1) PDC Transfer Status Register
AT91C_SSC1_TNCR           EQU (0xFFFD411C) ;- (PDC_SSC1) Transmit Next Counter Register
AT91C_SSC1_RNCR           EQU (0xFFFD4114) ;- (PDC_SSC1) Receive Next Counter Register
AT91C_SSC1_TCR            EQU (0xFFFD410C) ;- (PDC_SSC1) Transmit Counter Register
AT91C_SSC1_RCR            EQU (0xFFFD4104) ;- (PDC_SSC1) Receive Counter Register
;- ========== Register definition for SSC1 peripheral ========== 
AT91C_SSC1_RFMR           EQU (0xFFFD4014) ;- (SSC1) Receive Frame Mode Register
AT91C_SSC1_CMR            EQU (0xFFFD4004) ;- (SSC1) Clock Mode Register
AT91C_SSC1_IDR            EQU (0xFFFD4048) ;- (SSC1) Interrupt Disable Register
AT91C_SSC1_SR             EQU (0xFFFD4040) ;- (SSC1) Status Register
AT91C_SSC1_RC0R           EQU (0xFFFD4038) ;- (SSC1) Receive Compare 0 Register
AT91C_SSC1_RSHR           EQU (0xFFFD4030) ;- (SSC1) Receive Sync Holding Register
AT91C_SSC1_RHR            EQU (0xFFFD4020) ;- (SSC1) Receive Holding Register
AT91C_SSC1_TCMR           EQU (0xFFFD4018) ;- (SSC1) Transmit Clock Mode Register
AT91C_SSC1_RCMR           EQU (0xFFFD4010) ;- (SSC1) Receive Clock ModeRegister
AT91C_SSC1_CR             EQU (0xFFFD4000) ;- (SSC1) Control Register
AT91C_SSC1_IMR            EQU (0xFFFD404C) ;- (SSC1) Interrupt Mask Register
AT91C_SSC1_IER            EQU (0xFFFD4044) ;- (SSC1) Interrupt Enable Register
AT91C_SSC1_RC1R           EQU (0xFFFD403C) ;- (SSC1) Receive Compare 1 Register
AT91C_SSC1_TSHR           EQU (0xFFFD4034) ;- (SSC1) Transmit Sync Holding Register
AT91C_SSC1_THR            EQU (0xFFFD4024) ;- (SSC1) Transmit Holding Register
AT91C_SSC1_TFMR           EQU (0xFFFD401C) ;- (SSC1) Transmit Frame Mode Register
;- ========== Register definition for PDC_SSC0 peripheral ========== 
AT91C_SSC0_PTCR           EQU (0xFFFD0120) ;- (PDC_SSC0) PDC Transfer Control Register
AT91C_SSC0_TNPR           EQU (0xFFFD0118) ;- (PDC_SSC0) Transmit Next Pointer Register
AT91C_SSC0_RNPR           EQU (0xFFFD0110) ;- (PDC_SSC0) Receive Next Pointer Register
AT91C_SSC0_TPR            EQU (0xFFFD0108) ;- (PDC_SSC0) Transmit Pointer Register
AT91C_SSC0_RPR            EQU (0xFFFD0100) ;- (PDC_SSC0) Receive Pointer Register
AT91C_SSC0_PTSR           EQU (0xFFFD0124) ;- (PDC_SSC0) PDC Transfer Status Register
AT91C_SSC0_TNCR           EQU (0xFFFD011C) ;- (PDC_SSC0) Transmit Next Counter Register
AT91C_SSC0_RNCR           EQU (0xFFFD0114) ;- (PDC_SSC0) Receive Next Counter Register
AT91C_SSC0_TCR            EQU (0xFFFD010C) ;- (PDC_SSC0) Transmit Counter Register
AT91C_SSC0_RCR            EQU (0xFFFD0104) ;- (PDC_SSC0) Receive Counter Register
;- ========== Register definition for SSC0 peripheral ========== 
AT91C_SSC0_IMR            EQU (0xFFFD004C) ;- (SSC0) Interrupt Mask Register
AT91C_SSC0_IER            EQU (0xFFFD0044) ;- (SSC0) Interrupt Enable Register
AT91C_SSC0_RC1R           EQU (0xFFFD003C) ;- (SSC0) Receive Compare 1 Register
AT91C_SSC0_TSHR           EQU (0xFFFD0034) ;- (SSC0) Transmit Sync Holding Register
AT91C_SSC0_THR            EQU (0xFFFD0024) ;- (SSC0) Transmit Holding Register
AT91C_SSC0_TFMR           EQU (0xFFFD001C) ;- (SSC0) Transmit Frame Mode Register
AT91C_SSC0_RFMR           EQU (0xFFFD0014) ;- (SSC0) Receive Frame Mode Register
AT91C_SSC0_CMR            EQU (0xFFFD0004) ;- (SSC0) Clock Mode Register
AT91C_SSC0_IDR            EQU (0xFFFD0048) ;- (SSC0) Interrupt Disable Register
AT91C_SSC0_SR             EQU (0xFFFD0040) ;- (SSC0) Status Register
AT91C_SSC0_RC0R           EQU (0xFFFD0038) ;- (SSC0) Receive Compare 0 Register
AT91C_SSC0_RSHR           EQU (0xFFFD0030) ;- (SSC0) Receive Sync Holding Register
AT91C_SSC0_RHR            EQU (0xFFFD0020) ;- (SSC0) Receive Holding Register
AT91C_SSC0_TCMR           EQU (0xFFFD0018) ;- (SSC0) Transmit Clock Mode Register
AT91C_SSC0_RCMR           EQU (0xFFFD0010) ;- (SSC0) Receive Clock ModeRegister
AT91C_SSC0_CR             EQU (0xFFFD0000) ;- (SSC0) Control Register
;- ========== Register definition for PDC_US3 peripheral ========== 
AT91C_US3_PTSR            EQU (0xFFFCC124) ;- (PDC_US3) PDC Transfer Status Register
AT91C_US3_TNCR            EQU (0xFFFCC11C) ;- (PDC_US3) Transmit Next Counter Register
AT91C_US3_RNCR            EQU (0xFFFCC114) ;- (PDC_US3) Receive Next Counter Register
AT91C_US3_TCR             EQU (0xFFFCC10C) ;- (PDC_US3) Transmit Counter Register
AT91C_US3_RCR             EQU (0xFFFCC104) ;- (PDC_US3) Receive Counter Register
AT91C_US3_PTCR            EQU (0xFFFCC120) ;- (PDC_US3) PDC Transfer Control Register
AT91C_US3_TNPR            EQU (0xFFFCC118) ;- (PDC_US3) Transmit Next Pointer Register
AT91C_US3_RNPR            EQU (0xFFFCC110) ;- (PDC_US3) Receive Next Pointer Register
AT91C_US3_TPR             EQU (0xFFFCC108) ;- (PDC_US3) Transmit Pointer Register
AT91C_US3_RPR             EQU (0xFFFCC100) ;- (PDC_US3) Receive Pointer Register
;- ========== Register definition for US3 peripheral ========== 
AT91C_US3_IF              EQU (0xFFFCC04C) ;- (US3) IRDA_FILTER Register
AT91C_US3_NER             EQU (0xFFFCC044) ;- (US3) Nb Errors Register
AT91C_US3_RTOR            EQU (0xFFFCC024) ;- (US3) Receiver Time-out Register
AT91C_US3_THR             EQU (0xFFFCC01C) ;- (US3) Transmitter Holding Register
AT91C_US3_CSR             EQU (0xFFFCC014) ;- (US3) Channel Status Register
AT91C_US3_IDR             EQU (0xFFFCC00C) ;- (US3) Interrupt Disable Register
AT91C_US3_MR              EQU (0xFFFCC004) ;- (US3) Mode Register
AT91C_US3_XXR             EQU (0xFFFCC048) ;- (US3) XON_XOFF Register
AT91C_US3_FIDI            EQU (0xFFFCC040) ;- (US3) FI_DI_Ratio Register
AT91C_US3_TTGR            EQU (0xFFFCC028) ;- (US3) Transmitter Time-guard Register
AT91C_US3_BRGR            EQU (0xFFFCC020) ;- (US3) Baud Rate Generator Register
AT91C_US3_RHR             EQU (0xFFFCC018) ;- (US3) Receiver Holding Register
AT91C_US3_IMR             EQU (0xFFFCC010) ;- (US3) Interrupt Mask Register
AT91C_US3_IER             EQU (0xFFFCC008) ;- (US3) Interrupt Enable Register
AT91C_US3_CR              EQU (0xFFFCC000) ;- (US3) Control Register
;- ========== Register definition for PDC_US2 peripheral ========== 
AT91C_US2_PTSR            EQU (0xFFFC8124) ;- (PDC_US2) PDC Transfer Status Register
AT91C_US2_TNCR            EQU (0xFFFC811C) ;- (PDC_US2) Transmit Next Counter Register
AT91C_US2_RNCR            EQU (0xFFFC8114) ;- (PDC_US2) Receive Next Counter Register
AT91C_US2_TCR             EQU (0xFFFC810C) ;- (PDC_US2) Transmit Counter Register
AT91C_US2_PTCR            EQU (0xFFFC8120) ;- (PDC_US2) PDC Transfer Control Register
AT91C_US2_RCR             EQU (0xFFFC8104) ;- (PDC_US2) Receive Counter Register
AT91C_US2_TNPR            EQU (0xFFFC8118) ;- (PDC_US2) Transmit Next Pointer Register
AT91C_US2_RPR             EQU (0xFFFC8100) ;- (PDC_US2) Receive Pointer Register
AT91C_US2_TPR             EQU (0xFFFC8108) ;- (PDC_US2) Transmit Pointer Register
AT91C_US2_RNPR            EQU (0xFFFC8110) ;- (PDC_US2) Receive Next Pointer Register
;- ========== Register definition for US2 peripheral ========== 
AT91C_US2_XXR             EQU (0xFFFC8048) ;- (US2) XON_XOFF Register
AT91C_US2_FIDI            EQU (0xFFFC8040) ;- (US2) FI_DI_Ratio Register
AT91C_US2_TTGR            EQU (0xFFFC8028) ;- (US2) Transmitter Time-guard Register
AT91C_US2_BRGR            EQU (0xFFFC8020) ;- (US2) Baud Rate Generator Register
AT91C_US2_RHR             EQU (0xFFFC8018) ;- (US2) Receiver Holding Register
AT91C_US2_IMR             EQU (0xFFFC8010) ;- (US2) Interrupt Mask Register
AT91C_US2_IER             EQU (0xFFFC8008) ;- (US2) Interrupt Enable Register
AT91C_US2_CR              EQU (0xFFFC8000) ;- (US2) Control Register
AT91C_US2_IF              EQU (0xFFFC804C) ;- (US2) IRDA_FILTER Register
AT91C_US2_NER             EQU (0xFFFC8044) ;- (US2) Nb Errors Register
AT91C_US2_RTOR            EQU (0xFFFC8024) ;- (US2) Receiver Time-out Register
AT91C_US2_THR             EQU (0xFFFC801C) ;- (US2) Transmitter Holding Register
AT91C_US2_CSR             EQU (0xFFFC8014) ;- (US2) Channel Status Register
AT91C_US2_IDR             EQU (0xFFFC800C) ;- (US2) Interrupt Disable Register
AT91C_US2_MR              EQU (0xFFFC8004) ;- (US2) Mode Register
;- ========== Register definition for PDC_US1 peripheral ========== 
AT91C_US1_PTSR            EQU (0xFFFC4124) ;- (PDC_US1) PDC Transfer Status Register
AT91C_US1_TNCR            EQU (0xFFFC411C) ;- (PDC_US1) Transmit Next Counter Register
AT91C_US1_RNCR            EQU (0xFFFC4114) ;- (PDC_US1) Receive Next Counter Register
AT91C_US1_TCR             EQU (0xFFFC410C) ;- (PDC_US1) Transmit Counter Register
AT91C_US1_RCR             EQU (0xFFFC4104) ;- (PDC_US1) Receive Counter Register
AT91C_US1_PTCR            EQU (0xFFFC4120) ;- (PDC_US1) PDC Transfer Control Register
AT91C_US1_TNPR            EQU (0xFFFC4118) ;- (PDC_US1) Transmit Next Pointer Register
AT91C_US1_RNPR            EQU (0xFFFC4110) ;- (PDC_US1) Receive Next Pointer Register
AT91C_US1_TPR             EQU (0xFFFC4108) ;- (PDC_US1) Transmit Pointer Register
AT91C_US1_RPR             EQU (0xFFFC4100) ;- (PDC_US1) Receive Pointer Register
;- ========== Register definition for US1 peripheral ========== 
AT91C_US1_XXR             EQU (0xFFFC4048) ;- (US1) XON_XOFF Register
AT91C_US1_RHR             EQU (0xFFFC4018) ;- (US1) Receiver Holding Register
AT91C_US1_IMR             EQU (0xFFFC4010) ;- (US1) Interrupt Mask Register
AT91C_US1_IER             EQU (0xFFFC4008) ;- (US1) Interrupt Enable Register
AT91C_US1_CR              EQU (0xFFFC4000) ;- (US1) Control Register
AT91C_US1_RTOR            EQU (0xFFFC4024) ;- (US1) Receiver Time-out Register
AT91C_US1_THR             EQU (0xFFFC401C) ;- (US1) Transmitter Holding Register
AT91C_US1_CSR             EQU (0xFFFC4014) ;- (US1) Channel Status Register
AT91C_US1_IDR             EQU (0xFFFC400C) ;- (US1) Interrupt Disable Register
AT91C_US1_FIDI            EQU (0xFFFC4040) ;- (US1) FI_DI_Ratio Register
AT91C_US1_BRGR            EQU (0xFFFC4020) ;- (US1) Baud Rate Generator Register
AT91C_US1_TTGR            EQU (0xFFFC4028) ;- (US1) Transmitter Time-guard Register
AT91C_US1_IF              EQU (0xFFFC404C) ;- (US1) IRDA_FILTER Register
AT91C_US1_NER             EQU (0xFFFC4044) ;- (US1) Nb Errors Register
AT91C_US1_MR              EQU (0xFFFC4004) ;- (US1) Mode Register
;- ========== Register definition for PDC_US0 peripheral ========== 
AT91C_US0_PTCR            EQU (0xFFFC0120) ;- (PDC_US0) PDC Transfer Control Register
AT91C_US0_TNPR            EQU (0xFFFC0118) ;- (PDC_US0) Transmit Next Pointer Register
AT91C_US0_RNPR            EQU (0xFFFC0110) ;- (PDC_US0) Receive Next Pointer Register
AT91C_US0_TPR             EQU (0xFFFC0108) ;- (PDC_US0) Transmit Pointer Register
AT91C_US0_RPR             EQU (0xFFFC0100) ;- (PDC_US0) Receive Pointer Register
AT91C_US0_PTSR            EQU (0xFFFC0124) ;- (PDC_US0) PDC Transfer Status Register
AT91C_US0_TNCR            EQU (0xFFFC011C) ;- (PDC_US0) Transmit Next Counter Register
AT91C_US0_RNCR            EQU (0xFFFC0114) ;- (PDC_US0) Receive Next Counter Register
AT91C_US0_TCR             EQU (0xFFFC010C) ;- (PDC_US0) Transmit Counter Register
AT91C_US0_RCR             EQU (0xFFFC0104) ;- (PDC_US0) Receive Counter Register
;- ========== Register definition for US0 peripheral ========== 
AT91C_US0_TTGR            EQU (0xFFFC0028) ;- (US0) Transmitter Time-guard Register
AT91C_US0_BRGR            EQU (0xFFFC0020) ;- (US0) Baud Rate Generator Register
AT91C_US0_RHR             EQU (0xFFFC0018) ;- (US0) Receiver Holding Register
AT91C_US0_IMR             EQU (0xFFFC0010) ;- (US0) Interrupt Mask Register
AT91C_US0_NER             EQU (0xFFFC0044) ;- (US0) Nb Errors Register
AT91C_US0_RTOR            EQU (0xFFFC0024) ;- (US0) Receiver Time-out Register
AT91C_US0_XXR             EQU (0xFFFC0048) ;- (US0) XON_XOFF Register
AT91C_US0_FIDI            EQU (0xFFFC0040) ;- (US0) FI_DI_Ratio Register
AT91C_US0_CR              EQU (0xFFFC0000) ;- (US0) Control Register
AT91C_US0_IER             EQU (0xFFFC0008) ;- (US0) Interrupt Enable Register
AT91C_US0_IF              EQU (0xFFFC004C) ;- (US0) IRDA_FILTER Register
AT91C_US0_MR              EQU (0xFFFC0004) ;- (US0) Mode Register
AT91C_US0_IDR             EQU (0xFFFC000C) ;- (US0) Interrupt Disable Register
AT91C_US0_CSR             EQU (0xFFFC0014) ;- (US0) Channel Status Register
AT91C_US0_THR             EQU (0xFFFC001C) ;- (US0) Transmitter Holding Register
;- ========== Register definition for TWI peripheral ========== 
AT91C_TWI_RHR             EQU (0xFFFB8030) ;- (TWI) Receive Holding Register
AT91C_TWI_IDR             EQU (0xFFFB8028) ;- (TWI) Interrupt Disable Register
AT91C_TWI_SR              EQU (0xFFFB8020) ;- (TWI) Status Register
AT91C_TWI_CWGR            EQU (0xFFFB8010) ;- (TWI) Clock Waveform Generator Register
AT91C_TWI_SMR             EQU (0xFFFB8008) ;- (TWI) Slave Mode Register
AT91C_TWI_CR              EQU (0xFFFB8000) ;- (TWI) Control Register
AT91C_TWI_THR             EQU (0xFFFB8034) ;- (TWI) Transmit Holding Register
AT91C_TWI_IMR             EQU (0xFFFB802C) ;- (TWI) Interrupt Mask Register
AT91C_TWI_IER             EQU (0xFFFB8024) ;- (TWI) Interrupt Enable Register
AT91C_TWI_IADR            EQU (0xFFFB800C) ;- (TWI) Internal Address Register
AT91C_TWI_MMR             EQU (0xFFFB8004) ;- (TWI) Master Mode Register
;- ========== Register definition for PDC_MCI peripheral ========== 
AT91C_MCI_PTCR            EQU (0xFFFB4120) ;- (PDC_MCI) PDC Transfer Control Register
AT91C_MCI_TNPR            EQU (0xFFFB4118) ;- (PDC_MCI) Transmit Next Pointer Register
AT91C_MCI_RNPR            EQU (0xFFFB4110) ;- (PDC_MCI) Receive Next Pointer Register
AT91C_MCI_TPR             EQU (0xFFFB4108) ;- (PDC_MCI) Transmit Pointer Register
AT91C_MCI_RPR             EQU (0xFFFB4100) ;- (PDC_MCI) Receive Pointer Register
AT91C_MCI_PTSR            EQU (0xFFFB4124) ;- (PDC_MCI) PDC Transfer Status Register
AT91C_MCI_TNCR            EQU (0xFFFB411C) ;- (PDC_MCI) Transmit Next Counter Register
AT91C_MCI_RNCR            EQU (0xFFFB4114) ;- (PDC_MCI) Receive Next Counter Register
AT91C_MCI_TCR             EQU (0xFFFB410C) ;- (PDC_MCI) Transmit Counter Register
AT91C_MCI_RCR             EQU (0xFFFB4104) ;- (PDC_MCI) Receive Counter Register
;- ========== Register definition for MCI peripheral ========== 
AT91C_MCI_IDR             EQU (0xFFFB4048) ;- (MCI) MCI Interrupt Disable Register
AT91C_MCI_SR              EQU (0xFFFB4040) ;- (MCI) MCI Status Register
AT91C_MCI_RDR             EQU (0xFFFB4030) ;- (MCI) MCI Receive Data Register
AT91C_MCI_RSPR            EQU (0xFFFB4020) ;- (MCI) MCI Response Register
AT91C_MCI_ARGR            EQU (0xFFFB4010) ;- (MCI) MCI Argument Register
AT91C_MCI_DTOR            EQU (0xFFFB4008) ;- (MCI) MCI Data Timeout Register
AT91C_MCI_CR              EQU (0xFFFB4000) ;- (MCI) MCI Control Register
AT91C_MCI_IMR             EQU (0xFFFB404C) ;- (MCI) MCI Interrupt Mask Register
AT91C_MCI_IER             EQU (0xFFFB4044) ;- (MCI) MCI Interrupt Enable Register
AT91C_MCI_TDR             EQU (0xFFFB4034) ;- (MCI) MCI Transmit Data Register
AT91C_MCI_CMDR            EQU (0xFFFB4014) ;- (MCI) MCI Command Register
AT91C_MCI_SDCR            EQU (0xFFFB400C) ;- (MCI) MCI SD Card Register
AT91C_MCI_MR              EQU (0xFFFB4004) ;- (MCI) MCI Mode Register
;- ========== Register definition for UDP peripheral ========== 
AT91C_UDP_ISR             EQU (0xFFFB001C) ;- (UDP) Interrupt Status Register
AT91C_UDP_IDR             EQU (0xFFFB0014) ;- (UDP) Interrupt Disable Register
AT91C_UDP_GLBSTATE        EQU (0xFFFB0004) ;- (UDP) Global State Register
AT91C_UDP_FDR             EQU (0xFFFB0050) ;- (UDP) Endpoint FIFO Data Register
AT91C_UDP_CSR             EQU (0xFFFB0030) ;- (UDP) Endpoint Control and Status Register
AT91C_UDP_RSTEP           EQU (0xFFFB0028) ;- (UDP) Reset Endpoint Register
AT91C_UDP_ICR             EQU (0xFFFB0020) ;- (UDP) Interrupt Clear Register
AT91C_UDP_IMR             EQU (0xFFFB0018) ;- (UDP) Interrupt Mask Register
AT91C_UDP_IER             EQU (0xFFFB0010) ;- (UDP) Interrupt Enable Register
AT91C_UDP_FADDR           EQU (0xFFFB0008) ;- (UDP) Function Address Register
AT91C_UDP_NUM             EQU (0xFFFB0000) ;- (UDP) Frame Number Register
;- ========== Register definition for TC5 peripheral ========== 
AT91C_TC5_CMR             EQU (0xFFFA4084) ;- (TC5) Channel Mode Register
AT91C_TC5_IDR             EQU (0xFFFA40A8) ;- (TC5) Interrupt Disable Register
AT91C_TC5_SR              EQU (0xFFFA40A0) ;- (TC5) Status Register
AT91C_TC5_RB              EQU (0xFFFA4098) ;- (TC5) Register B
AT91C_TC5_CV              EQU (0xFFFA4090) ;- (TC5) Counter Value
AT91C_TC5_CCR             EQU (0xFFFA4080) ;- (TC5) Channel Control Register
AT91C_TC5_IMR             EQU (0xFFFA40AC) ;- (TC5) Interrupt Mask Register
AT91C_TC5_IER             EQU (0xFFFA40A4) ;- (TC5) Interrupt Enable Register
AT91C_TC5_RC              EQU (0xFFFA409C) ;- (TC5) Register C
AT91C_TC5_RA              EQU (0xFFFA4094) ;- (TC5) Register A
;- ========== Register definition for TC4 peripheral ========== 
AT91C_TC4_IMR             EQU (0xFFFA406C) ;- (TC4) Interrupt Mask Register
AT91C_TC4_IER             EQU (0xFFFA4064) ;- (TC4) Interrupt Enable Register
AT91C_TC4_RC              EQU (0xFFFA405C) ;- (TC4) Register C
AT91C_TC4_RA              EQU (0xFFFA4054) ;- (TC4) Register A
AT91C_TC4_CMR             EQU (0xFFFA4044) ;- (TC4) Channel Mode Register
AT91C_TC4_IDR             EQU (0xFFFA4068) ;- (TC4) Interrupt Disable Register
AT91C_TC4_SR              EQU (0xFFFA4060) ;- (TC4) Status Register
AT91C_TC4_RB              EQU (0xFFFA4058) ;- (TC4) Register B
AT91C_TC4_CV              EQU (0xFFFA4050) ;- (TC4) Counter Value
AT91C_TC4_CCR             EQU (0xFFFA4040) ;- (TC4) Channel Control Register
;- ========== Register definition for TC3 peripheral ========== 
AT91C_TC3_IMR             EQU (0xFFFA402C) ;- (TC3) Interrupt Mask Register
AT91C_TC3_CV              EQU (0xFFFA4010) ;- (TC3) Counter Value
AT91C_TC3_CCR             EQU (0xFFFA4000) ;- (TC3) Channel Control Register
AT91C_TC3_IER             EQU (0xFFFA4024) ;- (TC3) Interrupt Enable Register
AT91C_TC3_CMR             EQU (0xFFFA4004) ;- (TC3) Channel Mode Register
AT91C_TC3_RA              EQU (0xFFFA4014) ;- (TC3) Register A
AT91C_TC3_RC              EQU (0xFFFA401C) ;- (TC3) Register C
AT91C_TC3_IDR             EQU (0xFFFA4028) ;- (TC3) Interrupt Disable Register
AT91C_TC3_RB              EQU (0xFFFA4018) ;- (TC3) Register B
AT91C_TC3_SR              EQU (0xFFFA4020) ;- (TC3) Status Register
;- ========== Register definition for TCB1 peripheral ========== 
AT91C_TCB1_BCR            EQU (0xFFFA4140) ;- (TCB1) TC Block Control Register
AT91C_TCB1_BMR            EQU (0xFFFA4144) ;- (TCB1) TC Block Mode Register
;- ========== Register definition for TC2 peripheral ========== 
AT91C_TC2_IMR             EQU (0xFFFA00AC) ;- (TC2) Interrupt Mask Register
AT91C_TC2_IER             EQU (0xFFFA00A4) ;- (TC2) Interrupt Enable Register
AT91C_TC2_RC              EQU (0xFFFA009C) ;- (TC2) Register C
AT91C_TC2_RA              EQU (0xFFFA0094) ;- (TC2) Register A
AT91C_TC2_CMR             EQU (0xFFFA0084) ;- (TC2) Channel Mode Register
AT91C_TC2_IDR             EQU (0xFFFA00A8) ;- (TC2) Interrupt Disable Register
AT91C_TC2_SR              EQU (0xFFFA00A0) ;- (TC2) Status Register
AT91C_TC2_RB              EQU (0xFFFA0098) ;- (TC2) Register B
AT91C_TC2_CV              EQU (0xFFFA0090) ;- (TC2) Counter Value
AT91C_TC2_CCR             EQU (0xFFFA0080) ;- (TC2) Channel Control Register
;- ========== Register definition for TC1 peripheral ========== 
AT91C_TC1_IMR             EQU (0xFFFA006C) ;- (TC1) Interrupt Mask Register
AT91C_TC1_IER             EQU (0xFFFA0064) ;- (TC1) Interrupt Enable Register
AT91C_TC1_RC              EQU (0xFFFA005C) ;- (TC1) Register C
AT91C_TC1_RA              EQU (0xFFFA0054) ;- (TC1) Register A
AT91C_TC1_CMR             EQU (0xFFFA0044) ;- (TC1) Channel Mode Register
AT91C_TC1_IDR             EQU (0xFFFA0068) ;- (TC1) Interrupt Disable Register
AT91C_TC1_SR              EQU (0xFFFA0060) ;- (TC1) Status Register
AT91C_TC1_RB              EQU (0xFFFA0058) ;- (TC1) Register B
AT91C_TC1_CV              EQU (0xFFFA0050) ;- (TC1) Counter Value
AT91C_TC1_CCR             EQU (0xFFFA0040) ;- (TC1) Channel Control Register
;- ========== Register definition for TC0 peripheral ========== 
AT91C_TC0_IMR             EQU (0xFFFA002C) ;- (TC0) Interrupt Mask Register
AT91C_TC0_IER             EQU (0xFFFA0024) ;- (TC0) Interrupt Enable Register
AT91C_TC0_RC              EQU (0xFFFA001C) ;- (TC0) Register C
AT91C_TC0_RA              EQU (0xFFFA0014) ;- (TC0) Register A
AT91C_TC0_CMR             EQU (0xFFFA0004) ;- (TC0) Channel Mode Register
AT91C_TC0_IDR             EQU (0xFFFA0028) ;- (TC0) Interrupt Disable Register
AT91C_TC0_SR              EQU (0xFFFA0020) ;- (TC0) Status Register
AT91C_TC0_RB              EQU (0xFFFA0018) ;- (TC0) Register B
AT91C_TC0_CV              EQU (0xFFFA0010) ;- (TC0) Counter Value
AT91C_TC0_CCR             EQU (0xFFFA0000) ;- (TC0) Channel Control Register
;- ========== Register definition for TCB0 peripheral ========== 
AT91C_TCB0_BMR            EQU (0xFFFA00C4) ;- (TCB0) TC Block Mode Register
AT91C_TCB0_BCR            EQU (0xFFFA00C0) ;- (TCB0) TC Block Control Register
;- ========== Register definition for UHP peripheral ========== 
AT91C_UHP_HcRhDescriptorA EQU (0x00300048) ;- (UHP) Root Hub characteristics A
AT91C_UHP_HcRhPortStatus  EQU (0x00300054) ;- (UHP) Root Hub Port Status Register
AT91C_UHP_HcRhDescriptorB EQU (0x0030004C) ;- (UHP) Root Hub characteristics B
AT91C_UHP_HcControl       EQU (0x00300004) ;- (UHP) Operating modes for the Host Controller
AT91C_UHP_HcInterruptStatus EQU (0x0030000C) ;- (UHP) Interrupt Status Register
AT91C_UHP_HcRhStatus      EQU (0x00300050) ;- (UHP) Root Hub Status register
AT91C_UHP_HcRevision      EQU (0x00300000) ;- (UHP) Revision
AT91C_UHP_HcCommandStatus EQU (0x00300008) ;- (UHP) Command & status Register
AT91C_UHP_HcInterruptEnable EQU (0x00300010) ;- (UHP) Interrupt Enable Register
AT91C_UHP_HcHCCA          EQU (0x00300018) ;- (UHP) Pointer to the Host Controller Communication Area
AT91C_UHP_HcControlHeadED EQU (0x00300020) ;- (UHP) First Endpoint Descriptor of the Control list
AT91C_UHP_HcInterruptDisable EQU (0x00300014) ;- (UHP) Interrupt Disable Register
AT91C_UHP_HcPeriodCurrentED EQU (0x0030001C) ;- (UHP) Current Isochronous or Interrupt Endpoint Descriptor
AT91C_UHP_HcControlCurrentED EQU (0x00300024) ;- (UHP) Endpoint Control and Status Register
AT91C_UHP_HcBulkCurrentED EQU (0x0030002C) ;- (UHP) Current endpoint of the Bulk list
AT91C_UHP_HcFmInterval    EQU (0x00300034) ;- (UHP) Bit time between 2 consecutive SOFs
AT91C_UHP_HcBulkHeadED    EQU (0x00300028) ;- (UHP) First endpoint register of the Bulk list
AT91C_UHP_HcBulkDoneHead  EQU (0x00300030) ;- (UHP) Last completed transfer descriptor
AT91C_UHP_HcFmRemaining   EQU (0x00300038) ;- (UHP) Bit time remaining in the current Frame
AT91C_UHP_HcPeriodicStart EQU (0x00300040) ;- (UHP) Periodic Start
AT91C_UHP_HcLSThreshold   EQU (0x00300044) ;- (UHP) LS Threshold
AT91C_UHP_HcFmNumber      EQU (0x0030003C) ;- (UHP) Frame number
;- ========== Register definition for EMAC peripheral ========== 
AT91C_EMAC_RSR            EQU (0xFFFBC020) ;- (EMAC) Receive Status Register
AT91C_EMAC_MAN            EQU (0xFFFBC034) ;- (EMAC) PHY Maintenance Register
AT91C_EMAC_HSH            EQU (0xFFFBC090) ;- (EMAC) Hash Address High[63:32]
AT91C_EMAC_MCOL           EQU (0xFFFBC048) ;- (EMAC) Multiple Collision Frame Register
AT91C_EMAC_IER            EQU (0xFFFBC028) ;- (EMAC) Interrupt Enable Register
AT91C_EMAC_SA2H           EQU (0xFFFBC0A4) ;- (EMAC) Specific Address 2 High, Last 2 bytes
AT91C_EMAC_HSL            EQU (0xFFFBC094) ;- (EMAC) Hash Address Low[31:0]
AT91C_EMAC_LCOL           EQU (0xFFFBC05C) ;- (EMAC) Late Collision Register
AT91C_EMAC_OK             EQU (0xFFFBC04C) ;- (EMAC) Frames Received OK Register
AT91C_EMAC_CFG            EQU (0xFFFBC004) ;- (EMAC) Network Configuration Register
AT91C_EMAC_SA3L           EQU (0xFFFBC0A8) ;- (EMAC) Specific Address 3 Low, First 4 bytes
AT91C_EMAC_SEQE           EQU (0xFFFBC050) ;- (EMAC) Frame Check Sequence Error Register
AT91C_EMAC_ECOL           EQU (0xFFFBC060) ;- (EMAC) Excessive Collision Register
AT91C_EMAC_ELR            EQU (0xFFFBC070) ;- (EMAC) Excessive Length Error Register
AT91C_EMAC_SR             EQU (0xFFFBC008) ;- (EMAC) Network Status Register
AT91C_EMAC_RBQP           EQU (0xFFFBC018) ;- (EMAC) Receive Buffer Queue Pointer
AT91C_EMAC_CSE            EQU (0xFFFBC064) ;- (EMAC) Carrier Sense Error Register
AT91C_EMAC_RJB            EQU (0xFFFBC074) ;- (EMAC) Receive Jabber Register
AT91C_EMAC_USF            EQU (0xFFFBC078) ;- (EMAC) Undersize Frame Register
AT91C_EMAC_IDR            EQU (0xFFFBC02C) ;- (EMAC) Interrupt Disable Register
AT91C_EMAC_SA1L           EQU (0xFFFBC098) ;- (EMAC) Specific Address 1 Low, First 4 bytes
AT91C_EMAC_IMR            EQU (0xFFFBC030) ;- (EMAC) Interrupt Mask Register
AT91C_EMAC_FRA            EQU (0xFFFBC040) ;- (EMAC) Frames Transmitted OK Register
AT91C_EMAC_SA3H           EQU (0xFFFBC0AC) ;- (EMAC) Specific Address 3 High, Last 2 bytes
AT91C_EMAC_SA1H           EQU (0xFFFBC09C) ;- (EMAC) Specific Address 1 High, Last 2 bytes
AT91C_EMAC_SCOL           EQU (0xFFFBC044) ;- (EMAC) Single Collision Frame Register
AT91C_EMAC_ALE            EQU (0xFFFBC054) ;- (EMAC) Alignment Error Register
AT91C_EMAC_TAR            EQU (0xFFFBC00C) ;- (EMAC) Transmit Address Register
AT91C_EMAC_SA4L           EQU (0xFFFBC0B0) ;- (EMAC) Specific Address 4 Low, First 4 bytes
AT91C_EMAC_SA2L           EQU (0xFFFBC0A0) ;- (EMAC) Specific Address 2 Low, First 4 bytes
AT91C_EMAC_TUE            EQU (0xFFFBC068) ;- (EMAC) Transmit Underrun Error Register
AT91C_EMAC_DTE            EQU (0xFFFBC058) ;- (EMAC) Deferred Transmission Frame Register
AT91C_EMAC_TCR            EQU (0xFFFBC010) ;- (EMAC) Transmit Control Register
AT91C_EMAC_CTL            EQU (0xFFFBC000) ;- (EMAC) Network Control Register
AT91C_EMAC_SA4H           EQU (0xFFFBC0B4) ;- (EMAC) Specific Address 4 High, Last 2 bytesr
AT91C_EMAC_CDE            EQU (0xFFFBC06C) ;- (EMAC) Code Error Register
AT91C_EMAC_SQEE           EQU (0xFFFBC07C) ;- (EMAC) SQE Test Error Register
AT91C_EMAC_TSR            EQU (0xFFFBC014) ;- (EMAC) Transmit Status Register
AT91C_EMAC_DRFC           EQU (0xFFFBC080) ;- (EMAC) Discarded RX Frame Register
;- ========== Register definition for EBI peripheral ========== 
AT91C_EBI_CFGR            EQU (0xFFFFFF64) ;- (EBI) Configuration Register
AT91C_EBI_CSA             EQU (0xFFFFFF60) ;- (EBI) Chip Select Assignment Register
;- ========== Register definition for SMC2 peripheral ========== 
AT91C_SMC2_CSR            EQU (0xFFFFFF70) ;- (SMC2) SMC2 Chip Select Register
;- ========== Register definition for SDRC peripheral ========== 
AT91C_SDRC_IMR            EQU (0xFFFFFFAC) ;- (SDRC) SDRAM Controller Interrupt Mask Register
AT91C_SDRC_IER            EQU (0xFFFFFFA4) ;- (SDRC) SDRAM Controller Interrupt Enable Register
AT91C_SDRC_SRR            EQU (0xFFFFFF9C) ;- (SDRC) SDRAM Controller Self Refresh Register
AT91C_SDRC_TR             EQU (0xFFFFFF94) ;- (SDRC) SDRAM Controller Refresh Timer Register
AT91C_SDRC_ISR            EQU (0xFFFFFFB0) ;- (SDRC) SDRAM Controller Interrupt Mask Register
AT91C_SDRC_IDR            EQU (0xFFFFFFA8) ;- (SDRC) SDRAM Controller Interrupt Disable Register
AT91C_SDRC_LPR            EQU (0xFFFFFFA0) ;- (SDRC) SDRAM Controller Low Power Register
AT91C_SDRC_CR             EQU (0xFFFFFF98) ;- (SDRC) SDRAM Controller Configuration Register
AT91C_SDRC_MR             EQU (0xFFFFFF90) ;- (SDRC) SDRAM Controller Mode Register
;- ========== Register definition for BFC peripheral ========== 
AT91C_BFC_MR              EQU (0xFFFFFFC0) ;- (BFC) BFC Mode Register

;- *****************************************************************************
;-               PIO DEFINITIONS FOR AT91RM9200
;- *****************************************************************************
AT91C_PIO_PA0             EQU (1:SHL:0) ;- Pin Controlled by PA0
AT91C_PA0_MISO            EQU (AT91C_PIO_PA0) ;-  SPI Master In Slave
AT91C_PA0_PCK3            EQU (AT91C_PIO_PA0) ;-  PMC Programmable Clock Output 3
AT91C_PIO_PA1             EQU (1:SHL:1) ;- Pin Controlled by PA1
AT91C_PA1_MOSI            EQU (AT91C_PIO_PA1) ;-  SPI Master Out Slave
AT91C_PA1_PCK0            EQU (AT91C_PIO_PA1) ;-  PMC Programmable Clock Output 0
AT91C_PIO_PA10            EQU (1:SHL:10) ;- Pin Controlled by PA10
AT91C_PA10_ETX1           EQU (AT91C_PIO_PA10) ;-  Ethernet MAC Transmit Data 1
AT91C_PA10_MCDB1          EQU (AT91C_PIO_PA10) ;-  Multimedia Card B Data 1
AT91C_PIO_PA11            EQU (1:SHL:11) ;- Pin Controlled by PA11
AT91C_PA11_ECRS_ECRSDV    EQU (AT91C_PIO_PA11) ;-  Ethernet MAC Carrier Sense/Carrier Sense and Data Valid
AT91C_PA11_MCDB2          EQU (AT91C_PIO_PA11) ;-  Multimedia Card B Data 2
AT91C_PIO_PA12            EQU (1:SHL:12) ;- Pin Controlled by PA12
AT91C_PA12_ERX0           EQU (AT91C_PIO_PA12) ;-  Ethernet MAC Receive Data 0
AT91C_PA12_MCDB3          EQU (AT91C_PIO_PA12) ;-  Multimedia Card B Data 3
AT91C_PIO_PA13            EQU (1:SHL:13) ;- Pin Controlled by PA13
AT91C_PA13_ERX1           EQU (AT91C_PIO_PA13) ;-  Ethernet MAC Receive Data 1
AT91C_PA13_TCLK0          EQU (AT91C_PIO_PA13) ;-  Timer Counter 0 external clock input
AT91C_PIO_PA14            EQU (1:SHL:14) ;- Pin Controlled by PA14
AT91C_PA14_ERXER          EQU (AT91C_PIO_PA14) ;-  Ethernet MAC Receive Error
AT91C_PA14_TCLK1          EQU (AT91C_PIO_PA14) ;-  Timer Counter 1 external clock input
AT91C_PIO_PA15            EQU (1:SHL:15) ;- Pin Controlled by PA15
AT91C_PA15_EMDC           EQU (AT91C_PIO_PA15) ;-  Ethernet MAC Management Data Clock
AT91C_PA15_TCLK2          EQU (AT91C_PIO_PA15) ;-  Timer Counter 2 external clock input
AT91C_PIO_PA16            EQU (1:SHL:16) ;- Pin Controlled by PA16
AT91C_PA16_EMDIO          EQU (AT91C_PIO_PA16) ;-  Ethernet MAC Management Data Input/Output
AT91C_PA16_IRQ6           EQU (AT91C_PIO_PA16) ;-  AIC Interrupt input 6
AT91C_PIO_PA17            EQU (1:SHL:17) ;- Pin Controlled by PA17
AT91C_PA17_TXD0           EQU (AT91C_PIO_PA17) ;-  USART 0 Transmit Data
AT91C_PA17_TIOA0          EQU (AT91C_PIO_PA17) ;-  Timer Counter 0 Multipurpose Timer I/O Pin A
AT91C_PIO_PA18            EQU (1:SHL:18) ;- Pin Controlled by PA18
AT91C_PA18_RXD0           EQU (AT91C_PIO_PA18) ;-  USART 0 Receive Data
AT91C_PA18_TIOB0          EQU (AT91C_PIO_PA18) ;-  Timer Counter 0 Multipurpose Timer I/O Pin B
AT91C_PIO_PA19            EQU (1:SHL:19) ;- Pin Controlled by PA19
AT91C_PA19_SCK0           EQU (AT91C_PIO_PA19) ;-  USART 0 Serial Clock
AT91C_PA19_TIOA1          EQU (AT91C_PIO_PA19) ;-  Timer Counter 1 Multipurpose Timer I/O Pin A
AT91C_PIO_PA2             EQU (1:SHL:2) ;- Pin Controlled by PA2
AT91C_PA2_SPCK            EQU (AT91C_PIO_PA2) ;-  SPI Serial Clock
AT91C_PA2_IRQ4            EQU (AT91C_PIO_PA2) ;-  AIC Interrupt Input 4
AT91C_PIO_PA20            EQU (1:SHL:20) ;- Pin Controlled by PA20
AT91C_PA20_CTS0           EQU (AT91C_PIO_PA20) ;-  USART 0 Clear To Send
AT91C_PA20_TIOB1          EQU (AT91C_PIO_PA20) ;-  Timer Counter 1 Multipurpose Timer I/O Pin B
AT91C_PIO_PA21            EQU (1:SHL:21) ;- Pin Controlled by PA21
AT91C_PA21_RTS0           EQU (AT91C_PIO_PA21) ;-  Usart 0 Ready To Send
AT91C_PA21_TIOA2          EQU (AT91C_PIO_PA21) ;-  Timer Counter 2 Multipurpose Timer I/O Pin A
AT91C_PIO_PA22            EQU (1:SHL:22) ;- Pin Controlled by PA22
AT91C_PA22_RXD2           EQU (AT91C_PIO_PA22) ;-  USART 2 Receive Data
AT91C_PA22_TIOB2          EQU (AT91C_PIO_PA22) ;-  Timer Counter 2 Multipurpose Timer I/O Pin B
AT91C_PIO_PA23            EQU (1:SHL:23) ;- Pin Controlled by PA23
AT91C_PA23_TXD2           EQU (AT91C_PIO_PA23) ;-  USART 2 Transmit Data
AT91C_PA23_IRQ3           EQU (AT91C_PIO_PA23) ;-  Interrupt input 3
AT91C_PIO_PA24            EQU (1:SHL:24) ;- Pin Controlled by PA24
AT91C_PA24_SCK2           EQU (AT91C_PIO_PA24) ;-  USART2 Serial Clock
AT91C_PA24_PCK1           EQU (AT91C_PIO_PA24) ;-  PMC Programmable Clock Output 1
AT91C_PIO_PA25            EQU (1:SHL:25) ;- Pin Controlled by PA25
AT91C_PA25_TWD            EQU (AT91C_PIO_PA25) ;-  TWI Two-wire Serial Data
AT91C_PA25_IRQ2           EQU (AT91C_PIO_PA25) ;-  Interrupt input 2
AT91C_PIO_PA26            EQU (1:SHL:26) ;- Pin Controlled by PA26
AT91C_PA26_TWCK           EQU (AT91C_PIO_PA26) ;-  TWI Two-wire Serial Clock
AT91C_PA26_IRQ1           EQU (AT91C_PIO_PA26) ;-  Interrupt input 1
AT91C_PIO_PA27            EQU (1:SHL:27) ;- Pin Controlled by PA27
AT91C_PA27_MCCK           EQU (AT91C_PIO_PA27) ;-  Multimedia Card Clock
AT91C_PA27_TCLK3          EQU (AT91C_PIO_PA27) ;-  Timer Counter 3 External Clock Input
AT91C_PIO_PA28            EQU (1:SHL:28) ;- Pin Controlled by PA28
AT91C_PA28_MCCDA          EQU (AT91C_PIO_PA28) ;-  Multimedia Card A Command
AT91C_PA28_TCLK4          EQU (AT91C_PIO_PA28) ;-  Timer Counter 4 external Clock Input
AT91C_PIO_PA29            EQU (1:SHL:29) ;- Pin Controlled by PA29
AT91C_PA29_MCDA0          EQU (AT91C_PIO_PA29) ;-  Multimedia Card A Data 0
AT91C_PA29_TCLK5          EQU (AT91C_PIO_PA29) ;-  Timer Counter 5 external clock input
AT91C_PIO_PA3             EQU (1:SHL:3) ;- Pin Controlled by PA3
AT91C_PA3_NPCS0           EQU (AT91C_PIO_PA3) ;-  SPI Peripheral Chip Select 0
AT91C_PA3_IRQ5            EQU (AT91C_PIO_PA3) ;-  AIC Interrupt Input 5
AT91C_PIO_PA30            EQU (1:SHL:30) ;- Pin Controlled by PA30
AT91C_PA30_DRXD           EQU (AT91C_PIO_PA30) ;-  DBGU Debug Receive Data
AT91C_PA30_CTS2           EQU (AT91C_PIO_PA30) ;-  Usart 2 Clear To Send
AT91C_PIO_PA31            EQU (1:SHL:31) ;- Pin Controlled by PA31
AT91C_PA31_DTXD           EQU (AT91C_PIO_PA31) ;-  DBGU Debug Transmit Data
AT91C_PA31_RTS2           EQU (AT91C_PIO_PA31) ;-  USART 2 Ready To Send
AT91C_PIO_PA4             EQU (1:SHL:4) ;- Pin Controlled by PA4
AT91C_PA4_NPCS1           EQU (AT91C_PIO_PA4) ;-  SPI Peripheral Chip Select 1
AT91C_PA4_PCK1            EQU (AT91C_PIO_PA4) ;-  PMC Programmable Clock Output 1
AT91C_PIO_PA5             EQU (1:SHL:5) ;- Pin Controlled by PA5
AT91C_PA5_NPCS2           EQU (AT91C_PIO_PA5) ;-  SPI Peripheral Chip Select 2
AT91C_PA5_TXD3            EQU (AT91C_PIO_PA5) ;-  USART 3 Transmit Data
AT91C_PIO_PA6             EQU (1:SHL:6) ;- Pin Controlled by PA6
AT91C_PA6_NPCS3           EQU (AT91C_PIO_PA6) ;-  SPI Peripheral Chip Select 3
AT91C_PA6_RXD3            EQU (AT91C_PIO_PA6) ;-  USART 3 Receive Data
AT91C_PIO_PA7             EQU (1:SHL:7) ;- Pin Controlled by PA7
AT91C_PA7_ETXCK_EREFCK    EQU (AT91C_PIO_PA7) ;-  Ethernet MAC Transmit Clock/Reference Clock
AT91C_PA7_PCK2            EQU (AT91C_PIO_PA7) ;-  PMC Programmable Clock 2
AT91C_PIO_PA8             EQU (1:SHL:8) ;- Pin Controlled by PA8
AT91C_PA8_ETXEN           EQU (AT91C_PIO_PA8) ;-  Ethernet MAC Transmit Enable
AT91C_PA8_MCCDB           EQU (AT91C_PIO_PA8) ;-  Multimedia Card B Command
AT91C_PIO_PA9             EQU (1:SHL:9) ;- Pin Controlled by PA9
AT91C_PA9_ETX0            EQU (AT91C_PIO_PA9) ;-  Ethernet MAC Transmit Data 0
AT91C_PA9_MCDB0           EQU (AT91C_PIO_PA9) ;-  Multimedia Card B Data 0
AT91C_PIO_PB0             EQU (1:SHL:0) ;- Pin Controlled by PB0
AT91C_PB0_TF0             EQU (AT91C_PIO_PB0) ;-  SSC Transmit Frame Sync 0
AT91C_PB0_TIOB3           EQU (AT91C_PIO_PB0) ;-  Timer Counter 3 Multipurpose Timer I/O Pin B
AT91C_PIO_PB1             EQU (1:SHL:1) ;- Pin Controlled by PB1
AT91C_PB1_TK0             EQU (AT91C_PIO_PB1) ;-  SSC Transmit Clock 0
AT91C_PB1_CTS3            EQU (AT91C_PIO_PB1) ;-  USART 3 Clear To Send
AT91C_PIO_PB10            EQU (1:SHL:10) ;- Pin Controlled by PB10
AT91C_PB10_RK1            EQU (AT91C_PIO_PB10) ;-  SSC Receive Clock 1
AT91C_PB10_TIOA5          EQU (AT91C_PIO_PB10) ;-  Timer Counter 5 Multipurpose Timer I/O Pin A
AT91C_PIO_PB11            EQU (1:SHL:11) ;- Pin Controlled by PB11
AT91C_PB11_RF1            EQU (AT91C_PIO_PB11) ;-  SSC Receive Frame Sync 1
AT91C_PB11_TIOB5          EQU (AT91C_PIO_PB11) ;-  Timer Counter 5 Multipurpose Timer I/O Pin B
AT91C_PIO_PB12            EQU (1:SHL:12) ;- Pin Controlled by PB12
AT91C_PB12_TF2            EQU (AT91C_PIO_PB12) ;-  SSC Transmit Frame Sync 2
AT91C_PB12_ETX2           EQU (AT91C_PIO_PB12) ;-  Ethernet MAC Transmit Data 2
AT91C_PIO_PB13            EQU (1:SHL:13) ;- Pin Controlled by PB13
AT91C_PB13_TK2            EQU (AT91C_PIO_PB13) ;-  SSC Transmit Clock 2
AT91C_PB13_ETX3           EQU (AT91C_PIO_PB13) ;-  Ethernet MAC Transmit Data 3
AT91C_PIO_PB14            EQU (1:SHL:14) ;- Pin Controlled by PB14
AT91C_PB14_TD2            EQU (AT91C_PIO_PB14) ;-  SSC Transmit Data 2
AT91C_PB14_ETXER          EQU (AT91C_PIO_PB14) ;-  Ethernet MAC Transmikt Coding Error
AT91C_PIO_PB15            EQU (1:SHL:15) ;- Pin Controlled by PB15
AT91C_PB15_RD2            EQU (AT91C_PIO_PB15) ;-  SSC Receive Data 2
AT91C_PB15_ERX2           EQU (AT91C_PIO_PB15) ;-  Ethernet MAC Receive Data 2
AT91C_PIO_PB16            EQU (1:SHL:16) ;- Pin Controlled by PB16
AT91C_PB16_RK2            EQU (AT91C_PIO_PB16) ;-  SSC Receive Clock 2
AT91C_PB16_ERX3           EQU (AT91C_PIO_PB16) ;-  Ethernet MAC Receive Data 3
AT91C_PIO_PB17            EQU (1:SHL:17) ;- Pin Controlled by PB17
AT91C_PB17_RF2            EQU (AT91C_PIO_PB17) ;-  SSC Receive Frame Sync 2
AT91C_PB17_ERXDV          EQU (AT91C_PIO_PB17) ;-  Ethernet MAC Receive Data Valid
AT91C_PIO_PB18            EQU (1:SHL:18) ;- Pin Controlled by PB18
AT91C_PB18_RI1            EQU (AT91C_PIO_PB18) ;-  USART 1 Ring Indicator
AT91C_PB18_ECOL           EQU (AT91C_PIO_PB18) ;-  Ethernet MAC Collision Detected
AT91C_PIO_PB19            EQU (1:SHL:19) ;- Pin Controlled by PB19
AT91C_PB19_DTR1           EQU (AT91C_PIO_PB19) ;-  USART 1 Data Terminal ready
AT91C_PB19_ERXCK          EQU (AT91C_PIO_PB19) ;-  Ethernet MAC Receive Clock
AT91C_PIO_PB2             EQU (1:SHL:2) ;- Pin Controlled by PB2
AT91C_PB2_TD0             EQU (AT91C_PIO_PB2) ;-  SSC Transmit data
AT91C_PB2_SCK3            EQU (AT91C_PIO_PB2) ;-  USART 3 Serial Clock
AT91C_PIO_PB20            EQU (1:SHL:20) ;- Pin Controlled by PB20
AT91C_PB20_TXD1           EQU (AT91C_PIO_PB20) ;-  USART 1 Transmit Data
AT91C_PIO_PB21            EQU (1:SHL:21) ;- Pin Controlled by PB21
AT91C_PB21_RXD1           EQU (AT91C_PIO_PB21) ;-  USART 1 Receive Data
AT91C_PIO_PB22            EQU (1:SHL:22) ;- Pin Controlled by PB22
AT91C_PB22_SCK1           EQU (AT91C_PIO_PB22) ;-  USART1 Serial Clock
AT91C_PIO_PB23            EQU (1:SHL:23) ;- Pin Controlled by PB23
AT91C_PB23_DCD1           EQU (AT91C_PIO_PB23) ;-  USART 1 Data Carrier Detect
AT91C_PIO_PB24            EQU (1:SHL:24) ;- Pin Controlled by PB24
AT91C_PB24_CTS1           EQU (AT91C_PIO_PB24) ;-  USART 1 Clear To Send
AT91C_PIO_PB25            EQU (1:SHL:25) ;- Pin Controlled by PB25
AT91C_PB25_DSR1           EQU (AT91C_PIO_PB25) ;-  USART 1 Data Set ready
AT91C_PB25_EF100          EQU (AT91C_PIO_PB25) ;-  Ethernet MAC Force 100 Mbits/sec
AT91C_PIO_PB26            EQU (1:SHL:26) ;- Pin Controlled by PB26
AT91C_PB26_RTS1           EQU (AT91C_PIO_PB26) ;-  Usart 0 Ready To Send
AT91C_PIO_PB27            EQU (1:SHL:27) ;- Pin Controlled by PB27
AT91C_PB27_PCK0           EQU (AT91C_PIO_PB27) ;-  PMC Programmable Clock Output 0
AT91C_PIO_PB28            EQU (1:SHL:28) ;- Pin Controlled by PB28
AT91C_PB28_FIQ            EQU (AT91C_PIO_PB28) ;-  AIC Fast Interrupt Input
AT91C_PIO_PB29            EQU (1:SHL:29) ;- Pin Controlled by PB29
AT91C_PB29_IRQ0           EQU (AT91C_PIO_PB29) ;-  Interrupt input 0
AT91C_PIO_PB3             EQU (1:SHL:3) ;- Pin Controlled by PB3
AT91C_PB3_RD0             EQU (AT91C_PIO_PB3) ;-  SSC Receive Data
AT91C_PB3_MCDA1           EQU (AT91C_PIO_PB3) ;-  Multimedia Card A Data 1
AT91C_PIO_PB4             EQU (1:SHL:4) ;- Pin Controlled by PB4
AT91C_PB4_RK0             EQU (AT91C_PIO_PB4) ;-  SSC Receive Clock
AT91C_PB4_MCDA2           EQU (AT91C_PIO_PB4) ;-  Multimedia Card A Data 2
AT91C_PIO_PB5             EQU (1:SHL:5) ;- Pin Controlled by PB5
AT91C_PB5_RF0             EQU (AT91C_PIO_PB5) ;-  SSC Receive Frame Sync 0
AT91C_PB5_MCDA3           EQU (AT91C_PIO_PB5) ;-  Multimedia Card A Data 3
AT91C_PIO_PB6             EQU (1:SHL:6) ;- Pin Controlled by PB6
AT91C_PB6_TF1             EQU (AT91C_PIO_PB6) ;-  SSC Transmit Frame Sync 1
AT91C_PB6_TIOA3           EQU (AT91C_PIO_PB6) ;-  Timer Counter 4 Multipurpose Timer I/O Pin A
AT91C_PIO_PB7             EQU (1:SHL:7) ;- Pin Controlled by PB7
AT91C_PB7_TK1             EQU (AT91C_PIO_PB7) ;-  SSC Transmit Clock 1
AT91C_PB7_TIOB3           EQU (AT91C_PIO_PB7) ;-  Timer Counter 3 Multipurpose Timer I/O Pin B
AT91C_PIO_PB8             EQU (1:SHL:8) ;- Pin Controlled by PB8
AT91C_PB8_TD1             EQU (AT91C_PIO_PB8) ;-  SSC Transmit Data 1
AT91C_PB8_TIOA4           EQU (AT91C_PIO_PB8) ;-  Timer Counter 4 Multipurpose Timer I/O Pin A
AT91C_PIO_PB9             EQU (1:SHL:9) ;- Pin Controlled by PB9
AT91C_PB9_RD1             EQU (AT91C_PIO_PB9) ;-  SSC Receive Data 1
AT91C_PB9_TIOB4           EQU (AT91C_PIO_PB9) ;-  Timer Counter 4 Multipurpose Timer I/O Pin B
AT91C_PIO_PC0             EQU (1:SHL:0) ;- Pin Controlled by PC0
AT91C_PC0_BFCK            EQU (AT91C_PIO_PC0) ;-  Burst Flash Clock
AT91C_PIO_PC1             EQU (1:SHL:1) ;- Pin Controlled by PC1
AT91C_PC1_BFRDY_SMOE      EQU (AT91C_PIO_PC1) ;-  Burst Flash Ready
AT91C_PIO_PC10            EQU (1:SHL:10) ;- Pin Controlled by PC10
AT91C_PC10_NCS4_CFCS      EQU (AT91C_PIO_PC10) ;-  Compact Flash Chip Select
AT91C_PIO_PC11            EQU (1:SHL:11) ;- Pin Controlled by PC11
AT91C_PC11_NCS5_CFCE1     EQU (AT91C_PIO_PC11) ;-  Chip Select 5 / Compact Flash Chip Enable 1
AT91C_PIO_PC12            EQU (1:SHL:12) ;- Pin Controlled by PC12
AT91C_PC12_NCS6_CFCE2     EQU (AT91C_PIO_PC12) ;-  Chip Select 6 / Compact Flash Chip Enable 2
AT91C_PIO_PC13            EQU (1:SHL:13) ;- Pin Controlled by PC13
AT91C_PC13_NCS7           EQU (AT91C_PIO_PC13) ;-  Chip Select 7
AT91C_PIO_PC14            EQU (1:SHL:14) ;- Pin Controlled by PC14
AT91C_PIO_PC15            EQU (1:SHL:15) ;- Pin Controlled by PC15
AT91C_PIO_PC16            EQU (1:SHL:16) ;- Pin Controlled by PC16
AT91C_PC16_D16            EQU (AT91C_PIO_PC16) ;-  Data Bus [16]
AT91C_PIO_PC17            EQU (1:SHL:17) ;- Pin Controlled by PC17
AT91C_PC17_D17            EQU (AT91C_PIO_PC17) ;-  Data Bus [17]
AT91C_PIO_PC18            EQU (1:SHL:18) ;- Pin Controlled by PC18
AT91C_PC18_D18            EQU (AT91C_PIO_PC18) ;-  Data Bus [18]
AT91C_PIO_PC19            EQU (1:SHL:19) ;- Pin Controlled by PC19
AT91C_PC19_D19            EQU (AT91C_PIO_PC19) ;-  Data Bus [19]
AT91C_PIO_PC2             EQU (1:SHL:2) ;- Pin Controlled by PC2
AT91C_PC2_BFAVD           EQU (AT91C_PIO_PC2) ;-  Burst Flash Address Valid
AT91C_PIO_PC20            EQU (1:SHL:20) ;- Pin Controlled by PC20
AT91C_PC20_D20            EQU (AT91C_PIO_PC20) ;-  Data Bus [20]
AT91C_PIO_PC21            EQU (1:SHL:21) ;- Pin Controlled by PC21
AT91C_PC21_D21            EQU (AT91C_PIO_PC21) ;-  Data Bus [21]
AT91C_PIO_PC22            EQU (1:SHL:22) ;- Pin Controlled by PC22
AT91C_PC22_D22            EQU (AT91C_PIO_PC22) ;-  Data Bus [22]
AT91C_PIO_PC23            EQU (1:SHL:23) ;- Pin Controlled by PC23
AT91C_PC23_D23            EQU (AT91C_PIO_PC23) ;-  Data Bus [23]
AT91C_PIO_PC24            EQU (1:SHL:24) ;- Pin Controlled by PC24
AT91C_PC24_D24            EQU (AT91C_PIO_PC24) ;-  Data Bus [24]
AT91C_PIO_PC25            EQU (1:SHL:25) ;- Pin Controlled by PC25
AT91C_PC25_D25            EQU (AT91C_PIO_PC25) ;-  Data Bus [25]
AT91C_PIO_PC26            EQU (1:SHL:26) ;- Pin Controlled by PC26
AT91C_PC26_D26            EQU (AT91C_PIO_PC26) ;-  Data Bus [26]
AT91C_PIO_PC27            EQU (1:SHL:27) ;- Pin Controlled by PC27
AT91C_PC27_D27            EQU (AT91C_PIO_PC27) ;-  Data Bus [27]
AT91C_PIO_PC28            EQU (1:SHL:28) ;- Pin Controlled by PC28
AT91C_PC28_D28            EQU (AT91C_PIO_PC28) ;-  Data Bus [28]
AT91C_PIO_PC29            EQU (1:SHL:29) ;- Pin Controlled by PC29
AT91C_PC29_D29            EQU (AT91C_PIO_PC29) ;-  Data Bus [29]
AT91C_PIO_PC3             EQU (1:SHL:3) ;- Pin Controlled by PC3
AT91C_PC3_BFBAA_SMWE      EQU (AT91C_PIO_PC3) ;-  Burst Flash Address Advance / SmartMedia Write Enable
AT91C_PIO_PC30            EQU (1:SHL:30) ;- Pin Controlled by PC30
AT91C_PC30_D30            EQU (AT91C_PIO_PC30) ;-  Data Bus [30]
AT91C_PIO_PC31            EQU (1:SHL:31) ;- Pin Controlled by PC31
AT91C_PC31_D31            EQU (AT91C_PIO_PC31) ;-  Data Bus [31]
AT91C_PIO_PC4             EQU (1:SHL:4) ;- Pin Controlled by PC4
AT91C_PC4_BFOE            EQU (AT91C_PIO_PC4) ;-  Burst Flash Output Enable
AT91C_PIO_PC5             EQU (1:SHL:5) ;- Pin Controlled by PC5
AT91C_PC5_BFWE            EQU (AT91C_PIO_PC5) ;-  Burst Flash Write Enable
AT91C_PIO_PC6             EQU (1:SHL:6) ;- Pin Controlled by PC6
AT91C_PC6_NWAIT           EQU (AT91C_PIO_PC6) ;-  NWAIT
AT91C_PIO_PC7             EQU (1:SHL:7) ;- Pin Controlled by PC7
AT91C_PC7_A23             EQU (AT91C_PIO_PC7) ;-  Address Bus[23]
AT91C_PIO_PC8             EQU (1:SHL:8) ;- Pin Controlled by PC8
AT91C_PC8_A24             EQU (AT91C_PIO_PC8) ;-  Address Bus[24]
AT91C_PIO_PC9             EQU (1:SHL:9) ;- Pin Controlled by PC9
AT91C_PC9_A25_CFRNW       EQU (AT91C_PIO_PC9) ;-  Address Bus[25] /  Compact Flash Read Not Write
AT91C_PIO_PD0             EQU (1:SHL:0) ;- Pin Controlled by PD0
AT91C_PD0_ETX0            EQU (AT91C_PIO_PD0) ;-  Ethernet MAC Transmit Data 0
AT91C_PIO_PD1             EQU (1:SHL:1) ;- Pin Controlled by PD1
AT91C_PD1_ETX1            EQU (AT91C_PIO_PD1) ;-  Ethernet MAC Transmit Data 1
AT91C_PIO_PD10            EQU (1:SHL:10) ;- Pin Controlled by PD10
AT91C_PD10_PCK3           EQU (AT91C_PIO_PD10) ;-  PMC Programmable Clock Output 3
AT91C_PD10_TPS1           EQU (AT91C_PIO_PD10) ;-  ETM ARM9 pipeline status 1
AT91C_PIO_PD11            EQU (1:SHL:11) ;- Pin Controlled by PD11
AT91C_PD11_               EQU (AT91C_PIO_PD11) ;-  
AT91C_PD11_TPS2           EQU (AT91C_PIO_PD11) ;-  ETM ARM9 pipeline status 2
AT91C_PIO_PD12            EQU (1:SHL:12) ;- Pin Controlled by PD12
AT91C_PD12_               EQU (AT91C_PIO_PD12) ;-  
AT91C_PD12_TPK0           EQU (AT91C_PIO_PD12) ;-  ETM Trace Packet 0
AT91C_PIO_PD13            EQU (1:SHL:13) ;- Pin Controlled by PD13
AT91C_PD13_               EQU (AT91C_PIO_PD13) ;-  
AT91C_PD13_TPK1           EQU (AT91C_PIO_PD13) ;-  ETM Trace Packet 1
AT91C_PIO_PD14            EQU (1:SHL:14) ;- Pin Controlled by PD14
AT91C_PD14_               EQU (AT91C_PIO_PD14) ;-  
AT91C_PD14_TPK2           EQU (AT91C_PIO_PD14) ;-  ETM Trace Packet 2
AT91C_PIO_PD15            EQU (1:SHL:15) ;- Pin Controlled by PD15
AT91C_PD15_TD0            EQU (AT91C_PIO_PD15) ;-  SSC Transmit data
AT91C_PD15_TPK3           EQU (AT91C_PIO_PD15) ;-  ETM Trace Packet 3
AT91C_PIO_PD16            EQU (1:SHL:16) ;- Pin Controlled by PD16
AT91C_PD16_TD1            EQU (AT91C_PIO_PD16) ;-  SSC Transmit Data 1
AT91C_PD16_TPK4           EQU (AT91C_PIO_PD16) ;-  ETM Trace Packet 4
AT91C_PIO_PD17            EQU (1:SHL:17) ;- Pin Controlled by PD17
AT91C_PD17_TD2            EQU (AT91C_PIO_PD17) ;-  SSC Transmit Data 2
AT91C_PD17_TPK5           EQU (AT91C_PIO_PD17) ;-  ETM Trace Packet 5
AT91C_PIO_PD18            EQU (1:SHL:18) ;- Pin Controlled by PD18
AT91C_PD18_NPCS1          EQU (AT91C_PIO_PD18) ;-  SPI Peripheral Chip Select 1
AT91C_PD18_TPK6           EQU (AT91C_PIO_PD18) ;-  ETM Trace Packet 6
AT91C_PIO_PD19            EQU (1:SHL:19) ;- Pin Controlled by PD19
AT91C_PD19_NPCS2          EQU (AT91C_PIO_PD19) ;-  SPI Peripheral Chip Select 2
AT91C_PD19_TPK7           EQU (AT91C_PIO_PD19) ;-  ETM Trace Packet 7
AT91C_PIO_PD2             EQU (1:SHL:2) ;- Pin Controlled by PD2
AT91C_PD2_ETX2            EQU (AT91C_PIO_PD2) ;-  Ethernet MAC Transmit Data 2
AT91C_PIO_PD20            EQU (1:SHL:20) ;- Pin Controlled by PD20
AT91C_PD20_NPCS3          EQU (AT91C_PIO_PD20) ;-  SPI Peripheral Chip Select 3
AT91C_PD20_TPK8           EQU (AT91C_PIO_PD20) ;-  ETM Trace Packet 8
AT91C_PIO_PD21            EQU (1:SHL:21) ;- Pin Controlled by PD21
AT91C_PD21_RTS0           EQU (AT91C_PIO_PD21) ;-  Usart 0 Ready To Send
AT91C_PD21_TPK9           EQU (AT91C_PIO_PD21) ;-  ETM Trace Packet 9
AT91C_PIO_PD22            EQU (1:SHL:22) ;- Pin Controlled by PD22
AT91C_PD22_RTS1           EQU (AT91C_PIO_PD22) ;-  Usart 0 Ready To Send
AT91C_PD22_TPK10          EQU (AT91C_PIO_PD22) ;-  ETM Trace Packet 10
AT91C_PIO_PD23            EQU (1:SHL:23) ;- Pin Controlled by PD23
AT91C_PD23_RTS2           EQU (AT91C_PIO_PD23) ;-  USART 2 Ready To Send
AT91C_PD23_TPK11          EQU (AT91C_PIO_PD23) ;-  ETM Trace Packet 11
AT91C_PIO_PD24            EQU (1:SHL:24) ;- Pin Controlled by PD24
AT91C_PD24_RTS3           EQU (AT91C_PIO_PD24) ;-  USART 3 Ready To Send
AT91C_PD24_TPK12          EQU (AT91C_PIO_PD24) ;-  ETM Trace Packet 12
AT91C_PIO_PD25            EQU (1:SHL:25) ;- Pin Controlled by PD25
AT91C_PD25_DTR1           EQU (AT91C_PIO_PD25) ;-  USART 1 Data Terminal ready
AT91C_PD25_TPK13          EQU (AT91C_PIO_PD25) ;-  ETM Trace Packet 13
AT91C_PIO_PD26            EQU (1:SHL:26) ;- Pin Controlled by PD26
AT91C_PD26_TPK14          EQU (AT91C_PIO_PD26) ;-  ETM Trace Packet 14
AT91C_PIO_PD27            EQU (1:SHL:27) ;- Pin Controlled by PD27
AT91C_PD27_TPK15          EQU (AT91C_PIO_PD27) ;-  ETM Trace Packet 15
AT91C_PIO_PD3             EQU (1:SHL:3) ;- Pin Controlled by PD3
AT91C_PD3_ETX3            EQU (AT91C_PIO_PD3) ;-  Ethernet MAC Transmit Data 3
AT91C_PIO_PD4             EQU (1:SHL:4) ;- Pin Controlled by PD4
AT91C_PD4_ETXEN           EQU (AT91C_PIO_PD4) ;-  Ethernet MAC Transmit Enable
AT91C_PIO_PD5             EQU (1:SHL:5) ;- Pin Controlled by PD5
AT91C_PD5_ETXER           EQU (AT91C_PIO_PD5) ;-  Ethernet MAC Transmikt Coding Error
AT91C_PIO_PD6             EQU (1:SHL:6) ;- Pin Controlled by PD6
AT91C_PD6_DTXD            EQU (AT91C_PIO_PD6) ;-  DBGU Debug Transmit Data
AT91C_PIO_PD7             EQU (1:SHL:7) ;- Pin Controlled by PD7
AT91C_PD7_PCK0            EQU (AT91C_PIO_PD7) ;-  PMC Programmable Clock Output 0
AT91C_PD7_TSYNC           EQU (AT91C_PIO_PD7) ;-  ETM Synchronization signal
AT91C_PIO_PD8             EQU (1:SHL:8) ;- Pin Controlled by PD8
AT91C_PD8_PCK1            EQU (AT91C_PIO_PD8) ;-  PMC Programmable Clock Output 1
AT91C_PD8_TCLK            EQU (AT91C_PIO_PD8) ;-  ETM Trace Clock signal
AT91C_PIO_PD9             EQU (1:SHL:9) ;- Pin Controlled by PD9
AT91C_PD9_PCK2            EQU (AT91C_PIO_PD9) ;-  PMC Programmable Clock 2
AT91C_PD9_TPS0            EQU (AT91C_PIO_PD9) ;-  ETM ARM9 pipeline status 0

;- *****************************************************************************
;-               PERIPHERAL ID DEFINITIONS FOR AT91RM9200
;- *****************************************************************************
AT91C_ID_FIQ              EQU ( 0) ;- Advanced Interrupt Controller (FIQ)
AT91C_ID_SYS              EQU ( 1) ;- System Peripheral
AT91C_ID_PIOA             EQU ( 2) ;- Parallel IO Controller A 
AT91C_ID_PIOB             EQU ( 3) ;- Parallel IO Controller B
AT91C_ID_PIOC             EQU ( 4) ;- Parallel IO Controller C
AT91C_ID_PIOD             EQU ( 5) ;- Parallel IO Controller D
AT91C_ID_US0              EQU ( 6) ;- USART 0
AT91C_ID_US1              EQU ( 7) ;- USART 1
AT91C_ID_US2              EQU ( 8) ;- USART 2
AT91C_ID_US3              EQU ( 9) ;- USART 3
AT91C_ID_MCI              EQU (10) ;- Multimedia Card Interface
AT91C_ID_UDP              EQU (11) ;- USB Device Port
AT91C_ID_TWI              EQU (12) ;- Two-Wire Interface
AT91C_ID_SPI              EQU (13) ;- Serial Peripheral Interface
AT91C_ID_SSC0             EQU (14) ;- Serial Synchronous Controller 0
AT91C_ID_SSC1             EQU (15) ;- Serial Synchronous Controller 1
AT91C_ID_SSC2             EQU (16) ;- Serial Synchronous Controller 2
AT91C_ID_TC0              EQU (17) ;- Timer Counter 0
AT91C_ID_TC1              EQU (18) ;- Timer Counter 1
AT91C_ID_TC2              EQU (19) ;- Timer Counter 2
AT91C_ID_TC3              EQU (20) ;- Timer Counter 3
AT91C_ID_TC4              EQU (21) ;- Timer Counter 4
AT91C_ID_TC5              EQU (22) ;- Timer Counter 5
AT91C_ID_UHP              EQU (23) ;- USB Host port
AT91C_ID_EMAC             EQU (24) ;- Ethernet MAC
AT91C_ID_IRQ0             EQU (25) ;- Advanced Interrupt Controller (IRQ0)
AT91C_ID_IRQ1             EQU (26) ;- Advanced Interrupt Controller (IRQ1)
AT91C_ID_IRQ2             EQU (27) ;- Advanced Interrupt Controller (IRQ2)
AT91C_ID_IRQ3             EQU (28) ;- Advanced Interrupt Controller (IRQ3)
AT91C_ID_IRQ4             EQU (29) ;- Advanced Interrupt Controller (IRQ4)
AT91C_ID_IRQ5             EQU (30) ;- Advanced Interrupt Controller (IRQ5)
AT91C_ID_IRQ6             EQU (31) ;- Advanced Interrupt Controller (IRQ6)

;- *****************************************************************************
;-               BASE ADDRESS DEFINITIONS FOR AT91RM9200
;- *****************************************************************************
AT91C_BASE_SYS            EQU (0xFFFFF000) ;- (SYS) Base Address
AT91C_BASE_MC             EQU (0xFFFFFF00) ;- (MC) Base Address
AT91C_BASE_RTC            EQU (0xFFFFFE00) ;- (RTC) Base Address
AT91C_BASE_ST             EQU (0xFFFFFD00) ;- (ST) Base Address
AT91C_BASE_PMC            EQU (0xFFFFFC00) ;- (PMC) Base Address
AT91C_BASE_CKGR           EQU (0xFFFFFC20) ;- (CKGR) Base Address
AT91C_BASE_PIOD           EQU (0xFFFFFA00) ;- (PIOD) Base Address
AT91C_BASE_PIOC           EQU (0xFFFFF800) ;- (PIOC) Base Address
AT91C_BASE_PIOB           EQU (0xFFFFF600) ;- (PIOB) Base Address
AT91C_BASE_PIOA           EQU (0xFFFFF400) ;- (PIOA) Base Address
AT91C_BASE_DBGU           EQU (0xFFFFF200) ;- (DBGU) Base Address
AT91C_BASE_PDC_DBGU       EQU (0xFFFFF300) ;- (PDC_DBGU) Base Address
AT91C_BASE_AIC            EQU (0xFFFFF000) ;- (AIC) Base Address
AT91C_BASE_PDC_SPI        EQU (0xFFFE0100) ;- (PDC_SPI) Base Address
AT91C_BASE_SPI            EQU (0xFFFE0000) ;- (SPI) Base Address
AT91C_BASE_PDC_SSC2       EQU (0xFFFD8100) ;- (PDC_SSC2) Base Address
AT91C_BASE_SSC2           EQU (0xFFFD8000) ;- (SSC2) Base Address
AT91C_BASE_PDC_SSC1       EQU (0xFFFD4100) ;- (PDC_SSC1) Base Address
AT91C_BASE_SSC1           EQU (0xFFFD4000) ;- (SSC1) Base Address
AT91C_BASE_PDC_SSC0       EQU (0xFFFD0100) ;- (PDC_SSC0) Base Address
AT91C_BASE_SSC0           EQU (0xFFFD0000) ;- (SSC0) Base Address
AT91C_BASE_PDC_US3        EQU (0xFFFCC100) ;- (PDC_US3) Base Address
AT91C_BASE_US3            EQU (0xFFFCC000) ;- (US3) Base Address
AT91C_BASE_PDC_US2        EQU (0xFFFC8100) ;- (PDC_US2) Base Address
AT91C_BASE_US2            EQU (0xFFFC8000) ;- (US2) Base Address
AT91C_BASE_PDC_US1        EQU (0xFFFC4100) ;- (PDC_US1) Base Address
AT91C_BASE_US1            EQU (0xFFFC4000) ;- (US1) Base Address
AT91C_BASE_PDC_US0        EQU (0xFFFC0100) ;- (PDC_US0) Base Address
AT91C_BASE_US0            EQU (0xFFFC0000) ;- (US0) Base Address
AT91C_BASE_TWI            EQU (0xFFFB8000) ;- (TWI) Base Address
AT91C_BASE_PDC_MCI        EQU (0xFFFB4100) ;- (PDC_MCI) Base Address
AT91C_BASE_MCI            EQU (0xFFFB4000) ;- (MCI) Base Address
AT91C_BASE_UDP            EQU (0xFFFB0000) ;- (UDP) Base Address
AT91C_BASE_TC5            EQU (0xFFFA4080) ;- (TC5) Base Address
AT91C_BASE_TC4            EQU (0xFFFA4040) ;- (TC4) Base Address
AT91C_BASE_TC3            EQU (0xFFFA4000) ;- (TC3) Base Address
AT91C_BASE_TCB1           EQU (0xFFFA4080) ;- (TCB1) Base Address
AT91C_BASE_TC2            EQU (0xFFFA0080) ;- (TC2) Base Address
AT91C_BASE_TC1            EQU (0xFFFA0040) ;- (TC1) Base Address
AT91C_BASE_TC0            EQU (0xFFFA0000) ;- (TC0) Base Address
AT91C_BASE_TCB0           EQU (0xFFFA0000) ;- (TCB0) Base Address
AT91C_BASE_UHP            EQU (0x00300000) ;- (UHP) Base Address
AT91C_BASE_EMAC           EQU (0xFFFBC000) ;- (EMAC) Base Address
AT91C_BASE_EBI            EQU (0xFFFFFF60) ;- (EBI) Base Address
AT91C_BASE_SMC2           EQU (0xFFFFFF70) ;- (SMC2) Base Address
AT91C_BASE_SDRC           EQU (0xFFFFFF90) ;- (SDRC) Base Address
AT91C_BASE_BFC            EQU (0xFFFFFFC0) ;- (BFC) Base Address

;- *****************************************************************************
;-               MEMORY MAPPING DEFINITIONS FOR AT91RM9200
;- *****************************************************************************
AT91C_ISRAM               EQU (0x00200000) ;- Internal SRAM base address
AT91C_ISRAM_SIZE          EQU (0x00004000) ;- Internal SRAM size in byte (16 Kbyte)
AT91C_IROM                EQU (0x00100000) ;- Internal ROM base address
AT91C_IROM_SIZE           EQU (0x00020000) ;- Internal ROM size in byte (128 Kbyte)


	END
