/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : RTL8373 switch high-level API for RTL8373
 * Feature :
 *
 */

#include <rtl8373_asicdrv.h>

#if defined(RTK_X86_ASICDRV)
#include <I2Clib.h>
#else
#include <rtl8373_smi.h>
#endif

/*for driver verify testing only*/
#ifdef CONFIG_RTL8373_ASICDRV_TEST
#define CLE_VIRTUAL_REG_SIZE        0x10000
rtk_uint16 CleVirtualReg[CLE_VIRTUAL_REG_SIZE];
#endif

#if defined(CONFIG_RTL865X_CLE) || defined (RTK_X86_CLE)
rtk_uint32 cleDebuggingDisplay;
#endif

#ifdef EMBEDDED_SUPPORT
extern void setReg(rtk_uint16, rtk_uint16);
extern rtk_uint16 getReg(rtk_uint16);
#endif

/* Function Name:
 *      rtl8373_setAsicRegBit
 * Description:
 *      Set a bit value of a specified register
 * Input:
 *      reg     - register's address
 *      offset     - offset location
 *      value   - value to set. It can be value 0 or 1.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      Set a bit of a specified register to 1 or 0.
 */
ret_t rtl8373_setAsicRegBit(rtk_uint32 reg, rtk_uint32 offset, rtk_uint32 value)
{

#if defined(RTK_X86_ASICDRV)
    rtk_uint32 regData;
    ret_t retVal;

    if(offset >= RTL8373_REGBITLENGTH)
        return RT_ERR_INPUT;

    retVal = Access_Read(reg, 4, &regData);
    if(TRUE != retVal)
        return RT_ERR_SMI;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n", reg, regData);

    if(value)
        regData = regData | (1 << offset);
    else
        regData = regData & (~(1 << offset));

    retVal = Access_Write(reg,4, regData);
    if(TRUE != retVal)
        return RT_ERR_SMI;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n", reg, regData);


#elif defined(CONFIG_RTL8373_ASICDRV_TEST)

    if(offset >= RTL8373_REGBITLENGTH)
        return RT_ERR_INPUT;

    else if(reg >= CLE_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    if(value)
    {
        CleVirtualReg[reg] =  CleVirtualReg[reg] | (1 << offset);
    }
    else
    {
        CleVirtualReg[reg] =  CleVirtualReg[reg] & (~(1 << offset));
    }

    if(0x8367B == cleDebuggingDisplay)
        PRINT("W[0x%8.8x]=0x%8.8x\n", reg, CleVirtualReg[reg]);

#elif defined(EMBEDDED_SUPPORT)
    rtk_uint16 tmp;

    if(reg > RTL8373_REGDATAMAX || value > 1)
        return RT_ERR_INPUT;

    tmp = getReg(reg);
    tmp &= (1 << offset);
    tmp |= (value << offset);
    setReg(reg, tmp);

#else
    rtk_uint32 regData;
    ret_t retVal;


    if(offset >= RTL8373_REGBITLENGTH)
        return RT_ERR_INPUT;

    retVal = rtl8373_smi_read(reg, &regData);
    if(retVal != RT_ERR_OK)
        return RT_ERR_SMI;

  #ifdef CONFIG_RTL865X_CLE
    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%8.8x]=0x%8.8x\n", reg, regData);
  #endif
    if(value)
        regData = regData | (1 << offset);
    else
        regData = regData & (~(1 << offset));

    retVal = rtl8373_smi_write(reg, regData);
    if(retVal != RT_ERR_OK)
        return RT_ERR_SMI;

  #ifdef CONFIG_RTL865X_CLE
    if(0x8367B == cleDebuggingDisplay)
        PRINT("W[0x%8.8x]=0x%8.8x\n", reg, regData);
  #endif

#endif

    //PRINT("W[0x%8.8x]=0x%8.8x\n", reg, regData);
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8373_getAsicRegBit
 * Description:
 *      Get a bit value of a specified register
 * Input:
 *      reg     - register's address
 *      offset     - bit location
 *      value   - value to get.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      None
 */
ret_t rtl8373_getAsicRegBit(rtk_uint32 reg, rtk_uint32 offset, rtk_uint32 *pValue)
{

#if defined(RTK_X86_ASICDRV)

    rtk_uint32 regData;
    ret_t retVal;

    if(offset >= RTL8373_REGBITLENGTH)
        return RT_ERR_INPUT;

    retVal = Access_Read(reg, 4, &regData);
    if(TRUE != retVal)
        return RT_ERR_SMI;

    //*pValue = (regData & (0x1 << offset)) >> offset;
    *pValue = ((regData  >> offset ) & 0x1);
    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%8.8x]=0x%8.8x\n", reg, regData);

#elif defined(CONFIG_RTL8373_ASICDRV_TEST)

    if(bit >= RTL8373_REGBITLENGTH)
        return RT_ERR_INPUT;

    if(reg >= CLE_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    *pValue = (CleVirtualReg[reg] & (0x1 << offset)) >> offset;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%8.8x]=0x%8.8x\n", reg, CleVirtualReg[reg]);

#elif defined(EMBEDDED_SUPPORT)
    rtk_uint16 tmp;

    if(reg > RTL8373_REGDATAMAX )
        return RT_ERR_INPUT;

    tmp = getReg(reg);
    tmp = tmp >> offset;
    tmp &= 1;
    *pValue = tmp;
#else
    rtk_uint32 regData;
    ret_t retVal;

    retVal = rtl8373_smi_read(reg, &regData);
    if(retVal != RT_ERR_OK)
        return RT_ERR_SMI;

  #ifdef CONFIG_RTL865X_CLE
    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%8.8x]=0x%8.8x\n", reg, regData);
  #endif

    *pValue = (regData & (0x1 << offset)) >> offset;

