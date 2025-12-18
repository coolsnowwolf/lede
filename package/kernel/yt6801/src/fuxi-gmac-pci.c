// SPDX-License-Identifier: GPL-2.0+
/* Copyright (c) 2021 Motor-comm Corporation. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "fuxi-gmac.h"
#include "fuxi-gmac-reg.h"

/* declarations */
static void fxgmac_shutdown(struct pci_dev *pdev);

static int fxgmac_probe(struct pci_dev *pcidev, const struct pci_device_id *id)
{
    struct device *dev = &pcidev->dev;
    struct fxgmac_resources res;
    int i, ret;

    ret = pcim_enable_device(pcidev);
    if (ret) {
        dev_err(dev, "ERROR: fxgmac_probe failed to enable device\n");
        return ret;
    }

    for (i = 0; i <= PCI_STD_RESOURCE_END; i++) {
        if (pci_resource_len(pcidev, i) == 0)
            continue;

        ret = pcim_iomap_regions(pcidev, BIT(i), FXGMAC_DRV_NAME);
        if (ret)
        {
            dev_err(dev, "fxgmac_probe pcim_iomap_regions failed\n");
            return ret;
        }
        /* DPRINTK(KERN_INFO "fxgmac_probe iomap_region i=%#x,ret=%#x\n",(int)i,(int)ret); */
        break;
    }

    pci_set_master(pcidev);

    memset(&res, 0, sizeof(res));
    res.irq = pcidev->irq;
    res.addr = pcim_iomap_table(pcidev)[i];

    return fxgmac_drv_probe(&pcidev->dev, &res);
}

static void fxgmac_remove(struct pci_dev *pcidev)
{
    struct net_device *netdev;
    struct fxgmac_pdata *pdata;
    (void)pdata;

    netdev = dev_get_drvdata(&pcidev->dev);
    pdata = netdev_priv(netdev);

    fxgmac_drv_remove(&pcidev->dev);

#ifdef CONFIG_PCI_MSI
    if(FXGMAC_GET_REG_BITS(pdata->expansion.int_flags, FXGMAC_FLAG_MSIX_POS,
                           FXGMAC_FLAG_MSIX_LEN)){
        pci_disable_msix(pcidev);
        kfree(pdata->expansion.msix_entries);
        pdata->expansion.msix_entries = NULL;
    }
#endif

    DPRINTK("%s has been removed\n", netdev->name);
}

/* for Power management, 20210628 */
static int __fxgmac_shutdown(struct pci_dev *pdev, bool *enable_wake)
{
    struct net_device *netdev = dev_get_drvdata(&pdev->dev);
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    u32 wufc = pdata->expansion.wol;
#ifdef CONFIG_PM
    int retval = 0;
#endif

    DPRINTK("fxpm,_fxgmac_shutdown, callin\n");

    rtnl_lock();

    /* for linux shutdown, we just treat it as power off wol can be ignored
     * for suspend, we do need recovery by wol
     */
    fxgmac_net_powerdown(pdata, (unsigned int)!!wufc);
    netif_device_detach(netdev);
    rtnl_unlock();

#ifdef CONFIG_PM
    retval = pci_save_state(pdev);
    if (retval) {
        DPRINTK("fxpm,_fxgmac_shutdown, save pci state failed.\n");
        return retval;
    }
#endif

    DPRINTK("fxpm,_fxgmac_shutdown, save pci state done.\n");

    pci_wake_from_d3(pdev, !!wufc);
    *enable_wake = !!wufc;

    pci_disable_device(pdev);

    DPRINTK("fxpm,_fxgmac_shutdown callout, enable wake=%d.\n", *enable_wake);

    return 0;
}

static void fxgmac_shutdown(struct pci_dev *pdev)
{
    struct net_device *netdev = dev_get_drvdata(&pdev->dev);
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    bool wake;

    DPRINTK("fxpm, fxgmac_shutdown callin\n");

    fxgmac_lock(pdata);
    __fxgmac_shutdown(pdev, &wake);
    hw_ops->led_under_shutdown(pdata);

    if (system_state == SYSTEM_POWER_OFF) {
        pci_wake_from_d3(pdev, wake);
        pci_set_power_state(pdev, PCI_D3hot);
    }
    DPRINTK("fxpm, fxgmac_shutdown callout, system power off=%d\n", (system_state == SYSTEM_POWER_OFF)? 1 : 0);
    fxgmac_unlock(pdata);
}

