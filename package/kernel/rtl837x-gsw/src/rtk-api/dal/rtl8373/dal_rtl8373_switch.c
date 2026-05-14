/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of Switch Global API
 *
 * Feature : The file have include the following module and sub-modules
 * 		  (1) Switch parameter settings
 *
 */

/*
 * Include Files
 */
#include <dal/rtl8373/dal_rtl8373_switch.h>
#include <dal/rtl8373/rtl8373_asicdrv.h>
#include <dal_rtl8373_drv.h>
#include <dal_rtl8373_port.h>
#include <string.h>

/*
 * Symbol Definition
 */
#ifndef ENABLE
#define ENABLE 1
#endif

#ifndef DISABLE
#define DISABLE 0
#endif

#define PHY_PATCH 1
#define SDS_FIBER_FC_EN (1)
#define SDS_NWAY_EN (1)

static rtk_uint32 spd_8221_last = 0xff;

/*
 * Data Declaration
 */
rtk_uint32 currentVersion, FWVersion;
rtk_uint32 Ver8373_72 = 0xf;
rtk_uint32 Ver8224 = 0xf;
rtk_uint32 Ver8366U = 0xf;
rtk_uint32 Ver8372N = 0xf;
rtk_uint32 Ver8373N = 0xf;
rtk_uint32 Ver8224N = 0xf;

chip_mode_type_t CHIP_TYPE;

rtk_uint16 an_10p3125g_chipa[][3] ={
    {0x21 ,0x10, 0x4480}, {0x21, 0x13 ,0x0400}, {0x21, 0x18, 0x6d02}, {0x21 ,0x1b,0x424e} ,
    {0x21, 0x1d, 0x0002}, {0x36, 0x1c ,0x1390} ,{0x2e ,0x4 ,0x0080} ,{0x2e ,0x6  ,0x0408} ,
    {0x2e, 0x7,  0x020d}, {0x2e, 0x9  ,0x0601}, {0x2e ,0xb, 0x222c}, {0x2e, 0xc, 0x9217} ,
    {0x2e ,0x16, 0x0743}, {0x2e, 0x1d  ,0xabb0} 
};

rtk_uint16 an_10p3125g_chipb[][3] ={
    {0x21 ,0x10, 0x4480}, {0x21, 0x13 ,0x0400}, {0x21, 0x18, 0x6d02}, {0x21 ,0x1b,0x424e} ,
    {0x21, 0x1d, 0x0002}, {0x36, 0x1c ,0x1390} ,{0x36 ,0x14 ,0x003f} ,{0x36 ,0x10,0x0200} ,
    {0x2e, 0x4,  0x0080}, {0x2e, 0x6  ,0x0408}, {0x2e ,0x7, 0x020d}, {0x2e, 0x9, 0x0601} ,
    {0x2e ,0xb, 0x222c},  {0x2e, 0x0c  ,0xa217}, {0x2e, 0x0d, 0xfe40},{0x2e, 0x15, 0xf5c1}, 
    {0x2e, 0x16, 0x0443}, {0x2e, 0x1d, 0xabb0}
};



rtk_uint16  an_3p125g_chipa[][3]= {
    {0x21, 0x10, 0x6480}, {0x21, 0x13 ,0x400 }, {0x21, 0x18 ,0x6d02}, {0x21 ,0x1b ,0x424e},
    {0x21 ,0x1d ,0x2   } ,{0x36 ,0x1c ,0x1390}, {0x28, 0x4  ,0x0080}, {0x28, 0x9  ,0x601 },
    {0x28 ,0xb , 0x232c} ,{0x28, 0xc,  0x9217} ,{0x28 ,0xf,	0x5b50}, {0x28, 0x16, 0x443 },
    {0x28, 0x1d, 0xabb0}
};

rtk_uint16  an_3p125g_chipb[][3]= {
    {0x21, 0x10, 0x6480}, {0x21, 0x13 ,0x400 }, {0x21, 0x18 ,0x6d02}, {0x21 ,0x1b ,0x424e},
    {0x21 ,0x1d ,0x2   }, {0x36 ,0x1c ,0x1390}, {0x36, 0x14, 0x003F}, {0x36, 0x10, 0x0200},
    {0x28, 0x4  ,0x0080}, {0x28, 0x7, 0x1201},  {0x28, 0x9  ,0x601 }, {0x28 ,0xb , 0x232c} ,
    {0x28, 0xc,	0x9217}, {0x28 ,0xf,  0x5b50}, {0x28, 0x15, 0xe7f1}, {0x28, 0x16, 0x443 },
    {0x28, 0x1d, 0xabb0}
};

rtk_uint16 an_1p25g_chipa[][3]= {
    {0x21, 0x10, 0x6480}, {0x21 ,0x13 ,0x0400}, {0x21 ,0x18 ,0x6d02}, {0x21, 0x1b ,0x424e},
    {0x21, 0x1d, 0x0002}, {0x36 ,0x1c, 0x1390} ,{0x36, 0x14, 0x003F}, {0x36, 0x10, 0x0300},
    {0x24 ,0x04 ,0x0080}, {0x24, 0x7, 0x1201},  {0x24 ,0x09, 0x0601}, {0x24 ,0x0b, 0x232c}, 
    {0x24, 0x0c ,0x9217}, {0x24 ,0x0f ,0x5B50} ,{0x24, 0x15, 0xe7c1}, {0x24 ,0x16, 0x0443}, 
    {0x24, 0x1d ,0xabb0}
};

rtk_uint16 an_1p25g_chipb[][3]= {
    {0x21, 0x10, 0x6480}, {0x21, 0x13, 0x0400},                       {0x21, 0x18, 0x6d02},
    {0x21, 0x1b, 0x424e}, {0x21, 0x1d, 0x0002}, {0x36, 0x1c, 0x1390}, {0x36, 0x14, 0x003F},
    {0x36, 0x10, 0x0300}, {0x24, 0x04, 0x0080}, {0x24, 0x07, 0x1201}, {0x24, 0x09, 0x0601},
    {0x24, 0x0b, 0x232c}, {0x24, 0x0c, 0x9217}, {0x24, 0x0f, 0x5b50}, {0x24, 0x15, 0xe7C1},
    {0x24, 0x16, 0x0443}, {0x24, 0x1d, 0xabb0}
};

rtk_uint16 an_125m_chipa[][3]= {
    {0x21, 0x10, 0x6480}, {0x21, 0x13, 0x0400}, {0x21, 0x18, 0x6d02}, {0x21, 0x1b, 0x424e},
    {0x21, 0x1d, 0x0002}, {0x36, 0x1c, 0x1390}, {0x26, 0x04, 0x0080}, {0x26, 0x09, 0x0601},
    {0x26, 0x0b, 0x232c}, {0x26, 0x0c, 0x9217}, {0x26, 0x0f, 0x5B50}, {0x26, 0x16, 0x0443},
    {0x26, 0x1d, 0xabb0} 
};

rtk_uint16 an_125m_chipb[][3]= {
    {0x21, 0x10, 0x6480}, {0x21, 0x13, 0x0400}, {0x21, 0x18, 0x6d02},
    {0x21, 0x1b, 0x424e}, {0x21, 0x1d, 0x0002}, {0x36, 0x1c, 0x1390}, {0x36, 0x14, 0x003F},
    {0x36, 0x10, 0x0300}, {0x26, 0x04, 0x0080}, {0x26, 0x07, 0x1201}, {0x26, 0x09, 0x0601},
    {0x26, 0x0b, 0x232c}, {0x26, 0x0c, 0x9217}, {0x26, 0x0f, 0x5b50}, {0x26, 0x15, 0xe7C1},
    {0x26, 0x16, 0x0443}, {0x26, 0x1d, 0xabb0}
};



rtk_uint16 dig_patch_mac[][3]=  {
    {6 , 18 ,0x5078}, {7,  6  ,0x9401}, {7 , 8 , 0x9401}, {7  ,10 ,0x9401},
    {7 , 12 ,0x9401}, {31 ,11 ,0x0003}, {6, 3, 0xc45c}, {6, 31, 0x2100}  
};
rtk_uint16 dig_patch_phy[][3]= {
    {6,	18 ,0x5078}, {6, 3, 0xc45c}, {6, 30, 0x000C}, {6, 31, 0x2100} 
};

rtk_uint16 patch_list[][2] =
{
    {0xC202, 0xC1},
    {0xC203, 0x01},
    {0xC204, 0xFF},
    {0xC205, 0x02},
    {0xC206, 0x6A},
    {0xC207, 0x02},
    {0xC208, 0xD9},
    {0xC209, 0x03},
    {0xC20A, 0x5C},
    {0xC20B, 0x03},
    {0xC20C, 0xF6},
    {0xC20D, 0x04},
    {0xC20E, 0x88},
    {0xC20F, 0x05},
    {0xC210, 0x40},
    {0xC211, 0x06},
    {0xC212, 0x3D},
    {0xC213, 0x07},
    {0xC214, 0x24},
    {0xC215, 0x08},
    {0xC216, 0x2B},
    {0xC217, 0x09},
    {0xC218, 0x84},
    {0xC219, 0x0B},
    {0xC21A, 0x41},
    {0xC21B, 0x0C},
    {0xC21C, 0x44},
    {0xC21D, 0x0E},
    {0xC21E, 0x00},
    {0xC21F, 0xF6},
    {0xC220, 0xF6},
    {0xC221, 0xF5},
    {0xC222, 0xF3},
    {0xC223, 0xF3},
    {0xC224, 0xEF},
    {0xC225, 0xEB},
    {0xC226, 0xE7},
    {0xC227, 0xE4},
    {0xC228, 0xE2},
    {0xC229, 0xDF},
    {0xC22A, 0xDD},
    {0xC22B, 0xDB},
    {0xC22C, 0xDA},
    {0xC22D, 0xD9},
    {0xC270, 0x18},
    {0xC271, 0x19},
    {0xC272, 0x1C},
    {0xC273, 0x1E},
    {0xC274, 0x20},
    {0xC275, 0x21},
    {0xC276, 0x22},
    {0xC277, 0x23},
    {0xC278, 0x23},
    {0xC279, 0x24},
    {0xC27A, 0x25},
    {0xC27B, 0x25},
    {0xC27C, 0x26},
    {0xC27D, 0x26},
    {0xC27E, 0x26},
    {0xC27F, 0x4A},
    {0xC280, 0x4A},
    {0xC281, 0x4A},
    {0xC282, 0x4D},
    {0xC283, 0x4D},
    {0xC284, 0x50},
    {0xC285, 0x55},
    {0xC286, 0x52},
    {0xC287, 0x52},
    {0xC288, 0x52},
    {0xC289, 0x52},
    {0xC28A, 0x52},
    {0xC28B, 0x51},
    {0xC28C, 0x52},
    {0xC28D, 0x52},
    {0xC201, 0x01},
    {0xC22E, 0xD7},
    {0xC26F, 0x18},
    {0xC28E, 0x52},
};

rtk_uint16 alg_tune_giga_220411_patch[][2] =
    {
        {0x80b5, 0x98},
        {0x80b6, 0xc3},
        {0x80b7, 0xf6},
        {0x80b8, 0x04},
        {0x80b9, 0xd2},
        {0x80ba, 0x06},
        {0x80bb, 0xfa},
        {0x80bc, 0xbc},
        {0x80bd, 0x66},
        {0x80be, 0x19},
        {0x80bf, 0x2f},

        {0x80c9, 0xc0},
        {0x80ca, 0xbb},
        {0x80cb, 0xf6},
        {0x80cc, 0x04},
        {0x80cd, 0x9e},
        {0x80ce, 0x06},
        {0x80cf, 0xfa},
        {0x80d0, 0xc4},
        {0x80d1, 0x63},
        {0x80d2, 0x19},
        {0x80d3, 0x2f},
        {0x80c0, 0x0c},
        {0x80d4, 0x0c}};

rtk_uint16 alg_tune_fnet_6818_220628_patch[][2] =
    {
        {0x80a1, 0x48},
        {0x80a2, 0x9c},
        {0x80a3, 0xb6},
        {0x80a4, 0x03},
        {0x80a5, 0xda},
        {0x80a6, 0x0a},
        {0x80a7, 0xea},
        {0x80a8, 0xa6},
        {0x80a9, 0xe3},
        {0x80aa, 0x1f},
        {0x80ab, 0x3b},
        {0x80ac, 0x1e}};

/*
 * Function Declaration
 */

void delay_loop(rtk_uint32 loop)
{
    rtk_uint32 i, j;

    for (i = 0; i < loop; ++i)
        for (j = 0; j < 40; j++)
            ;
}

void Pin_Reset_8224_via_8373(void)
{
    //******************** Begin to Reset RTL8224 by RTL8373_GPIO36 ***************************
    rtl8373_setAsicRegBit(RTL8373_GPIO_OUT1_ADDR, 4, 0);
    rtl8373_setAsicRegBit(RTL8373_GPIO_OE1_ADDR, 4, 1);

    delay_loop(100);
    rtl8373_setAsicRegBit(RTL8373_GPIO_OUT1_ADDR, 4, 1);
    delay_loop(500);
}

void uc1_sram_write_8b(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 val)
{

    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, addr);
    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xff << 8, val);
}

rtk_uint32 uc1_sram_read_8b(rtk_uint32 port, rtk_uint32 addr)
{
    rtk_uint32 regdata;

    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, addr);
    dal_rtl8373_phy_regbits_read(port, 31, 0xa438, 0xff << 8, &regdata);

    return regdata;
}

void uc2_sram_write_8b(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 val)
{

    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb87c, 0xffff, addr);
    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb87e, 0xff << 8, val);
}

rtk_uint32 uc2_sram_read_8b(rtk_uint32 port, rtk_uint32 addr)
{
    rtk_uint32 regdata;

    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb87c, 0xffff, addr);
    dal_rtl8373_phy_regbits_read(port, 31, 0xb87e, 0xff << 8, &regdata);

    return regdata;
}

void data_ram_write_8b(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 val)
{

    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb88e, 0xffff, addr);

    if (addr % 2)
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xB890, 0xff, val);
    else
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xB890, 0xff << 8, val);
}

rtk_uint32 uc_sram_read_16b(rtk_uint32 port, rtk_uint32 addr)
{
    rtk_uint32 val;

    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, addr);
    dal_rtl8373_phy_regbits_read(port, 31, 0xa438, 0xffff, &val);

    return val;
}

void uc_sram_write_16b(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 val)
{
    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, addr);
    dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, val);
}

rtk_uint16 n0_patch_6818B_230724_patch[][2]=
{
{0xa436, 0xA016}, {0xa438, 0x0000}, {0xa436, 0xA012}, {0xa438, 0x0000}, 
    {0xa436, 0xA014}, {0xa438, 0x1800}, {0xa438, 0x8010}, {0xa438, 0x1800}, 
    {0xa438, 0x8016}, {0xa438, 0x1800}, {0xa438, 0x8029}, {0xa438, 0x1800}, 
    {0xa438, 0x802d}, {0xa438, 0x1800}, {0xa438, 0x8031}, {0xa438, 0x1800}, 
    {0xa438, 0x8048}, {0xa438, 0x1800}, {0xa438, 0x8058}, {0xa438, 0x1800}, 
    {0xa438, 0x8068}, {0xa438, 0x8808}, {0xa438, 0xd711}, {0xa438, 0x404f}, 
    {0xa438, 0xa808}, {0xa438, 0x1800}, {0xa438, 0x1dff}, {0xa438, 0x416e}, 
    {0xa438, 0x1000}, {0xa438, 0x09cc}, {0xa438, 0x1000}, {0xa438, 0x0a9b}, 
    {0xa438, 0xd501}, {0xa438, 0xa103}, {0xa438, 0x8203}, {0xa438, 0xd500}, 
    {0xa438, 0x1800}, {0xa438, 0x0188}, {0xa438, 0x1000}, {0xa438, 0x0aa1}, 
    {0xa438, 0xd501}, {0xa438, 0x8103}, {0xa438, 0xa203}, {0xa438, 0xd500}, 
    {0xa438, 0x1800}, {0xa438, 0x0188}, {0xa438, 0xac03}, {0xa438, 0xa138}, 
    {0xa438, 0x1800}, {0xa438, 0x1ded}, {0xa438, 0xac03}, {0xa438, 0xa138}, 
    {0xa438, 0x1800}, {0xa438, 0x1cb8}, {0xa438, 0xd504}, {0xa438, 0xaa18}, 
    {0xa438, 0xd500}, {0xa438, 0x1000}, {0xa438, 0x14e4}, {0xa438, 0xd719}, 
    {0xa438, 0x7fac}, {0xa438, 0x1000}, {0xa438, 0x14e4}, {0xa438, 0xd704}, 
    {0xa438, 0x5fbc}, {0xa438, 0xd504}, {0xa438, 0xa001}, {0xa438, 0xa1e0}, 
    {0xa438, 0xd500}, {0xa438, 0xd031}, {0xa438, 0xd1c5}, {0xa438, 0x1000}, 
    {0xa438, 0x14e4}, {0xa438, 0xd704}, {0xa438, 0x5fbc}, {0xa438, 0x1800}, 
    {0xa438, 0x111f}, {0xa438, 0xbf80}, {0xa438, 0xd712}, {0xa438, 0x4077}, 
    {0xa438, 0xd71e}, {0xa438, 0x4159}, {0xa438, 0xd4c0}, {0xa438, 0xd71e}, 
    {0xa438, 0x6099}, {0xa438, 0x7f04}, {0xa438, 0x1800}, {0xa438, 0x1bc0}, 
    {0xa438, 0x9f80}, {0xa438, 0x1800}, {0xa438, 0x1e27}, {0xa438, 0x1800}, 
    {0xa438, 0x1e29}, {0xa438, 0xbf80}, {0xa438, 0xd712}, {0xa438, 0x4077}, 
    {0xa438, 0xd71e}, {0xa438, 0x4159}, {0xa438, 0xd4c1}, {0xa438, 0xd71e}, 
    {0xa438, 0x6099}, {0xa438, 0x7f04}, {0xa438, 0x1800}, {0xa438, 0x1bc0}, 
    {0xa438, 0x9f80}, {0xa438, 0x1800}, {0xa438, 0x1cf6}, {0xa438, 0x1800}, 
    {0xa438, 0x1cf9}, {0xa438, 0xd708}, {0xa438, 0x3b0f}, {0xa438, 0x806d}, 
    {0xa438, 0x1800}, {0xa438, 0x150c}, {0xa438, 0xd503}, {0xa438, 0x8970}, 
    {0xa438, 0x0c07}, {0xa438, 0x0901}, {0xa438, 0xa008}, {0xa438, 0x8002}, 
    {0xa438, 0xd500}, {0xa438, 0x1800}, {0xa438, 0x1504}, {0xa436, 0xA026}, 
    {0xa438, 0x1500}, {0xa436, 0xA024}, {0xa438, 0x1cec}, {0xa436, 0xA022}, 
    {0xa438, 0x1e1d}, {0xa436, 0xA020}, {0xa438, 0x1111}, {0xa436, 0xA006}, 
    {0xa438, 0x1cb7}, {0xa436, 0xA004}, {0xa438, 0x1dec}, {0xa436, 0xA002}, 
    {0xa438, 0x017f}, {0xa436, 0xA000}, {0xa438, 0x1dfe}, {0xa436, 0xA008}, 
    {0xa438, 0xff00}, {0xa436, 0xA012}, {0xa438, 0x0ff8}, {0xa436, 0xA014}, 
    {0xa438, 0xc114}, {0xa438, 0xd04a}, {0xa438, 0xd069}, {0xa438, 0xd71f}, 
    {0xa438, 0x40ff}, {0xa436, 0xA152}, {0xa438, 0x0a20}, {0xa436, 0xA154}, 
    {0xa438, 0x10f6}, {0xa436, 0xA156}, {0xa438, 0x122c}, {0xa436, 0xA158}, 
    {0xa438, 0x09af}, {0xa436, 0xA15a}, {0xa438, 0x09b0}, {0xa436, 0xA150}, 
    {0xa438, 0x001f}
};
void n0_patch_RL6818B_230724(rtk_uint32 phymask)
{

    rtk_uint16 port, i, addr, val, len;

    len = sizeof(n0_patch_6818B_230724_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = n0_patch_6818B_230724_patch[i][0];
                val = n0_patch_6818B_230724_patch[i][1];

                dal_rtl8373_phy_write(1 << port, 31, addr, val);
            }
        }
    }
}

rtk_uint16 n2_tp3_patch_6818B_230724_patch[][2]=
{
    {0xa436, 0xA016}, {0xa438, 0x0020}, {0xa436, 0xA012}, {0xa438, 0x0000}, 
    {0xa436, 0xA014}, {0xa438, 0x1800}, {0xa438, 0x8010}, {0xa438, 0x1800}, 
    {0xa438, 0x8027}, {0xa438, 0x1800}, {0xa438, 0x8036}, {0xa438, 0x1800}, 
    {0xa438, 0x8039}, {0xa438, 0x1800}, {0xa438, 0x8044}, {0xa438, 0x1800}, 
    {0xa438, 0x8044}, {0xa438, 0x1800}, {0xa438, 0x8044}, {0xa438, 0x1800}, 
    {0xa438, 0x8044}, {0xa438, 0xa710}, {0xa438, 0xa00a}, {0xa438, 0x8190}, 
    {0xa438, 0x8204}, {0xa438, 0x8280}, {0xa438, 0xa404}, {0xa438, 0xd130}, 
    {0xa438, 0xd047}, {0xa438, 0x1000}, {0xa438, 0x0b46}, {0xa438, 0xd700}, 
    {0xa438, 0x5fb4}, {0xa438, 0xa110}, {0xa438, 0xa204}, {0xa438, 0xa280}, 
    {0xa438, 0xd130}, {0xa438, 0xd047}, {0xa438, 0x1000}, {0xa438, 0x0b46}, 
    {0xa438, 0xd700}, {0xa438, 0x5fb4}, {0xa438, 0x1800}, {0xa438, 0x0505}, 
    {0xa438, 0xa301}, {0xa438, 0x1000}, {0xa438, 0x0b46}, {0xa438, 0xd700}, 
    {0xa438, 0x5f9f}, {0xa438, 0xd131}, {0xa438, 0xd047}, {0xa438, 0x1000}, 
    {0xa438, 0x0b46}, {0xa438, 0xd700}, {0xa438, 0x5edf}, {0xa438, 0xd700}, 
    {0xa438, 0x5f74}, {0xa438, 0x1800}, {0xa438, 0x032a}, {0xa438, 0x9580}, 
    {0xa438, 0x1800}, {0xa438, 0x073d}, {0xa438, 0x1000}, {0xa438, 0x0b15}, 
    {0xa438, 0xd14a}, {0xa438, 0xd048}, {0xa438, 0xd700}, {0xa438, 0x4014}, 
    {0xa438, 0xd417}, {0xa438, 0x1000}, {0xa438, 0x0b15}, {0xa438, 0x1800}, 
    {0xa438, 0x0661}, {0xa436, 0xA10E}, {0xa438, 0xffff}, {0xa436, 0xA10C}, 
    {0xa438, 0xffff}, {0xa436, 0xA10A}, {0xa438, 0xffff}, {0xa436, 0xA108}, 
    {0xa438, 0xffff}, {0xa436, 0xA106}, {0xa438, 0x065f}, {0xa436, 0xA104}, 
    {0xa438, 0x073b}, {0xa436, 0xA102}, {0xa438, 0x0325}, {0xa436, 0xA100}, 
    {0xa438, 0x04ff}, {0xa436, 0xA110}, {0xa438, 0x000f}
};

void n2_patch_6818B_230724(rtk_uint32 phymask)
{
    rtk_uint32 port, i, addr, val, len;

    len = sizeof(n2_tp3_patch_6818B_230724_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = n2_tp3_patch_6818B_230724_patch[i][0];
                val = n2_tp3_patch_6818B_230724_patch[i][1];

                dal_rtl8373_phy_write(1 << port, 31, addr, val);
            }
        }
    }
}