#endif

    //PRINT("R[0x%8.8x]=0x%8.8x\n", reg, regData);
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8373_setAsicRegBits
 * Description:
 *      Set bits value of a specified register
 * Input:
 *      reg     - register's address
 *      bits    - bits mask for setting
 *      value   - bits value for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      Set bits of a specified register to value. Both bits and value are be treated as bit-mask
 */
ret_t rtl8373_setAsicRegBits(rtk_uint32 reg, rtk_uint32 bitsMask, rtk_uint32 value)
{

#if defined(RTK_X86_ASICDRV)

    rtk_uint32 regData;
    ret_t retVal;
    rtk_uint32 bitsShift;
    rtk_uint32 valueShifted;

    if( !bitsMask )
           return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    valueShifted = value << bitsShift;
    if(valueShifted > RTL8373_REGDATAMAX)
        return RT_ERR_INPUT;

    retVal = Access_Read(reg, 4, &regData);
    if(TRUE != retVal)
        return RT_ERR_SMI;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n", reg, regData);

    regData = regData & (~bitsMask);
    regData = regData | (valueShifted & bitsMask);

    retVal = Access_Write(reg,4, regData);
    if(TRUE != retVal)
        return RT_ERR_SMI;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n", reg, regData);

#elif defined(CONFIG_RTL8373_ASICDRV_TEST)
    rtk_uint32 regData;
    rtk_uint32 bitsShift;
    rtk_uint32 valueShifted;

    if(!bitsMask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    valueShifted = value << bitsShift;

    if(valueShifted > RTL8373_REGDATAMAX)
        return RT_ERR_INPUT;

    if(reg >= CLE_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    regData = CleVirtualReg[reg] & (~bitsMask);
    regData = regData | (valueShifted & bitsMask);

    CleVirtualReg[reg] = regData;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n", reg, regData);

#elif defined(EMBEDDED_SUPPORT)
    rtk_uint32 regData;
    rtk_uint32 bitsShift;
    rtk_uint32 valueShifted;

    if(reg > RTL8373_REGDATAMAX )
        return RT_ERR_INPUT;

    if(bitsMask >= (1 << RTL8373_REGBITLENGTH) )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    valueShifted = value << bitsShift;
    if(valueShifted > RTL8373_REGDATAMAX)
        return RT_ERR_INPUT;

    regData = getReg(reg);
    regData = regData & (~bitsMask);
    regData = regData | (valueShifted & bitsMask);

    setReg(reg, regData);

#else
    rtk_uint32 regData;
    ret_t retVal;
    rtk_uint32 bitsShift;
    rtk_uint32 valueShifted;

    if( !bitsMask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }
    valueShifted = value << bitsShift;

    if(valueShifted > RTL8373_REGDATAMAX)
        return RT_ERR_INPUT;

    retVal = rtl8373_smi_read(reg, &regData);
    if(retVal != RT_ERR_OK)
        return RT_ERR_SMI;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n", reg, regData);
  #endif

    regData = regData & (~bitsMask);
    regData = regData | (valueShifted & bitsMask);

    retVal = rtl8373_smi_write(reg, regData);
    if(retVal != RT_ERR_OK)
        return RT_ERR_SMI;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8367B == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n", reg, regData);
  #endif
#endif

    //PRINT("W[0x%4.4x]=0x%4.4x\n", reg, regData);
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8373_getAsicRegBits
 * Description:
 *      Get bits value of a specified register
 * Input:
 *      reg     - register's address
 *      bits    - bits mask for setting
 *      value   - bits value for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      None
 */
ret_t rtl8373_getAsicRegBits(rtk_uint32 reg, rtk_uint32 bitsMask, rtk_uint32 *pValue)
{

#if defined(RTK_X86_ASICDRV)

    rtk_uint32 regData;
    ret_t retVal;
    rtk_uint32 bitsShift;

    if( !bitsMask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    retVal = Access_Read(reg, 4, &regData);
    if(TRUE != retVal)
        return RT_ERR_SMI;

    *pValue = (regData & bitsMask) >> bitsShift;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n", reg, regData);

#elif defined(CONFIG_RTL8373_ASICDRV_TEST)
    rtk_uint32 bitsShift;

    if( !bitsMask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    if(reg >= CLE_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

     *pValue = (CleVirtualReg[reg] & bitsMask) >> bitsShift;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n", reg, CleVirtualReg[reg]);

#elif defined(EMBEDDED_SUPPORT)
    rtk_uint32 regData;
    rtk_uint32 bitsShift;

    if(reg > RTL8373_REGDATAMAX )
        return RT_ERR_INPUT;

    if(bitsMask >= (1UL << RTL8373_REGBITLENGTH) )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1UL << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    regData = getReg(reg);
    *value = (regData & bitsMask) >> bitsShift;

#else
    rtk_uint32 regData;
    ret_t retVal;
    rtk_uint32 bitsShift;

    if( !bitsMask )
        return RT_ERR_INPUT;

    bitsShift = 0;
    while(!(bitsMask & (1 << bitsShift)))
    {
        bitsShift++;
        if(bitsShift >= RTL8373_REGBITLENGTH)
            return RT_ERR_INPUT;
    }

    retVal = rtl8373_smi_read(reg, &regData);
    if(retVal != RT_ERR_OK) return RT_ERR_SMI;

    *pValue = (regData & bitsMask) >> bitsShift;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n",reg, regData);
  #endif

#endif

    //PRINT("R[0x%4.4x]=0x%4.4x\n",reg, regData);
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8373_setAsicReg
 * Description:
 *      Set content of asic register
 * Input:
 *      reg     - register's address
 *      value   - Value setting to register
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      The value will be set to ASIC mapping address only and it is always return RT_ERR_OK while setting un-mapping address registers
 */
ret_t rtl8373_setAsicReg(rtk_uint32 reg, rtk_uint32 value)
{
#if defined(RTK_X86_ASICDRV)/*RTK-CNSD2-NickWu-20061222: for x86 compile*/

    ret_t retVal;

    retVal = Access_Write(reg,4,value);
    if(TRUE != retVal) 
        return RT_ERR_SMI;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("W[0x%8.8x]=0x%8.8x\n",reg,value);

#elif defined(CONFIG_RTL8373_ASICDRV_TEST)
    #if 0
    /*MIBs emulating*/
    if(reg == RTL8373_REG_MIB_ADDRESS)
    {
        CleVirtualReg[RTL8373_REG_MIB_COUNTER0] = 0x1;
        CleVirtualReg[RTL8373_REG_MIB_COUNTER0+1] = 0x2;
        CleVirtualReg[RTL8373_REG_MIB_COUNTER0+2] = 0x3;
        CleVirtualReg[RTL8373_REG_MIB_COUNTER0+3] = 0x4;
    }
    #endif
    if(reg >= CLE_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    CleVirtualReg[reg] = value;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("W[0x%8.8x]=0x%8.8x\n",reg,CleVirtualReg[reg]);

#elif defined(EMBEDDED_SUPPORT)
    if(reg > RTL8373_REGDATAMAX || value > RTL8373_REGDATAMAX )
        return RT_ERR_INPUT;

    setReg(reg, value);

#else
    ret_t retVal;

    retVal = rtl8373_smi_write(reg, value);
    if(retVal != RT_ERR_OK)
        return RT_ERR_SMI;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8367B == cleDebuggingDisplay)
        PRINT("W[0x%4.4x]=0x%4.4x\n",reg,value);
  #endif

#endif

    //PRINT("W[0x%4.4x]=0x%4.4x\n",reg,value);
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8373_getAsicReg
 * Description:
 *      Get content of asic register
 * Input:
 *      reg     - register's address
 *      value   - Value setting to register
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      Value 0x0000 will be returned for ASIC un-mapping address
 */
ret_t rtl8373_getAsicReg(rtk_uint32 reg, rtk_uint32 *pValue)
{

#if defined(RTK_X86_ASICDRV)

    rtk_uint32 regData;
    ret_t retVal;

    retVal = Access_Read(reg, 4, &regData);
    
    if(TRUE != retVal)
        return RT_ERR_SMI;

    *pValue = regData;

    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%4.4x]=0x%4.4x\n", reg, regData);

#elif defined(CONFIG_RTL8373_ASICDRV_TEST)
    if(reg >= CLE_VIRTUAL_REG_SIZE)
        return RT_ERR_OUT_OF_RANGE;

    *pValue = CleVirtualReg[reg];

    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%8.8x]=0x%8.8x\n", reg, CleVirtualReg[reg]);

#elif defined(EMBEDDED_SUPPORT)
    if(reg > RTL8373_REGDATAMAX  )
        return RT_ERR_INPUT;

    *value = getReg(reg);

#else
    rtk_uint32 regData;
    ret_t retVal;

    retVal = rtl8373_smi_read(reg, &regData);
    if(retVal != RT_ERR_OK)
        return RT_ERR_SMI;

    *pValue = regData;
  #ifdef CONFIG_RTL865X_CLE
    if(0x8367B == cleDebuggingDisplay)
        PRINT("R[0x%8.8x]=0x%8.8x\n", reg, regData);
  #endif

#endif

    //PRINT("R[0x%8.8x]=0x%8.8x\n", reg, regData);
    return RT_ERR_OK;
}

