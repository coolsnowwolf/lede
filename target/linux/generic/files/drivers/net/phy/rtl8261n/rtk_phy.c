/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#include <linux/module.h>
#include <linux/phy.h>

#include "phy_rtl826xb_patch.h"
#include "rtk_phylib_rtl826xb.h"
#include "rtk_phylib.h"

#define REALTEK_PHY_ID_RTL8261N         0x001CCAF3
#define REALTEK_PHY_ID_RTL8264B         0x001CC813

static int rtl826xb_get_features(struct phy_device *phydev)
{
    int ret;
    ret = genphy_c45_pma_read_abilities(phydev);
    if (ret)
        return ret;

    linkmode_or(phydev->supported, phydev->supported, PHY_BASIC_FEATURES);


    linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
                       phydev->supported);
    linkmode_set_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
                       phydev->supported);

    /* not support 10M modes */
    linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
                       phydev->supported);
    linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT,
                       phydev->supported);

    return 0;
}

static int rtl826xb_probe(struct phy_device *phydev)
{
    struct rtk_phy_priv *priv = NULL;

    priv = devm_kzalloc(&phydev->mdio.dev, sizeof(struct rtk_phy_priv), GFP_KERNEL);
    if (!priv)
    {
        return -ENOMEM;
    }
    memset(priv, 0, sizeof(struct rtk_phy_priv));

    if (phy_rtl826xb_patch_db_init(0, phydev, &(priv->patch)) != RT_ERR_OK)
        return -ENOMEM;

    priv->phytype = (phydev->drv->phy_id == REALTEK_PHY_ID_RTL8261N) ? (RTK_PHYLIB_RTL8261N) : (RTK_PHYLIB_RTL8264B);
    priv->isBasePort = (phydev->drv->phy_id == REALTEK_PHY_ID_RTL8261N) ? (1) : (((phydev->mdio.addr % 4) == 0) ? (1) : (0));
    phydev->priv = priv;

    return 0;
}

static int rtkphy_config_init(struct phy_device *phydev)
{
    int ret = 0;
    switch (phydev->drv->phy_id)
    {
        case REALTEK_PHY_ID_RTL8261N:
        case REALTEK_PHY_ID_RTL8264B:
            phydev_info(phydev, "%s:%u [RTL8261N/RTL826XB] phy_id: 0x%X PHYAD:%d\n", __FUNCTION__, __LINE__, phydev->drv->phy_id, phydev->mdio.addr);


          #if 1 /* toggle reset */
            phy_modify_mmd_changed(phydev, 30, 0x145, BIT(0)  , 1);
            phy_modify_mmd_changed(phydev, 30, 0x145, BIT(0)  , 0);
            mdelay(30);
          #endif

            ret = phy_patch(0, phydev, 0, PHY_PATCH_MODE_NORMAL);
            if (ret)
            {
                phydev_err(phydev, "%s:%u [RTL8261N/RTL826XB] patch failed!! 0x%X\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
          #if 0 /* Debug: patch check */
            ret = phy_patch(0, phydev, 0, PHY_PATCH_MODE_CMP);
            if (ret)
            {
                phydev_err(phydev, "%s:%u [RTL8261N/RTL826XB] phy_patch failed!! 0x%X\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            printk("[%s,%u] patch chk %s\n", __FUNCTION__, __LINE__, (ret == 0) ? "PASS" : "FAIL");
          #endif
          #if 0 /* Debug: USXGMII*/
            {
                uint32 data = 0;
                rtk_phylib_826xb_sds_read(phydev, 0x07, 0x10, 15, 0, &data);
                printk("[%s,%u] SDS 0x07, 0x10 : 0x%X\n", __FUNCTION__, __LINE__, data);
                rtk_phylib_826xb_sds_read(phydev, 0x06, 0x12, 15, 0, &data);
                printk("[%s,%u] SDS 0x06, 0x12 : 0x%X\n", __FUNCTION__, __LINE__, data);
            }
            {
                u16 sdspage = 0x5, sdsreg = 0x0;
                u16 regData = (sdspage & 0x3f) | ((sdsreg & 0x1f) << 6) | BIT(15);
                u16 readData = 0;
                phy_write_mmd(phydev, 30, 323, regData);
                do
                {
                    udelay(10);
                    readData = phy_read_mmd(phydev, 30, 323);
                } while ((readData & BIT(15)) != 0);
                readData = phy_read_mmd(phydev, 30, 322);
                printk("[%s,%d] sds link [%s] (0x%X)\n", __FUNCTION__, __LINE__, (readData & BIT(12)) ? "UP" : "DOWN", readData);
            }
          #endif

            break;
        default:
            phydev_err(phydev, "%s:%u Unknow phy_id: 0x%X\n", __FUNCTION__, __LINE__, phydev->drv->phy_id);
            return -EPERM;
    }

    return ret;
}

static int rtkphy_c45_suspend(struct phy_device *phydev)
{
    int ret = 0;

    ret = rtk_phylib_c45_power_low(phydev);

    phydev->speed = SPEED_UNKNOWN;
    phydev->duplex = DUPLEX_UNKNOWN;
    phydev->pause = 0;
    phydev->asym_pause = 0;

    return ret;
}

static int rtkphy_c45_resume(struct phy_device *phydev)
{
    return rtk_phylib_c45_power_normal(phydev);
}

static int rtkphy_c45_config_aneg(struct phy_device *phydev)
{
    bool changed = false;
    u16 reg = 0;
    int ret = 0;

    phydev->mdix_ctrl = ETH_TP_MDI_AUTO;
    if (phydev->autoneg == AUTONEG_DISABLE)
        return genphy_c45_pma_setup_forced(phydev);

    ret = genphy_c45_an_config_aneg(phydev);
    if (ret < 0)
        return ret;
    if (ret > 0)
        changed = true;

    reg = 0;
    if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
                  phydev->advertising))
        reg |= BIT(9);

    if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT,
                  phydev->advertising))
        reg |= BIT(8);

    ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2, 0xA412,
                     BIT(9) | BIT(8) , reg);
    if (ret < 0)
        return ret;
    if (ret > 0)
        changed = true;

    return genphy_c45_check_and_restart_aneg(phydev, changed);
}