rtk_uint16 uc2_patch_6818B_230815_patch[][2]=
{
    {0xa436, 0xb87c}, {0xa438, 0x858b}, {0xa436, 0xb87e}, {0xa438, 0xaf85}, 
    {0xa438, 0xa3af}, {0xa438, 0x85b8}, {0xa438, 0xaf85}, {0xa438, 0xc3af}, 
    {0xa438, 0x865b}, {0xa438, 0xaf86}, {0xa438, 0x64af}, {0xa438, 0x866d}, 
    {0xa438, 0xaf86}, {0xa438, 0x73af}, {0xa438, 0x8692}, {0xa438, 0xa001}, 
    {0xa438, 0x0602}, {0xa438, 0x8692}, {0xa438, 0xaf4f}, {0xa438, 0x68a0}, 
    {0xa438, 0x0206}, {0xa438, 0x0287}, {0xa438, 0x55af}, {0xa438, 0x4f68}, 
    {0xa438, 0xaf4f}, {0xa438, 0x45ee}, {0xa438, 0x8fff}, {0xa438, 0x01ee}, 
    {0xa438, 0x838c}, {0xa438, 0x01af}, {0xa438, 0x5299}, {0xa438, 0xee8f}, 
    {0xa438, 0xfe00}, {0xa438, 0xef02}, {0xa438, 0x1b03}, {0xa438, 0xd100}, 
    {0xa438, 0x1b10}, {0xa438, 0xac2d}, {0xa438, 0x0fa2}, {0xa438, 0x0006}, 
    {0xa438, 0xd700}, {0xa438, 0x0eaf}, {0xa438, 0x860c}, {0xa438, 0xd700}, 
    {0xa438, 0x11af}, {0xa438, 0x860c}, {0xa438, 0xa001}, {0xa438, 0x0fa2}, 
    {0xa438, 0x0106}, {0xa438, 0xd700}, {0xa438, 0x12af}, {0xa438, 0x860c}, 
    {0xa438, 0xd700}, {0xa438, 0x15af}, {0xa438, 0x860c}, {0xa438, 0xa002}, 
    {0xa438, 0x0fa2}, {0xa438, 0x0206}, {0xa438, 0xd700}, {0xa438, 0x17af}, 
    {0xa438, 0x860c}, {0xa438, 0xd700}, {0xa438, 0x1baf}, {0xa438, 0x860c}, 
    {0xa438, 0xee8f}, {0xa438, 0xfe01}, {0xa438, 0xaf44}, {0xa438, 0x5102}, 
    {0xa438, 0x87b0}, {0xa438, 0xe080}, {0xa438, 0x28ac}, {0xa438, 0x2009}, 
    {0xa438, 0xbf62}, {0xa438, 0x5902}, {0xa438, 0x5f29}, {0xa438, 0xac28}, 
    {0xa438, 0x1fbf}, {0xa438, 0x61e7}, {0xa438, 0x025f}, {0xa438, 0x29ef}, 
    {0xa438, 0x21bf}, {0xa438, 0x61ea}, {0xa438, 0x025f}, {0xa438, 0x29ef}, 
    {0xa438, 0x31ef}, {0xa438, 0x021b}, {0xa438, 0x03a0}, {0xa438, 0xff13}, 
    {0xa438, 0xa200}, {0xa438, 0x0bd1}, {0xa438, 0x70af}, {0xa438, 0x444e}, 
    {0xa438, 0x0288}, {0xa438, 0x5faf}, {0xa438, 0x444e}, {0xa438, 0xd15f}, 
    {0xa438, 0xaf44}, {0xa438, 0x4ea0}, {0xa438, 0xfe0d}, {0xa438, 0xa200}, 
    {0xa438, 0x05d1}, {0xa438, 0x59af}, {0xa438, 0x444e}, {0xa438, 0xd14c}, 
    {0xa438, 0xaf44}, {0xa438, 0x4eaf}, {0xa438, 0x4451}, {0xa438, 0x0245}, 
    {0xa438, 0xfc02}, {0xa438, 0x3f89}, {0xa438, 0xaf20}, {0xa438, 0xf802}, 
    {0xa438, 0x8930}, {0xa438, 0x0222}, {0xa438, 0x0baf}, {0xa438, 0x20cc}, 
    {0xa438, 0x028c}, {0xa438, 0x41af}, {0xa438, 0x211c}, {0xa438, 0xf8fb}, 
    {0xa438, 0xef79}, {0xa438, 0xfbe0}, {0xa438, 0x8012}, {0xa438, 0xad25}, 
    {0xa438, 0x04ee}, {0xa438, 0x838c}, {0xa438, 0x00d1}, {0xa438, 0x00bf}, 
    {0xa438, 0x61f9}, {0xa438, 0x025f}, {0xa438, 0x0aff}, {0xa438, 0xef97}, 
    {0xa438, 0xfffc}, {0xa438, 0xaf4f}, {0xa438, 0x28f8}, {0xa438, 0xf9ef}, 
    {0xa438, 0x59e1}, {0xa438, 0x8fff}, {0xa438, 0x3901}, {0xa438, 0x9e64}, 
    {0xa438, 0xe083}, {0xa438, 0x97e1}, {0xa438, 0x8200}, {0xa438, 0x1b01}, 
    {0xa438, 0x9f07}, {0xa438, 0xee83}, {0xa438, 0x8c05}, {0xa438, 0xaf87}, 
    {0xa438, 0x50d1}, {0xa438, 0x0fbf}, {0xa438, 0x620e}, {0xa438, 0x025f}, 
    {0xa438, 0x0abf}, {0xa438, 0x61f9}, {0xa438, 0x025f}, {0xa438, 0x29e5}, 
    {0xa438, 0x838f}, {0xa438, 0xbf62}, {0xa438, 0x0b02}, {0xa438, 0x65f9}, 
    {0xa438, 0xbf62}, {0xa438, 0x1102}, {0xa438, 0x65f9}, {0xa438, 0xbf62}, 
    {0xa438, 0x1402}, {0xa438, 0x65f0}, {0xa438, 0xd103}, {0xa438, 0xbf61}, 
    {0xa438, 0xf902}, {0xa438, 0x5f0a}, {0xa438, 0xe182}, {0xa438, 0x01bf}, 
    {0xa438, 0x6217}, {0xa438, 0x025f}, {0xa438, 0x0abf}, {0xa438, 0x621a}, 
    {0xa438, 0x0265}, {0xa438, 0xf9bf}, {0xa438, 0x621a}, {0xa438, 0x0265}, 
    {0xa438, 0xf0bf}, {0xa438, 0x621d}, {0xa438, 0x0265}, {0xa438, 0xf9bf}, 
    {0xa438, 0x621d}, {0xa438, 0x0265}, {0xa438, 0xf0ee}, {0xa438, 0x838c}, 
    {0xa438, 0x02af}, {0xa438, 0x8750}, {0xa438, 0xd10f}, {0xa438, 0xbf62}, 
    {0xa438, 0x0e02}, {0xa438, 0x5f0a}, {0xa438, 0xbf61}, {0xa438, 0xf902}, 
    {0xa438, 0x5f29}, {0xa438, 0xe583}, {0xa438, 0x8fbf}, {0xa438, 0x620b}, 
    {0xa438, 0x0265}, {0xa438, 0xf9bf}, {0xa438, 0x6211}, {0xa438, 0x0265}, 
    {0xa438, 0xf9bf}, {0xa438, 0x6214}, {0xa438, 0x0265}, {0xa438, 0xf0d1}, 
    {0xa438, 0x03bf}, {0xa438, 0x61f9}, {0xa438, 0x025f}, {0xa438, 0x0ad1}, 
    {0xa438, 0x40bf}, {0xa438, 0x6217}, {0xa438, 0x025f}, {0xa438, 0x0abf}, 
    {0xa438, 0x621a}, {0xa438, 0x0265}, {0xa438, 0xf9bf}, {0xa438, 0x621a}, 
    {0xa438, 0x0265}, {0xa438, 0xf0bf}, {0xa438, 0x621d}, {0xa438, 0x0265}, 
    {0xa438, 0xf9bf}, {0xa438, 0x621d}, {0xa438, 0x0265}, {0xa438, 0xf0ee}, 
    {0xa438, 0x838c}, {0xa438, 0x02ef}, {0xa438, 0x95fd}, {0xa438, 0xfc04}, 
    {0xa438, 0xf8f9}, {0xa438, 0xef59}, {0xa438, 0xbf62}, {0xa438, 0x2002}, 
    {0xa438, 0x5f29}, {0xa438, 0xa100}, {0xa438, 0x49bf}, {0xa438, 0x620b}, 
    {0xa438, 0x0265}, {0xa438, 0xf0e1}, {0xa438, 0x838f}, {0xa438, 0xbf61}, 
    {0xa438, 0xf902}, {0xa438, 0x5f0a}, {0xa438, 0xbf62}, {0xa438, 0x0e02}, 
    {0xa438, 0x65f0}, {0xa438, 0xbf62}, {0xa438, 0x1102}, {0xa438, 0x65f0}, 
    {0xa438, 0xbf62}, {0xa438, 0x1402}, {0xa438, 0x65f0}, {0xa438, 0xbf62}, 
    {0xa438, 0x1702}, {0xa438, 0x65f0}, {0xa438, 0xe18f}, {0xa438, 0xffa1}, 
    {0xa438, 0x010b}, {0xa438, 0xee83}, {0xa438, 0x8c07}, {0xa438, 0xee8f}, 
    {0xa438, 0xff00}, {0xa438, 0xaf87}, {0xa438, 0xabe1}, {0xa438, 0x8394}, 
    {0xa438, 0xa101}, {0xa438, 0x07ee}, {0xa438, 0x838c}, {0xa438, 0x05af}, 
    {0xa438, 0x87ab}, {0xa438, 0xee83}, {0xa438, 0x8c03}, {0xa438, 0xef95}, 
    {0xa438, 0xfdfc}, {0xa438, 0x04f8}, {0xa438, 0xfaef}, {0xa438, 0x69fb}, 
    {0xa438, 0xbf61}, {0xa438, 0xfc02}, {0xa438, 0x5f29}, {0xa438, 0x0d45}, 
    {0xa438, 0x5c07}, {0xa438, 0xff1c}, {0xa438, 0x47ac}, {0xa438, 0x2704}, 
    {0xa438, 0x0c41}, {0xa438, 0xae03}, {0xa438, 0xd4ff}, {0xa438, 0xffbf}, 
    {0xa438, 0x615a}, {0xa438, 0x025f}, {0xa438, 0x0abf}, {0xa438, 0x614e}, 
    {0xa438, 0x0265}, {0xa438, 0xf9bf}, {0xa438, 0x614e}, {0xa438, 0x0265}, 
    {0xa438, 0xf0bf}, {0xa438, 0x61ff}, {0xa438, 0x025f}, {0xa438, 0x290d}, 
    {0xa438, 0x455c}, {0xa438, 0x07ff}, {0xa438, 0x1c47}, {0xa438, 0xac27}, 
    {0xa438, 0x040c}, {0xa438, 0x41ae}, {0xa438, 0x03d4}, {0xa438, 0xffff}, 
    {0xa438, 0xbf61}, {0xa438, 0x5a02}, {0xa438, 0x5f0a}, {0xa438, 0xbf61}, 
    {0xa438, 0x5102}, {0xa438, 0x65f9}, {0xa438, 0xbf61}, {0xa438, 0x5102}, 
    {0xa438, 0x65f0}, {0xa438, 0xbf62}, {0xa438, 0x0202}, {0xa438, 0x5f29}, 
    {0xa438, 0x0d45}, {0xa438, 0x5c07}, {0xa438, 0xff1c}, {0xa438, 0x47ac}, 
    {0xa438, 0x2704}, {0xa438, 0x0c41}, {0xa438, 0xae03}, {0xa438, 0xd4ff}, 
    {0xa438, 0xffbf}, {0xa438, 0x615a}, {0xa438, 0x025f}, {0xa438, 0x0abf}, 
    {0xa438, 0x6154}, {0xa438, 0x0265}, {0xa438, 0xf9bf}, {0xa438, 0x6154}, 
    {0xa438, 0x0265}, {0xa438, 0xf0bf}, {0xa438, 0x6205}, {0xa438, 0x025f}, 
    {0xa438, 0x290d}, {0xa438, 0x455c}, {0xa438, 0x07ff}, {0xa438, 0x1c47}, 
    {0xa438, 0xac27}, {0xa438, 0x040c}, {0xa438, 0x41ae}, {0xa438, 0x03d4}, 
    {0xa438, 0xffff}, {0xa438, 0xbf61}, {0xa438, 0x5a02}, {0xa438, 0x5f0a}, 
    {0xa438, 0xbf61}, {0xa438, 0x5702}, {0xa438, 0x65f9}, {0xa438, 0xbf61}, 
    {0xa438, 0x5702}, {0xa438, 0x65f0}, {0xa438, 0xffef}, {0xa438, 0x96fe}, 
    {0xa438, 0xfc04}, {0xa438, 0xf9fa}, {0xa438, 0xfba3}, {0xa438, 0x0008}, 
    {0xa438, 0xd646}, {0xa438, 0x9ed2}, {0xa438, 0x00af}, {0xa438, 0x889e}, 
    {0xa438, 0xa301}, {0xa438, 0x13a2}, {0xa438, 0x0008}, {0xa438, 0xd638}, 
    {0xa438, 0x19d2}, {0xa438, 0x00af}, {0xa438, 0x889e}, {0xa438, 0xd63b}, 
    {0xa438, 0xe6d2}, {0xa438, 0x01af}, {0xa438, 0x889e}, {0xa438, 0xa302}, 
    {0xa438, 0x13a2}, {0xa438, 0x0008}, {0xa438, 0xd62c}, {0xa438, 0x8fd2}, 
    {0xa438, 0x00af}, {0xa438, 0x889e}, {0xa438, 0xd62f}, {0xa438, 0x94d2}, 
    {0xa438, 0x01af}, {0xa438, 0x889e}, {0xa438, 0xd625}, {0xa438, 0xcbd2}, 
    {0xa438, 0x0102}, {0xa438, 0x88b4}, {0xa438, 0x0266}, {0xa438, 0xfa0d}, 
    {0xa438, 0x75ef}, {0xa438, 0x47a0}, {0xa438, 0x0002}, {0xa438, 0xae03}, 
    {0xa438, 0xd400}, {0xa438, 0xffff}, {0xa438, 0xfefd}, {0xa438, 0x04f8}, 
    {0xa438, 0xf9fa}, {0xa438, 0xef69}, {0xa438, 0xface}, {0xa438, 0xfa1f}, 
    {0xa438, 0x33be}, {0xa438, 0x0000}, {0xa438, 0xac30}, {0xa438, 0x06bf}, 
    {0xa438, 0x83c0}, {0xa438, 0xaf88}, {0xa438, 0xcdbf}, {0xa438, 0x83c2}, 
    {0xa438, 0xef13}, {0xa438, 0x0c16}, {0xa438, 0x1f00}, {0xa438, 0x1a94}, 
    {0xa438, 0xd819}, {0xa438, 0xd9ef}, {0xa438, 0x74bf}, {0xa438, 0x83e2}, 
    {0xa438, 0xef13}, {0xa438, 0x0c16}, {0xa438, 0x1f00}, {0xa438, 0x1a94}, 
    {0xa438, 0xd819}, {0xa438, 0xd9ef}, {0xa438, 0x64ef}, {0xa438, 0x03d3}, 
    {0xa438, 0x0c02}, {0xa438, 0x6602}, {0xa438, 0x0c74}, {0xa438, 0xef30}, 
    {0xa438, 0xac30}, {0xa438, 0x06d6}, {0xa438, 0x91b1}, {0xa438, 0xaf89}, 
    {0xa438, 0x01d6}, {0xa438, 0x886f}, {0xa438, 0x0266}, {0xa438, 0xfa0d}, 
    {0xa438, 0x725f}, {0xa438, 0x3fff}, {0xa438, 0x2713}, {0xa438, 0xbf60}, 
    {0xa438, 0xaf02}, {0xa438, 0x5f29}, {0xa438, 0xad28}, {0xa438, 0x07a3}, 
    {0xa438, 0x02aa}, {0xa438, 0x0c81}, {0xa438, 0xae03}, {0xa438, 0xa304}, 
    {0xa438, 0xa3cf}, {0xa438, 0xef47}, {0xa438, 0xe482}, {0xa438, 0xc8e5}, 
    {0xa438, 0x82c9}, {0xa438, 0xfec6}, {0xa438, 0xfeef}, {0xa438, 0x96fe}, 
    {0xa438, 0xfdfc}, {0xa438, 0x04f8}, {0xa438, 0xf9ef}, {0xa438, 0x59f9}, 
    {0xa438, 0xfafb}, {0xa438, 0xd200}, {0xa438, 0xef32}, {0xa438, 0x0c34}, 
    {0xa438, 0xe78f}, {0xa438, 0xfdbf}, {0xa438, 0x61ae}, {0xa438, 0xef02}, 
    {0xa438, 0x4803}, {0xa438, 0x1a90}, {0xa438, 0x025f}, {0xa438, 0x29ef}, 
    {0xa438, 0x64e3}, {0xa438, 0x8fd7}, {0xa438, 0x028c}, {0xa438, 0x89ef}, 
    {0xa438, 0x460d}, {0xa438, 0x46e3}, {0xa438, 0x8fd6}, {0xa438, 0x1a43}, 
    {0xa438, 0xe48f}, {0xa438, 0xd4e5}, {0xa438, 0x8fd5}, {0xa438, 0x1f66}, 
    {0xa438, 0x1f77}, {0xa438, 0xe38f}, {0xa438, 0xd91a}, {0xa438, 0x63e3}, 
    {0xa438, 0x8fd7}, {0xa438, 0x028c}, {0xa438, 0x890d}, {0xa438, 0x661b}, 
    {0xa438, 0x46ef}, {0xa438, 0x64e3}, {0xa438, 0x8fd6}, {0xa438, 0x1a73}, 
    {0xa438, 0xe38f}, {0xa438, 0xd81a}, {0xa438, 0x7302}, {0xa438, 0x8b30}, 
    {0xa438, 0xef47}, {0xa438, 0xe48f}, {0xa438, 0xdbe5}, {0xa438, 0x8fdc}, 
    {0xa438, 0xd301}, {0xa438, 0x028c}, {0xa438, 0x7a1f}, {0xa438, 0x66e0}, 
    {0xa438, 0x8fd4}, {0xa438, 0xe18f}, {0xa438, 0xd5e3}, {0xa438, 0x8fda}, 
    {0xa438, 0x1a63}, {0xa438, 0xe38f}, {0xa438, 0xd702}, {0xa438, 0x8c89}, 
    {0xa438, 0x0d66}, {0xa438, 0x1a64}, {0xa438, 0xbf65}, {0xa438, 0x0eef}, 
    {0xa438, 0x0248}, {0xa438, 0x031a}, {0xa438, 0x9002}, {0xa438, 0x5f29}, 
    {0xa438, 0xac2a}, {0xa438, 0x05d7}, {0xa438, 0x017f}, {0xa438, 0xae03}, 
    {0xa438, 0xd700}, {0xa438, 0xff02}, {0xa438, 0x8b40}, {0xa438, 0xef47}, 
    {0xa438, 0xe48f}, {0xa438, 0xdde5}, {0xa438, 0x8fde}, {0xa438, 0xd302}, 
    {0xa438, 0x028c}, {0xa438, 0x7a02}, {0xa438, 0x8b4d}, {0xa438, 0xd303}, 
    {0xa438, 0x028c}, {0xa438, 0x7ae0}, {0xa438, 0x8fdb}, {0xa438, 0xe18f}, 
    {0xa438, 0xdcef}, {0xa438, 0x64e0}, {0xa438, 0x8fdd}, {0xa438, 0xe18f}, 
    {0xa438, 0xdeef}, {0xa438, 0x7402}, {0xa438, 0x8bd2}, {0xa438, 0xd304}, 
    {0xa438, 0x028c}, {0xa438, 0x7abf}, {0xa438, 0x8fe3}, {0xa438, 0xef32}, 
    {0xa438, 0x4b02}, {0xa438, 0x1a93}, {0xa438, 0xe88f}, {0xa438, 0xf919}, 
    {0xa438, 0xe88f}, {0xa438, 0xfad0}, {0xa438, 0x00e1}, {0xa438, 0x8fe0}, 
    {0xa438, 0x1a64}, {0xa438, 0xe08f}, {0xa438, 0xdde1}, {0xa438, 0x8fde}, 
    {0xa438, 0x1b46}, {0xa438, 0xaa18}, {0xa438, 0xe08f}, {0xa438, 0xf9e1}, 
    {0xa438, 0x8ffa}, {0xa438, 0xe38f}, {0xa438, 0xe01a}, {0xa438, 0x43e4}, 
    {0xa438, 0x8ff3}, {0xa438, 0xe58f}, {0xa438, 0xf4d3}, {0xa438, 0x0502}, 
    {0xa438, 0x8c7a}, {0xa438, 0xae03}, {0xa438, 0xaf8a}, {0xa438, 0xa3e0}, 
    {0xa438, 0x8ff3}, {0xa438, 0xe18f}, {0xa438, 0xf40c}, {0xa438, 0x41bf}, 
    {0xa438, 0x8cd4}, {0xa438, 0x1a94}, {0xa438, 0xd819}, {0xa438, 0xd9ad}, 
    {0xa438, 0x2704}, {0xa438, 0x7cff}, {0xa438, 0xff14}, {0xa438, 0xe38f}, 
    {0xa438, 0xdfef}, {0xa438, 0x6402}, {0xa438, 0x8c89}, {0xa438, 0x0d64}, 
    {0xa438, 0xe08f}, {0xa438, 0xfbe1}, {0xa438, 0x8ffc}, {0xa438, 0x1b64}, 
    {0xa438, 0xab0f}, {0xa438, 0xe08f}, {0xa438, 0xf3e1}, {0xa438, 0x8ff4}, 
    {0xa438, 0x14e4}, {0xa438, 0x8ff3}, {0xa438, 0xe58f}, {0xa438, 0xf4ae}, 
    {0xa438, 0x21bf}, {0xa438, 0x8feb}, {0xa438, 0xef32}, {0xa438, 0x4b02}, 
    {0xa438, 0x1a93}, {0xa438, 0xe88f}, {0xa438, 0xf319}, {0xa438, 0xe88f}, 
    {0xa438, 0xf4e0}, {0xa438, 0x8ff3}, {0xa438, 0xe18f}, {0xa438, 0xf4e3}, 
    {0xa438, 0x8fe0}, {0xa438, 0x1a43}, {0xa438, 0xe48f}, {0xa438, 0xf3e5}, 
    {0xa438, 0x8ff4}, {0xa438, 0xef64}, {0xa438, 0xe08f}, {0xa438, 0xdde1}, 
    {0xa438, 0x8fde}, {0xa438, 0x1b46}, {0xa438, 0xab99}, {0xa438, 0xbf8f}, 
    {0xa438, 0xebef}, {0xa438, 0x324b}, {0xa438, 0x021a}, {0xa438, 0x93d8}, 
    {0xa438, 0x19d9}, {0xa438, 0xa400}, {0xa438, 0x0015}, {0xa438, 0xd306}, 
    {0xa438, 0x028c}, {0xa438, 0x7abf}, {0xa438, 0x8feb}, {0xa438, 0xef32}, 
    {0xa438, 0x4b02}, {0xa438, 0x1a93}, {0xa438, 0xe88f}, {0xa438, 0xf919}, 
    {0xa438, 0xe88f}, {0xa438, 0xfad3}, {0xa438, 0x0702}, {0xa438, 0x8c7a}, 
    {0xa438, 0xbf8f}, {0xa438, 0xebef}, {0xa438, 0x324b}, {0xa438, 0x021a}, 
    {0xa438, 0x93d8}, {0xa438, 0x19d9}, {0xa438, 0xe38f}, {0xa438, 0xd61b}, 
    {0xa438, 0x43ef}, {0xa438, 0x740c}, {0xa438, 0x761f}, {0xa438, 0x44e1}, 
    {0xa438, 0x8fd7}, {0xa438, 0xef64}, {0xa438, 0xd300}, {0xa438, 0x0266}, 
    {0xa438, 0x02ef}, {0xa438, 0x47bf}, {0xa438, 0x8ff5}, {0xa438, 0x1a92}, 
    {0xa438, 0xddd3}, {0xa438, 0x0802}, {0xa438, 0x8c7a}, {0xa438, 0xbf8f}, 
    {0xa438, 0xebef}, {0xa438, 0x324b}, {0xa438, 0x021a}, {0xa438, 0x93d8}, 
    {0xa438, 0x19d9}, {0xa438, 0xe38f}, {0xa438, 0xd61b}, {0xa438, 0x43e3}, 
    {0xa438, 0x8fd8}, {0xa438, 0x1b43}, {0xa438, 0x9f1c}, {0xa438, 0xd309}, 
    {0xa438, 0x028c}, {0xa438, 0x7abf}, {0xa438, 0x61ae}, {0xa438, 0xef02}, 
    {0xa438, 0x4803}, {0xa438, 0x1a90}, {0xa438, 0x025f}, {0xa438, 0x29bf}, 
    {0xa438, 0x8ff5}, {0xa438, 0x1a92}, {0xa438, 0xdb1b}, {0xa438, 0x31aa}, 
    {0xa438, 0x01dd}, {0xa438, 0x12a2}, {0xa438, 0x0402}, {0xa438, 0xae03}, 
    {0xa438, 0xaf89}, {0xa438, 0x39ff}, {0xa438, 0xfefd}, {0xa438, 0xef95}, 
    {0xa438, 0xfdfc}, {0xa438, 0x04f8}, {0xa438, 0xfaef}, {0xa438, 0x46ac}, 
    {0xa438, 0x4f06}, {0xa438, 0x1b67}, {0xa438, 0xaa02}, {0xa438, 0xef74}, 
    {0xa438, 0xfefc}, {0xa438, 0x04f8}, {0xa438, 0xfaef}, {0xa438, 0x461b}, 
    {0xa438, 0x67ab}, {0xa438, 0x02ef}, {0xa438, 0x74fe}, {0xa438, 0xfc04}, 
    {0xa438, 0xf8f9}, {0xa438, 0xfafb}, {0xa438, 0xef79}, {0xa438, 0xfb02}, 
    {0xa438, 0x6683}, {0xa438, 0xd340}, {0xa438, 0xbf8c}, {0xa438, 0xd4a3}, 
    {0xa438, 0x6002}, {0xa438, 0xae0e}, {0xa438, 0x1a32}, {0xa438, 0x0246}, 
    {0xa438, 0x9a02}, {0xa438, 0x8bab}, {0xa438, 0x1b32}, {0xa438, 0x2b04}, 
    {0xa438, 0xaeed}, {0xa438, 0xef32}, {0xa438, 0xef69}, {0xa438, 0xbf65}, 
    {0xa438, 0x0eef}, {0xa438, 0x0248}, {0xa438, 0x031a}, {0xa438, 0x9002}, 
    {0xa438, 0x5f29}, {0xa438, 0xac2a}, {0xa438, 0x1bef}, {0xa438, 0x96ef}, 
    {0xa438, 0x234a}, {0xa438, 0x02d3}, {0xa438, 0x60a3}, {0xa438, 0x8002}, 
    {0xa438, 0xae0e}, {0xa438, 0x1a32}, {0xa438, 0x0246}, {0xa438, 0x9a02}, 
    {0xa438, 0x8bab}, {0xa438, 0x1b32}, {0xa438, 0x2b08}, {0xa438, 0xaeed}, 
    {0xa438, 0xbf63}, {0xa438, 0x0102}, {0xa438, 0x65f0}, {0xa438, 0xffef}, 
    {0xa438, 0x97ff}, {0xa438, 0xfefd}, {0xa438, 0xfc04}, {0xa438, 0xf8f9}, 
    {0xa438, 0xfad2}, {0xa438, 0x00ef}, {0xa438, 0x691f}, {0xa438, 0x44d0}, 
    {0xa438, 0x00ef}, {0xa438, 0x12bf}, {0xa438, 0x63d3}, {0xa438, 0x025f}, 
    {0xa438, 0x0a12}, {0xa438, 0xbf63}, {0xa438, 0xdf02}, {0xa438, 0x5f29}, 
    {0xa438, 0xef96}, {0xa438, 0xdc19}, {0xa438, 0xdd19}, {0xa438, 0xa220}, 
    {0xa438, 0xe2fe}, {0xa438, 0xfdfc}, {0xa438, 0x04f8}, {0xa438, 0xf9ef}, 
    {0xa438, 0x59f9}, {0xa438, 0xbf8c}, {0xa438, 0xd4ee}, {0xa438, 0x8ff9}, 
    {0xa438, 0x00ee}, {0xa438, 0x8ffa}, {0xa438, 0x00ee}, {0xa438, 0x8ffb}, 
    {0xa438, 0x00ee}, {0xa438, 0x8ffc}, {0xa438, 0x004e}, {0xa438, 0x0002}, 
    {0xa438, 0x1a96}, {0xa438, 0x174f}, {0xa438, 0x0002}, {0xa438, 0x2f8c}, 
    {0xa438, 0xd4ef}, {0xa438, 0x691b}, {0xa438, 0x679e}, {0xa438, 0x2fe0}, 
    {0xa438, 0x8ffb}, {0xa438, 0xe18f}, {0xa438, 0xfcda}, {0xa438, 0x19db}, 
    {0xa438, 0x19ad}, {0xa438, 0x3704}, {0xa438, 0x7dff}, {0xa438, 0xff15}, 
    {0xa438, 0x1b45}, {0xa438, 0x9e02}, {0xa438, 0xabe3}, {0xa438, 0xe68f}, 
    {0xa438, 0xfbe7}, {0xa438, 0x8ffc}, {0xa438, 0xef59}, {0xa438, 0x3d8c}, 
    {0xa438, 0xd43d}, {0xa438, 0x0002}, {0xa438, 0x0d51}, {0xa438, 0xe68f}, 
    {0xa438, 0xf9e7}, {0xa438, 0x8ffa}, {0xa438, 0xaecb}, {0xa438, 0xe28f}, 
    {0xa438, 0xf9e3}, {0xa438, 0x8ffa}, {0xa438, 0xef65}, {0xa438, 0xe28f}, 
    {0xa438, 0xfbe3}, {0xa438, 0x8ffc}, {0xa438, 0xef75}, {0xa438, 0xfdef}, 
    {0xa438, 0x95fd}, {0xa438, 0xfc04}, {0xa438, 0xee8f}, {0xa438, 0xeb00}, 
    {0xa438, 0xee8f}, {0xa438, 0xec00}, {0xa438, 0xee8f}, {0xa438, 0xed00}, 
    {0xa438, 0xee8f}, {0xa438, 0xee00}, {0xa438, 0xee8f}, {0xa438, 0xef00}, 
    {0xa438, 0xee8f}, {0xa438, 0xf000}, {0xa438, 0xee8f}, {0xa438, 0xf100}, 
    {0xa438, 0xee8f}, {0xa438, 0xf200}, {0xa438, 0xee8f}, {0xa438, 0xf300}, 
    {0xa438, 0xee8f}, {0xa438, 0xf400}, {0xa438, 0xee8f}, {0xa438, 0xf900}, 
    {0xa438, 0xee8f}, {0xa438, 0xfa00}, {0xa438, 0xee8f}, {0xa438, 0xfb00}, 
    {0xa438, 0xee8f}, {0xa438, 0xfc00}, {0xa438, 0x04f9}, {0xa438, 0xe28f}, 
    {0xa438, 0xfd5a}, {0xa438, 0xf05b}, {0xa438, 0x0f1e}, {0xa438, 0x23e6}, 
    {0xa438, 0x8ffd}, {0xa438, 0xfd04}, {0xa438, 0xf81f}, {0xa438, 0x44ef}, 
    {0xa438, 0x131c}, {0xa438, 0x64fc}, {0xa438, 0x0400}, {0xa436, 0xb85e}, 
    {0xa438, 0x4f35}, {0xa436, 0xb860}, {0xa438, 0x5295}, {0xa436, 0xb862}, 
    {0xa438, 0x4404}, {0xa436, 0xb864}, {0xa438, 0x20f5}, {0xa436, 0xb886}, 
    {0xa438, 0x20c9}, {0xa436, 0xb888}, {0xa438, 0x2119}, {0xa436, 0xb88a}, 
    {0xa438, 0x4f1c}, {0xa436, 0xb88c}, {0xa438, 0xffff}, {0xa436, 0xb838}, 
    {0xa438, 0x007f}
};

