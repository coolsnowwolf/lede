/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 63932 $
 * $Date: 2015-12-08 14:06:29 +0800 (周二, 08 十二月 2015) $
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in i2c module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <port.h>
#include <string.h>
#include <rtl8367c_reg.h>

#include <rtl8367c_asicdrv_i2c.h>
#include <rtk_switch.h>
#include <rtl8367c_asicdrv.h>
#include <rtk_types.h>
#include <i2c.h>


static rtk_I2C_16bit_mode_t rtk_i2c_mode = I2C_LSB_16BIT_MODE;


/* Function Name:
 *      rtk_i2c_init
 * Description:
 *      I2C smart function initialization.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API is used to initialize EEE status.
 *      need used GPIO pins
 *      OpenDrain and clock
 */
rtk_api_ret_t rtk_i2c_init(void)
{
    rtk_uint32 retVal;
  switch_chip_t ChipID;
  /* probe switch */
  if((retVal = rtk_switch_probe(&ChipID)) != RT_ERR_OK)
      return retVal;

  if( ChipID == CHIP_RTL8370B )
  {
   /*set GPIO8, GPIO9, OpenDrain as I2C, clock = 252KHZ   */
      if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_M_I2C_SYS_CTL, 0x5c3f)) != RT_ERR_OK)
        return retVal;
  }
  else
      return RT_ERR_FAILED;
  return  RT_ERR_OK;
}


/* Function Name:
 *      rtk_i2c_mode_set
 * Description:
 *      Set I2C data byte-order.
 * Input:
 *      i2cmode - byte-order mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      This API can set I2c traffic's byte-order .
 */
