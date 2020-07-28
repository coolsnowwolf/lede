/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * Purpose : RTL8367C switch low-level function for access register
 * Feature : SMI related functions
 *
 */


#include <rtk_types.h>
#include <smi.h>
#include "rtk_error.h"


#if defined(MDC_MDIO_OPERATION)
/*******************************************************************************/
/*  MDC/MDIO porting                                                           */
/*******************************************************************************/
/* define the PHY ID currently used */
/* carlos */
#if 0
#define MDC_MDIO_PHY_ID     0  /* PHY ID 0 or 29 */
#else
#define MDC_MDIO_PHY_ID     29  /* PHY ID 0 or 29 */
#endif

/* MDC/MDIO, redefine/implement the following Macro */ /*carlos*/
#if 0
#define MDC_MDIO_WRITE(preamableLength, phyID, regID, data)
#define MDC_MDIO_READ(preamableLength, phyID, regID, pData)
#else
#define u32      unsigned int
extern u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
extern u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

#define MDC_MDIO_WRITE(preamableLength, phyID, regID, data) mii_mgr_write(phyID, regID, data)
#define MDC_MDIO_READ(preamableLength, phyID, regID, pData) mii_mgr_read(phyID, regID, pData)
#endif





#elif defined(SPI_OPERATION)
/*******************************************************************************/
/*  SPI porting                                                                */
/*******************************************************************************/
/* SPI, redefine/implement the following Macro */
#define SPI_WRITE(data, length)
#define SPI_READ(pData, length)





#else
/*******************************************************************************/
/*  I2C porting                                                                */
/*******************************************************************************/
/* Define the GPIO ID for SCK & SDA */
rtk_uint32  smi_SCK = 1;    /* GPIO used for SMI Clock Generation */
rtk_uint32  smi_SDA = 2;    /* GPIO used for SMI Data signal */

/* I2C, redefine/implement the following Macro */
#define GPIO_DIRECTION_SET(gpioID, direction)
#define GPIO_DATA_SET(gpioID, data)
#define GPIO_DATA_GET(gpioID, pData)





#endif

static void rtlglue_drvMutexLock(void)
{
    /* It is empty currently. Implement this function if Lock/Unlock function is needed */
    return;
}

static void rtlglue_drvMutexUnlock(void)
{
    /* It is empty currently. Implement this function if Lock/Unlock function is needed */
    return;
}



#if defined(MDC_MDIO_OPERATION) || defined(SPI_OPERATION)
    /* No local function in MDC/MDIO & SPI mode */
#else
static void _smi_start(void)
{

    /* change GPIO pin to Output only */
    GPIO_DIRECTION_SET(smi_SCK, GPIO_DIR_OUT);
    GPIO_DIRECTION_SET(smi_SDA, GPIO_DIR_OUT);

    /* Initial state: SCK: 0, SDA: 1 */
    GPIO_DATA_SET(smi_SCK, 0);
    GPIO_DATA_SET(smi_SDA, 1);
    CLK_DURATION(DELAY);

    /* CLK 1: 0 -> 1, 1 -> 0 */
    GPIO_DATA_SET(smi_SCK, 1);
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SCK, 0);
    CLK_DURATION(DELAY);

    /* CLK 2: */
    GPIO_DATA_SET(smi_SCK, 1);
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SDA, 0);
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SCK, 0);
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SDA, 1);

}



static void _smi_writeBit(rtk_uint16 signal, rtk_uint32 bitLen)
{
    for( ; bitLen > 0; bitLen--)
    {
        CLK_DURATION(DELAY);

        /* prepare data */
        if ( signal & (1<<(bitLen-1)) )
        {
            GPIO_DATA_SET(smi_SDA, 1);
        }
        else
        {
            GPIO_DATA_SET(smi_SDA, 0);
        }
        CLK_DURATION(DELAY);

        /* clocking */
        GPIO_DATA_SET(smi_SCK, 1);
        CLK_DURATION(DELAY);
        GPIO_DATA_SET(smi_SCK, 0);
    }
}



static void _smi_readBit(rtk_uint32 bitLen, rtk_uint32 *rData)
{
    rtk_uint32 u = 0;

    /* change GPIO pin to Input only */
    GPIO_DIRECTION_SET(smi_SDA, GPIO_DIR_IN);

    for (*rData = 0; bitLen > 0; bitLen--)
    {
        CLK_DURATION(DELAY);

        /* clocking */
        GPIO_DATA_SET(smi_SCK, 1);
        CLK_DURATION(DELAY);
        GPIO_DATA_GET(smi_SDA, &u);
        GPIO_DATA_SET(smi_SCK, 0);

        *rData |= (u << (bitLen - 1));
    }

    /* change GPIO pin to Output only */
    GPIO_DIRECTION_SET(smi_SDA, GPIO_DIR_OUT);
}



static void _smi_stop(void)
{

    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SDA, 0);
    GPIO_DATA_SET(smi_SCK, 1);
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SDA, 1);
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SCK, 1);
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SCK, 0);
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SCK, 1);

    /* add a click */
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SCK, 0);
    CLK_DURATION(DELAY);
    GPIO_DATA_SET(smi_SCK, 1);


    /* change GPIO pin to Input only */
    GPIO_DIRECTION_SET(smi_SDA, GPIO_DIR_IN);
    GPIO_DIRECTION_SET(smi_SCK, GPIO_DIR_IN);
}

