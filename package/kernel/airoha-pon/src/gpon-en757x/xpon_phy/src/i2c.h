#ifndef _I2C_H_
#define _I2C_H_

#define CONFIG_I2C_BASE_ADDR				0x1FBF0300
#define CONFIG_I2C_IRQ						17

/*************************************************************************
*************************************************************************/
#define I2C_CSR_DATA_PORT					(base+0x0000)
#define I2C_CSR_SLAVE_ADDR					(base+0x0004)
#define I2C_CSR_INTR_MASK					(base+0x0008)
#define I2C_CSR_INTR_STAT					(base+0x000c)
#define I2C_CSR_CONTROL						(base+0x0010)
#define I2C_CSR_TRANSFER_LEN				(base+0x0014)
#define I2C_CSR_TRANSAC_LEN					(base+0x0018)
#define I2C_CSR_DELAY_LEN					(base+0x001c)
#define I2C_CSR_TIMING						(base+0x0020)
#define I2C_CSR_START						(base+0x0024)
#define I2C_CSR_FIFO_STAT					(base+0x0030)
#define I2C_CSR_FIFO_THRESH					(base+0x0034)
#define I2C_CSR_FIFO_ADDR_CLR				(base+0x0038)
#define I2C_CSR_IO_CONFIG					(base+0x0040)
#define I2C_CSR_DEBUG						(base+0x0044)
#define I2C_CSR_HS							(base+0x0048)
#define I2C_CSR_SOFTRESET					(base+0x0050)
#define I2C_CSR_IRQSEL						(base+0x0060)
#define I2C_CSR_DEBUGSTAT					(base+0x0064)
#define I2C_CSR_DEBUGCTRL					(base+0x0068)
                                        	
                                        	
#define I2C_INT_DEBUG						(1 << 3)
#define I2C_INT_HS_NACKERR					(1 << 2)
#define I2C_INT_ACKERR						(1 << 1)
#define I2C_INT_COMPLETE					(1 << 0)
                                        	
                                        	
                                        	
#define I2C_START_TRANSAC       			{ \
												mb() ; \
												iowrite32(0x1,(void __iomem *)I2C_CSR_START) ; \
											}
#define I2C_FIFO_CLR_ADDR       			iowrite32(0x1,(void __iomem *)I2C_CSR_FIFO_ADDR_CLR)
                                        	
#define I2C_SET_BITS(BS,REG)				((*(volatile uint*)(REG)) |= (uint)(BS))
#define I2C_CLR_BITS(BS,REG)				((*(volatile uint*)(REG)) &= ~((uint)(BS)))
                                        	
                                        	
#define I2C_TX_THR_OFFSET					8
#define I2C_RX_THR_OFFSET					0
#define I2C_SET_FIFO_THRESH(tx,rx)			{ \
   												uint val = (((tx) & 0x7) << I2C_TX_THR_OFFSET) | (((rx) & 0x7) << I2C_RX_THR_OFFSET) ; \
        										iowrite32(val, (void __iomem *)I2C_CSR_FIFO_THRESH) ; \
   											} 
                                        	
#define I2C_SET_INTR_MASK(mask) 			I2C_SET_BITS(mask, I2C_CSR_INTR_MASK)
#define I2C_CLR_INTR_MASK(mask)				I2C_CLR_BITS(mask, I2C_CSR_INTR_MASK)
                                        	
                                        	
#define I2C_SET_SLAVE_ADDR(addr)        	iowrite32((addr)&0xFF, (void __iomem *)I2C_CSR_SLAVE_ADDR)
                                        	
#define I2C_CSR_TRANS_LEN_MASK          	 (0xff)
#define I2C_CSR_TRANS_AUX_LEN_MASK      	 (0x1f << 8)
#define I2C_SET_TRANS_LEN(len)				{ \
												uint val = ioread32((void __iomem *)I2C_CSR_TRANSFER_LEN) & ~(I2C_CSR_TRANS_LEN_MASK); \
        										val |= ((len) & I2C_CSR_TRANS_LEN_MASK) ; \
        										iowrite32(val, (void __iomem *)I2C_CSR_TRANSFER_LEN) ; \
    										}
                                        	
#define I2C_SET_TRANS_AUX_LEN(len)			{ \
												uint val = ioread32((void __iomem *)I2C_CSR_TRANSFER_LEN) & ~(I2C_CSR_TRANS_AUX_LEN_MASK); \
							        			val |= (((len) << 8) & I2C_CSR_TRANS_AUX_LEN_MASK); \
							        			iowrite32(val, (void __iomem *)I2C_CSR_TRANSFER_LEN); \
    										}
                                        	
#define I2C_SET_TRANSAC_LEN(len)			iowrite32(len, (void __iomem *)I2C_CSR_TRANSAC_LEN)
#define I2C_SET_TRANS_DELAY(delay)			iowrite32(delay, (void __iomem *)I2C_CSR_DELAY_LEN)
                                        	
#define I2C_CSR_CONTROL_MASK				(0x3f << 1)
#define I2C_SET_TRANS_CTRL(ctrl)			{ \
												uint val = ioread32((void __iomem *)I2C_CSR_CONTROL) & ~I2C_CSR_CONTROL_MASK; \
												val |= ((ctrl) & I2C_CSR_CONTROL_MASK); \
												iowrite32(val, (void __iomem *)I2C_CSR_CONTROL) ; \
											}
                                        	
#define I2C_SET_HS_MODE(on_off)				{ \
												uint val = ioread32((void __iomem *)I2C_CSR_HS) & ~0x1; \
												val |= (on_off & 0x1); \
												iowrite32(val, (void __iomem *)I2C_CSR_HS) ; \
    										}

#define I2C_READ_BYTE()						ioread32((void __iomem *)I2C_CSR_DATA_PORT)
#define I2C_WRITE_BYTE(byte)				iowrite32(byte, (void __iomem *)I2C_CSR_DATA_PORT)
#define I2C_CLR_INTR_STATUS(status)			iowrite32(status, (void __iomem *)I2C_CSR_INTR_STAT)
#define I2C_INTR_STATUS 					ioread32((void __iomem *)I2C_CSR_INTR_STAT)

/* mt i2c control bits */
#define CONTROL_TRANS_LEN_CHANGE			(1 << 6)
#define CONTROL_ACKERR_DET_EN				(1 << 5)
#define CONTROL_DIR_CHANGE					(1 << 4)
#define CONTROL_CLK_EXT						(1 << 3)
#define	CONTROL_DMA_EN						(1 << 2)
#define	CONTROL_RS_FLAG						(1 << 1)




int i2c_init(void) ;
void i2c_exit(void) ;

uint ioReadPhyReg32(ushort phyReg) ;
int ioWritePhyReg32(ushort phyReg, uint value) ;
int ioWritePhyReg8(ushort phyReg, unchar value) ;



#endif /* _I2C_H_ */