rtk_api_ret_t rtk_i2c_mode_set( rtk_I2C_16bit_mode_t i2cmode )
{
    if(i2cmode >= I2C_Mode_END)
    {
        return RT_ERR_INPUT;
    }
    else if(i2cmode == I2C_70B_LSB_16BIT_MODE)
    {
        rtk_i2c_mode = I2C_70B_LSB_16BIT_MODE;

        return RT_ERR_OK;
    }
    else if( i2cmode == I2C_LSB_16BIT_MODE)
    {
        rtk_i2c_mode = I2C_LSB_16BIT_MODE;
        return RT_ERR_OK;
    }
    else
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_i2c_mode_get
 * Description:
 *      Get i2c traffic byte-order setting.
 * Input:
 *      None
 * Output:
 *      pI2cMode - i2c byte-order
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *      The API can get i2c traffic byte-order setting.
 */
rtk_api_ret_t rtk_i2c_mode_get( rtk_I2C_16bit_mode_t * pI2cMode)
{
    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    if(NULL == pI2cMode)
        return RT_ERR_NULL_POINTER;
    if(rtk_i2c_mode == I2C_70B_LSB_16BIT_MODE)
        *pI2cMode = 1;
    else if ((rtk_i2c_mode == I2C_LSB_16BIT_MODE))
        *pI2cMode = 0;
    else
        return RT_ERR_FAILED;
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_i2c_gpioPinGroup_set
 * Description:
 *      Set i2c SDA & SCL used GPIO pins group.
 * Input:
 *      pins_group - GPIO pins group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      The API can set i2c used gpio pins group.
 *      There are three group pins could be used
 */
rtk_api_ret_t rtk_i2c_gpioPinGroup_set( rtk_I2C_gpio_pin_t pins_group )
{
    rtk_uint32 retVal;


    if( ( pins_group > I2C_GPIO_PIN_END )|| ( pins_group < I2C_GPIO_PIN_8_9) )
        return RT_ERR_INPUT;

    if( (retVal = rtl8367c_setAsicI2CGpioPinGroup(pins_group) ) != RT_ERR_OK )
        return retVal ;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_i2c_gpioPinGroup_get
 * Description:
 *      Get i2c SDA & SCL used GPIO pins group.
 * Input:
 *      None
 * Output:
 *      pPins_group - GPIO pins group
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *      The API can get i2c used gpio pins group.
 *      There are three group pins could be used
 */
rtk_api_ret_t rtk_i2c_gpioPinGroup_get( rtk_I2C_gpio_pin_t * pPins_group )
{
    rtk_uint32 retVal;
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPins_group)
        return RT_ERR_NULL_POINTER;
    if( (retVal = rtl8367c_getAsicI2CGpioPinGroup(pPins_group) ) != RT_ERR_OK )
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_i2c_data_read
 * Description:
 *      read i2c slave device register.
 * Input:
 *      deviceAddr   -   access Slave device address
 *      slaveRegAddr -   access Slave register address
 * Output:
 *      pRegData     -   read data
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_NULL_POINTER     - input parameter is null pointer
 * Note:
 *      The API can access i2c slave and read i2c slave device register.
 */
rtk_api_ret_t rtk_i2c_data_read(rtk_uint8 deviceAddr, rtk_uint32 slaveRegAddr, rtk_uint32 *pRegData)
{
     rtk_uint32 retVal, counter=0;
     rtk_uint8 controlByte_W, controlByte_R;
     rtk_uint8 slaveRegAddr_L, slaveRegAddr_H = 0x0, temp;
     rtk_uint8 regData_L, regData_H;

   /* control byte :deviceAddress + W,  deviceAddress + R   */
    controlByte_W = (rtk_uint8)(deviceAddr << 1) ;
    controlByte_R = (rtk_uint8)(controlByte_W | 0x1);

    slaveRegAddr_L = (rtk_uint8) (slaveRegAddr & 0x00FF) ;
    slaveRegAddr_H = (rtk_uint8) (slaveRegAddr >>8) ;

    if( rtk_i2c_mode == I2C_70B_LSB_16BIT_MODE)
    {
        temp = slaveRegAddr_L ;
        slaveRegAddr_L = slaveRegAddr_H;
        slaveRegAddr_H = temp;
    }


  /*check bus state: idle*/
  for(counter = 3000; counter>0; counter--)
  {
    if ( (retVal = rtl8367c_setAsicI2C_checkBusIdle() ) == RT_ERR_OK)
         break;
  }
  if( counter ==0 )
      return retVal; /*i2c is busy*/

   /*tx Start cmd*/
   if( (retVal = rtl8367c_setAsicI2CStartCmd() ) != RT_ERR_OK )
       return retVal ;


  /*tx control _W*/
   if( (retVal = rtl8367c_setAsicI2CTxOneCharCmd(controlByte_W))!= RT_ERR_OK )
      return retVal ;


  /*check if RX ack from slave*/
   if( (retVal = rtl8367c_setAsicI2CcheckRxAck()) != RT_ERR_OK )
        return retVal;

    /* tx slave buffer address low 8 bits */
   if( (retVal = rtl8367c_setAsicI2CTxOneCharCmd(slaveRegAddr_L))!= RT_ERR_OK )
         return retVal  ;

   /*check if RX ack from slave*/
   if( (retVal = rtl8367c_setAsicI2CcheckRxAck()) != RT_ERR_OK )
        return retVal;



        /* tx slave buffer address high 8 bits */
   if( (retVal = rtl8367c_setAsicI2CTxOneCharCmd(slaveRegAddr_H))!= RT_ERR_OK )
         return retVal  ;


   /*check if RX ack from slave*/
   if( (retVal = rtl8367c_setAsicI2CcheckRxAck()) != RT_ERR_OK )
        return retVal;


   /*tx Start cmd*/
   if( (retVal = rtl8367c_setAsicI2CStartCmd() ) != RT_ERR_OK )
       return retVal ;

      /*tx control _R*/
   if( (retVal = rtl8367c_setAsicI2CTxOneCharCmd(controlByte_R))!= RT_ERR_OK )
       return retVal ;


  /*check if RX ack from slave*/
   if( (retVal = rtl8367c_setAsicI2CcheckRxAck()) != RT_ERR_OK )
        return retVal;


    /* rx low 8bit data*/
   if( ( retVal = rtl8367c_setAsicI2CRxOneCharCmd( &regData_L) ) != RT_ERR_OK )
        return retVal;



    /* tx ack to slave, keep receive */
    if( (retVal = rtl8367c_setAsicI2CTxAckCmd()) != RT_ERR_OK )
        return retVal;

     /* rx high 8bit data*/
    if( ( retVal = rtl8367c_setAsicI2CRxOneCharCmd( &regData_H) ) != RT_ERR_OK )
        return retVal;



    /* tx Noack to slave, Stop receive */
     if( (retVal = rtl8367c_setAsicI2CTxNoAckCmd()) != RT_ERR_OK )
        return retVal;


    /*tx Stop cmd */
    if( (retVal = rtl8367c_setAsicI2CStopCmd()) != RT_ERR_OK )
        return retVal;

    *pRegData = (regData_H << 8) | regData_L;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_i2c_data_write
 * Description:
 *      write data to i2c slave device register
 * Input:
 *      deviceAddr   -   access Slave device address
 *      slaveRegAddr -   access Slave register address
 *      regData      -   data to set
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 * Note:
 *      The API can access i2c slave and setting i2c slave device register.
 */
rtk_api_ret_t rtk_i2c_data_write(rtk_uint8 deviceAddr, rtk_uint32 slaveRegAddr, rtk_uint32 regData)
{
     rtk_uint32 retVal,counter;
     rtk_uint8 controlByte_W;
     rtk_uint8 slaveRegAddr_L, slaveRegAddr_H = 0x0, temp;
     rtk_uint8 regData_L, regData_H;

  /* control byte :deviceAddress + W    */
    controlByte_W = (rtk_uint8)(deviceAddr<< 1) ;

    slaveRegAddr_L = (rtk_uint8) (slaveRegAddr & 0x00FF) ;
    slaveRegAddr_H = (rtk_uint8) (slaveRegAddr >>8) ;

    regData_H   = (rtk_uint8) (regData>> 8);
    regData_L   = (rtk_uint8) (regData & 0x00FF);

    if( rtk_i2c_mode == I2C_70B_LSB_16BIT_MODE)
    {
        temp = slaveRegAddr_L ;
        slaveRegAddr_L = slaveRegAddr_H;
        slaveRegAddr_H = temp;
    }


  /*check bus state: idle*/
  for(counter = 3000; counter>0; counter--)
  {
    if ( (retVal = rtl8367c_setAsicI2C_checkBusIdle() ) == RT_ERR_OK)
        break;
  }

  if( counter ==0 )
      return retVal; /*i2c is busy*/


   /*tx Start cmd*/
   if( (retVal = rtl8367c_setAsicI2CStartCmd() ) != RT_ERR_OK )
       return retVal ;


  /*tx control _W*/
   if( (retVal = rtl8367c_setAsicI2CTxOneCharCmd(controlByte_W))!= RT_ERR_OK )
      return retVal ;


  /*check if RX ack from slave*/
   if( (retVal = rtl8367c_setAsicI2CcheckRxAck()) != RT_ERR_OK )
        return retVal;


    /* tx slave buffer address low 8 bits */
   if( (retVal = rtl8367c_setAsicI2CTxOneCharCmd(slaveRegAddr_L))!= RT_ERR_OK )
        return retVal;


   /*check if RX ack from slave*/
   if( (retVal = rtl8367c_setAsicI2CcheckRxAck()) != RT_ERR_OK )
        return retVal;


   /* tx slave buffer address high 8 bits */
   if( (retVal = rtl8367c_setAsicI2CTxOneCharCmd(slaveRegAddr_H))!= RT_ERR_OK )
        return retVal;


   /*check if RX ack from slave*/
   if( (retVal = rtl8367c_setAsicI2CcheckRxAck()) != RT_ERR_OK )
        return retVal;


     /*tx Datavlue LSB*/
   if( (retVal = rtl8367c_setAsicI2CTxOneCharCmd(regData_L))!= RT_ERR_OK )
        return retVal;


   /*check if RX ack from slave*/
   if( (retVal = rtl8367c_setAsicI2CcheckRxAck()) != RT_ERR_OK )
        return retVal;


   /*tx Datavlue MSB*/
   if( (retVal = rtl8367c_setAsicI2CTxOneCharCmd(regData_H))!= RT_ERR_OK )
        return retVal;


   /*check if RX ack from slave*/
   if( (retVal = rtl8367c_setAsicI2CcheckRxAck()) != RT_ERR_OK )
        return retVal;


    /*tx Stop cmd */
    if( (retVal = rtl8367c_setAsicI2CStopCmd()) != RT_ERR_OK )
        return retVal;

    return RT_ERR_OK;
}



