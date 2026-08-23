typedef union
{
    struct
    {
        uint32_t resv1     : 5;        //bit 0:4
	    uint32_t work_mode : 1;        //bit 5
	    uint32_t tx_mode   : 3;        //bit 6:8
	    uint32_t resv2     : 2;        //bit 9:10
	    uint32_t data_bit  : 5;        //bit 11:15
	    uint32_t bclk_cycle_num : 5;   //bit 16:20
	    uint32_t resv3     : 2;        //bit 21:22
	    uint32_t ch_num    : 4;        //bit 23:26
	    uint32_t resv4     : 5;        //bit 27:31
	} bits;
	uint32_t word;
	
} ETDM_CFG0;

typedef union
{
    struct
	{
        uint32_t initial_count   : 5;  //bit 0:4
        uint32_t initial_point   : 5;  //bit 5:9
        uint32_t lrck_auto_off   : 1;  //bit 10
        uint32_t bck_auto_off    : 1;  //bit 11
        uint32_t chen_sel        : 1;  //bit 12
        uint32_t resv1           : 6;  //bit 13:18		
        uint32_t lr_align        : 1;  //bit 19
        uint32_t lrck_width      : 9;  //bit 20:28
        uint32_t lrck_auto_mode  : 1;  //bit 29	
        uint32_t master_BCLK_gen : 1;  //bit 30
        uint32_t sixteenb_compact_mode: 1;  //bit 31
    } bits;
	uint32_t word;
	
} ETDM_CFG1;

typedef union
{
    struct
	{
        uint32_t ch_num      : 5;     //bit 0:4
        uint32_t hd_audio_on : 1;     //bit 5
	    uint32_t resv1       : 2;     //bit 6:7
        uint32_t axi_minLen  : 4;     //bit 8:11
        uint32_t axi_maxLen  : 4;     //bit 12:15
		uint32_t pbuf_size   : 2;     //bit 16:17
        uint32_t resv2       : 14;    //bit 18:31
    } bits;
    uint32_t word;
	
} AFE_DL1_CFG0;

typedef union
{
    struct
	{
        uint32_t resv1             : 5;  //bit 0:4
        uint32_t hd_audio_on       : 1;  //bit 5
	    uint32_t resv2             : 2;  //bit 6:7
        uint32_t axi_minLen        : 4;  //bit 8:11
        uint32_t axi_maxLen        : 4;  //bit 12:15
		uint32_t odd_use_even      : 1;  //bit 16
        uint32_t resv3             : 7;  //bit 17:23
        uint32_t sw_clear_buf_full : 1;  //bit 24
        uint32_t force_no_mask     : 1;  //bit 25	
        uint32_t resv4             : 6;  //bit 26:31			
    } bits;
    uint32_t word;
	
} AFE_UL1_CFG0;

typedef union
{
    struct
    {
        uint32_t enable        : 1;   //bit 0	
		uint32_t resv1         : 3;   //bit 1:3
        uint32_t int_requester : 1;   //bit 4
        uint32_t resv2         : 27;  //bit 5:31
    } bits;
	uint32_t word;

} AFE_IRQ_CFG0;

extern struct device* get_i2s_dev(void);

extern int get_i2s_irq(void);
extern void I2sLoopbackTest_HwTrig_Control(uint8_t isEnable);
extern uint32_t get_afe_irq_mon(uint8_t irq_reg_num, uint8_t mon_num);
extern uint32_t get_irq_ctrl_status(uint8_t irq_reg_index);

extern void i2s_write_afe_dl1_base(dma_addr_t afe_dl1_base_addr);
extern void i2s_write_afe_dl_end(dma_addr_t afe_dl1_end_addr);
extern void i2s_write_afe_ul1_base(dma_addr_t afe_ul1_base_addr);
extern void i2s_write_afe_ul1_end(dma_addr_t afe_ul1_end_addr);

extern void etdm_in1_slave_sel(uint8_t etdmSrc);
extern void etdm_in1_sdata0_sel(uint8_t etdmSdataSrc);

extern void set_irq_count(uint32_t irq_count, uint8_t irq_idx);
extern uint32_t get_irq_count(uint8_t irq_idx);

extern void I2S_Etdm_Cfg0(ETDM_CFG0 in1, ETDM_CFG0 out1);
extern void I2S_Etdm_Cfg1(ETDM_CFG1 in1, ETDM_CFG1 out1);
extern void I2S_AFE_Cfg0_DL1(AFE_DL1_CFG0 dl1);
extern void I2S_AFE_Cfg0_UL1(AFE_UL1_CFG0 ul1);
extern void I2S_Afe_Irq_Cfg0(AFE_IRQ_CFG0 afe_irq, uint8_t irq_reg_sel);