void uc2_patch_6818B_230815(rtk_uint32 phymask)
{
    rtk_uint32 port, i, addr, val, len;

    len = sizeof(uc2_patch_6818B_230815_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = uc2_patch_6818B_230815_patch[i][0];
                val = uc2_patch_6818B_230815_patch[i][1];
                dal_rtl8373_phy_write(1 << port, 31, addr, val);
            }
        }
    }
}

rtk_uint16 uc_patch_6818B_20221211_patch[][2] =
{
    {0xa436, 0x847d}, {0xa438, 0xaf84}, {0xa438, 0x95af}, {0xa438, 0x849a},
    {0xa438, 0xaf84}, {0xa438, 0xa0af}, {0xa438, 0x84ae}, {0xa438, 0xaf84},
    {0xa438, 0xdbaf}, {0xa438, 0x84fa}, {0xa438, 0xaf84}, {0xa438, 0xfaaf},
    {0xa438, 0x84fa}, {0xa438, 0x2901}, {0xa438, 0xaf16}, {0xa438, 0x5802},
    {0xa438, 0x84fa}, {0xa438, 0xaf16}, {0xa438, 0x2fee}, {0xa438, 0x8ffe},
    {0xa438, 0xffee}, {0xa438, 0x8fff}, {0xa438, 0xffbf}, {0xa438, 0x161c},
    {0xa438, 0xaf16}, {0xa438, 0x00e2}, {0xa438, 0x8700}, {0xa438, 0xad30},
    {0xa438, 0x21e2}, {0xa438, 0x8701}, {0xa438, 0xe387}, {0xa438, 0x021b},
    {0xa438, 0x549f}, {0xa438, 0x17ef}, {0xa438, 0x54e1}, {0xa438, 0x8018},
    {0xa438, 0x0d14}, {0xa438, 0xbf85}, {0xa438, 0x8402}, {0xa438, 0x5598},
    {0xa438, 0x0d11}, {0xa438, 0xbf85}, {0xa438, 0x8702}, {0xa438, 0x5598},
    {0xa438, 0xef45}, {0xa438, 0xe281}, {0xa438, 0xc2af}, {0xa438, 0x4056},
    {0xa438, 0xe187}, {0xa438, 0x00ad}, {0xa438, 0x2813}, {0xa438, 0xe180},
    {0xa438, 0x180d}, {0xa438, 0x16bf}, {0xa438, 0x8584}, {0xa438, 0x0255},
    {0xa438, 0x980d}, {0xa438, 0x11bf}, {0xa438, 0x8587}, {0xa438, 0x0255},
    {0xa438, 0x9802}, {0xa438, 0x3b96}, {0xa438, 0xaf3a}, {0xa438, 0xfc02},
    {0xa438, 0x8501}, {0xa438, 0x0285}, {0xa438, 0x5904}, {0xa438, 0xf8f9},
    {0xa438, 0xfafb}, {0xa438, 0xef79}, {0xa438, 0xfbd7}, {0xa438, 0x0000},
    {0xa438, 0xd500}, {0xa438, 0x00a2}, {0xa438, 0x0005}, {0xa438, 0xbf57},
    {0xa438, 0x55ae}, {0xa438, 0x13a2}, {0xa438, 0x0105}, {0xa438, 0xbf57},
    {0xa438, 0x58ae}, {0xa438, 0x0ba2}, {0xa438, 0x0205}, {0xa438, 0xbf57},
    {0xa438, 0x5bae}, {0xa438, 0x03bf}, {0xa438, 0x575e}, {0xa438, 0x0255},
    {0xa438, 0xb7ef}, {0xa438, 0x6402}, {0xa438, 0x5b00}, {0xa438, 0x13e1},
    {0xa438, 0x8ffb}, {0xa438, 0x1b13}, {0xa438, 0x9fd5}, {0xa438, 0xe083},
    {0xa438, 0x7380}, {0xa438, 0x1b02}, {0xa438, 0x9f0a}, {0xa438, 0xef47},
    {0xa438, 0xe48f}, {0xa438, 0xfce5}, {0xa438, 0x8ffd}, {0xa438, 0xae06},
    {0xa438, 0x2a01}, {0xa438, 0xd300}, {0xa438, 0xaebd}, {0xa438, 0xffef},
    {0xa438, 0x97ff}, {0xa438, 0xfefd}, {0xa438, 0xfc04}, {0xa438, 0xf8f9},
    {0xa438, 0xfafb}, {0xa438, 0xe28f}, {0xa438, 0xfce3}, {0xa438, 0x8ffd},
    {0xa438, 0xef65}, {0xa438, 0xe08f}, {0xa438, 0xfee1}, {0xa438, 0x8fff},
    {0xa438, 0xef74}, {0xa438, 0x025a}, {0xa438, 0xe5ad}, {0xa438, 0x500c},
    {0xa438, 0xe68f}, {0xa438, 0xfee7}, {0xa438, 0x8fff}, {0xa438, 0xe283},
    {0xa438, 0x4ee6}, {0xa438, 0x834f}, {0xa438, 0xfffe}, {0xa438, 0xfdfc},
    {0xa438, 0x04cc}, {0xa438, 0xc010}, {0xa438, 0x00c0}, {0xa438, 0x2000},
    {0xa436, 0xb818}, {0xa438, 0x164d}, {0xa436, 0xb81a}, {0xa438, 0x162c},
    {0xa436, 0xb81c}, {0xa438, 0x15fd}, {0xa436, 0xb81e}, {0xa438, 0x4053},
    {0xa436, 0xb850}, {0xa438, 0x3af9}, {0xa436, 0xb852}, {0xa438, 0x0000},
    {0xa436, 0xb878}, {0xa438, 0x0000}, {0xa436, 0xb884}, {0xa438, 0x0000},
    {0xa436, 0xb832}, {0xa438, 0x001f}
};

void uc_patch_6818B_20221211(rtk_uint32 phymask)
{

    rtk_uint16 port, i, addr, val, len;

    len = sizeof(uc_patch_6818B_20221211_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = uc_patch_6818B_20221211_patch[i][0];
                val = uc_patch_6818B_20221211_patch[i][1];

                dal_rtl8373_phy_write(1 << port, 31, addr, val);
            }
        }
    }
}

rtk_uint16 data_ram_patch_6818B_220714_patch[][2]=
{
        {0xC202,0xFA} ,{0xC203,0x02} ,{0xC204,0x4D} ,{0xC205,0x02},
        {0xC206,0xB1} ,{0xC207,0x03} ,{0xC208,0x30} ,{0xC209,0x03},
        {0xC20A,0xB5} ,{0xC20B,0x04} ,{0xC20C,0x3A} ,{0xC20D,0x04},
        {0xC20E,0xE7} ,{0xC20F,0x05} ,{0xC210,0xAC} ,{0xC211,0x06},
        {0xC212,0x94} ,{0xC213,0x07} ,{0xC214,0x98} ,{0xC215,0x08},
        {0xC216,0xD7} ,{0xC217,0x0A} ,{0xC218,0x5C} ,{0xC219,0x0C},
        {0xC21A,0x5E} ,{0xC21B,0x0E} ,{0xC21C,0x01} ,{0xC21D,0x10},
        {0xC21E,0x5E} ,{0xC21F,0xF6} ,
        
        {0xC220,0xF6} ,{0xC221,0xF5} ,{0xC222,0xF3} ,{0xC223,0xF3},
        {0xC224,0xEF} ,{0xC225,0xEB} ,{0xC226,0xE7} ,{0xC227,0xE4},
        {0xC228,0xE2} ,{0xC229,0xDF} ,{0xC22A,0xDD} ,{0xC22B,0xDB},
        {0xC22C,0xDA} ,{0xC22D,0xD9} ,
        
        {0xC270,0x18} ,{0xC271,0x19} ,{0xC272,0x1C} ,{0xC273,0x1E},
        {0xC274,0x20} ,{0xC275,0x21} ,{0xC276,0x22} ,{0xC277,0x23},
        {0xC278,0x23} ,{0xC279,0x24} ,{0xC27A,0x25} ,{0xC27B,0x25},
        {0xC27C,0x26} ,{0xC27D,0x26} ,{0xC27E,0x26} ,{0xC27F,0x4A},
        
        {0xC280,0x4A} ,{0xC281,0x4A} ,{0xC282,0x4D} ,{0xC283,0x4D},
        {0xC284,0x50} ,{0xC285,0x55} ,{0xC286,0x52} ,{0xC287,0x52},
        {0xC288,0x52} ,{0xC289,0x52} ,{0xC28A,0x52} ,{0xC28B,0x51},
        {0xC28C,0x52} ,{0xC28D,0x52} ,
        
        {0xC201,0x01} ,{0xC22E,0xD7} ,{0xC26F,0x18} ,{0xC28E,0x52}
};

void data_ram_patch_6818B_220714(rtk_uint32 phymask)
{
    rtk_uint32 port, i, data_ram_addr, data_ram_val, len;

    len = sizeof(data_ram_patch_6818B_220714_patch) / 4;

    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb896, 1, 0);
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb892, 0xff00, 0);
            for (i = 0; i < len; i++)
            {
                data_ram_addr = data_ram_patch_6818B_220714_patch[i][0];
                data_ram_val = data_ram_patch_6818B_220714_patch[i][1];
                data_ram_write_8b(port, data_ram_addr, data_ram_val);
            }

            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb896, 1, 1);
            uc2_sram_write_8b(port, 0x8217, 0x1e);
        }
    }
}

rtk_uint16 alg_tune_2p5G_6818B_220701_patch[][2]=
{
    {0x8066, 0x10},
    {0x8067 ,0x40},
    {0x8068, 0x0a},
    {0x8069, 0x40},
    {0x806e, 0x02},
    {0x806f, 0xa0},

    {0x8084 ,0x10},
    {0x8085 ,0x40},
    {0x8086 ,0x0a},
    {0x8087, 0x40},
    {0x808c ,0x02},
    {0x808d ,0xa0},
    {0x8029, 0x0e}
};

void alg_tune_2p5G_6818B_220701(rtk_uint32 phymask)
{
    rtk_uint32 port, i, uc2_sram_addr, uc2_sram_val, len;

    len = sizeof(alg_tune_2p5G_6818B_220701_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            uc1_sram_write_8b(port, 0x8017, 0x7);
            for (i = 0; i < len; i++)
            {
                uc2_sram_addr = alg_tune_2p5G_6818B_220701_patch[i][0];
                uc2_sram_val = alg_tune_2p5G_6818B_220701_patch[i][1];
                uc2_sram_write_8b(port, uc2_sram_addr, uc2_sram_val);
            }
        }
    }
}

rtk_uint16 alg_tune_giga_6818B_220617_patch[][2] =
    {
        {0x80b5, 0x95},
        {0x80b6, 0xcb},
        {0x80b7, 0xf6},
        {0x80b8, 0x04},
        {0x80b9, 0xd2},
        {0x80ba, 0x06},
        {0x80bb, 0xf7},
        {0x80bc, 0xbc},
        {0x80bd, 0x4b},
        {0x80be, 0x19},
        {0x80bf, 0x2f},

        {0x80c9, 0xa0},
        {0x80ca, 0xcb},
        {0x80cb, 0xf6},
        {0x80cc, 0x04},
        {0x80cd, 0x9e},
        {0x80ce, 0x06},
        {0x80cf, 0xf5},
        {0x80d0, 0xc4},
        {0x80d1, 0x4c},
        {0x80d2, 0x19},
        {0x80d3, 0x2f},

        {0x80c0, 0x0c},
        {0x80d4, 0x0c}};
void alg_tune_giga_6818B_220617(rtk_uint32 phymask)
{
    rtk_uint32 port, i, uc1_sram_addr, uc1_sram_val, len;

    len = sizeof(alg_tune_giga_6818B_220617_patch) / 4;

    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                uc1_sram_addr = alg_tune_giga_6818B_220617_patch[i][0];
                uc1_sram_val = alg_tune_giga_6818B_220617_patch[i][1];
                uc1_sram_write_8b(port, uc1_sram_addr, uc1_sram_val);
            }

            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa87e, 0x1f << 8, 0x6);
        }
    }
}

void afe_patch_6818B_220607(rtk_uint32 phymask)
{
    //   #cen port 在baseaddr+1 port: e.x. p1/p5
    //   #adjust LDO to improve TM2 jitter
    rtk_uint32 port;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xbf84, 0x7 << 0, 0x4);
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xbf8c, 0x1f << 6, 0x0);

            // setPhyMaskRegBits(1<<port,31,0xbf84,2,0,4);
            // setPhyMaskRegBits(1<<port,31,0xbf8c,10,6,0);
        }
    }
}

rtk_uint16 rtct_para_6818B_220713_patch[][2]=
{
    {0x81a3, 0x2e}, {0x81a4, 0xe0}, {0x81a5, 0x2e}, {0x81a6, 0xe0},
    {0x81a8, 0x1d}, {0x81a9, 0x00}, {0x81af, 0x2d}, {0x81b0, 0x05},
    {0x81b2, 0x09}, {0x81b3, 0x1f}, {0x81bc, 0x1d}, {0x81bd, 0x00},
    {0x81be, 0x00}, {0x81bf, 0xea}, {0x81c0, 0x03}, {0x81c1, 0xca},
    {0x81c2, 0x1d}, {0x81c3, 0x00}, {0x81c4, 0x00}, {0x81c5, 0x3b},
    {0x81c6, 0x00}, {0x81c7, 0x1a}, {0x81c8, 0x00}, {0x81c9, 0x54},
    {0x81ca, 0xff}, {0x81cb, 0xd5}, {0x81cc, 0x07}, {0x81cd, 0xfa},
    {0x81ce, 0xff}, {0x81cf, 0x14}, {0x81d0, 0x00}, {0x81d1, 0x74},
    {0x81d2, 0xff}, {0x81d3, 0xdb}, {0x81d4, 0x09}, {0x81d5, 0xf0},
    {0x81d6, 0xff}, {0x81d7, 0x97}, {0x81d8, 0xff}, {0x81d9, 0xc2},
    {0x81da, 0x00}, {0x81db, 0x28}, {0x81dc, 0xf1}, {0x81dd, 0xa0},
    {0x81de, 0x00}, {0x81df, 0x94}, {0x81e0, 0x00}, {0x81e1, 0x76},
    {0x81e2, 0xff}, {0x81e3, 0xb8}, {0x81e4, 0xeb}, {0x81e5, 0xd0},
    {0x81e6, 0x01}, {0x81e7, 0x6b}, {0x81e8, 0xff}, {0x81e9, 0xaa},
    {0x81ea, 0x00}, {0x81eb, 0x06}, {0x81ec, 0x05}, {0x81ed, 0x0a},
    {0x81ee, 0xff}, {0x81ef, 0x9d}, {0x81f0, 0x00}, {0x81f1, 0x10},
    {0x81f2, 0x00}, {0x81f3, 0x05}, {0x81f4, 0x01}, {0x81f5, 0xe0},
    {0x81f6, 0x00}, {0x81f7, 0x00}, {0x81f8, 0x00}, {0x81f9, 0x00},
    {0x81fa, 0x00}, {0x81fb, 0x00}
};

void rtct_para_6818B_220713(rtk_uint16 phymask)
{

    rtk_uint16 port, i, addr, val, len;

    len = sizeof(rtct_para_6818B_220713_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = rtct_para_6818B_220713_patch[i][0];
                val = rtct_para_6818B_220713_patch[i][1];
                uc1_sram_write_8b(port, addr, val);
            }
        }
    }
}