#endif /* End of #if defined(MDC_MDIO_OPERATION) || defined(SPI_OPERATION) */

rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData)
{
#if (!defined(MDC_MDIO_OPERATION) && !defined(SPI_OPERATION))
    rtk_uint32 rawData=0, ACK;
    rtk_uint8  con;
    rtk_uint32 ret = RT_ERR_OK;
#endif

    if(mAddrs > 0xFFFF)
        return RT_ERR_INPUT;

    if(rData == NULL)
        return RT_ERR_NULL_POINTER;

#if defined(MDC_MDIO_OPERATION)

    /* Lock */
    rtlglue_drvMutexLock();

    /* Write address control code to register 31 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_PHY_ID, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);

    /* Write address to register 23 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_PHY_ID, MDC_MDIO_ADDRESS_REG, mAddrs);

    /* Write read control code to register 21 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_PHY_ID, MDC_MDIO_CTRL1_REG, MDC_MDIO_READ_OP);

    /* Read data from register 25 */
    MDC_MDIO_READ(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_PHY_ID, MDC_MDIO_DATA_READ_REG, rData);

    /* Unlock */
    rtlglue_drvMutexUnlock();

    return RT_ERR_OK;

#elif defined(SPI_OPERATION)

    /* Lock */
    rtlglue_drvMutexLock();

    /* Write 8 bits READ OP_CODE */
    SPI_WRITE(SPI_READ_OP, SPI_READ_OP_LEN);

    /* Write 16 bits register address */
    SPI_WRITE(mAddrs, SPI_REG_LEN);

    /* Read 16 bits data */
    SPI_READ(rData, SPI_DATA_LEN);

    /* Unlock */
    rtlglue_drvMutexUnlock();

    return RT_ERR_OK;

#else

    /*Disable CPU interrupt to ensure that the SMI operation is atomic.
      The API is based on RTL865X, rewrite the API if porting to other platform.*/
    rtlglue_drvMutexLock();

    _smi_start();                                /* Start SMI */

    _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370 */

    _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

    _smi_writeBit(0x1, 1);                        /* 1: issue READ command */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for issuing READ command*/
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK by RTL8369 */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_readBit(8, &rawData);                    /* Read DATA [7:0] */
    *rData = rawData&0xff;

    _smi_writeBit(0x00, 1);                        /* ACK by CPU */

    _smi_readBit(8, &rawData);                    /* Read DATA [15: 8] */

    _smi_writeBit(0x01, 1);                        /* ACK by CPU */
    *rData |= (rawData<<8);

    _smi_stop();

    rtlglue_drvMutexUnlock();/*enable CPU interrupt*/

    return ret;
#endif /* end of #if defined(MDC_MDIO_OPERATION) */
}



rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData)
{
#if (!defined(MDC_MDIO_OPERATION) && !defined(SPI_OPERATION))
    rtk_int8 con;
    rtk_uint32 ACK;
    rtk_uint32 ret = RT_ERR_OK;
#endif

    if(mAddrs > 0xFFFF)
        return RT_ERR_INPUT;

    if(rData > 0xFFFF)
        return RT_ERR_INPUT;

#if defined(MDC_MDIO_OPERATION)

    /* Lock */
    rtlglue_drvMutexLock();

    /* Write address control code to register 31 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_PHY_ID, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);

    /* Write address to register 23 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_PHY_ID, MDC_MDIO_ADDRESS_REG, mAddrs);

    /* Write data to register 24 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_PHY_ID, MDC_MDIO_DATA_WRITE_REG, rData);

    /* Write data control code to register 21 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_PHY_ID, MDC_MDIO_CTRL1_REG, MDC_MDIO_WRITE_OP);

    /* Unlock */
    rtlglue_drvMutexUnlock();

    return RT_ERR_OK;

#elif defined(SPI_OPERATION)

    /* Lock */
    rtlglue_drvMutexLock();

    /* Write 8 bits WRITE OP_CODE */
    SPI_WRITE(SPI_WRITE_OP, SPI_WRITE_OP_LEN);

    /* Write 16 bits register address */
    SPI_WRITE(mAddrs, SPI_REG_LEN);

    /* Write 16 bits data */
    SPI_WRITE(rData, SPI_DATA_LEN);

    /* Unlock */
    rtlglue_drvMutexUnlock();

    return RT_ERR_OK;
#else

    /*Disable CPU interrupt to ensure that the SMI operation is atomic.
      The API is based on RTL865X, rewrite the API if porting to other platform.*/
    rtlglue_drvMutexLock();

    _smi_start();                                /* Start SMI */

    _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370*/

    _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

    _smi_writeBit(0x0, 1);                        /* 0: issue WRITE command */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for issuing WRITE command*/
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit(rData&0xff, 8);                /* Write Data [7:0] out */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for writting data [7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit(rData>>8, 8);                    /* Write Data [15:8] out */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                        /* ACK for writting data [15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_stop();

    rtlglue_drvMutexUnlock();/*enable CPU interrupt*/

    return ret;
#endif /* end of #if defined(MDC_MDIO_OPERATION) */
}