static int rtkphy_c45_aneg_done(struct phy_device *phydev)
{
    return genphy_c45_aneg_done(phydev);
}

static int rtkphy_c45_read_status(struct phy_device *phydev)
{
    int ret = 0, status = 0;
    phydev->speed = SPEED_UNKNOWN;
    phydev->duplex = DUPLEX_UNKNOWN;
    phydev->pause = 0;
    phydev->asym_pause = 0;

    ret = genphy_c45_read_link(phydev);
    if (ret)
        return ret;

    if (phydev->autoneg == AUTONEG_ENABLE)
    {
        linkmode_clear_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
           phydev->lp_advertising);

        ret = genphy_c45_read_lpa(phydev);
        if (ret)
            return ret;

        status =  phy_read_mmd(phydev, 31, 0xA414);
        if (status < 0)
            return status;
        linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
            phydev->lp_advertising, status & BIT(11));

        phy_resolve_aneg_linkmode(phydev);
    }
    else
    {
        ret = genphy_c45_read_pma(phydev);
    }

    /* mdix*/
    status = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBT_SWAPPOL);
    if (status < 0)
        return status;

    switch (status & 0x3)
    {
        case MDIO_PMA_10GBT_SWAPPOL_ABNX | MDIO_PMA_10GBT_SWAPPOL_CDNX:
            phydev->mdix = ETH_TP_MDI;
            break;

        case 0:
            phydev->mdix = ETH_TP_MDI_X;
            break;

        default:
            phydev->mdix = ETH_TP_MDI_INVALID;
            break;
    }

    return ret;
}


static struct phy_driver rtk_phy_drivers[] = {
    {
        PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8261N),
        .name               = "Realtek RTL8261N",
        .get_features       = rtl826xb_get_features,
        .config_init        = rtkphy_config_init,
        .probe              = rtl826xb_probe,
        .suspend            = rtkphy_c45_suspend,
        .resume             = rtkphy_c45_resume,
        .config_aneg        = rtkphy_c45_config_aneg,
        .aneg_done          = rtkphy_c45_aneg_done,
        .read_status        = rtkphy_c45_read_status,
    },
    {
        PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8264B),
        .name               = "Realtek RTL8264B",
        .get_features       = rtl826xb_get_features,
        .config_init        = rtkphy_config_init,
        .probe              = rtl826xb_probe,
        .suspend            = rtkphy_c45_suspend,
        .resume             = rtkphy_c45_resume,
        .config_aneg        = rtkphy_c45_config_aneg,
        .aneg_done          = rtkphy_c45_aneg_done,
        .read_status        = rtkphy_c45_read_status,
    },
};

module_phy_driver(rtk_phy_drivers);


static struct mdio_device_id __maybe_unused rtk_phy_tbl[] = {
    { PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8261N) },
    { PHY_ID_MATCH_EXACT(REALTEK_PHY_ID_RTL8264B) },
    { },
};

MODULE_DEVICE_TABLE(mdio, rtk_phy_tbl);

MODULE_AUTHOR("Realtek");
MODULE_DESCRIPTION("Realtek PHY drivers");
MODULE_LICENSE("GPL");