rtk_uint16 rtct_para_6818B_221211_patch[][4]=
{
    {0xa436, 15, 0, 0x81a3}, {0xa438, 15, 8, 0x32}, {0xa436, 15, 0, 0x81a4}, {0xa438, 15, 8, 0xc0}, 
    {0xa436, 15, 0, 0x81a5}, {0xa438, 15, 8, 0x32}, {0xa436, 15, 0, 0x81a8}, {0xa438, 15, 8, 0x1d}, 
    {0xa436, 15, 0, 0x81af}, {0xa438, 15, 8, 0x2d}, {0xa436, 15, 0, 0x81b2}, {0xa438, 15, 8, 0x09}, 
    {0xa436, 15, 0, 0x81b6}, {0xa438, 15, 8, 0x48}, {0xa436, 15, 0, 0x81b7}, {0xa438, 15, 8, 0x48}, 
    {0xa436, 15, 0, 0x81b8}, {0xa438, 15, 8, 0x08}, {0xa436, 15, 0, 0x81b9}, {0xa438, 15, 8, 0x04}, 
    {0xa436, 15, 0, 0x81ba}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81bb}, {0xa438, 15, 8, 0x20}, 
    {0xa436, 15, 0, 0x81bc}, {0xa438, 15, 8, 0x04}, {0xa436, 15, 0, 0x81bd}, {0xa438, 15, 8, 0x20}, 
    {0xa436, 15, 0, 0x81be}, {0xa438, 15, 8, 0x1a}, {0xa436, 15, 0, 0x81bf}, {0xa438, 15, 8, 0xe0}, 
    {0xa436, 15, 0, 0x81c0}, {0xa438, 15, 8, 0x01}, {0xa436, 15, 0, 0x81c1}, {0xa438, 15, 8, 0x3a}, 
    {0xa436, 15, 0, 0x81c2}, {0xa438, 15, 8, 0x1c}, {0xa436, 15, 0, 0x81c3}, {0xa438, 15, 8, 0x60}, 
    {0xa436, 15, 0, 0x81c4}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81c5}, {0xa438, 15, 8, 0x11}, 
    {0xa436, 15, 0, 0x81c6}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81c7}, {0xa438, 15, 8, 0xcf}, 
    {0xa436, 15, 0, 0x81c8}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81c9}, {0xa438, 15, 8, 0xb0}, 
    {0xa436, 15, 0, 0x81ca}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81cb}, {0xa438, 15, 8, 0x0f}, 
    {0xa436, 15, 0, 0x81cc}, {0xa438, 15, 8, 0x0d}, {0xa436, 15, 0, 0x81cd}, {0xa438, 15, 8, 0xad}, 
    {0xa436, 15, 0, 0x81ce}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81cf}, {0xa438, 15, 8, 0x0e}, 
    {0xa436, 15, 0, 0x81d0}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81d1}, {0xa438, 15, 8, 0xbe}, 
    {0xa436, 15, 0, 0x81d2}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81d3}, {0xa438, 15, 8, 0x18}, 
    {0xa436, 15, 0, 0x81d4}, {0xa438, 15, 8, 0x08}, {0xa436, 15, 0, 0x81d5}, {0xa438, 15, 8, 0x70}, 
    {0xa436, 15, 0, 0x81d6}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81d7}, {0xa438, 15, 8, 0x37}, 
    {0xa436, 15, 0, 0x81d8}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81d9}, {0xa438, 15, 8, 0x48}, 
    {0xa436, 15, 0, 0x81da}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81db}, {0xa438, 15, 8, 0xf4}, 
    {0xa436, 15, 0, 0x81dc}, {0xa438, 15, 8, 0xeb}, {0xa436, 15, 0, 0x81dd}, {0xa438, 15, 8, 0xa0}, 
    {0xa436, 15, 0, 0x81de}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81df}, {0xa438, 15, 8, 0x2b}, 
    {0xa436, 15, 0, 0x81e0}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81e1}, {0xa438, 15, 8, 0x9c}, 
    {0xa436, 15, 0, 0x81e2}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81e3}, {0xa438, 15, 8, 0xb2}, 
    {0xa436, 15, 0, 0x81e4}, {0xa438, 15, 8, 0xeb}, {0xa436, 15, 0, 0x81e5}, {0xa438, 15, 8, 0xaf}, 
    {0xa436, 15, 0, 0x81e6}, {0xa438, 15, 8, 0x02}, {0xa436, 15, 0, 0x81e7}, {0xa438, 15, 8, 0x92}, 
    {0xa436, 15, 0, 0x81e8}, {0xa438, 15, 8, 0xfe}, {0xa436, 15, 0, 0x81e9}, {0xa438, 15, 8, 0xe4}, 
    {0xa436, 15, 0, 0x81ea}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81eb}, {0xa438, 15, 8, 0x3c}, 
    {0xa436, 15, 0, 0x81ec}, {0xa438, 15, 8, 0x06}, {0xa436, 15, 0, 0x81ed}, {0xa438, 15, 8, 0xf2}, 
    {0xa436, 15, 0, 0x81ee}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81ef}, {0xa438, 15, 8, 0x3a}, 
    {0xa436, 15, 0, 0x81f0}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81f1}, {0xa438, 15, 8, 0x8a}, 
    {0xa436, 15, 0, 0x81f2}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81f3}, {0xa438, 15, 8, 0xd0}, 
    {0xa436, 15, 0, 0x81f4}, {0xa438, 15, 8, 0x0a}, {0xa436, 15, 0, 0x81f5}, {0xa438, 15, 8, 0x09}, 
    {0xa436, 15, 0, 0x81f6}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81f7}, {0xa438, 15, 8, 0xdc}, 
    {0xa436, 15, 0, 0x81f8}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81f9}, {0xa438, 15, 8, 0x30}, 
    {0xa436, 15, 0, 0x81fa}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x8700}, {0xa438, 15, 8, 0x01}, 
    {0xa436, 15, 0, 0x8701}, {0xa438, 15, 8, 0x04}, {0xa436, 15, 0, 0x8018}, {0xa438, 15, 8, 0x70}, 
    {0xa436, 15, 0, 0x81a6}, {0xa438, 15, 8, 0xc0}, {0xa436, 15, 0, 0x81a9}, {0xa438, 15, 8, 0x00}, 
    {0xa436, 15, 0, 0x81b0}, {0xa438, 15, 8, 0x05}, {0xa436, 15, 0, 0x81b3}, {0xa438, 15, 8, 0x1c}, 
    {0xa436, 15, 0, 0x81fb}, {0xa438, 15, 8, 0x6c}, {0xa436, 15, 0, 0x8702}, {0xa438, 15, 8, 0x50}
};

void rtct_para_6818B_221211(rtk_uint32 phymask)
{

    rtk_uint16 port, i, addr, val, len, msb, lsb;
    rtk_uint32 maskbits, j, masklen;

    len = sizeof(rtct_para_6818B_221211_patch) / 8;

    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = rtct_para_6818B_221211_patch[i][0];
                msb = rtct_para_6818B_221211_patch[i][1];
                lsb = rtct_para_6818B_221211_patch[i][2];
                val = rtct_para_6818B_221211_patch[i][3];

                masklen = msb - lsb;
                maskbits = 1;
                for (j = 0; j < masklen; j++)
                {
                    maskbits = (maskbits << 1) | 1;
                }
                maskbits = maskbits << lsb;
                dal_rtl8373_phy_regbits_write(1 << port, 31, addr, maskbits, val);
            }
        }
    }
}

rtk_api_ret_t SDS_MODE_SET_SW(rtk_uint32 CHIP_MODE, rtk_uint32 SDS_INDX, rtk_uint32 SDS_MODE)
{

    if ((CHIP_MODE != CHIP_RTL8224_MODE) && (CHIP_MODE != CHIP_RTL8224N_MODE))
    {
        if (SDS_MODE == SERDES_10GQXG)
        {
            if (SDS_INDX == 0)
            {
                rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_USX_SUB_MODE_MASK, 2);
                rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK, 0xD);
            }
            else if (SDS_INDX == 1)
            {
                rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_USX_SUB_MODE_MASK, 2);
                rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_MODE_SEL_MASK, 0xD);
            }
        }
        else
        {
            if (SDS_INDX == 0)
            {
                rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_USX_SUB_MODE_MASK, 0);
                rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK, SDS_MODE);
            }
            else if (SDS_INDX == 1)
            {
                rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_USX_SUB_MODE_MASK, 0);
                rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_MODE_SEL_MASK, SDS_MODE);
            }
        }

        /*8221B SETTING OUTSIDE THIS PROGRES*/
        serdes_patch(CHIP_MODE, SDS_INDX, SDS_MODE);
        fiber_fc_en(SDS_INDX, SDS_MODE, SDS_FIBER_FC_EN);
        sds_nway_set(SDS_INDX, SDS_MODE, SDS_NWAY_EN);

        delay_loop(1000);
        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x3);
        delay_loop(10);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x1);
        delay_loop(100);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x1);
        delay_loop(10);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x3);
        delay_loop(100);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x3);
        delay_loop(10);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x1);
        delay_loop(10);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x1);
        delay_loop(10);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x3);
        delay_loop(100);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x0);
        delay_loop(10);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x3);
        delay_loop(10);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x1);
        delay_loop(100);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x0);
        delay_loop(10);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x1);
        delay_loop(10);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x3);
        delay_loop(100);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x0);
        delay_loop(100);

        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x1F, 0x00, 0xffff << 0, 0xB);
        delay_loop(100);
        dal_rtl8373_sds_regbits_write(SDS_INDX, 0x1F, 0x00, 0xffff << 0, 0x0);
        delay_loop(100);
    }
    else
    {
        if (SDS_MODE == SERDES_10GQXG)
        {
            if (SDS_INDX == 0)
            {
                dal_rtl8224_top_regbits_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_USX_SUB_MODE_MASK, 2);
                dal_rtl8224_top_regbits_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK, 0xD);
            }
            else if (SDS_INDX == 1)
            {
                dal_rtl8224_top_regbits_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_USX_SUB_MODE_MASK, 2);
                dal_rtl8224_top_regbits_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_MODE_SEL_MASK, 0xD);
            }
        }
        else
        {
            if (SDS_INDX == 0)
            {
                dal_rtl8224_top_regbits_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_USX_SUB_MODE_MASK, 0);
                dal_rtl8224_top_regbits_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK, SDS_MODE);
            }
            else if (SDS_INDX == 1)
            {
                dal_rtl8224_top_regbits_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_USX_SUB_MODE_MASK, 0);
                dal_rtl8224_top_regbits_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS1_MODE_SEL_MASK, SDS_MODE);
            }
        }

        dal_rtl8224_top_regbit_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_CFG_MAC3_8221B_OFFSET, 0);
        dal_rtl8224_top_regbit_write(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_CFG_MAC8_8221B_OFFSET, 0);
        serdes_patch(CHIP_MODE, SDS_INDX, SDS_MODE);
        delay_loop(1000);
        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x3);
        delay_loop(10);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x1);
        delay_loop(100);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x1);
        delay_loop(10);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x3);
        delay_loop(100);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x3);
        delay_loop(10);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x1);
        delay_loop(10);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x1);
        delay_loop(10);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x3);
        delay_loop(100);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 10, 0x0);
        delay_loop(10);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x3);
        delay_loop(10);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x1);
        delay_loop(100);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 6, 0x0);
        delay_loop(10);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x1);
        delay_loop(10);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x3);
        delay_loop(100);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x20, 0x00, 0x3 << 4, 0x0);
        delay_loop(100);

        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x1F, 0x00, 0xffff << 0, 0xB);
        delay_loop(100);
        dal_rtl8224_sds_regbits_write(SDS_INDX, 0x1F, 0x00, 0xffff << 0, 0x0);
        delay_loop(100);
    }

    return RT_ERR_OK;
}

void get_version_8373(void)
{
    rtl8373_setAsicRegBits(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_CHIP_INFO_EN_MASK, 0xa);
    rtl8373_getAsicRegBits(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_RL_VID_MASK, &Ver8373_72);
    rtl8373_getAsicRegBits(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_RL_VID_MASK, &Ver8373N);
    rtl8373_getAsicRegBits(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_RL_VID_MASK, &Ver8372N);
    rtl8373_setAsicRegBits(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_CHIP_INFO_EN_MASK, 0);
}

void get_version_8224(void)
{

    dal_rtl8224_top_regbits_write(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_CHIP_INFO_EN_MASK, 0xa);
    dal_rtl8224_top_regbits_read(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_RL_VID_MASK, &Ver8224);
    dal_rtl8224_top_regbits_read(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_RL_VID_MASK, &Ver8224N);
    dal_rtl8224_top_regbits_write(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_CHIP_INFO_EN_MASK, 0);
}

void get_version_8366u(void)
{
    rtl8373_setAsicRegBits(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_CHIP_INFO_EN_MASK, 0xa);
    rtl8373_getAsicRegBits(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_RL_VID_MASK, &Ver8366U);
    rtl8373_setAsicRegBits(RTL8373_CHIP_INFO_ADDR, RTL8373_CHIP_INFO_CHIP_INFO_EN_MASK, 0);
}

void fiber_fc_en(rtk_uint32 SDS_INDX, rtk_uint32 SDS_MODE, rtk_uint32 fc_en)
{
    switch (SDS_MODE)
    {
    case SERDES_100FX:
        dal_rtl8373_sds_regbits_write(SDS_INDX, 31, 5, 0x1 << 2, 0x1);
        dal_rtl8373_sds_regbits_write(SDS_INDX, 31, 5, 0x1 << 3, 0x1);
        if (fc_en)
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 2, 4, 11, 10, 0x3);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 2, 4, 0x3 << 10, 0x3);
        }
        else
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 2, 4, 11, 10, 0x0);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 2, 4, 0x3 << 10, 0x0);
        }
        break;
    case SERDES_1000BASEX:
    case SERDES_2500BASEX:
        dal_rtl8373_sds_regbits_write(SDS_INDX, 31, 5, 0x1 << 2, 0x1);
        dal_rtl8373_sds_regbits_write(SDS_INDX, 31, 5, 0x1 << 3, 0x0);
        if (fc_en)
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 2, 4, 8, 7, 0x3);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 2, 4, 0x3 << 7, 0x3);
        }
        else
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 2, 4, 8, 7, 0x0);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 2, 4, 0x3 << 7, 0x0);
        }
        break;
    case SERDES_10GR:
        if (fc_en)
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 31, 11, 3, 2, 0x3);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 31, 11, 0x3 << 2, 0x3);
        }
        else
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 31, 11, 3, 2, 0x0);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 31, 11, 0x3 << 2, 0x0);
        }
        break;
    default:
        break;
    }
}

void sds_nway_set(rtk_uint32 SDS_INDX, rtk_uint32 SDS_MODE, rtk_uint32 an_en)
{
    switch (SDS_MODE)
    {
    case SERDES_100FX:
    case SERDES_10GR:
        break;
    case SERDES_1000BASEX:
    case SERDES_2500BASEX:
    case SERDES_SG:
    case SERDES_HSG:
        if (an_en)
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 2, 0, 12, 12, 0x1);
            // rtl8371b_sds_reg_setbits(SDS_INDX, 0, 4, 2, 2, 0x1);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 0, 2, 0x3 << 8, 0x3);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 0, 4, 0x1 << 2, 0x1);
        }
        else
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 2, 0, 12, 12, 0x0);
            // rtl8371b_sds_reg_setbits(SDS_INDX, 0, 4, 2, 2, 0x1);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 0, 2, 0x3 << 8, 0x1);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 0, 4, 0x1 << 2, 0x1);
        }
        break;
    case SERDES_10GUSXG:
    case SERDES_10GQXG:
        if (an_en)
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 7, 17, 3, 0, 0xf);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 7, 17, 0xf << 0, 0xf);
        }
        else
        {
            // rtl8371b_sds_reg_setbits(SDS_INDX, 7, 17, 3, 0, 0x0);
            dal_rtl8373_sds_regbits_write(SDS_INDX, 7, 17, 0xf << 0, 0x0);
        }
        break;
    default:
        break;
    }
}

rtk_api_ret_t serdes_patch(rtk_uint32 CHIP_MODE, rtk_uint32 SDS_INDX, rtk_uint32 SDS_MODE)
{
    rtk_uint16 dig_patch[8][3] = {{0}};
    rtk_uint16 an_patch[18][3] = {{0}};
    rtk_uint16 dig_patch_len, an_patch_len, i;

    switch (SDS_MODE)
    {
    case SERDES_10GUSXG: // usxg
    case SERDES_10GQXG:
    case SERDES_10GR: // usxg

        if ((Ver8373_72 == 0) || (Ver8224 == 0))
        {
            an_patch_len = sizeof(an_10p3125g_chipa);

            memcpy(an_patch, an_10p3125g_chipa, an_patch_len);
        }
        else if ((Ver8373N == 2) || (Ver8224N == 2) || (Ver8366U == 2) || (Ver8372N == 2))
        {
            an_patch_len = sizeof(an_10p3125g_chipb);
            memcpy(an_patch, an_10p3125g_chipb, an_patch_len);
        }
        else
        {
            an_patch_len = sizeof(an_10p3125g_chipb);

            memcpy(an_patch, an_10p3125g_chipb, an_patch_len);
        }

        if ((CHIP_MODE == CHIP_RTL8224_MODE) && ((SDS_MODE == SERDES_10GUSXG) || (SDS_MODE == SERDES_10GQXG)))
        {
            dig_patch_len = sizeof(dig_patch_phy);
            memcpy(dig_patch, dig_patch_phy, dig_patch_len);
        }
        else if ((CHIP_MODE == CHIP_RTL8224N_MODE) && ((SDS_MODE == SERDES_10GUSXG) || (SDS_MODE == SERDES_10GQXG)))
        {
            dig_patch_len = sizeof(dig_patch_phy);
            memcpy(dig_patch, dig_patch_phy, dig_patch_len);
        }
        else
        {
            dig_patch_len = sizeof(dig_patch_mac);
            memcpy(dig_patch, dig_patch_mac, dig_patch_len);
        }
        break;

    case SERDES_HSG: // hisxg
    case SERDES_2500BASEX:
        if ((Ver8373_72 == 0) || (Ver8224 == 0)) // chipa
        {
            an_patch_len = sizeof(an_3p125g_chipa);
            memcpy(an_patch, an_3p125g_chipa, an_patch_len);
        }
        else if ((Ver8373N == 2) || (Ver8224N == 2) || (Ver8366U == 2) || (Ver8372N == 2))
        {
            an_patch_len = sizeof(an_3p125g_chipb);
            memcpy(an_patch, an_3p125g_chipb, an_patch_len);
        }
        else // chip b,c
        {
            an_patch_len = sizeof(an_3p125g_chipb);
            memcpy(an_patch, an_3p125g_chipb, an_patch_len);
        }

        if ((CHIP_MODE == CHIP_RTL8224_MODE) && (SDS_MODE == SERDES_HSG))
        {
            dig_patch_len = sizeof(dig_patch_phy);
            memcpy(dig_patch, dig_patch_phy, dig_patch_len);
        }
        else if ((CHIP_MODE == CHIP_RTL8224N_MODE) && (SDS_MODE == SERDES_HSG))
        {
            dig_patch_len = sizeof(dig_patch_phy);
            memcpy(dig_patch, dig_patch_phy, dig_patch_len);
        }
        else
        {
            dig_patch_len = sizeof(dig_patch_mac);
            memcpy(dig_patch, dig_patch_mac, dig_patch_len);
        }
        break;

    case SERDES_SG: // sgmii
    case SERDES_1000BASEX:
        if ((Ver8373_72 == 0) || (Ver8224 == 0)) // chipa
        {
            an_patch_len = sizeof(an_1p25g_chipa);
            memcpy(an_patch, an_1p25g_chipa, an_patch_len);
        }
        else if ((Ver8373N == 2) || (Ver8224N == 2) || (Ver8366U == 2) || (Ver8372N == 2))
        {
            an_patch_len = sizeof(an_1p25g_chipb);
            memcpy(an_patch, an_1p25g_chipb, an_patch_len);
        }
        else
        {
            an_patch_len = sizeof(an_1p25g_chipb);
            memcpy(an_patch, an_1p25g_chipb, an_patch_len);
        }

        if ((CHIP_MODE == CHIP_RTL8224_MODE) && (SDS_MODE == SERDES_SG))
        {
            dig_patch_len = sizeof(dig_patch_phy);
            memcpy(dig_patch, dig_patch_phy, dig_patch_len);
        }
        else if ((CHIP_MODE == CHIP_RTL8224N_MODE) && (SDS_MODE == SERDES_SG))
        {
            dig_patch_len = sizeof(dig_patch_phy);
            memcpy(dig_patch, dig_patch_phy, dig_patch_len);
        }
        else
        {
            dig_patch_len = sizeof(dig_patch_mac);
            memcpy(dig_patch, dig_patch_mac, dig_patch_len);
        }
        break;
    case SERDES_100FX:
        if ((Ver8373_72 == 0) || (Ver8224 == 0)) // chipa
        {
            an_patch_len = sizeof(an_125m_chipa);
            memcpy(an_patch, an_125m_chipa, an_patch_len);
        }
        else if ((Ver8373N == 2) || (Ver8224N == 2) || (Ver8366U == 2) || (Ver8372N == 2))
        {
            an_patch_len = sizeof(an_125m_chipb);
            memcpy(an_patch, an_125m_chipb, an_patch_len);
        }
        else
        {
            an_patch_len = sizeof(an_125m_chipb);
            memcpy(an_patch, an_125m_chipb, an_patch_len);
        }

        if (CHIP_MODE == CHIP_RTL8224_MODE)
        {

            dig_patch_len = sizeof(dig_patch_phy);
            memcpy(dig_patch, dig_patch_phy, dig_patch_len);
        }
        else if ((CHIP_MODE == CHIP_RTL8224N_MODE) && (SDS_MODE == SERDES_100FX))
        {
            dig_patch_len = sizeof(dig_patch_phy);
            memcpy(dig_patch, dig_patch_phy, dig_patch_len);
        }
        else
        {

            dig_patch_len = sizeof(dig_patch_mac);
            memcpy(dig_patch, dig_patch_mac, dig_patch_len);
        }
        break;
    default:
        return RT_ERR_INPUT;
        break;
    }

    switch (CHIP_MODE)
    {
    case CHIP_RTL8372_MODE:
    case CHIP_RTL8373_MODE:
    case CHIP_RTL8221B_MODE:
    case CHIP_RTL8366U_MODE:
    case CHIP_RTL8372N_MODE:
    case CHIP_RTL8373N_MODE:
        for (i = 0; i < an_patch_len / 6; i++)
        {
            dal_rtl8373_sds_reg_write(SDS_INDX, an_patch[i][0], an_patch[i][1], an_patch[i][2]);
        }
        for (i = 0; i < dig_patch_len / 6; i++)
        {
            dal_rtl8373_sds_reg_write(SDS_INDX, dig_patch[i][0], dig_patch[i][1], dig_patch[i][2]);
        }
        break;
    case CHIP_RTL8224_MODE:
    case CHIP_RTL8224N_MODE:
        for (i = 0; i < an_patch_len / 6; i++)
        {
            dal_rtl8224_sds_reg_write(SDS_INDX, an_patch[i][0], an_patch[i][1], an_patch[i][2]);
        }
        for (i = 0; i < dig_patch_len / 6; i++)
        {
            dal_rtl8224_sds_reg_write(SDS_INDX, dig_patch[i][0], dig_patch[i][1], dig_patch[i][2]);
        }
        break;
    default:
        return RT_ERR_OK;
        break;
    }

    return RT_ERR_OK;
}

rtk_api_ret_t fw_reset_flow_tgr_8224(rtk_uint32 sdsid)
{
    rtk_uint32 REG_31_21, RXIDLE, NSQ, SYNC_OK, REG_5_0, LINK_OK, HI_BER;
    ret_t retVal;
    rtk_uint32 rx_sts;

    retVal = dal_rtl8224_sds_regbits_read(sdsid, 0x20, 0, (0x3 << 4), &rx_sts);
    if (retVal != RT_ERR_OK)
        return retVal;
    if (rx_sts == 0x1) /// 1:rx off, do nothing; 0,2,3:rx on, continue;
    {
        return RT_ERR_OK;
    }
    // #切debug port 看SIG_OK

    retVal = dal_rtl8224_sds_regbits_write(sdsid, 0x21, 0, (1 << 2), 1); // #RX_Test_EN
    if (retVal != RT_ERR_OK)
        return retVal;

    retVal = dal_rtl8224_sds_regbits_write(sdsid, 0x36, 0x05, (0xf << 11), 8); // ##reg0_debug_sel
    if (retVal != RT_ERR_OK)
        return retVal;

    retVal = dal_rtl8224_sds_reg_write(sdsid, 0x1f, 0x2, 0x1f); // 切debug port
    if (retVal != RT_ERR_OK)
        return retVal;

    retVal = dal_rtl8224_sds_reg_read(sdsid, 0x1f, 0x15, &REG_31_21);
    if (retVal != RT_ERR_OK)
        return retVal;

    RXIDLE = ((REG_31_21 >> 7) & 1);
    NSQ = ((REG_31_21 >> 6) & 1);
    if (NSQ == 1 || RXIDLE == 0)
    {
        // #NSQ=1或RXIDLE=0后，若无SYNC_OK则 Rx reset
        retVal = dal_rtl8224_sds_reg_read(sdsid, 0x5, 0x0, &REG_5_0);
        if (retVal != RT_ERR_OK)
            return retVal;

        SYNC_OK = (REG_5_0 & 1);
        if (SYNC_OK == 0)
        {
            retVal = dal_rtl8224_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 3); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
            if (retVal != RT_ERR_OK)
                return retVal;
            delay_loop(100);
            retVal = dal_rtl8224_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 1); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
            if (retVal != RT_ERR_OK)
                return retVal;
            delay_loop(100);
            retVal = dal_rtl8224_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 3); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
            if (retVal != RT_ERR_OK)
                return retVal;
            delay_loop(100);
        }
        else
        {
            LINK_OK = ((REG_5_0 >> 12) & 1);
            HI_BER = ((REG_5_0 >> 1) & 1);
            if ((LINK_OK == 0) || (HI_BER == 1))
            {

                retVal = dal_rtl8224_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 3); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
                if (retVal != RT_ERR_OK)
                    return retVal;
                delay_loop(100);
                retVal = dal_rtl8224_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 1); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
                if (retVal != RT_ERR_OK)
                    return retVal;
                delay_loop(100);
                retVal = dal_rtl8224_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 3); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
                if (retVal != RT_ERR_OK)
                    return retVal;
                delay_loop(100);
            }
        }
    }

    return RT_ERR_OK;
}