#ifdef CONFIG_PM
/* yzhang, 20210628 for PM */
static int fxgmac_suspend(struct pci_dev *pdev,
            pm_message_t __always_unused state)
{
    struct net_device *netdev = dev_get_drvdata(&pdev->dev);
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    int retval = 0;
    bool wake;

    DPRINTK("fxpm, fxgmac_suspend callin\n");

    fxgmac_lock(pdata);
    if (pdata->expansion.dev_state != FXGMAC_DEV_START)
        goto unlock;

    if (netif_running(netdev)) {
 #ifdef FXGMAC_ASPM_ENABLED
        fxgmac_cancel_aspm_config_work(pdata);
        pdata->expansion.aspm_en = false;
        pdata->expansion.aspm_work_active = false;
        pdata->expansion.recover_from_aspm = false;
#endif
        retval = __fxgmac_shutdown(pdev, &wake);
        if (retval)
            goto unlock;
    } else {
        wake = !!(pdata->expansion.wol);
    }
    hw_ops->led_under_sleep(pdata);

    if (wake) {
        pci_prepare_to_sleep(pdev);
    } else {
        pci_wake_from_d3(pdev, false);
        pci_set_power_state(pdev, PCI_D3hot);
    }

    pdata->expansion.recover_phy_state = 1;
    pdata->expansion.dev_state = FXGMAC_DEV_SUSPEND;
    DPRINTK("fxpm, fxgmac_suspend callout to %s\n", wake ? "sleep" : "D3hot");

unlock:
    fxgmac_unlock(pdata);
    return retval;
}

static int fxgmac_resume(struct pci_dev *pdev)
{
    struct net_device *netdev = dev_get_drvdata(&pdev->dev);
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    u32 err = 0;

    DPRINTK("fxpm, fxgmac_resume callin\n");

    fxgmac_lock(pdata);
    if (pdata->expansion.dev_state != FXGMAC_DEV_SUSPEND)
        goto unlock;

    pdata->expansion.dev_state = FXGMAC_DEV_RESUME;

    pci_set_power_state(pdev, PCI_D0);
    pci_restore_state(pdev);
    /*
     * pci_restore_state clears dev->state_saved so call
     * pci_save_state to restore it.
     */
    pci_save_state(pdev);

    err = pci_enable_device_mem(pdev);
    if (err) {
        dev_err(pdata->dev, "fxgmac_resume, failed to enable PCI device from suspend\n");
        goto unlock;
    }
    smp_mb__before_atomic();
    __clear_bit(FXGMAC_POWER_STATE_DOWN, &pdata->expansion.powerstate);
    pci_set_master(pdev);

    pci_wake_from_d3(pdev, false);

    rtnl_lock();
    err = 0;
    if (!err && netif_running(netdev))
        fxgmac_net_powerup(pdata);

    if (!err)
        netif_device_attach(netdev);

    rtnl_unlock();

    DPRINTK("fxpm, fxgmac_resume callout\n");
unlock:
    fxgmac_unlock(pdata);
    return err;
}
#endif

static const struct pci_device_id fxgmac_pci_tbl[] = {
    { PCI_DEVICE(0x1f0a, 0x6801) },
    { 0 }
};
MODULE_DEVICE_TABLE(pci, fxgmac_pci_tbl);

static struct pci_driver fxgmac_pci_driver = {
    .name       = FXGMAC_DRV_NAME,
    .id_table   = fxgmac_pci_tbl,
    .probe      = fxgmac_probe,
    .remove     = fxgmac_remove,
#ifdef CONFIG_PM
    /* currently, we only use USE_LEGACY_PM_SUPPORT */
    .suspend    = fxgmac_suspend,
    .resume     = fxgmac_resume,
#endif
    .shutdown   = fxgmac_shutdown,
};

module_pci_driver(fxgmac_pci_driver);

MODULE_DESCRIPTION(FXGMAC_DRV_DESC);
MODULE_VERSION(FXGMAC_DRV_VERSION);
MODULE_AUTHOR("Motorcomm Electronic Tech. Co., Ltd.");
MODULE_LICENSE("GPL");