rtk_api_ret_t fw_reset_flow_tgr(rtk_uint32 sdsid)
{
    rtk_uint32 REG_31_21, RXIDLE, NSQ, SYNC_OK, REG_5_0, LINK_OK, HI_BER, test;
    ret_t retVal;
    rtk_uint32 rx_sts;

    retVal = dal_rtl8373_sds_regbits_read(sdsid, 0x20, 0, (0x3 << 4), &rx_sts);
    if (retVal != RT_ERR_OK)
        return retVal;
    if (rx_sts == 0x1) /// 1:rx off, do nothing; 0,2,3:rx on, continue;
    {
        return RT_ERR_OK;
    }

    // #切debug port 看SIG_OK
    retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x21, 0, (1 << 2), 1); // #RX_Test_EN
    if (retVal != RT_ERR_OK)
        return retVal;

    retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x36, 0x5, (0xf << 11), 8); // ##reg0_debug_sel
    if (retVal != RT_ERR_OK)
        return retVal;

    retVal = dal_rtl8373_sds_reg_write(sdsid, 0x1f, 0x2, 0x1f); // 切debug port
    if (retVal != RT_ERR_OK)
        return retVal;

    /*REG_31_21=rtl8371b_sds_reg_get(sds,0x1F,0x15);*/
    retVal = dal_rtl8373_sds_reg_read(sdsid, 0x1f, 0x15, &REG_31_21);
    if (retVal != RT_ERR_OK)
        return retVal;

    RXIDLE = ((REG_31_21 >> 7) & 1);
    NSQ = ((REG_31_21 >> 6) & 1);
    if (NSQ == 1 || RXIDLE == 0)
    {
        // #NSQ=1或RXIDLE=0后，若无SYNC_OK则 Rx reset
        retVal = dal_rtl8373_sds_reg_read(sdsid, 0x5, 0x0, &REG_5_0);
        if (retVal != RT_ERR_OK)
            return retVal;

        SYNC_OK = (REG_5_0 & 1);
        if (SYNC_OK == 0)
        {
            retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 3); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
            if (retVal != RT_ERR_OK)
                return retVal;
            delay_loop(100);
            retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 1); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
            if (retVal != RT_ERR_OK)
                return retVal;
            delay_loop(100);
            retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 3); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
            if (retVal != RT_ERR_OK)
                return retVal;
            delay_loop(100);
            retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 0); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
            if (retVal != RT_ERR_OK)
                return retVal;
            delay_loop(100);
            retVal = dal_rtl8373_sds_reg_read(sdsid, 0x5, 0, &test);
            if (retVal != RT_ERR_OK)
                return retVal;
        }
        else
        {
            retVal = dal_rtl8373_sds_reg_read(sdsid, 0x5, 0, &REG_5_0);
            if (retVal != RT_ERR_OK)
                return retVal;

            LINK_OK = ((REG_5_0 >> 12) & 1);
            HI_BER = ((REG_5_0 >> 1) & 1);
            // puts "LINK_OK = $LINK_OK, HI_BER = $HI_BER;"
            if ((LINK_OK == 0) || (HI_BER == 1))
            {
                retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 3); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
                if (retVal != RT_ERR_OK)
                    return retVal;
                delay_loop(100);
                retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 1); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
                if (retVal != RT_ERR_OK)
                    return retVal;
                delay_loop(100);
                retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 3); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
                if (retVal != RT_ERR_OK)
                    return retVal;
                delay_loop(100);
                retVal = dal_rtl8373_sds_regbits_write(sdsid, 0x20, 0, (0x3 << 4), 0); // FRC_RX_EN_VAL[5] FRC_RX_EN_ON[4]
                if (retVal != RT_ERR_OK)
                    return retVal;
            }
        }
    }

    return RT_ERR_OK;
}

void afe_patch_6818_220325(void)
{
    if (CHIP_TYPE == CHIP_RTL8373_MODE)
    {
        // setPhyMaskRegBits(1<<1|1<<5,31,0xbf84,2,0,7);
        // setPhyMaskRegBits(1<<1|1<<5,31,0xbf8c,10,6,0);
        dal_rtl8373_phy_regbits_write(1 << 1 | 1 << 5, 31, 0xbf84, 0x7, 7);
        dal_rtl8373_phy_regbits_write(1 << 1 | 1 << 5, 31, 0xbf8c, 0x1f << 6, 0);
    }
    else if (CHIP_TYPE == CHIP_RTL8372_MODE)
    {
        // setPhyMaskRegBits(1<<5,31,0xbf84,2,0,7);
        // setPhyMaskRegBits(1<<5,31,0xbf8c,10,6,0);
        dal_rtl8373_phy_regbits_write(1 << 5, 31, 0xbf84, 0x7, 7);
        dal_rtl8373_phy_regbits_write(1 << 5, 31, 0xbf8c, 0x1f << 6, 0);
    }
}

void alg_tune_fnet_6818_220628(rtk_uint32 phymask)
{
    rtk_uint16 port, i, data_ram_addr, data_ram_val, len;

    len = sizeof(alg_tune_fnet_6818_220628_patch) / 4;

    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                data_ram_addr = alg_tune_fnet_6818_220628_patch[i][0];
                data_ram_val = alg_tune_fnet_6818_220628_patch[i][1];
                uc1_sram_write_8b(port, data_ram_addr, data_ram_val);
            }
        }
    }
}

void RL6818B_pwr_on_patch_phy_v009_rls_lockmain(rtk_uint32 phymask)
{
    rtk_uint32 port, pcs_state, counter, tmp;

    for (port = 0; port < 8; port++)
    {
        tmp = (1 << port) & phymask;
        if (tmp == 0)
            continue;

        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa4a0, 1 << 10, 0);
        counter = 30;
        do
        {
            dal_rtl8373_phy_regbits_read(port, 31, 0xa600, 0xff, &pcs_state);
            counter--;
        } while (!(pcs_state == 1 || counter == 0));

        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, 0x801e);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, currentVersion);
    }
}

void RL6818B_pwr_on_patch_phy_v009(rtk_uint32 phymask)
{
    rtk_uint32 port, fnet_patch_en, giga_patch_en, xg_patch_en, patch_key_addr, patch_key;
    rtk_uint32 patch_rdy, counter, pcs_state;
    rtk_uint32 sel_patch_nc0, sel_patch_nc1, sel_patch_nc2, sel_patch_uc, sel_patch_uc2;
    rtk_uint32 tmp, regdata;
    rtk_uint32 ICVersion;

    currentVersion = 0x9;

    for (port = 0; port < 8; port++)
    {
        tmp = (1 << port) & phymask;
        if (tmp == 0)
            continue;

        ICVersion = uc1_sram_read_8b(port, 0x0005);
        if (ICVersion != 1)
            continue;

        dal_rtl8373_phy_write(1 << port, 31, 0xa436, 0x801e);

        dal_rtl8373_phy_read(port, 31, 0xa438, &regdata);
        FWVersion = regdata;
        if (FWVersion == currentVersion)
            continue;

        fnet_patch_en = 1;
        giga_patch_en = 1;
        xg_patch_en = 1;

        patch_key_addr = 0x8023;
        patch_key = 0x1801;

        sel_patch_nc0 = xg_patch_en;
        sel_patch_nc1 = xg_patch_en;
        sel_patch_nc2 = xg_patch_en;
        sel_patch_uc = xg_patch_en;
        sel_patch_uc2 = xg_patch_en;

        if (sel_patch_nc0 | sel_patch_nc1 | sel_patch_nc2 | sel_patch_uc | sel_patch_uc2)
        {

            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb820, 1 << 4, 1);

            counter = 30;
            patch_rdy = 0;

            do
            {
                dal_rtl8373_phy_regbits_read(port, 31, 0xb800, 1 << 6, &patch_rdy);
                delay_loop(10);
                counter--;
            } while (!(patch_rdy == 1 || counter == 0));

            // # Set patch_key & patch_lock
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, patch_key_addr);
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, patch_key);
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, 0xb82e);
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, 1);

            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb820, 1 << 7, 1);

            if (sel_patch_nc0)
            {
                n0_patch_RL6818B_230724(1 << port);
            }
            if (sel_patch_nc1)
            {
            }
            if (sel_patch_nc2)
            {
                n2_patch_6818B_230724(1 << port);
            }
            if (sel_patch_uc2)
            {
                uc2_patch_6818B_230815(1 << port);
            }
        }

        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb820, 1 << 7, 0);

        if (sel_patch_uc)
        {
            uc_patch_6818B_20221211(1 << port);
        }

        // # ----------------------------- data_ram_patch START--------------------------------

        data_ram_patch_6818B_220714(1 << port);

        //  # Clear patch_key & patch_lock
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, 0);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, 0);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb82e, 1, 0);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, patch_key_addr);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, 0);

        // # Release patch request & wait patch_rdy = 0
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb820, 1 << 4, 0);

        counter = 30;
        do
        {
            delay_loop(10);
            dal_rtl8373_phy_regbits_read(port, 31, 0xb800, 1 << 6, &patch_rdy);
            counter--;
        } while (!(patch_rdy == 0 || counter == 0));

        // ## Lock Main
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa4a0, 1 << 10, 1);

        counter = 30;
        do
        {
            delay_loop(10);
            dal_rtl8373_phy_regbits_read(port, 31, 0xa600, 0xff, &pcs_state);
            counter--;
        } while (!(pcs_state == 1 || counter == 0));

        if (xg_patch_en)
        {
            alg_tune_2p5G_6818B_220701(1 << port);
        }
        if (fnet_patch_en || giga_patch_en)
        {
            alg_tune_giga_6818B_220617(1 << port);
            alg_tune_fnet_6818_220628(1 << port);
        }

        rtct_para_6818B_221211(1 << port);

        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa86a, 1, 0);
        uc1_sram_write_8b(port, 0x8ffb, 0x1);
        uc1_sram_write_8b(port, 0x80dc, 0xa);
        uc1_sram_write_8b(port, 0x8378, 0x22);

        uc2_sram_write_8b(port, 0x8384, 0x04);
        uc2_sram_write_8b(port, 0x8fd4, 0x00);
        uc2_sram_write_8b(port, 0x8fd5, 0x00);
        uc2_sram_write_8b(port, 0x8fd6, 0x0c);
        uc2_sram_write_8b(port, 0x8fd7, 0x80);
        uc2_sram_write_8b(port, 0x8fd8, 0x0a);
        uc2_sram_write_8b(port, 0x8fd9, 0x19);
        uc2_sram_write_8b(port, 0x8fda, 0x19);
        uc2_sram_write_8b(port, 0x8fdb, 0x00);
        uc2_sram_write_8b(port, 0x8fdc, 0x00);
        uc2_sram_write_8b(port, 0x8fdd, 0x00);
        uc2_sram_write_8b(port, 0x8fde, 0x00);
        uc2_sram_write_8b(port, 0x8fdf, 0x20);
        uc2_sram_write_8b(port, 0x8fe0, 0x0c);

        afe_patch_6818B_220607(1 << port);

        dal_rtl8373_phy_write(1 << port, 31, 0xa5d0, 0);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa428, 1 << 9, 0);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa47e, 3 << 6, 1);
    }

    rtlglue_printf("%d, RL6818B_pwr_on_patch_phy_v009 , patch 0x%x finished!\n", __LINE__, phymask);
}

void RL6818C_pwr_on_patch_phy_v008_rls_lockmain(rtk_uint32 phymask)
{
    rtk_uint32 port, pcs_state, counter, tmp;

    for (port = 0; port < 8; port++)
    {
        tmp = (1 << port) & phymask;
        if (tmp == 0)
            continue;

        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa4a0, 1 << 10, 0);
        counter = 30;
        do
        {
            dal_rtl8373_phy_regbits_read(port, 31, 0xa600, 0xff, &pcs_state);
            counter--;
        } while (pcs_state == 1 || counter == 0);

        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, 0x801e);

        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, currentVersion);
    }
}

void RL6818C_pwr_on_patch_phy_v008(rtk_uint32 phymask)
{
    rtk_uint32 port, xg_patch_en, patch_key_addr, patch_key;
    rtk_uint32 patch_rdy, counter, pcs_state;
    rtk_uint32 sel_patch_nc0, sel_patch_nc1, sel_patch_nc2, sel_patch_uc, sel_patch_uc2;
    rtk_uint32 tmp, regdata;
    rtk_uint32 ICVersion;

    currentVersion = 0x008;

    for (port = 0; port < 8; port++)
    {
        tmp = (1 << port) & phymask;
        if (tmp == 0)
            continue;

        ICVersion = uc1_sram_read_8b(port, 0x0005);
        if (ICVersion != 2)
            continue;

        dal_rtl8373_phy_write(1 << port, 31, 0xa436, 0x801e);

        dal_rtl8373_phy_read(port, 31, 0xa438, &regdata);
        FWVersion = regdata;
        if (FWVersion == currentVersion)
            continue;

        xg_patch_en = 1;

        patch_key_addr = 0x8023;
        patch_key = 0x1802;

        sel_patch_nc0 = xg_patch_en;
        sel_patch_nc1 = xg_patch_en;
        sel_patch_nc2 = xg_patch_en;
        sel_patch_uc = xg_patch_en;
        sel_patch_uc2 = xg_patch_en;

        if (sel_patch_nc0 | sel_patch_nc1 | sel_patch_nc2 | sel_patch_uc | sel_patch_uc2)
        {

            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb820, 1 << 4, 1);

            counter = 30;
            patch_rdy = 0;

            do
            {
                dal_rtl8373_phy_regbits_read(port, 31, 0xb800, 1 << 6, &patch_rdy);
                delay_loop(10);
                counter--;
            } while (patch_rdy != 1 || counter == 0);

            // # Set patch_key & patch_lock
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, patch_key_addr);
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, patch_key);
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, 0xb82e);
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, 1);

            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb820, 1 << 7, 1);

            if (sel_patch_nc0)
            {
                n0_patch_RL6818C_230703(1 << port);
            }
            if (sel_patch_nc1)
            {
            }
            if (sel_patch_nc2)
            {
                n2_patch_6818C_231206(1 << port);
            }
            if (sel_patch_uc2)
            {
                uc2_patch_6818C_231206(1 << port);
            }
        }

        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb820, 1 << 7, 0);

        if (sel_patch_uc)
        {
            uc_patch_6818C_221117(1 << port);
        }

        // # ----------------------------- data_ram_patch START--------------------------------
        data_ram_patch_6818C_221026(1 << port);

        //  # Clear patch_key & patch_lock
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, 0);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, 0);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb82e, 1, 0);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa436, 0xffff, patch_key_addr);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa438, 0xffff, 0);

        // # Release patch request & wait patch_rdy = 0
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb820, 1 << 4, 0);

        counter = 30;
        do
        {
            delay_loop(10);
            dal_rtl8373_phy_regbits_read(port, 31, 0xb800, 1 << 6, &patch_rdy);
            counter--;
        } while (patch_rdy != 0 || counter == 0);

        // ## Lock Main
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa4a0, 1 << 10, 1);

        counter = 30;
        do
        {
            delay_loop(10);
            dal_rtl8373_phy_regbits_read(port, 31, 0xa600, 0xff, &pcs_state);
            counter--;
        } while (pcs_state != 1 || counter == 0);

        // RTCT patch

        RTCT_para_6818C_231206(1 << port);

        uc1_sram_write_8b(port, 0x8ffb, 0x1);
        uc1_sram_write_8b(port, 0x80dc, 0xa);
        uc1_sram_write_8b(port, 0x8378, 0x22);

        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa47e, 0x3 << 6, 0x1);
        uc2_sram_write_8b(port, 0x8217, 0x1e);
        uc2_sram_write_8b(port, 0x8384, 0x4);

        // # add for sub_echo_chnest
        uc2_sram_write_8b(port, 0x8fd6, 0x00);
        uc2_sram_write_8b(port, 0x8fd7, 0x00);
        uc2_sram_write_8b(port, 0x8fd8, 0x0c);
        uc2_sram_write_8b(port, 0x8fd9, 0x80);
        uc2_sram_write_8b(port, 0x8fda, 0x0a);
        uc2_sram_write_8b(port, 0x8fdb, 0x19);
        uc2_sram_write_8b(port, 0x8fdc, 0x19);
        uc2_sram_write_8b(port, 0x8fdd, 0x00);
        uc2_sram_write_8b(port, 0x8fde, 0x00);
        uc2_sram_write_8b(port, 0x8fdf, 0x00);
        uc2_sram_write_8b(port, 0x8fe0, 0x00);
        uc2_sram_write_8b(port, 0x8fe1, 0x20);
        uc2_sram_write_8b(port, 0x8fe2, 0x0c);

        uc2_sram_write_8b(port, 0x8fd3, 0x00);
        uc2_sram_write_8b(port, 0x8fd4, 0x15);
        uc2_sram_write_8b(port, 0x8fd5, 0x15);

        afe_patch_6818C_220607(1 << port);

        dal_rtl8373_phy_write(1 << port, 31, 0xa5d0, 0);
        dal_rtl8373_phy_regbits_write(1 << port, 31, 0xa428, 1 << 9, 0);
    }

    rtlglue_printf("%d, RL6818C_pwr_on_patch_phy_v008 , patch 0x%x finished!\n", __LINE__, phymask);
}

rtk_uint16 rst_smtr_patch_6818C_230703_patch[][2]=
{
        {0xa436, 0xA016}, {0xa438, 0x0000}, {0xa436, 0xA012}, {0xa438, 0x0000}, 
        {0xa436, 0xA014}, {0xa438, 0x1800}, {0xa438, 0x8010}, {0xa438, 0x1800}, 
        {0xa438, 0x8023}, {0xa438, 0x1800}, {0xa438, 0x8027}, {0xa438, 0x1800}, 
        {0xa438, 0x802b}, {0xa438, 0x1800}, {0xa438, 0x8042}, {0xa438, 0x1800}, 
        {0xa438, 0x8052}, {0xa438, 0x1800}, {0xa438, 0x8062}, {0xa438, 0x1800}, 
        {0xa438, 0x8070}, {0xa438, 0x416e}, {0xa438, 0x1000}, {0xa438, 0x09cc}, 
        {0xa438, 0x1000}, {0xa438, 0x0a9b}, {0xa438, 0xd501}, {0xa438, 0xa103}, 
        {0xa438, 0x8203}, {0xa438, 0xd500}, {0xa438, 0x1800}, {0xa438, 0x0188}, 
        {0xa438, 0x1000}, {0xa438, 0x0aa1}, {0xa438, 0xd501}, {0xa438, 0x8103}, 
        {0xa438, 0xa203}, {0xa438, 0xd500}, {0xa438, 0x1800}, {0xa438, 0x0188}, 
        {0xa438, 0xac03}, {0xa438, 0xa138}, {0xa438, 0x1800}, {0xa438, 0x1ded}, 
        {0xa438, 0xac03}, {0xa438, 0xa138}, {0xa438, 0x1800}, {0xa438, 0x1cb8}, 
        {0xa438, 0xd504}, {0xa438, 0xaa18}, {0xa438, 0xd500}, {0xa438, 0x1000}, 
        {0xa438, 0x14e4}, {0xa438, 0xd719}, {0xa438, 0x7fac}, {0xa438, 0x1000}, 
        {0xa438, 0x14e4}, {0xa438, 0xd704}, {0xa438, 0x5fbc}, {0xa438, 0xd504}, 
        {0xa438, 0xa001}, {0xa438, 0xa1e0}, {0xa438, 0xd500}, {0xa438, 0xd031}, 
        {0xa438, 0xd1c5}, {0xa438, 0x1000}, {0xa438, 0x14e4}, {0xa438, 0xd704}, 
        {0xa438, 0x5fbc}, {0xa438, 0x1800}, {0xa438, 0x111f}, {0xa438, 0xbf80}, 
        {0xa438, 0xd712}, {0xa438, 0x4077}, {0xa438, 0xd71e}, {0xa438, 0x4159}, 
        {0xa438, 0xd4c0}, {0xa438, 0xd71e}, {0xa438, 0x6099}, {0xa438, 0x7f04}, 
        {0xa438, 0x1800}, {0xa438, 0x1bc0}, {0xa438, 0x9f80}, {0xa438, 0x1800}, 
        {0xa438, 0x1e2a}, {0xa438, 0x1800}, {0xa438, 0x1e2c}, {0xa438, 0xbf80}, 
        {0xa438, 0xd712}, {0xa438, 0x4077}, {0xa438, 0xd71e}, {0xa438, 0x4159}, 
        {0xa438, 0xd4c1}, {0xa438, 0xd71e}, {0xa438, 0x6099}, {0xa438, 0x7f04}, 
        {0xa438, 0x1800}, {0xa438, 0x1bc0}, {0xa438, 0x9f80}, {0xa438, 0x1800}, 
        {0xa438, 0x1cf6}, {0xa438, 0x1800}, {0xa438, 0x1cf9}, {0xa438, 0xd708}, 
        {0xa438, 0x3b0f}, {0xa438, 0x8067}, {0xa438, 0x1800}, {0xa438, 0x150c}, 
        {0xa438, 0xd503}, {0xa438, 0x8970}, {0xa438, 0x0c07}, {0xa438, 0x0901}, 
        {0xa438, 0xa008}, {0xa438, 0x8002}, {0xa438, 0xd500}, {0xa438, 0x1800}, 
        {0xa438, 0x1504}, {0xa436, 0xA026}, {0xa438, 0xffff}, {0xa436, 0xA024}, 
        {0xa438, 0x1500}, {0xa436, 0xA022}, {0xa438, 0x1cec}, {0xa436, 0xA020}, 
        {0xa438, 0x1e20}, {0xa436, 0xA006}, {0xa438, 0x1111}, {0xa436, 0xA004}, 
        {0xa438, 0x1cb7}, {0xa436, 0xA002}, {0xa438, 0x1dec}, {0xa436, 0xA000}, 
        {0xa438, 0x017f}, {0xa436, 0xA008}, {0xa438, 0x7f00}, {0xa436, 0xA012}, 
        {0xa438, 0x0ff8}, {0xa436, 0xA014}, {0xa438, 0xc114}, {0xa438, 0xd04a}, 
        {0xa438, 0xd069}, {0xa438, 0xd71f}, {0xa438, 0x40ff}, {0xa436, 0xA152}, 
        {0xa438, 0x0a20}, {0xa436, 0xA154}, {0xa438, 0x10f6}, {0xa436, 0xA156}, 
        {0xa438, 0x122c}, {0xa436, 0xA158}, {0xa438, 0x09af}, {0xa436, 0xA15a}, 
        {0xa438, 0x09b0}, {0xa436, 0xA150}, {0xa438, 0x001f}  
};

void n0_patch_RL6818C_230703(rtk_uint32 phymask)
{
    rtk_uint16 port, i, addr, val, len;

    len = sizeof(rst_smtr_patch_6818C_230703_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = rst_smtr_patch_6818C_230703_patch[i][0];
                val = rst_smtr_patch_6818C_230703_patch[i][1];

                dal_rtl8373_phy_write(1 << port, 31, addr, val);
            }
        }
    }
}

rtk_uint16 uc2_patch_6818C_231206_patch[][2]=
{
    {0xa436, 0xb87c}, {0xa438, 0x8592}, {0xa436, 0xb87e}, {0xa438, 0xaf85}, 
    {0xa438, 0xaaaf}, {0xa438, 0x85b3}, {0xa438, 0xaf85}, {0xa438, 0xb9af}, 
    {0xa438, 0x85bf}, {0xa438, 0xaf85}, {0xa438, 0xdeaf}, {0xa438, 0x85de}, 
    {0xa438, 0xaf85}, {0xa438, 0xdeaf}, {0xa438, 0x85de}, {0xa438, 0x0246}, 
    {0xa438, 0xb802}, {0xa438, 0x3f92}, {0xa438, 0xaf21}, {0xa438, 0x0102}, 
    {0xa438, 0x85de}, {0xa438, 0xaf20}, {0xa438, 0xd202}, {0xa438, 0x8945}, 
    {0xa438, 0xaf21}, {0xa438, 0x25f8}, {0xa438, 0xfbef}, {0xa438, 0x79fb}, 
    {0xa438, 0xe080}, {0xa438, 0x12ad}, {0xa438, 0x2504}, {0xa438, 0xee83}, 
    {0xa438, 0x8c00}, {0xa438, 0xd100}, {0xa438, 0xbf62}, {0xa438, 0xda02}, 
    {0xa438, 0x5feb}, {0xa438, 0xffef}, {0xa438, 0x97ff}, {0xa438, 0xfcaf}, 
    {0xa438, 0x4fe8}, {0xa438, 0xf8e0}, {0xa438, 0x8fff}, {0xa438, 0xa000}, 
    {0xa438, 0x0502}, {0xa438, 0x8602}, {0xa438, 0xae16}, {0xa438, 0xa001}, 
    {0xa438, 0x0502}, {0xa438, 0x869d}, {0xa438, 0xae0e}, {0xa438, 0xa002}, 
    {0xa438, 0x0502}, {0xa438, 0x8789}, {0xa438, 0xae06}, {0xa438, 0xa003}, 
    {0xa438, 0x0302}, {0xa438, 0x8832}, {0xa438, 0xfc04}, {0xa438, 0xf8f9}, 
    {0xa438, 0xfafb}, {0xa438, 0xef79}, {0xa438, 0xfbbf}, {0xa438, 0x628f}, 
    {0xa438, 0xe08f}, {0xa438, 0xd248}, {0xa438, 0x031a}, {0xa438, 0x9002}, 
    {0xa438, 0x600a}, {0xa438, 0x1f55}, {0xa438, 0xe38f}, {0xa438, 0xd91c}, 
    {0xa438, 0x450d}, {0xa438, 0x46e3}, {0xa438, 0x8fd8}, {0xa438, 0x1a43}, 
    {0xa438, 0xe48f}, {0xa438, 0xd6e5}, {0xa438, 0x8fd7}, {0xa438, 0xef64}, 
    {0xa438, 0x1f44}, {0xa438, 0x1f55}, {0xa438, 0xe38f}, {0xa438, 0xdbe1}, 
    {0xa438, 0x8fd9}, {0xa438, 0x1c45}, {0xa438, 0x0d46}, {0xa438, 0x1b64}, 
    {0xa438, 0xe38f}, {0xa438, 0xd8ef}, {0xa438, 0x75e3}, {0xa438, 0x8fda}, 
    {0xa438, 0x1a75}, {0xa438, 0x0288}, {0xa438, 0xb7ef}, {0xa438, 0x47e4}, 
    {0xa438, 0x8fdd}, {0xa438, 0xe58f}, {0xa438, 0xde1f}, {0xa438, 0x661f}, 
    {0xa438, 0x55e0}, {0xa438, 0x8fd6}, {0xa438, 0xe18f}, {0xa438, 0xd7e3}, 
    {0xa438, 0x8fdc}, {0xa438, 0x1a63}, {0xa438, 0xe38f}, {0xa438, 0xd91c}, 
    {0xa438, 0x650d}, {0xa438, 0x661a}, {0xa438, 0x64bf}, {0xa438, 0x65ef}, 
    {0xa438, 0xe08f}, {0xa438, 0xd248}, {0xa438, 0x031a}, {0xa438, 0x9002}, 
    {0xa438, 0x600a}, {0xa438, 0xac2a}, {0xa438, 0x05d7}, {0xa438, 0x017f}, 
    {0xa438, 0xae03}, {0xa438, 0xd700}, {0xa438, 0xff02}, {0xa438, 0x88c7}, 
    {0xa438, 0xef47}, {0xa438, 0xe48f}, {0xa438, 0xdfe5}, {0xa438, 0x8fe0}, 
    {0xa438, 0xee8f}, {0xa438, 0xff01}, {0xa438, 0xae03}, {0xa438, 0x0222}, 
    {0xa438, 0x14ff}, {0xa438, 0xef97}, {0xa438, 0xfffe}, {0xa438, 0xfdfc}, 
    {0xa438, 0x04f8}, {0xa438, 0xf9fa}, {0xa438, 0xfbef}, {0xa438, 0x79fb}, 
    {0xa438, 0x0287}, {0xa438, 0x1abf}, {0xa438, 0x628f}, {0xa438, 0xe08f}, 
    {0xa438, 0xd248}, {0xa438, 0x031a}, {0xa438, 0x9002}, {0xa438, 0x600a}, 
    {0xa438, 0x1f55}, {0xa438, 0xe38f}, {0xa438, 0xd51b}, {0xa438, 0x45ab}, 
    {0xa438, 0x24e0}, {0xa438, 0x8ffd}, {0xa438, 0xe18f}, {0xa438, 0xfee2}, 
    {0xa438, 0x8fd3}, {0xa438, 0xe38f}, {0xa438, 0xd41b}, {0xa438, 0x45ab}, 
    {0xa438, 0x14bf}, {0xa438, 0x8ffb}, {0xa438, 0x1f44}, {0xa438, 0x1f55}, 
    {0xa438, 0xe18f}, {0xa438, 0xd8e3}, {0xa438, 0x8fda}, {0xa438, 0x1a45}, 
    {0xa438, 0xdc19}, {0xa438, 0xddae}, {0xa438, 0x2ebf}, {0xa438, 0x8fe5}, 
    {0xa438, 0xe38f}, {0xa438, 0xd24b}, {0xa438, 0x021a}, {0xa438, 0x93e8}, 
    {0xa438, 0x8ffb}, {0xa438, 0x19e8}, {0xa438, 0x8ffc}, {0xa438, 0xd000}, 
    {0xa438, 0xe18f}, {0xa438, 0xe2e2}, {0xa438, 0x8ffb}, {0xa438, 0xe38f}, 
    {0xa438, 0xfc1a}, {0xa438, 0x54e0}, {0xa438, 0x8fdf}, {0xa438, 0xe18f}, 
    {0xa438, 0xe01b}, {0xa438, 0x45aa}, {0xa438, 0x06ee}, {0xa438, 0x8fff}, 
    {0xa438, 0x02ae}, {0xa438, 0x0302}, {0xa438, 0x8927}, {0xa438, 0xffef}, 
    {0xa438, 0x97ff}, {0xa438, 0xfefd}, {0xa438, 0xfc04}, {0xa438, 0xf8f9}, 
    {0xa438, 0xef59}, {0xa438, 0xf9fa}, {0xa438, 0xfb02}, {0xa438, 0x6767}, 
    {0xa438, 0xee8f}, {0xa438, 0xfb00}, {0xa438, 0xee8f}, {0xa438, 0xfc00}, 
    {0xa438, 0xee8f}, {0xa438, 0xfd00}, {0xa438, 0xee8f}, {0xa438, 0xfe00}, 
    {0xa438, 0xe08f}, {0xa438, 0xdfe1}, {0xa438, 0x8fe0}, {0xa438, 0xe28f}, 
    {0xa438, 0xdde3}, {0xa438, 0x8fde}, {0xa438, 0xef74}, {0xa438, 0xef65}, 
    {0xa438, 0x0d55}, {0xa438, 0x0288}, {0xa438, 0xfde7}, {0xa438, 0x8fd1}, 
    {0xa438, 0xef46}, {0xa438, 0x1b47}, {0xa438, 0x9e29}, {0xa438, 0xef46}, 
    {0xa438, 0x0288}, {0xa438, 0xd4ad}, {0xa438, 0x2704}, {0xa438, 0x7cff}, 
    {0xa438, 0xff14}, {0xa438, 0xe28f}, {0xa438, 0xfde3}, {0xa438, 0x8ffe}, 
    {0xa438, 0x1b54}, {0xa438, 0x9e02}, {0xa438, 0xab0e}, {0xa438, 0xe48f}, 
    {0xa438, 0xfde5}, {0xa438, 0x8ffe}, {0xa438, 0xef46}, {0xa438, 0xe48f}, 
    {0xa438, 0xfbe5}, {0xa438, 0x8ffc}, {0xa438, 0x16ae}, {0xa438, 0xd1bf}, 
    {0xa438, 0x63e2}, {0xa438, 0x0266}, {0xa438, 0xd4ff}, {0xa438, 0xfefd}, 
    {0xa438, 0xef95}, {0xa438, 0xfdfc}, {0xa438, 0x04f8}, {0xa438, 0xf9fa}, 
    {0xa438, 0xfbef}, {0xa438, 0x79fb}, {0xa438, 0x0267}, {0xa438, 0x67e2}, 
    {0xa438, 0x8ffb}, {0xa438, 0xe38f}, {0xa438, 0xfce0}, {0xa438, 0x8fe2}, 
    {0xa438, 0x1a50}, {0xa438, 0xe68f}, {0xa438, 0xf9e7}, {0xa438, 0x8ffa}, 
    {0xa438, 0x0d55}, {0xa438, 0x0288}, {0xa438, 0xfde7}, {0xa438, 0x8fd1}, 
    {0xa438, 0xe08f}, {0xa438, 0xf9e1}, {0xa438, 0x8ffa}, {0xa438, 0x0288}, 
    {0xa438, 0xd4ad}, {0xa438, 0x2704}, {0xa438, 0x7cff}, {0xa438, 0xff14}, 
    {0xa438, 0x1f55}, {0xa438, 0xe38f}, {0xa438, 0xe11c}, {0xa438, 0x540d}, 
    {0xa438, 0x54e0}, {0xa438, 0x8ffd}, {0xa438, 0xe18f}, {0xa438, 0xfe1b}, 
    {0xa438, 0x54ab}, {0xa438, 0x0fe0}, {0xa438, 0x8ff9}, {0xa438, 0xe18f}, 
    {0xa438, 0xfa14}, {0xa438, 0xe48f}, {0xa438, 0xf9e5}, {0xa438, 0x8ffa}, 
    {0xa438, 0xae22}, {0xa438, 0xbf8f}, {0xa438, 0xede3}, {0xa438, 0x8fd2}, 
    {0xa438, 0x4b02}, {0xa438, 0x1a93}, {0xa438, 0xe88f}, {0xa438, 0xf919}, 
    {0xa438, 0xe88f}, {0xa438, 0xfae0}, {0xa438, 0x8ff9}, {0xa438, 0xe18f}, 
    {0xa438, 0xfae3}, {0xa438, 0x8fe2}, {0xa438, 0x1a43}, {0xa438, 0xe48f}, 
    {0xa438, 0xf9e5}, {0xa438, 0x8ffa}, {0xa438, 0xef64}, {0xa438, 0xe08f}, 
    {0xa438, 0xdfe1}, {0xa438, 0x8fe0}, {0xa438, 0x1b46}, {0xa438, 0xaba0}, 
    {0xa438, 0xbf8f}, {0xa438, 0xede3}, {0xa438, 0x8fd2}, {0xa438, 0x4b02}, 
    {0xa438, 0x1a93}, {0xa438, 0xd819}, {0xa438, 0xd9a4}, {0xa438, 0x0000}, 
    {0xa438, 0x0302}, {0xa438, 0x8927}, {0xa438, 0xbf63}, {0xa438, 0xe202}, 
    {0xa438, 0x66d4}, {0xa438, 0xee8f}, {0xa438, 0xff03}, {0xa438, 0xffef}, 
    {0xa438, 0x97ff}, {0xa438, 0xfefd}, {0xa438, 0xfc04}, {0xa438, 0xf8f9}, 
    {0xa438, 0xfafb}, {0xa438, 0xef79}, {0xa438, 0xfbbf}, {0xa438, 0x8fed}, 
    {0xa438, 0xe38f}, {0xa438, 0xd24b}, {0xa438, 0x021a}, {0xa438, 0x93d8}, 
    {0xa438, 0x19d9}, {0xa438, 0xe38f}, {0xa438, 0xd81b}, {0xa438, 0x43ef}, 
    {0xa438, 0x740c}, {0xa438, 0x761f}, {0xa438, 0x44e1}, {0xa438, 0x8fd9}, 
    {0xa438, 0xef64}, {0xa438, 0xd300}, {0xa438, 0x0266}, {0xa438, 0xe6ef}, 
    {0xa438, 0x47bf}, {0xa438, 0x8ff5}, {0xa438, 0xe38f}, {0xa438, 0xd21a}, 
    {0xa438, 0x93dd}, {0xa438, 0xbf8f}, {0xa438, 0xede3}, {0xa438, 0x8fd2}, 
    {0xa438, 0x4b02}, {0xa438, 0x1a93}, {0xa438, 0xd819}, {0xa438, 0xd9e3}, 
    {0xa438, 0x8fd8}, {0xa438, 0x1b43}, {0xa438, 0xe38f}, {0xa438, 0xda1b}, 
    {0xa438, 0x439f}, {0xa438, 0x1bbf}, {0xa438, 0x628f}, {0xa438, 0xe08f}, 
    {0xa438, 0xd248}, {0xa438, 0x031a}, {0xa438, 0x9002}, {0xa438, 0x600a}, 
    {0xa438, 0xbf8f}, {0xa438, 0xf5e2}, {0xa438, 0x8fd2}, {0xa438, 0x1a92}, 
    {0xa438, 0xdb1b}, {0xa438, 0x31aa}, {0xa438, 0x01dd}, {0xa438, 0xee8f}, 
    {0xa438, 0xff00}, {0xa438, 0xe08f}, {0xa438, 0xd210}, {0xa438, 0xe48f}, 
    {0xa438, 0xd2a0}, {0xa438, 0x0407}, {0xa438, 0xee8f}, {0xa438, 0xd200}, 
    {0xa438, 0x0222}, {0xa438, 0x14ff}, {0xa438, 0xef97}, {0xa438, 0xfffe}, 
    {0xa438, 0xfdfc}, {0xa438, 0x04f8}, {0xa438, 0xfaac}, {0xa438, 0x4f08}, 
    {0xa438, 0xef46}, {0xa438, 0x1b67}, {0xa438, 0xaa02}, {0xa438, 0xef74}, 
    {0xa438, 0xfefc}, {0xa438, 0x04f8}, {0xa438, 0xfaef}, {0xa438, 0x461b}, 
    {0xa438, 0x67ab}, {0xa438, 0x02ef}, {0xa438, 0x74fe}, {0xa438, 0xfc04}, 
    {0xa438, 0xf9ef}, {0xa438, 0x95f9}, {0xa438, 0xef54}, {0xa438, 0x0d55}, 
    {0xa438, 0xe28f}, {0xa438, 0xd11b}, {0xa438, 0x239e}, {0xa438, 0x0602}, 
    {0xa438, 0x88fd}, {0xa438, 0xe78f}, {0xa438, 0xd15c}, {0xa438, 0x001f}, 
    {0xa438, 0xbf64}, {0xa438, 0xb402}, {0xa438, 0x5feb}, {0xa438, 0xbf64}, 
    {0xa438, 0xc002}, {0xa438, 0x600a}, {0xa438, 0xfdef}, {0xa438, 0x95fd}, 
    {0xa438, 0x04f8}, {0xa438, 0xf9fb}, {0xa438, 0xef79}, {0xa438, 0xfbe2}, 
    {0xa438, 0x8fd2}, {0xa438, 0xac3b}, {0xa438, 0x0a5b}, {0xa438, 0x070c}, 
    {0xa438, 0x321a}, {0xa438, 0x322b}, {0xa438, 0x40ae}, {0xa438, 0x0a5b}, 
    {0xa438, 0x030c}, {0xa438, 0x330c}, {0xa438, 0x211a}, {0xa438, 0x322b}, 
    {0xa438, 0x6002}, {0xa438, 0x4756}, {0xa438, 0xffef}, {0xa438, 0x97ff}, 
    {0xa438, 0xfdfc}, {0xa438, 0x04f8}, {0xa438, 0xef49}, {0xa438, 0xf8bf}, 
    {0xa438, 0x8fed}, {0xa438, 0xe08f}, {0xa438, 0xd248}, {0xa438, 0x021a}, 
    {0xa438, 0x90e8}, {0xa438, 0x8ffb}, {0xa438, 0x19e8}, {0xa438, 0x8ffc}, 
    {0xa438, 0xee8f}, {0xa438, 0xff03}, {0xa438, 0xfcef}, {0xa438, 0x94fc}, 
    {0xa438, 0x04ee}, {0xa438, 0x8fed}, {0xa438, 0x00ee}, {0xa438, 0x8fee}, 
    {0xa438, 0x00ee}, {0xa438, 0x8fef}, {0xa438, 0x00ee}, {0xa438, 0x8ff0}, 
    {0xa438, 0x00ee}, {0xa438, 0x8ff1}, {0xa438, 0x00ee}, {0xa438, 0x8ff2}, 
    {0xa438, 0x00ee}, {0xa438, 0x8ff3}, {0xa438, 0x00ee}, {0xa438, 0x8ff4}, 
    {0xa438, 0x00ee}, {0xa438, 0x8ff9}, {0xa438, 0x00ee}, {0xa438, 0x8ffa}, 
    {0xa438, 0x00ee}, {0xa438, 0x8ffb}, {0xa438, 0x00ee}, {0xa438, 0x8ffc}, 
    {0xa438, 0x00ee}, {0xa438, 0x8ffd}, {0xa438, 0x00ee}, {0xa438, 0x8ffe}, 
    {0xa438, 0x00ee}, {0xa438, 0x8fd2}, {0xa438, 0x00ee}, {0xa438, 0x8fff}, 
    {0xa438, 0x00ee}, {0xa438, 0x8fd1}, {0xa438, 0x0004}, {0xa436, 0xb85e}, 
    {0xa438, 0x20fe}, {0xa436, 0xb860}, {0xa438, 0x20CF}, {0xa436, 0xb862}, 
    {0xa438, 0x2122}, {0xa436, 0xb864}, {0xa438, 0x4fdc}, {0xa436, 0xb886}, 
    {0xa438, 0xffff}, {0xa436, 0xb888}, {0xa438, 0xffff}, {0xa436, 0xb88a}, 
    {0xa438, 0xffff}, {0xa436, 0xb88c}, {0xa438, 0xffff}, {0xa436, 0xb838}, 
    {0xa438, 0x000f}
};
void uc2_patch_6818C_231206(rtk_uint32 phymask)
{
    rtk_uint16 port, i, addr, val, len;

    len = sizeof(uc2_patch_6818C_231206_patch) / 4;

    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = uc2_patch_6818C_231206_patch[i][0];
                val = uc2_patch_6818C_231206_patch[i][1];

                dal_rtl8373_phy_write(1 << port, 31, addr, val);
            }
        }
    }
}


rtk_uint16 n2_patch_6818C_231206_patch[][2]=
{
    {0xa436, 0xA016}, {0xa438, 0x0020}, {0xa436, 0xA012}, {0xa438, 0x0000}, 
    {0xa436, 0xA014}, {0xa438, 0x1800}, {0xa438, 0x8010}, {0xa438, 0x1800}, 
    {0xa438, 0x801f}, {0xa438, 0x1800}, {0xa438, 0x8022}, {0xa438, 0x1800}, 
    {0xa438, 0x8025}, {0xa438, 0x1800}, {0xa438, 0x8030}, {0xa438, 0x1800}, 
    {0xa438, 0x8030}, {0xa438, 0x1800}, {0xa438, 0x8030}, {0xa438, 0x1800}, 
    {0xa438, 0x8030}, {0xa438, 0xa301}, {0xa438, 0x1000}, {0xa438, 0x0b55}, 
    {0xa438, 0xd700}, {0xa438, 0x5f9f}, {0xa438, 0xd131}, {0xa438, 0xd047}, 
    {0xa438, 0x1000}, {0xa438, 0x0b55}, {0xa438, 0xd700}, {0xa438, 0x5edf}, 
    {0xa438, 0xd700}, {0xa438, 0x5f74}, {0xa438, 0x1800}, {0xa438, 0x032a}, 
    {0xa438, 0x8280}, {0xa438, 0x1800}, {0xa438, 0x0504}, {0xa438, 0x9580}, 
    {0xa438, 0x1800}, {0xa438, 0x074c}, {0xa438, 0x1000}, {0xa438, 0x0b24}, 
    {0xa438, 0xd14a}, {0xa438, 0xd048}, {0xa438, 0xd700}, {0xa438, 0x4014}, 
    {0xa438, 0xd417}, {0xa438, 0x1000}, {0xa438, 0x0b24}, {0xa438, 0x1800}, 
    {0xa438, 0x0670}, {0xa436, 0xA10E}, {0xa438, 0xffff}, {0xa436, 0xA10C}, 
    {0xa438, 0xffff}, {0xa436, 0xA10A}, {0xa438, 0xffff}, {0xa436, 0xA108}, 
    {0xa438, 0xffff}, {0xa436, 0xA106}, {0xa438, 0x066e}, {0xa436, 0xA104}, 
    {0xa438, 0x074a}, {0xa436, 0xA102}, {0xa438, 0x0503}, {0xa436, 0xA100}, 
    {0xa438, 0x0325}, {0xa436, 0xA110}, {0xa438, 0x000f}, {0xa436, 0xA012}, 
    {0xa438, 0x0ff8}, {0xa436, 0xA014}, {0xa438, 0xd186}, {0xa438, 0xd04e}, 
    {0xa436, 0xA164}, {0xa438, 0x049d}, {0xa436, 0xA166}, {0xa438, 0x049e}, 
    {0xa436, 0xA162}, {0xa438, 0x0003}
};

void n2_patch_6818C_231206(rtk_uint32 phymask)
{

    rtk_uint16 port, i, addr, val, len;

    len = sizeof(n2_patch_6818C_231206_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = n2_patch_6818C_231206_patch[i][0];
                val = n2_patch_6818C_231206_patch[i][1];
                dal_rtl8373_phy_write(1 << port, 31, addr, val);
            }
        }
    }
}

rtk_uint16 uc_patch_6818C_221117_patch[][2]=
{
    {0xa436, 0x847d}, {0xa438, 0xaf84}, {0xa438, 0x95af}, {0xa438, 0x849a}, 
    {0xa438, 0xaf84}, {0xa438, 0xa0af}, {0xa438, 0x84ae}, {0xa438, 0xaf84}, 
    {0xa438, 0xdbaf}, {0xa438, 0x84fa}, {0xa438, 0xaf84}, {0xa438, 0xfaaf}, 
    {0xa438, 0x84fa}, {0xa438, 0x2901}, {0xa438, 0xaf16}, {0xa438, 0x5802}, 
    {0xa438, 0x84fa}, {0xa438, 0xaf16}, {0xa438, 0x2fee}, {0xa438, 0x8ffe}, 
    {0xa438, 0xffee}, {0xa438, 0x8fff}, {0xa438, 0xffbf}, {0xa438, 0x161c}, 
    {0xa438, 0xaf16}, {0xa438, 0x00e2}, {0xa438, 0x8700}, {0xa438, 0xad30}, 
    {0xa438, 0x21e2}, {0xa438, 0x8701}, {0xa438, 0xe387}, {0xa438, 0x021b}, 
    {0xa438, 0x549f}, {0xa438, 0x17ef}, {0xa438, 0x54e1}, {0xa438, 0x8018}, 
    {0xa438, 0x0d14}, {0xa438, 0xbf85}, {0xa438, 0x8402}, {0xa438, 0x55a4}, 
    {0xa438, 0x0d11}, {0xa438, 0xbf85}, {0xa438, 0x8702}, {0xa438, 0x55a4}, 
    {0xa438, 0xef45}, {0xa438, 0xe281}, {0xa438, 0xc2af}, {0xa438, 0x4056}, 
    {0xa438, 0xe187}, {0xa438, 0x00ad}, {0xa438, 0x2813}, {0xa438, 0xe180}, 
    {0xa438, 0x180d}, {0xa438, 0x16bf}, {0xa438, 0x8584}, {0xa438, 0x0255}, 
    {0xa438, 0xa40d}, {0xa438, 0x11bf}, {0xa438, 0x8587}, {0xa438, 0x0255}, 
    {0xa438, 0xa402}, {0xa438, 0x3b96}, {0xa438, 0xaf3a}, {0xa438, 0xfc02}, 
    {0xa438, 0x8501}, {0xa438, 0x0285}, {0xa438, 0x5904}, {0xa438, 0xf8f9}, 
    {0xa438, 0xfafb}, {0xa438, 0xef79}, {0xa438, 0xfbd7}, {0xa438, 0x0000}, 
    {0xa438, 0xd500}, {0xa438, 0x00a2}, {0xa438, 0x0005}, {0xa438, 0xbf57}, 
    {0xa438, 0x61ae}, {0xa438, 0x13a2}, {0xa438, 0x0105}, {0xa438, 0xbf57}, 
    {0xa438, 0x64ae}, {0xa438, 0x0ba2}, {0xa438, 0x0205}, {0xa438, 0xbf57}, 
    {0xa438, 0x67ae}, {0xa438, 0x03bf}, {0xa438, 0x576a}, {0xa438, 0x0255}, 
    {0xa438, 0xc3ef}, {0xa438, 0x6402}, {0xa438, 0x5b12}, {0xa438, 0x13e1}, 
    {0xa438, 0x8ffb}, {0xa438, 0x1b13}, {0xa438, 0x9fd5}, {0xa438, 0xe083}, 
    {0xa438, 0x7380}, {0xa438, 0x1b02}, {0xa438, 0x9f0a}, {0xa438, 0xef47}, 
    {0xa438, 0xe48f}, {0xa438, 0xfce5}, {0xa438, 0x8ffd}, {0xa438, 0xae06}, 
    {0xa438, 0x2a01}, {0xa438, 0xd300}, {0xa438, 0xaebd}, {0xa438, 0xffef}, 
    {0xa438, 0x97ff}, {0xa438, 0xfefd}, {0xa438, 0xfc04}, {0xa438, 0xf8f9}, 
    {0xa438, 0xfafb}, {0xa438, 0xe28f}, {0xa438, 0xfce3}, {0xa438, 0x8ffd}, 
    {0xa438, 0xef65}, {0xa438, 0xe08f}, {0xa438, 0xfee1}, {0xa438, 0x8fff}, 
    {0xa438, 0xef74}, {0xa438, 0x025a}, {0xa438, 0xf7ad}, {0xa438, 0x500c}, 
    {0xa438, 0xe68f}, {0xa438, 0xfee7}, {0xa438, 0x8fff}, {0xa438, 0xe283}, 
    {0xa438, 0x4ee6}, {0xa438, 0x834f}, {0xa438, 0xfffe}, {0xa438, 0xfdfc}, 
    {0xa438, 0x04cc}, {0xa438, 0xc010}, {0xa438, 0x00c0}, {0xa438, 0x2000}, 
    {0xa436, 0xb818}, {0xa438, 0x164d}, {0xa436, 0xb81a}, {0xa438, 0x162c}, 
    {0xa436, 0xb81c}, {0xa438, 0x15fd}, {0xa436, 0xb81e}, {0xa438, 0x4053}, 
    {0xa436, 0xb850}, {0xa438, 0x3af9}, {0xa436, 0xb852}, {0xa438, 0x0000}, 
    {0xa436, 0xb878}, {0xa438, 0x0000}, {0xa436, 0xb884}, {0xa438, 0x0000}, 
    {0xa436, 0xb832}, {0xa438, 0x001f} 
};

void uc_patch_6818C_221117(rtk_uint32 phymask)
{
    rtk_uint16 port, i, addr, val, len;

    len = sizeof(uc_patch_6818C_221117_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = uc_patch_6818C_221117_patch[i][0];
                val = uc_patch_6818C_221117_patch[i][1];

                dal_rtl8373_phy_write(1 << port, 31, addr, val);
            }
        }
    }
}

rtk_uint16 data_ram_patch_6818C_221026_patch[][2] =
    {
        {0xC206, 0xB1}};
void data_ram_patch_6818C_221026(rtk_uint32 phymask)
{
    rtk_uint16 port, i, data_ram_addr, data_ram_val, len;

    len = sizeof(data_ram_patch_6818C_221026_patch) / 4;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb896, 0x1, 0);    // #disable data_mem_auto_inc
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb892, 0xff00, 0); // #set uc2 data ram page
            for (i = 0; i < len; i++)
            {
                data_ram_addr = data_ram_patch_6818C_221026_patch[i][0];
                data_ram_val = data_ram_patch_6818C_221026_patch[i][1];
                data_ram_write_8b(port, data_ram_addr, data_ram_val);
            }
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xb896, 0x1, 1); // # enable data_mem_auto_inc
        }
    }
}

void afe_patch_6818C_220607(rtk_uint16 phymask)
{
    rtk_uint16 port;
    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xbf84, 0x7, 4);
            dal_rtl8373_phy_regbits_write(1 << port, 31, 0xbf8c, 0x1f << 6, 0);
        }
    }
}

rtk_uint16 RTCT_para_6818C_231206_patch[][4]=
{
    {0xa436, 15, 0, 0x81a3}, {0xa438, 15, 8, 0x32}, {0xa436, 15, 0, 0x81a4}, {0xa438, 15, 8, 0xc0}, 
    {0xa436, 15, 0, 0x81a5}, {0xa438, 15, 8, 0x32}, {0xa436, 15, 0, 0x81a8}, {0xa438, 15, 8, 0x1d}, 
    {0xa436, 15, 0, 0x81af}, {0xa438, 15, 8, 0x25}, {0xa436, 15, 0, 0x81b2}, {0xa438, 15, 8, 0x09}, 
    {0xa436, 15, 0, 0x81b6}, {0xa438, 15, 8, 0x3f}, {0xa436, 15, 0, 0x81b7}, {0xa438, 15, 8, 0x48}, 
    {0xa436, 15, 0, 0x81b8}, {0xa438, 15, 8, 0x0c}, {0xa436, 15, 0, 0x81b9}, {0xa438, 15, 8, 0x04}, 
    {0xa436, 15, 0, 0x81ba}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81bb}, {0xa438, 15, 8, 0x20}, 
    {0xa436, 15, 0, 0x81bc}, {0xa438, 15, 8, 0x04}, {0xa436, 15, 0, 0x81bd}, {0xa438, 15, 8, 0x20}, 
    {0xa436, 15, 0, 0x81be}, {0xa438, 15, 8, 0x1a}, {0xa436, 15, 0, 0x81bf}, {0xa438, 15, 8, 0xe0}, 
    {0xa436, 15, 0, 0x81c0}, {0xa438, 15, 8, 0x01}, {0xa436, 15, 0, 0x81c1}, {0xa438, 15, 8, 0x3a}, 
    {0xa436, 15, 0, 0x81c2}, {0xa438, 15, 8, 0x1c}, {0xa436, 15, 0, 0x81c3}, {0xa438, 15, 8, 0x60}, 
    {0xa436, 15, 0, 0x81c4}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81c5}, {0xa438, 15, 8, 0x11}, 
    {0xa436, 15, 0, 0x81c6}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81c7}, {0xa438, 15, 8, 0xcf}, 
    {0xa436, 15, 0, 0x81c8}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81c9}, {0xa438, 15, 8, 0xb0}, 
    {0xa436, 15, 0, 0x81ca}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81cb}, {0xa438, 15, 8, 0x0f}, 
    {0xa436, 15, 0, 0x81cc}, {0xa438, 15, 8, 0x11}, {0xa436, 15, 0, 0x81cd}, {0xa438, 15, 8, 0xc0}, 
    {0xa436, 15, 0, 0x81ce}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81cf}, {0xa438, 15, 8, 0x0e}, 
    {0xa436, 15, 0, 0x81d0}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81d1}, {0xa438, 15, 8, 0xbe}, 
    {0xa436, 15, 0, 0x81d2}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81d3}, {0xa438, 15, 8, 0x18}, 
    {0xa436, 15, 0, 0x81d4}, {0xa438, 15, 8, 0x08}, {0xa436, 15, 0, 0x81d5}, {0xa438, 15, 8, 0x70}, 
    {0xa436, 15, 0, 0x81d6}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81d7}, {0xa438, 15, 8, 0x37}, 
    {0xa436, 15, 0, 0x81d8}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81d9}, {0xa438, 15, 8, 0x48}, 
    {0xa436, 15, 0, 0x81da}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81db}, {0xa438, 15, 8, 0xf4}, 
    {0xa436, 15, 0, 0x81dc}, {0xa438, 15, 8, 0xeb}, {0xa436, 15, 0, 0x81dd}, {0xa438, 15, 8, 0xa0}, 
    {0xa436, 15, 0, 0x81de}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81df}, {0xa438, 15, 8, 0x2b}, 
    {0xa436, 15, 0, 0x81e0}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81e1}, {0xa438, 15, 8, 0x9c}, 
    {0xa436, 15, 0, 0x81e2}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81e3}, {0xa438, 15, 8, 0xb2}, 
    {0xa436, 15, 0, 0x81e4}, {0xa438, 15, 8, 0xeb}, {0xa436, 15, 0, 0x81e5}, {0xa438, 15, 8, 0xaf}, 
    {0xa436, 15, 0, 0x81e6}, {0xa438, 15, 8, 0x02}, {0xa436, 15, 0, 0x81e7}, {0xa438, 15, 8, 0x92}, 
    {0xa436, 15, 0, 0x81e8}, {0xa438, 15, 8, 0xfe}, {0xa436, 15, 0, 0x81e9}, {0xa438, 15, 8, 0xe4}, 
    {0xa436, 15, 0, 0x81ea}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81eb}, {0xa438, 15, 8, 0x3c}, 
    {0xa436, 15, 0, 0x81ec}, {0xa438, 15, 8, 0x06}, {0xa436, 15, 0, 0x81ed}, {0xa438, 15, 8, 0xf2}, 
    {0xa436, 15, 0, 0x81ee}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81ef}, {0xa438, 15, 8, 0x3a}, 
    {0xa436, 15, 0, 0x81f0}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x81f1}, {0xa438, 15, 8, 0x8a}, 
    {0xa436, 15, 0, 0x81f2}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81f3}, {0xa438, 15, 8, 0xd0}, 
    {0xa436, 15, 0, 0x81f4}, {0xa438, 15, 8, 0x0a}, {0xa436, 15, 0, 0x81f5}, {0xa438, 15, 8, 0x09}, 
    {0xa436, 15, 0, 0x81f6}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81f7}, {0xa438, 15, 8, 0xdc}, 
    {0xa436, 15, 0, 0x81f8}, {0xa438, 15, 8, 0xff}, {0xa436, 15, 0, 0x81f9}, {0xa438, 15, 8, 0x30}, 
    {0xa436, 15, 0, 0x81fa}, {0xa438, 15, 8, 0x00}, {0xa436, 15, 0, 0x8700}, {0xa438, 15, 8, 0x01}, 
    {0xa436, 15, 0, 0x8701}, {0xa438, 15, 8, 0x04}, {0xa436, 15, 0, 0x8018}, {0xa438, 15, 8, 0x70}, 
    {0xa436, 15, 0, 0x81a6}, {0xa438, 15, 8, 0xc0}, {0xa436, 15, 0, 0x81a9}, {0xa438, 15, 8, 0x00}, 
    {0xa436, 15, 0, 0x81b0}, {0xa438, 15, 8, 0x05}, {0xa436, 15, 0, 0x81b3}, {0xa438, 15, 8, 0x1d}, 
    {0xa436, 15, 0, 0x81fb}, {0xa438, 15, 8, 0x6c}, {0xa436, 15, 0, 0x8702}, {0xa438, 15, 8, 0x50}
};

void RTCT_para_6818C_231206(rtk_uint32 phymask)
{
    rtk_uint16 port, i, addr, val, len, msb, lsb;
    rtk_uint32 maskbits, j, masklen;

    len = sizeof(RTCT_para_6818C_231206_patch) / 8;

    for (port = 0; port < 8; port++)
    {
        if ((1 << port) & phymask)
        {
            for (i = 0; i < len; i++)
            {
                addr = RTCT_para_6818C_231206_patch[i][0];
                msb = RTCT_para_6818C_231206_patch[i][1];
                lsb = RTCT_para_6818C_231206_patch[i][2];
                val = RTCT_para_6818C_231206_patch[i][3];

                masklen = msb - lsb;
                maskbits = 1;
                for (j = 0; j < masklen; j++)
                {
                    maskbits = (maskbits << 1) | 1;
                }
                maskbits = maskbits << lsb;
                dal_rtl8373_phy_regbits_write(1 << port, 31, addr, maskbits, val);
            }
        }
    }
}

void cfg_rl6637_sds_mode(rtk_uint32 phyId, rtk_uint32 sds_mode)
{

    dal_rtl8373_phy_regbits_write(1 << phyId, 30, 0x75f3, 1, 0);
    dal_rtl8373_phy_regbits_write(1 << phyId, 30, 0x697a, 0x3f, sds_mode);
#if defined EEE_ENABLE
    dal_rtl8373_phy_regbits_write(1 << phyId, 31, 0xa432, 1 << 5, 1);
    dal_rtl8373_phy_regbits_write(1 << phyId, 7, 0x3e, 1, 1);
#endif
    dal_rtl8373_phy_regbits_write(1 << phyId, 31, 0xa442, 3 << 2, 0);
    dal_rtl8373_phy_write(1 << phyId, 30, 0x75b5, 0xe084);

    dal_rtl8373_phy_regbits_write(1 << phyId, 30, 0x75b2, 3 << 5, 3);

    dal_rtl8373_phy_regbits_write(1 << phyId, 31, 0xd040, 3 << 8, 2);

    dal_rtl8373_phy_regbits_write(1 << phyId, 31, 0xa400, 1 << 14, 1);
    delay_loop(4000);
    dal_rtl8373_phy_regbits_write(1 << phyId, 31, 0xa400, 1 << 14, 0);
}

rtk_api_ret_t fw_reset_flow_tgx(rtk_uint32 sds)
{
    rtk_uint32 REG_1_29, SIG_OK, SYNC_OK, LINK_OK;
    ret_t retVal;
    rtk_uint32 rx_sts;

    retVal = dal_rtl8373_sds_regbits_read(sds, 0x20, 0, (0x3 << 4), &rx_sts);
    if (retVal != RT_ERR_OK)
        return retVal;
    if (rx_sts == 0x1) /// 1:rx off, do nothing; 0,2,3:rx on, continue;
    {
        return RT_ERR_OK;
    }

    dal_rtl8373_sds_reg_read(sds, 1, 0x1d, &REG_1_29);
    SIG_OK = ((REG_1_29 >> 8) & 1);
    SYNC_OK = (REG_1_29 & 1);
    LINK_OK = ((REG_1_29 >> 4) & 1);
    if (SIG_OK == 1)
    {
        if (SYNC_OK == 0)
        {

            dal_rtl8373_sds_regbits_write(sds, 0, 0, 1 << 1, 1);
            delay_loop(100);
            dal_rtl8373_sds_regbits_write(sds, 0, 0, 1 << 1, 0);
            delay_loop(100);
            dal_rtl8373_sds_regbits_write(sds, 0, 0, 1 << 1, 1);
        }
        else
        {
            if (LINK_OK == 0)
            {
                dal_rtl8373_sds_regbits_write(sds, 0, 0, 1 << 1, 1);
                delay_loop(100);
                dal_rtl8373_sds_regbits_write(sds, 0, 0, 1 << 1, 0);
                delay_loop(100);
                dal_rtl8373_sds_regbits_write(sds, 0, 0, 1 << 1, 1);
            }
        }
    }

    return RT_ERR_OK;
}

rtk_api_ret_t fw_reset_flow_tgr_tgx(rtk_uint32 sds)
{
    rtk_uint32 retVal;
    rtk_sds_mode_t mode;

    retVal = dal_rtl8373_sdsMode_get(sds, &mode);
    if (retVal != RT_ERR_OK)
        return retVal;

    if ((mode == SERDES_10GQXG) || (mode == SERDES_10GUSXG) || (mode == SERDES_10GR))
    {
        fw_reset_flow_tgr(sds);
    }
    else if ((mode == SERDES_HSG) || (mode == SERDES_2500BASEX) || (mode == SERDES_SG) || (mode == SERDES_1000BASEX) || (mode == SERDES_100FX))
    {
        fw_reset_flow_tgx(sds);
    }

    return RT_ERR_OK;
}

/* Function Name:
 * 	 dal_rtl8373_fw_reset_flow_8221B
 * Description:
 * 	 8221B reset flow
 * Input:
 * 	 sdsid: 8221B-sdsid
 * Output:
 * 	 None
 * Return:
 * 	 RT_ERR_OK			 - OK
 * 	 RT_ERR_FAILED		 - Failed
 * 	 RT_ERR_SMI 		 - SMI access error
 * Note:
 * 	 The API can reset 8221B, Dynamic change sds to match 8221B speed.
 */
rtk_api_ret_t dal_rtl8373_fw_reset_flow_8221B(rtk_uint32 sdsid)
{
    rtk_uint32 serdes0mode, serdes1mode;
    rtk_uint32 tmp;
    rtk_uint32 spd_8221_now = 0;

    if (sdsid == SERDES_ID0)
    {
        // sds0 8221b ,避免linkdown(8221b sds未开) 时，反复的reset sds
        rtl8373_getAsicRegBits(0x63f0, 0xf000, &tmp); // 当 mac_spd!=0xc 时,8221B sds 已经打开
        rtl8373_getAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, 0x1f, &serdes0mode);

        if (tmp != 0xc)
        {
            if ((serdes0mode == 0xd) || (serdes0mode == 0x1a))
            {
                fw_reset_flow_tgr(0);
            }
            else if (serdes0mode == 0x1f)
            {
            }
            else
            {
                fw_reset_flow_tgx(0);
            }
        }

        rtl8373_getAsicRegBits(0x6418, 0xf000, &spd_8221_now); // sds0-port3 phy spd , 比mac spd早

        if ((serdes0mode != SERDES_HSG) && (spd_8221_now == 5)) // ; 10m/100m/1G/down -> 2.5G ,
        {
            SDS_MODE_SET_SW(CHIP_RTL8372_MODE, 0, SERDES_HSG);
        }
        else if ((serdes0mode != SERDES_SG) && (spd_8221_now != 5)) /// 2.5G -> 10m/100m/1G/down
        {
            SDS_MODE_SET_SW(CHIP_RTL8372_MODE, 0, SERDES_SG);
        }
        spd_8221_last = spd_8221_now;
    }

    if (sdsid == SERDES_ID1)
    {
        // sds1-8221b ,避免linkdown(8221b sds未开) 时，反复的reset sds
        rtl8373_getAsicRegBits(0x63f4, 0xf, &tmp); // 当 mac_spd!=0xc 时,8221B sds 已经打开
        rtl8373_getAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, 0x3e0, &serdes1mode);
        if (tmp != 0xc)
        {
            if ((serdes1mode == 0xd) || (serdes1mode == 0x1a))
            {
                fw_reset_flow_tgr(1);
            }
            else if (serdes1mode == 0x1f)
            {
            }
            else
            {
                fw_reset_flow_tgx(1);
            }
        }

        rtl8373_getAsicRegBits(0x641c, 0xf, &spd_8221_now); // sds0-port3 phy spd , 比mac spd早
        if (spd_8221_now != spd_8221_last && spd_8221_now <= 9)
        {
            if ((serdes1mode != SERDES_HSG) && (spd_8221_now == 5)) // ; 10m/100m/1G/down -> 2.5G ,
            {
                SDS_MODE_SET_SW(CHIP_RTL8372_MODE, 1, SERDES_HSG);
            }
            else if ((serdes1mode != SERDES_SG) && (spd_8221_now != 5)) /// 2.5G -> 10m/100m/1G/down
            {
                SDS_MODE_SET_SW(CHIP_RTL8372_MODE, 1, SERDES_SG);
            }
        }
        spd_8221_last = spd_8221_now;
    }

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8373_8224_init(void)
{
    rtk_uint32 i;

    //******************** End to Reset RTL8224 by RTL8373_GPIO36 ***************************
    Pin_Reset_8224_via_8373();
    //******************** Begin to Initial RTL8373 MDC/MDIO ********************************
    //  puts "MDC/MDIO pad initial"

    rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT8_TYPE_MASK, 0); // MAC_PORT8_TYPE=0 port type: RTL8373 default set port0-7 polling 10G/2.5GPHY
    rtl8373_setAsicRegBits(RTL8373_SMI_PORT_POLLING_SEL_ADDR, 0xff, 0xff);                                    // RTL8373 set port0-8 polling internal resolution reg
    rtl8373_setAsicRegBits(RTL8373_SMI_CTRL_ADDR, 0x7000, 7);                                                 // enable SMI0/1/2 MDC clock output, default is 0
                                                              //******************** End to Initial RTL8373 MDC/MDIO ********************************

    //>50ms
    delay_loop(5000000);

    get_version_8373();
    get_version_8224();
    rtlglue_printf("8373_8224 init, 8373 ver is %d, 8224 ver is %d\n", Ver8373_72, Ver8224);

    delay_loop(100);

    rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_USX_SUB_MODE_MASK, 2); // SDS0_USX_SUB_MODE = 0x2, default is 0x2 10G-QXGMII
    rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK, 0xD);   // SDS0_MODE_SEL = 0xD
    dal_rtl8373_sds_regbits_write(0, 6, 1, 0x4, 1);
    delay_loop(20);
    dal_rtl8373_sds_regbits_write(0, 6, 1, 0x4, 0);

    rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK, 0x1F); // SDS0_MODE_SEL = 0x1F
    delay_loop(100);

    rtl8373_setAsicRegBit(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_CFG_MAC3_8221B_OFFSET, 0);
    rtl8373_setAsicRegBit(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_CFG_MAC8_8221B_OFFSET, 0);

    SDS_MODE_SET_SW(CHIP_RTL8373_MODE, 0, SERDES_10GQXG);
    SDS_MODE_SET_SW(CHIP_RTL8224_MODE, 0, SERDES_10GQXG);

    delay_loop(50);
    fw_reset_flow_tgr_8224(0);

    delay_loop(50);

    fw_reset_flow_tgr(0);

    rtl8373_setAsicRegBits(RTL8373_CFG_PHY_MDI_REVERSE_ADDR, 0xf, 0xc);
    dal_rtl8224_top_regbits_write(RTL8373_CFG_PHY_MDI_REVERSE_ADDR, 0xf, 0xc);
    dal_rtl8373_phy_write(0xff, 31, 0xa610, 0x2858);

    // ## ---------------------------Patch MAC--------------------------
    // #set MAC_L2_GLOBAL_CTRL0 0x5FD4
    rtl8373_setAsicRegBit(RTL8373_MAC_L2_GLOBAL_CTRL0_ADDR, RTL8373_MAC_L2_GLOBAL_CTRL0_FWD_INVLD_MAC_CTRL_EN_OFFSET, 1);
    rtl8373_setAsicRegBit(RTL8373_MAC_L2_GLOBAL_CTRL0_ADDR, RTL8373_MAC_L2_GLOBAL_CTRL0_FWD_UNKN_OPCODE_EN_OFFSET, 1);

    //  #BYP_TX_CRC=1
    for (i = 0; i < 9; i++)
    {
        rtl8373_setAsicRegBit(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), RTL8373_MAC_L2_PORT_CTRL_RX_CHK_CRC_EN_OFFSET, 1);
        rtl8373_setAsicRegBit(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), RTL8373_MAC_L2_PORT_CTRL_CLOCK_SWITCH_OFFSET, 1);
    }

    rtl8373_setAsicRegBit(RTL8373_RS_LAYER_CONFIG_ADDR, RTL8373_RS_LAYER_CONFIG_RS_LINK_FAULT_INDI_OFF_OFFSET, 1);

    for (i = 0; i < 10; i++)
    {
        rtl8373_setAsicReg(RTL8373_FC_PORT_ACT_CTRL_ADDR(i), 0x1050);
    }

    // ## ---------------------------Init END--------------------------

    rtl8373_setAsicRegBit(RTL8373_DW8051_CFG_ADDR, RTL8373_DW8051_CFG_DW8051_READY_OFFSET, ENABLE);

    delay_loop(50);

#if PHY_PATCH
    if (Ver8224 == 1) // chipB
    {
        RL6818B_pwr_on_patch_phy_v009(0xf);
        RL6818B_pwr_on_patch_phy_v009_rls_lockmain(0xf);
    }
    else if (Ver8224 == 2) // chipC
    {
        RL6818C_pwr_on_patch_phy_v008(0xf);
        RL6818C_pwr_on_patch_phy_v008_rls_lockmain(0xf);
    }

    if (Ver8373_72 == 1) // chipB
    {
        RL6818B_pwr_on_patch_phy_v009(0xf0);
        RL6818B_pwr_on_patch_phy_v009_rls_lockmain(0xf0);
    }
    else if (Ver8373_72 == 2) // chipC
    {
        RL6818C_pwr_on_patch_phy_v008(0xf0);
        RL6818C_pwr_on_patch_phy_v008_rls_lockmain(0xf0);
    }
#endif

    dal_rtl8373_phy_write(0xff, 31, 0xa610, 0x2058);
    rtl8373_setAsicRegBits(RTL8373_SMI_GLB_CTRL_ADDR, RTL8373_SMI_GLB_CTRL_SMI_POLLING_MASK_MASK, 0x1ff);
    rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT8_TYPE_MASK, 0);

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8372_init(void)
{
    rtk_uint32 i;
    //******************** Begin to Initial RTL8373 MDC/MDIO ********************************
    //  puts "MDC/MDIO pad initial"

    rtl8373_setAsicRegBits(RTL8373_SMI_PORT_POLLING_SEL_ADDR, 0xf0, 0xf); // RTL8372 set port4-7 polling internal resolution reg
    rtl8373_setAsicRegBits(RTL8373_SMI_CTRL_ADDR, 0x7000, 7);             // enable SMI0/1/2 MDC clock output, default is 0
    //******************** End to Initial RTL8373 MDC/MDIO ********************************

    get_version_8373();
    //   rtlglue_printf("8372 init, 8372 ver is %d\n", Ver8373_72);

    delay_loop(100);

    rtl8373_setAsicRegBit(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_CFG_MAC3_8221B_OFFSET, 0);
    rtl8373_setAsicRegBit(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_CFG_MAC3_8221B_OFFSET, 0);
    //   ## tgr reset flow
    delay_loop(50);

    //******************** Begin to Initial RTL8372 PHY configuration ***************************

    // #MDI reverse configuration for Demo Tap UP RJ45, RTL8372
    rtl8373_setAsicRegBits(RTL8373_CFG_PHY_MDI_REVERSE_ADDR, 0xf, 0xc);

    //  puts "Power down PHY 4~7"
    dal_rtl8373_phy_write(0xf0, 31, 0xa610, 0x2858);

    //  ## ---------------------------Patch MAC--------------------------

    rtl8373_setAsicRegBit(RTL8373_MAC_L2_GLOBAL_CTRL0_ADDR, RTL8373_MAC_L2_GLOBAL_CTRL0_FWD_INVLD_MAC_CTRL_EN_OFFSET, 1);
    rtl8373_setAsicRegBit(RTL8373_MAC_L2_GLOBAL_CTRL0_ADDR, RTL8373_MAC_L2_GLOBAL_CTRL0_FWD_UNKN_OPCODE_EN_OFFSET, 1);

    //  #BYP_TX_CRC=1
    for (i = 3; i < 9; i++)
    {
        rtl8373_setAsicRegBit(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), RTL8373_MAC_L2_PORT_CTRL_RX_CHK_CRC_EN_OFFSET, 1);
        rtl8373_setAsicRegBit(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), RTL8373_MAC_L2_PORT_CTRL_CLOCK_SWITCH_OFFSET, 1);
    }

    rtl8373_setAsicRegBit(RTL8373_RS_LAYER_CONFIG_ADDR, RTL8373_RS_LAYER_CONFIG_RS_LINK_FAULT_INDI_OFF_OFFSET, 1);

    for (i = 0; i < 10; i++)
    {
        rtl8373_setAsicReg(RTL8373_FC_PORT_ACT_CTRL_ADDR(i), 0x1050);
    }

    // ## ---------------------------Init END--------------------------
    rtl8373_setAsicRegBit(RTL8373_DW8051_CFG_ADDR, RTL8373_DW8051_CFG_DW8051_READY_OFFSET, ENABLE);

    delay_loop(200);

#if PHY_PATCH
    if (Ver8373_72 == 1)
    {
        RL6818B_pwr_on_patch_phy_v009(0xf0);
        RL6818B_pwr_on_patch_phy_v009_rls_lockmain(0xf0);
    }
    else if (Ver8373_72 == 2)
    {
        RL6818C_pwr_on_patch_phy_v008(0xf0);
        RL6818C_pwr_on_patch_phy_v008_rls_lockmain(0xf0);
    }
#endif

    dal_rtl8373_phy_write(0xf0, 31, 0xa610, 0x2058);
    rtl8373_setAsicRegBits(RTL8373_SMI_GLB_CTRL_ADDR, RTL8373_SMI_GLB_CTRL_SMI_POLLING_MASK_MASK, 0x1f8);
    rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT3_TYPE_MASK, 0);
    rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT8_TYPE_MASK, 0);

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8366u_init(void)
{
    rtk_uint32 i = 0;
    rtk_uint32 chip_mode = 0;

    rtl8373_getAsicRegBits(RTL8373_BOND_INFO_ADDR, 3, &chip_mode);

    if (chip_mode != 2)
    {
        return RT_ERR_CHIP_NOT_SUPPORTED; // chip mode error
    }

    get_version_8366u();
    rtlglue_printf("rtl8366u_init , 8366U ver is %d\n", Ver8366U);
    //******************** Begin to Initial RTL8373 MDC/MDIO ********************************
    //  puts "MDC/MDIO pad initial"
    rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT8_TYPE_MASK, 0); // #MAC_PORT8_TYPE=0 port type: sds_ablty
    rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT3_TYPE_MASK, 0); // #MAC_PORT3_TYPE=0 port type: sds_ablty
    rtl8373_setAsicRegBits(RTL8373_SMI_PORT_POLLING_SEL_ADDR, 0xf0, 0xf);                                     // #RTL8372/RTL8372N/RTL8366U set port4-7 polling internal resolution reg
    rtl8373_setAsicRegBits(RTL8373_SMI_CTRL_ADDR, 0x7000, 7);                                                 // enable SMI0/1/2 MDC clock output, default is 0
    delay_loop(1);

    //******************** End to Initial RTL8373 MDC/MDIO ********************************

    // ## ---------------------------Init LED--------------------------
    // led_8366U_cfg();

    // ## ---------------------------Init SDS--------------------------

    dal_rtl8373_sds_regbits_write(0, 0, 0, 0x200, 1); // #SDS0RX PN swap
    dal_rtl8373_sds_regbits_write(1, 0, 0, 0x200, 1); // #SDS1RX PN swap

    dal_rtl8373_sds_regbits_write(0, 6, 2, 0x2000, 1);
    dal_rtl8373_sds_regbits_write(1, 6, 2, 0x2000, 1);

    //   ## tgr reset flow
    delay_loop(5);
    fw_reset_flow_tgr(1); // 8366U SDS1 execute reset flow
    delay_loop(5);
    fw_reset_flow_tgr(0);

    // ## ---------------------------Patch PHY--------------------------

    // ##MDI reverse configuration for Demo Tap UP RJ45, RTL8366U/RTL8373N/RTL8372N
    rtl8373_setAsicRegBits(RTL8373_CFG_PHY_MDI_REVERSE_ADDR, 0xF, 0xC);
    rtl8373_setAsicRegBits(RTL8373_CFG_PHY_TX_POLARITY_SWAP_ADDR, 0xFFFF, 0x596A); // #TX_POLARITY_SWAP

    //  puts "Power down PHY 4~7"
    dal_rtl8373_phy_write(0xF0, 31, 0xa610, 0x2858);

    // ## ---------------------------Patch MAC--------------------------
    rtl8373_setAsicRegBits(RTL8373_MAC_L2_GLOBAL_CTRL0_ADDR, 0x180000, 3); // #cfg_FWD_INVLD_MAC_CTRL_EN,cfg_FWD_UNKN_OPCODE_EN

    //  set MAC_L2_PORT_CTRL_ADDR 0x1238; 0x1238+$port*0x100
    for (i = 3; i < 9; i++)
    {
        rtl8373_setAsicRegBits(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), 0x10, 1);  // #RX_CHK_CRC_EN=1
        rtl8373_setAsicRegBits(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), 0x100, 1); // #CLOCK_SWITCH=1; Îª½â¾öÄ³Ð©linkdownÊ±ÒòclockÍ£µô¶øÎÞ·¨drain outµÄÎÊÌâ
    }

    // #RS_LINK_FAULT_INDI_OFF=1 disable link fault flag, resolve port4-port7 linkdown dsc expand issue

    rtl8373_setAsicRegBits(RTL8373_RS_LAYER_CONFIG_ADDR, 0x20, 1);

    //
    for (i = 0; i < 10; i++)
    {
        rtl8373_setAsicReg(RTL8373_FC_PORT_ACT_CTRL_ADDR(i), 0x1050);
    }
    // ## ---------------------------Init END--------------------------
    rtl8373_setAsicRegBits(RTL8373_DW8051_CFG_ADDR, RTL8373_DW8051_CFG_DW8051_READY_MASK, ENABLE);
    delay_loop(100);

#if PHY_PATCH
    if (Ver8366U == 2)
    {
        RL6818C_pwr_on_patch_phy_v008(0xf0);
        RL6818C_pwr_on_patch_phy_v008_rls_lockmain(0xf0);
    }

#endif

    //  puts "Power up PHY 4~7"
    dal_rtl8373_phy_write(0xF0, 31, 0xa610, 0x2058);
    // RTL8372/RTL8372N/RTL8366U set polling mask 0x1f8, port 3/8 from serdes need config bit8=1
    rtl8373_setAsicRegBits(RTL8373_SMI_GLB_CTRL_ADDR, 0x1FF000, 0x1f8);

    delay_loop(50);

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8372n_init(void)
{
    rtk_uint32 i = 0;
    rtk_uint32 chip_mode = 0;

    rtl8373_getAsicRegBits(RTL8373_BOND_INFO_ADDR, 3, &chip_mode);

    if (chip_mode != 2)
    {
        return RT_ERR_CHIP_NOT_SUPPORTED; // chip mode error
    }

    get_version_8373();

    //******************** Begin to Initial RTL8373 MDC/MDIO ********************************
    //  puts "MDC/MDIO pad initial"
    rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT8_TYPE_MASK, 0); // #MAC_PORT8_TYPE=0 port type: sds_ablty
    rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT3_TYPE_MASK, 0); // #MAC_PORT3_TYPE=0 port type: sds_ablty
    rtl8373_setAsicRegBits(RTL8373_SMI_PORT_POLLING_SEL_ADDR, 0xf0, 0xf);                                     // #RTL8372/RTL8372N/RTL8366U set port4-7 polling internal resolution reg
    rtl8373_setAsicRegBits(RTL8373_SMI_CTRL_ADDR, 0x7000, 7);                                                 // enable SMI0/1/2 MDC clock output, default is 0
    delay_loop(1);
    //******************** End to Initial RTL8373 MDC/MDIO ********************************

    // ## ---------------------------Init LED--------------------------
    // led_8372n_cfg();

    // ## ---------------------------Init SDS--------------------------
    // Sx PN swap:
    // 	RX:
    // 		page0 reg0 bit9:1
    // 		page6 reg2 bit13:1
    // 	TX:
    // 		page0 reg0 bit8:1
    // 		page6 reg2 bit14:1

    // dal_rtl8373_sds_regbits_write(0, 0, 0, 0x200, 1); //#SDS0RX PN swap
    // dal_rtl8373_sds_regbits_write(0, 6, 2, 0x2000, 1);

    // dal_rtl8373_sds_regbits_write(0, 0, 0, 1 << 8, 1); //#SDS0RTX PN swap
    // dal_rtl8373_sds_regbits_write(0, 6, 2, 1 << 14, 1);

    // dal_rtl8373_sds_regbits_write(1, 0, 0, 0x200, 1); //#SDS1RX PN swap
    // dal_rtl8373_sds_regbits_write(1, 6, 2, 0x2000, 1);

    // dal_rtl8373_sds_regbits_write(1, 0, 0, 1 << 8, 1); //#SDS1TX PN swap
    // dal_rtl8373_sds_regbits_write(1, 6, 2, 1 << 14, 1);

    //   ## tgr reset flow
    delay_loop(5);
    fw_reset_flow_tgr(1); // 8366U SDS1 execute reset flow
    delay_loop(5);
    fw_reset_flow_tgr(0);

    // ## ---------------------------Patch PHY--------------------------

    // ##MDI reverse configuration for Demo Tap UP RJ45, RTL8366U/RTL8373N/RTL8372N
    // rtl8373_setAsicRegBits(RTL8373_CFG_PHY_MDI_REVERSE_ADDR, 0xF, 0xC);
    // rtl8373_setAsicRegBits(RTL8373_CFG_PHY_TX_POLARITY_SWAP_ADDR, 0xFFFF, 0x596A); //#TX_POLARITY_SWAP

    //  puts "Power down PHY 4~7"
    dal_rtl8373_phy_write(0xF0, 31, 0xa610, 0x2858);

    // ## ---------------------------Patch MAC--------------------------
    rtl8373_setAsicRegBits(RTL8373_MAC_L2_GLOBAL_CTRL0_ADDR, 0x180000, 3); // #cfg_FWD_INVLD_MAC_CTRL_EN,cfg_FWD_UNKN_OPCODE_EN

    //  set MAC_L2_PORT_CTRL_ADDR 0x1238; 0x1238+$port*0x100
    for (i = 3; i < 9; i++)
    {
        rtl8373_setAsicRegBits(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), 0x10, 1);  // #RX_CHK_CRC_EN=1
        rtl8373_setAsicRegBits(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), 0x100, 1); // #CLOCK_SWITCH=1; Îª½â¾öÄ³Ð©linkdownÊ±ÒòclockÍ£µô¶øÎÞ·¨drain outµÄÎÊÌâ
    }

    // #RS_LINK_FAULT_INDI_OFF=1 disable link fault flag, resolve port4-port7 linkdown dsc expand issue

    rtl8373_setAsicRegBits(RTL8373_RS_LAYER_CONFIG_ADDR, 0x20, 1);

    //
    for (i = 0; i < 10; i++)
    {
        rtl8373_setAsicReg(RTL8373_FC_PORT_ACT_CTRL_ADDR(i), 0x1050);
    }
    // ## ---------------------------Init END--------------------------
    rtl8373_setAsicRegBits(RTL8373_DW8051_CFG_ADDR, RTL8373_DW8051_CFG_DW8051_READY_MASK, ENABLE);
    delay_loop(100);

#if PHY_PATCH
    if (Ver8373_72 == 2)
    {
        RL6818C_pwr_on_patch_phy_v008(0xf0);
        RL6818C_pwr_on_patch_phy_v008_rls_lockmain(0xf0);
    }

#endif

    //  puts "Power up PHY 4~7"
    dal_rtl8373_phy_write(0xF0, 31, 0xa610, 0x2058);
    // RTL8372/RTL8372N/RTL8366U set polling mask 0x1f8, port 3/8 from serdes need config bit8=1
    rtl8373_setAsicRegBits(RTL8373_SMI_GLB_CTRL_ADDR, 0x1FF000, 0x1f8);

    delay_loop(50);

    return RT_ERR_OK;
}

void Pin_Reset_8224N_via_8373N(void)
{
    //******************** Begin to Reset RTL8224N by RTL8373N_GPIO30 ***************************
    rtl8373_setAsicRegBit(RTL8373_GPIO_OUT0_ADDR, 30, 0);   // #GPIO_OUT0.bit30=0 for GPIO30
    rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_2_ADDR, 3, 0); // #GPIO30 Function

    rtl8373_setAsicRegBit(RTL8373_GPIO_OE0_ADDR, 30, 1); // #GPIO_OE0.bit30=1 for enable GPIO30 output
    // after 11
    delay_loop(100);
    rtl8373_setAsicRegBit(RTL8373_GPIO_OUT0_ADDR, 30, 1); // #GPIO_OUT0.bit30=1 for GPIO30=1 to set RTL8224N nRESET pin high
    delay_loop(500);
}

rtk_api_ret_t rtl8373N_8224N_init(void)
{
    rtk_uint32 chip_mode = 0;
    rtk_uint32 i;
    rtk_uint32 phymask = 0xff;

    rtl8373_getAsicRegBits(RTL8373_BOND_INFO_ADDR, 3, &chip_mode);

    if (chip_mode != 0x3)
    {
        return RT_ERR_CHIP_NOT_SUPPORTED; // chip mode error
    }

    Pin_Reset_8224N_via_8373N();
    //******************** Begin to Initial RTL8373 MDC/MDIO ********************************
    //	puts "MDC/MDIO pad initial"
    rtl8373_setAsicRegBits(RTL8373_SMI_MAC_TYPE_CTRL_ADDR, RTL8373_SMI_MAC_TYPE_CTRL_MAC_PORT8_TYPE_MASK, 0); // MAC_PORT8_TYPE=0 port type: RTL8373 default set port0-7 polling 10G/2.5GPHY
    rtl8373_setAsicRegBits(RTL8373_SMI_PORT_POLLING_SEL_ADDR, 0xFF, 0xFF);                                    // #RTL8372/RTL8372N/RTL8366U set port4-7 polling internal resolution reg
    rtl8373_setAsicRegBits(RTL8373_SMI_CTRL_ADDR, 0X7000, 7);                                                 // enable SMI0/1/2 MDC clock output, default is 0

    //******************** End to Initial RTL8373 MDC/MDIO ********************************
    //>50ms
    delay_loop(5000000);

    get_version_8373();
    get_version_8224();
    rtlglue_printf("8373N_8224N init, 8373N ver is %d, 8224N ver is %d\n", Ver8373N, Ver8224N);

    delay_loop(100);
    // ## ---------------------------Init LED--------------------------
    // led_8373N_cfg();
    // ## ---------------------------Init SDS--------------------------
    rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_USX_SUB_MODE_MASK, 2); // SDS0_USX_SUB_MODE = 0x2, default is 0x2 10G-QXGMII
    rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK, 0xD);   // SDS0_MODE_SEL = 0xD

    dal_rtl8373_sds_regbits_write(0, 6, 1, 0x4, 1); // #serdes0 AFE loopback
    delay_loop(20);

    dal_rtl8373_sds_regbits_write(0, 6, 1, 0x4, 0);                                                   // #关闭loopback
    rtl8373_setAsicRegBits(RTL8373_SDS_MODE_SEL_ADDR, RTL8373_SDS_MODE_SEL_SDS0_MODE_SEL_MASK, 0x1F); // SDS0_MODE_SEL = 0x1F
    delay_loop(100);

    dal_rtl8373_sds_regbits_write(0, 6, 2, 0x2000, 1); // ##S0RX PN swap for 64B/66B
    dal_rtl8373_sds_regbits_write(1, 6, 2, 0x2000, 1); // S1RX PN swap for 64B/66B

    dal_rtl8373_sds_regbits_write(1, 0, 0, 0x200, 1); // #S1RX PN swap for 8B/10B

    dal_rtl8224_sds_regbits_write(0, 6, 2, 0x2000, 1); // ##S0RX PN swap for 64B/66B

    SDS_MODE_SET_SW(CHIP_RTL8373N_MODE, 0, SERDES_10GQXG);

    // ## ---------------------------Init 8224 SDS--------------------------
    SDS_MODE_SET_SW(CHIP_RTL8224N_MODE, 0, SERDES_10GQXG);
    delay_loop(50);
    fw_reset_flow_tgr_8224(0);

    delay_loop(50);
    fw_reset_flow_tgr(1);

    delay_loop(50);
    fw_reset_flow_tgr(0);

    // ## ---------------------------Patch PHY--------------------------

    // ##MDI reverse configuration for Demo Tap UP RJ45, RTL8366U/RTL8373N/RTL8372N
    rtl8373_setAsicRegBits(RTL8373_CFG_PHY_MDI_REVERSE_ADDR, 0xf, 0xc);
    rtl8373_setAsicRegBits(RTL8373_CFG_PHY_TX_POLARITY_SWAP_ADDR, 0xffff, 0x596A); // #TX_POLARITY_SWAP

    dal_rtl8224_top_regbits_write(RTL8373_CFG_PHY_MDI_REVERSE_ADDR, 0xf, 0xc);
    dal_rtl8224_top_regbits_write(RTL8373_CFG_PHY_TX_POLARITY_SWAP_ADDR, 0xffff, 0x596A);

    //	puts "Power down PHY 0~7"
    dal_rtl8373_phy_write(phymask, 31, 0xa610, 0x2858);

    // ## ---------------------------Patch MAC--------------------------
    rtl8373_setAsicRegBit(RTL8373_MAC_L2_GLOBAL_CTRL0_ADDR, RTL8373_MAC_L2_GLOBAL_CTRL0_FWD_INVLD_MAC_CTRL_EN_OFFSET, 1); // #cfg_FWD_INVLD_MAC_CTRL_EN,
    rtl8373_setAsicRegBit(RTL8373_MAC_L2_GLOBAL_CTRL0_ADDR, RTL8373_MAC_L2_GLOBAL_CTRL0_FWD_UNKN_OPCODE_EN_OFFSET, 1);    // cfg_FWD_UNKN_OPCODE_EN

    for (i = 0; i < 8; i++)
    {
        rtl8373_setAsicRegBit(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), RTL8373_MAC_L2_PORT_CTRL_RX_CHK_CRC_EN_OFFSET, 1); // #RX_CHK_CRC_EN=1
        rtl8373_setAsicRegBit(RTL8373_MAC_L2_PORT_CTRL_ADDR(i), RTL8373_MAC_L2_PORT_CTRL_CLOCK_SWITCH_OFFSET, 1);  // #CLOCK_SWITCH=1;
    }

    rtl8373_setAsicRegBit(RTL8373_RS_LAYER_CONFIG_ADDR, RTL8373_RS_LAYER_CONFIG_RS_LINK_FAULT_INDI_OFF_OFFSET, 1);

    for (i = 0; i < 10; i++)
    {
        rtl8373_setAsicReg(RTL8373_FC_PORT_ACT_CTRL_ADDR(i), 0x1050);
    }
    // ## ---------------------------Init END--------------------------

    rtl8373_setAsicRegBit(RTL8373_DW8051_CFG_ADDR, RTL8373_DW8051_CFG_DW8051_READY_OFFSET, ENABLE);
    delay_loop(50);

#if PHY_PATCH

    if (Ver8224N == 1) // chip B
    {
        RL6818B_pwr_on_patch_phy_v009(0xf);
        RL6818B_pwr_on_patch_phy_v009_rls_lockmain(0xf);
    }
    else if (Ver8224N == 2) // chip C
    {
        RL6818C_pwr_on_patch_phy_v008(0xf);
        RL6818C_pwr_on_patch_phy_v008_rls_lockmain(0xf);
    }

    if (Ver8373N == 1)
    {
        RL6818B_pwr_on_patch_phy_v009(0xf0);
        RL6818B_pwr_on_patch_phy_v009_rls_lockmain(0xf0);
    }
    else if (Ver8373N == 2)
    {
        RL6818C_pwr_on_patch_phy_v008(0xf0);
        RL6818C_pwr_on_patch_phy_v008_rls_lockmain(0xf0);
    }
#endif

    //  puts "Power up PHY 0~7"
    dal_rtl8373_phy_write(phymask, 31, 0xa610, 0x2058);
    rtl8373_setAsicRegBits(RTL8373_SMI_GLB_CTRL_ADDR, RTL8373_SMI_GLB_CTRL_SMI_POLLING_MASK_MASK, 0x1ff);

    delay_loop(50);

    return RT_ERR_OK;
}

/* Function Name:
 * 	 dal_rtl8373_switch_init
 * Description:
 * 	 Set chip to default configuration enviroment
 * Input:
 * 	 None
 * Output:
 * 	 None
 * Return:
 * 	 RT_ERR_OK			 - OK
 * 	 RT_ERR_FAILED		 - Failed
 * 	 RT_ERR_SMI 		 - SMI access error
 * Note:
 * 	 The API can set chip registers to default configuration for different release chip model.
 */
rtk_api_ret_t dal_rtl8373_switch_init()
{
    ret_t retVal;
    rtk_uint32 regdata;

    if ((retVal = rtl8373_getAsicReg(0x4, &regdata)) != RT_ERR_OK)
        return retVal;

    regdata = regdata >> 8;

    if (regdata == 0x837300)
    {
        CHIP_TYPE = CHIP_RTL8373_MODE;

        return rtl8373_8224_init();
    }
    else if (regdata == 0x837200)
    {
        CHIP_TYPE = CHIP_RTL8372_MODE;

        return rtl8372_init();
    }
    else if (regdata == 0x822400)
    {
        CHIP_TYPE = CHIP_RTL8224_MODE;
    }
    else if (regdata == 0x837370)
    {
        CHIP_TYPE = CHIP_RTL8373N_MODE;

        return rtl8373N_8224N_init();
    }
    else if (regdata == 0x837270)
    {
        CHIP_TYPE = CHIP_RTL8372N_MODE;

        return rtl8372n_init();
    }
    else if (regdata == 0x822470)
    {
        CHIP_TYPE = CHIP_RTL8224N_MODE;
    }
    else if (regdata == 0x8366A8)
    {
        CHIP_TYPE = CHIP_RTL8366U_MODE;

        return rtl8366u_init();
    }
    else
        return RT_ERR_CHIP_NOT_FOUND;

    return RT_ERR_OK;
}
