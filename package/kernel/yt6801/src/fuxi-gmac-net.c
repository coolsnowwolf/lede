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

static int fxgmac_one_poll_rx(struct napi_struct *, int);
static int fxgmac_one_poll_tx(struct napi_struct *, int);
static int fxgmac_all_poll(struct napi_struct *, int);
static int fxgmac_dev_read(struct fxgmac_channel *channel);

/* define for centos 7.0 */
#if (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7,0)) && (RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(7,8))

static inline void ____napi_schedule(struct softnet_data *sd,
                                     struct napi_struct *napi)
{
        list_add_tail(&napi->poll_list, &sd->poll_list);
        //__raise_softirq_irqoff(NET_RX_SOFTIRQ);
        or_softirq_pending(1UL << NET_RX_SOFTIRQ);
}

/**
 * __napi_schedule_irqoff - schedule for receive
 * @n: entry to schedule
 *
 * Variant of __napi_schedule() assuming hard irqs are masked
 */
void __napi_schedule_irqoff(struct napi_struct *n)
{
        ____napi_schedule(this_cpu_ptr(&softnet_data), n);
}

/**
 *      napi_schedule_irqoff - schedule NAPI poll
 *      @n: napi context
 *
 * Variant of napi_schedule(), assuming hard irqs are masked.
 */
static inline void napi_schedule_irqoff(struct napi_struct *n)
{
        if (napi_schedule_prep(n))
                __napi_schedule_irqoff(n);
}
#endif

void fxgmac_lock(struct fxgmac_pdata *pdata)
{
    mutex_lock(&pdata->expansion.mutex);
}

void fxgmac_unlock(struct fxgmac_pdata *pdata)
{
    mutex_unlock(&pdata->expansion.mutex);
}

#ifdef FXGMAC_ESD_CHECK_ENABLED
static void fxgmac_schedule_esd_work(struct fxgmac_pdata *pdata)
{
    set_bit(FXGMAC_FLAG_TASK_ESD_CHECK_PENDING, pdata->expansion.task_flags);
    schedule_delayed_work(&pdata->expansion.esd_work, FXGMAC_ESD_INTERVAL);
}

static void fxgmac_update_esd_stats(struct fxgmac_pdata *pdata)
{
    u32 value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_TXEXCESSIVECOLLSIONFRAMES);
    pdata->expansion.esd_stats.tx_abort_excess_collisions += value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_TXUNDERFLOWERROR_LO);
    pdata->expansion.esd_stats.tx_dma_underrun += value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_TXCARRIERERRORFRAMES);
    pdata->expansion.esd_stats.tx_lost_crs += value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_TXLATECOLLISIONFRAMES);
    pdata->expansion.esd_stats.tx_late_collisions += value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_RXCRCERROR_LO);
    pdata->expansion.esd_stats.rx_crc_errors += value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_RXALIGNERROR);
    pdata->expansion.esd_stats.rx_align_errors += value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_RXRUNTERROR);
    pdata->expansion.esd_stats.rx_runt_errors += value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_TXSINGLECOLLISION_G);
    pdata->expansion.esd_stats.single_collisions += value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_TXMULTIPLECOLLISION_G);
    pdata->expansion.esd_stats.multi_collisions += value;

    value = readreg(pdata->pAdapter, pdata->mac_regs + MMC_TXDEFERREDFRAMES);
    pdata->expansion.esd_stats.tx_deferred_frames += value;
}

static void fxgmac_check_esd_work(struct fxgmac_pdata *pdata)
{
    FXGMAC_ESD_STATS *stats = &pdata->expansion.esd_stats;
    int i = 0;
    u32 regval;

    /* ESD test will make recv crc errors more than 4,294,967,xxx in one second. */
    if (stats->rx_crc_errors > FXGMAC_ESD_ERROR_THRESHOLD ||
        stats->rx_align_errors > FXGMAC_ESD_ERROR_THRESHOLD ||
        stats->rx_runt_errors > FXGMAC_ESD_ERROR_THRESHOLD ||
        stats->tx_abort_excess_collisions > FXGMAC_ESD_ERROR_THRESHOLD ||
        stats->tx_dma_underrun > FXGMAC_ESD_ERROR_THRESHOLD ||
        stats->tx_lost_crs > FXGMAC_ESD_ERROR_THRESHOLD ||
        stats->tx_late_collisions > FXGMAC_ESD_ERROR_THRESHOLD ||
        stats->single_collisions > FXGMAC_ESD_ERROR_THRESHOLD ||
        stats->multi_collisions > FXGMAC_ESD_ERROR_THRESHOLD ||
        stats->tx_deferred_frames > FXGMAC_ESD_ERROR_THRESHOLD) {
        dev_info(pdata->dev, "%s - Error:\n", __func__);
        dev_info(pdata->dev, "rx_crc_errors %ul.\n", stats->rx_crc_errors);
        dev_info(pdata->dev, "rx_align_errors %ul.\n", stats->rx_align_errors);
        dev_info(pdata->dev, "rx_runt_errors %ul.\n", stats->rx_runt_errors);
        dev_info(pdata->dev, "tx_abort_excess_collisions %ul.\n", stats->tx_abort_excess_collisions);
        dev_info(pdata->dev, "tx_dma_underrun %ul.\n", stats->tx_dma_underrun);
        dev_info(pdata->dev, "tx_lost_crs %ul.\n", stats->tx_lost_crs);
        dev_info(pdata->dev, "tx_late_collisions %ul.\n", stats->tx_late_collisions);
        dev_info(pdata->dev, "single_collisions %ul.\n", stats->single_collisions);
        dev_info(pdata->dev, "multi_collisions %ul.\n", stats->multi_collisions);
        dev_info(pdata->dev, "tx_deferred_frames %ul.\n", stats->tx_deferred_frames);

        dev_info(pdata->dev, "esd error triggered, restart NIC...\n");
        cfg_r32(pdata, REG_PCI_COMMAND, &regval);
        while ((regval == FXGMAC_PCIE_LINK_DOWN) && (i++ < FXGMAC_PCIE_RECOVER_TIMES)) {
            usleep_range_ex(pdata->pAdapter, 200, 200);
            cfg_r32(pdata, REG_PCI_COMMAND, &regval);
            dev_info(pdata->dev, "pcie recovery link cost %d(200us)\n", i);
        }

        if (regval == FXGMAC_PCIE_LINK_DOWN) {
            dev_info(pdata->dev, "pcie link down, recovery failed.\n");
            return;
        }

        if (regval & FXGMAC_PCIE_IO_MEM_MASTER_ENABLE) {
            pdata->hw_ops.esd_restore_pcie_cfg(pdata);
            cfg_r32(pdata, REG_PCI_COMMAND, &regval);
            dev_info(pdata->dev, "pci command reg is %x after restoration.\n", regval);
            fxgmac_restart_dev(pdata);
        }
    }

    memset(stats, 0, sizeof(FXGMAC_ESD_STATS));
}

static void fxgmac_esd_work(struct work_struct *work)
{
    struct fxgmac_pdata *pdata = container_of(work,
                                             struct fxgmac_pdata,
                                             expansion.esd_work.work);

    rtnl_lock();
    if (!netif_running(pdata->netdev) ||
       !test_and_clear_bit(FXGMAC_FLAG_TASK_ESD_CHECK_PENDING, pdata->expansion.task_flags))
       goto out_unlock;

    fxgmac_update_esd_stats(pdata);
    fxgmac_check_esd_work(pdata);
    fxgmac_schedule_esd_work(pdata);

out_unlock:
    rtnl_unlock();
}

static void fxgmac_cancel_esd_work(struct fxgmac_pdata *pdata)
{
    struct work_struct *work = &pdata->expansion.esd_work.work;

    if (!work->func) {
        dev_info(pdata->dev, "work func is NULL.\n");
        return;
    }

    cancel_delayed_work_sync(&pdata->expansion.esd_work);
}
#endif

#ifdef FXGMAC_EPHY_LOOPBACK_DETECT_ENABLED
static void fxgmac_schedule_loopback_work(struct fxgmac_pdata *pdata)
{
    schedule_delayed_work(&pdata->expansion.loopback_work, FXGMAC_LOOPBACK_CHECK_INTERVAL);
}

static void fxgmac_loopback_work(struct work_struct *work)
{
    int ret;
    u32 regval;
    struct fxgmac_pdata *pdata = container_of(work,
                                             struct fxgmac_pdata,
                                             expansion.loopback_work.work);

    if (pdata->expansion.lb_test_flag || pdata->expansion.phy_link)
        goto reschedule;

    if (!pdata->expansion.lb_cable_flag) {
        ret = pdata->hw_ops.read_ephy_reg(pdata, REG_MII_STAT1000, &regval);
        if (ret < 0) {
            printk("%s:read ephy failed\n", __func__);
            goto reschedule;
        }

        // printk("%s: regval = 0x%x\n", __func__, regval);
        regval = FXGMAC_GET_REG_BITS(regval, PHY_MII_STAT1000_CFG_ERROR_POS,
                            PHY_MII_STAT1000_CFG_ERROR_LEN);
        if (regval == 1) {
            pdata->expansion.lb_cable_detect_count++;
            if (pdata->expansion.lb_cable_detect_count == FXGMAC_PHY_LOOPBACK_DETECT_THRESOLD) {
                pdata->expansion.lb_cable_flag = 1;
                pdata->hw_ops.setup_cable_loopback(pdata);
                pdata->expansion.lb_cable_detect_count = 0;
            }
        }
    }

reschedule:
    fxgmac_schedule_loopback_work(pdata);
}

static void fxgmac_cancel_loopback_work(struct fxgmac_pdata *pdata)
{
    struct work_struct *work = &pdata->expansion.loopback_work.work;

    if (!work->func) {
        dev_info(pdata->dev, "work func is NULL.\n");
        return;
    }

    cancel_delayed_work_sync(&pdata->expansion.loopback_work);
}
#endif

#ifdef FXGMAC_ASPM_ENABLED
void fxgmac_schedule_aspm_config_work(struct fxgmac_pdata *pdata)
{
    if (!pdata->expansion.aspm_work_active &&
        !pdata->expansion.aspm_en &&
        pdata->expansion.dev_state != FXGMAC_DEV_CLOSE) {
        schedule_delayed_work(&pdata->expansion.aspm_config_work, FXGMAC_ASPM_INTERVAL);
        pdata->expansion.aspm_work_active = true;
    }
}

static void fxgmac_aspm_config_work(struct work_struct *work)
{
    u32 pcie_low_power = PCIE_LP_ASPM_LTR | PCIE_LP_ASPM_L1SS | PCIE_LP_ASPM_L1;
    struct fxgmac_pdata *pdata = container_of(work,
                                             struct fxgmac_pdata,
                                             expansion.aspm_config_work.work);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    if (pdata->expansion.aspm_work_active) {
        hw_ops->pcie_init(pdata, pcie_low_power & PCIE_LP_ASPM_LTR,
                          pcie_low_power & PCIE_LP_ASPM_L1SS,
                          pcie_low_power & PCIE_LP_ASPM_L1,
                          pcie_low_power & PCIE_LP_ASPM_L0S);
        //hw_ops->set_pwr_clock_gate(pdata);
        pdata->expansion.aspm_en = true;
        printk("NIC set aspm at link down\n");
    }
    pdata->expansion.aspm_work_active = false;
}

void fxgmac_cancel_aspm_config_work(struct fxgmac_pdata *pdata)
{
    struct work_struct *work = &pdata->expansion.aspm_config_work.work;

    if (!work->func) {
        dev_info(pdata->dev, "work func is NULL.\n");
        return;
    }

    cancel_delayed_work_sync(&pdata->expansion.aspm_config_work);
}

bool fxgmac_aspm_action_linkup(struct fxgmac_pdata *pdata)
{
    if ((pdata->expansion.aspm_work_active || pdata->expansion.aspm_en)
        && !pdata->expansion.lb_cable_flag) {
        printk("cancel aspm work.\n");
        pdata->expansion.aspm_work_active = false;
        fxgmac_cancel_aspm_config_work(pdata);
        if (pdata->expansion.aspm_en) {
            printk("reset from aspm.\n");
            pdata->expansion.aspm_en = false;
            pdata->expansion.recover_from_aspm = true;
            schedule_work(&pdata->expansion.restart_work);
            return true;
        }
        pdata->expansion.aspm_en = false;
    }

    return false;
}

#endif

unsigned int fxgmac_get_netdev_ip4addr(struct fxgmac_pdata *pdata)
{
    struct net_device *netdev = pdata->netdev;
    struct in_ifaddr *ifa;
    unsigned int ipval = 0xc0a801ca; //here just hard code to 192.168.1.202

    rcu_read_lock();
    /* we only get the first IPv4 addr. */
    ifa = rcu_dereference(netdev->ip_ptr->ifa_list);
    if(ifa) {
        /* binary ipv4 addr with __be */
        ipval = (unsigned int)ifa->ifa_address;

        DPRINTK("%s, netdev %s IPv4 address %pI4, mask: %pI4\n",__FUNCTION__, ifa->ifa_label, &ifa->ifa_address, &ifa->ifa_mask);
    }
    //  ifa = rcu_dereference(ifa->ifa_next); // more ipv4 addr
    rcu_read_unlock();

    return ipval;
}

unsigned char * fxgmac_get_netdev_ip6addr(struct fxgmac_pdata *pdata, unsigned char *ipval, unsigned char *ip6addr_solicited, unsigned int ifa_flag)
{
    struct net_device *netdev = pdata->netdev;
    struct inet6_dev *i6dev;
    struct inet6_ifaddr *ifp;
    unsigned char local_ipval[16] = {0};
    unsigned char solicited_ipval[16] = {0};
    struct in6_addr *addr_ip6 = (struct in6_addr *)local_ipval;
    struct in6_addr *addr_ip6_solicited = (struct in6_addr *)solicited_ipval;
    int err = -EADDRNOTAVAIL;

    //in6_pton("fe80::35c6:dd1b:9745:fc9b", -1, (u8*)ipval, -1, NULL); //here just hard code for default
    if(ipval) {
        addr_ip6 = (struct in6_addr *)ipval;
    }

    if(ip6addr_solicited) {
        addr_ip6_solicited = (struct in6_addr *)ip6addr_solicited;
    }

    in6_pton("fe80::4808:8ffb:d93e:d753", -1, (u8*)addr_ip6, -1, NULL); //here just hard code for default

    if (ifa_flag & FXGMAC_NS_IFA_GLOBAL_UNICAST)
        DPRINTK ("%s FXGMAC_NS_IFA_GLOBAL_UNICAST is set, %x\n", __FUNCTION__, ifa_flag);

    if (ifa_flag & FXGMAC_NS_IFA_LOCAL_LINK)
        DPRINTK ("%s FXGMAC_NS_IFA_LOCAL_LINK is set, %x\n", __FUNCTION__, ifa_flag);

    rcu_read_lock();
    i6dev = __in6_dev_get(netdev);
    if (i6dev != NULL) {
        read_lock_bh(&i6dev->lock);
        list_for_each_entry(ifp, &i6dev->addr_list, if_list) {

            /* here we need only the ll addr, use scope to filter out it. */
            if (((ifa_flag & FXGMAC_NS_IFA_GLOBAL_UNICAST) && (ifp->scope != IFA_LINK)) || ((ifa_flag & FXGMAC_NS_IFA_LOCAL_LINK) && (ifp->scope == IFA_LINK)/* &&
                !(ifp->flags & IFA_F_TENTATIVE)*/)) {

                memcpy(addr_ip6, &ifp->addr, 16);
                addrconf_addr_solict_mult(addr_ip6, addr_ip6_solicited);
                err = 0;

                //DPRINTK("%s, netdev %s IPv6 local-link address %pI6\n",__FUNCTION__, netdev->name, addr_ip6);
                //DPRINTK("%s, netdev %s IPv6 solicited-node add %pI6\n",__FUNCTION__, netdev->name, addr_ip6_solicited);
                break;
            }
        }
        read_unlock_bh(&i6dev->lock);
    }
    rcu_read_unlock();

    if(err) DPRINTK("%s get ipv6 addr failed, use default.\n", __FUNCTION__);

    //DPRINTK("%s, netdev %s IPv6 local-link address %pI6\n",__FUNCTION__, netdev->name, addr_ip6);
    //DPRINTK("%s, netdev %s IPv6 solicited-node adr %pI6\n",__FUNCTION__, netdev->name, addr_ip6_solicited);

    return (err ? NULL : ipval);
}

inline unsigned int fxgmac_tx_avail_desc(struct fxgmac_ring *ring)
{
    //return (ring->dma_desc_count - (ring->cur - ring->dirty));
    unsigned int avail;

    if (ring->dirty > ring->cur)
        avail = ring->dirty - ring->cur;
    else
        avail = ring->dma_desc_count - ring->cur + ring->dirty;

    return avail;
}

inline unsigned int fxgmac_rx_dirty_desc(struct fxgmac_ring *ring)
{
    //return (ring->cur - ring->dirty);
    unsigned int dirty;

    if (ring->dirty <= ring->cur)
        dirty = ring->cur - ring->dirty;
    else
        dirty = ring->dma_desc_count - ring->dirty + ring->cur;

    return dirty;
}

static netdev_tx_t fxgmac_maybe_stop_tx_queue(
            struct fxgmac_channel *channel,
            struct fxgmac_ring *ring,
            unsigned int count)
{
    struct fxgmac_pdata *pdata = channel->pdata;

    if (count > fxgmac_tx_avail_desc(ring)) {
        if(netif_msg_tx_done(pdata)) {
            netif_info(pdata, drv, pdata->netdev,
                   "Tx queue stopped, not enough descriptors available\n");
        }

        /* Avoid wrongly optimistic queue wake-up: tx poll thread must
         * not miss a ring update when it notices a stopped queue.
         */
        smp_wmb();
        netif_stop_subqueue(pdata->netdev, channel->queue_index);
        ring->tx.queue_stopped = 1;

        /* Sync with tx poll:
         * - publish queue status and cur ring index (write barrier)
         * - refresh dirty ring index (read barrier).
         * May the current thread have a pessimistic view of the ring
         * status and forget to wake up queue, a racing tx poll thread
         * can't.
         */
        smp_mb();
        if (count <= fxgmac_tx_avail_desc(ring)) {
            ring->tx.queue_stopped = 0;
            netif_start_subqueue(pdata->netdev, channel->queue_index);
            fxgmac_tx_start_xmit(channel, ring);
        } else {
            /* If we haven't notified the hardware because of xmit_more
             * support, tell it now
             */
            if (ring->tx.xmit_more)
                fxgmac_tx_start_xmit(channel, ring);
            if(netif_msg_tx_done(pdata)) DPRINTK("about stop tx q, ret BUSY\n");
            return NETDEV_TX_BUSY;
        }
    }

    return NETDEV_TX_OK;
}

static void fxgmac_prep_vlan(struct sk_buff *skb,
                struct fxgmac_pkt_info *pkt_info)
{
    if (skb_vlan_tag_present(skb))
        pkt_info->vlan_ctag = skb_vlan_tag_get(skb);
}

static int fxgmac_prep_tso(struct fxgmac_pdata *pdata, struct sk_buff *skb,
                struct fxgmac_pkt_info *pkt_info)
{
    int ret;

    if (!FXGMAC_GET_REG_BITS(pkt_info->attributes,
                TX_PACKET_ATTRIBUTES_TSO_ENABLE_POS,
                TX_PACKET_ATTRIBUTES_TSO_ENABLE_LEN))
        return 0;

    ret = skb_cow_head(skb, 0);
    if (ret)
        return ret;

    pkt_info->header_len = skb_transport_offset(skb) + tcp_hdrlen(skb);
    pkt_info->tcp_header_len = tcp_hdrlen(skb);
    pkt_info->tcp_payload_len = skb->len - pkt_info->header_len;
    pkt_info->mss = skb_shinfo(skb)->gso_size;

    if(netif_msg_tx_done(pdata)){
        DPRINTK("header_len=%u\n", pkt_info->header_len);
        DPRINTK("tcp_header_len=%u, tcp_payload_len=%u\n",
                pkt_info->tcp_header_len, pkt_info->tcp_payload_len);
        DPRINTK("mss=%u\n", pkt_info->mss);
    }
    /* Update the number of packets that will ultimately be transmitted
     * along with the extra bytes for each extra packet
     */
    pkt_info->tx_packets = skb_shinfo(skb)->gso_segs;
    pkt_info->tx_bytes += (pkt_info->tx_packets - 1) * pkt_info->header_len;

    return 0;
}

static int fxgmac_is_tso(struct sk_buff *skb)
{
    if (skb->ip_summed != CHECKSUM_PARTIAL)
        return 0;

    if (!skb_is_gso(skb))
        return 0;

    return 1;
}

static void fxgmac_prep_tx_pkt(struct fxgmac_pdata *pdata,
                struct fxgmac_ring *ring,
                struct sk_buff *skb,
                struct fxgmac_pkt_info *pkt_info)
{
    skb_frag_t *frag;
    unsigned int context_desc;
    unsigned int len;
    unsigned int i;

    pkt_info->skb = skb;

    context_desc = 0;
    pkt_info->desc_count = 0;

    pkt_info->tx_packets = 1;
    pkt_info->tx_bytes = skb->len;
    if(netif_msg_tx_done(pdata))
        DPRINTK ("fxgmac_prep_tx_pkt callin,pkt desc cnt=%d,skb len=%d, skbheadlen=%d\n", pkt_info->desc_count, skb->len, skb_headlen(skb));

    if (fxgmac_is_tso(skb)) {
        /* TSO requires an extra descriptor if mss is different */
        if (skb_shinfo(skb)->gso_size != ring->tx.cur_mss) {
            context_desc = 1;
            pkt_info->desc_count++;
        }
        if(netif_msg_tx_done(pdata))
            DPRINTK("fxgmac_is_tso=%d, ip_summed=%d,skb gso=%d\n",((skb->ip_summed == CHECKSUM_PARTIAL) && (skb_is_gso(skb)))?1:0, skb->ip_summed, skb_is_gso(skb)?1:0);

        /* TSO requires an extra descriptor for TSO header */
        pkt_info->desc_count++;

        pkt_info->attributes = FXGMAC_SET_REG_BITS(
                    pkt_info->attributes,
                    TX_PACKET_ATTRIBUTES_TSO_ENABLE_POS,
                    TX_PACKET_ATTRIBUTES_TSO_ENABLE_LEN,
                    1);
        pkt_info->attributes = FXGMAC_SET_REG_BITS(
                    pkt_info->attributes,
                    TX_PACKET_ATTRIBUTES_CSUM_ENABLE_POS,
                    TX_PACKET_ATTRIBUTES_CSUM_ENABLE_LEN,
                    1);
        if(netif_msg_tx_done(pdata)) DPRINTK ("fxgmac_prep_tx_pkt,tso, pkt desc cnt=%d\n", pkt_info->desc_count);
    } else if (skb->ip_summed == CHECKSUM_PARTIAL)
        pkt_info->attributes = FXGMAC_SET_REG_BITS(
                    pkt_info->attributes,
                    TX_PACKET_ATTRIBUTES_CSUM_ENABLE_POS,
                    TX_PACKET_ATTRIBUTES_CSUM_ENABLE_LEN,
                    1);

    if (skb_vlan_tag_present(skb)) {
        /* VLAN requires an extra descriptor if tag is different */
        if (skb_vlan_tag_get(skb) != ring->tx.cur_vlan_ctag)
            /* We can share with the TSO context descriptor */
            if (!context_desc) {
                context_desc = 1;
                pkt_info->desc_count++;
            }

        pkt_info->attributes = FXGMAC_SET_REG_BITS(
                    pkt_info->attributes,
                    TX_PACKET_ATTRIBUTES_VLAN_CTAG_POS,
                    TX_PACKET_ATTRIBUTES_VLAN_CTAG_LEN,
                    1);
        if(netif_msg_tx_done(pdata)) DPRINTK ("fxgmac_prep_tx_pkt,VLAN, pkt desc cnt=%d,vlan=0x%04x\n", pkt_info->desc_count, skb_vlan_tag_get(skb));
    }

    for (len = skb_headlen(skb); len;) {
        pkt_info->desc_count++;
        len -= min_t(unsigned int, len, FXGMAC_TX_MAX_BUF_SIZE);
    }

    for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
        frag = &skb_shinfo(skb)->frags[i];
        for (len = skb_frag_size(frag); len; ) {
            pkt_info->desc_count++;
            len -= min_t(unsigned int, len, FXGMAC_TX_MAX_BUF_SIZE);
        }
    }
    if(netif_msg_tx_done(pdata))
        DPRINTK ("fxgmac_prep_tx_pkt callout,pkt desc cnt=%d,skb len=%d, skbheadlen=%d,frags=%d\n", pkt_info->desc_count, skb->len, skb_headlen(skb), skb_shinfo(skb)->nr_frags);
}

static int fxgmac_calc_rx_buf_size(struct net_device *netdev, unsigned int mtu)
{
    unsigned int rx_buf_size;
    unsigned int max_mtu;

    /* On the Linux platform, the MTU size does not include the length
     * of the MAC address and the length of the Type, but FXGMAC_JUMBO_PACKET_MTU include them.
     */
    max_mtu = FXGMAC_JUMBO_PACKET_MTU - ETH_HLEN;
    if (mtu > max_mtu) {
        netdev_alert(netdev, "MTU exceeds maximum supported value\n");
        return -EINVAL;
    }

    rx_buf_size = mtu + ETH_HLEN + ETH_FCS_LEN + VLAN_HLEN;
    rx_buf_size = clamp_val(rx_buf_size, FXGMAC_RX_MIN_BUF_SIZE, PAGE_SIZE * 4 /* follow yonggang's suggestion */);

    rx_buf_size = (rx_buf_size + FXGMAC_RX_BUF_ALIGN - 1) &
                ~(FXGMAC_RX_BUF_ALIGN - 1);

    return rx_buf_size;
}

#ifndef FXGMAC_MISC_NOT_ENABLED
static int fxgmac_misc_poll(struct napi_struct *napi, int budget)
{
    struct fxgmac_pdata *pdata = container_of(napi,
                                            struct fxgmac_pdata,
                                            expansion.napi_misc);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0))
    if (napi_complete_done(napi, 0))
        hw_ops->enable_msix_one_interrupt(pdata, MSI_ID_PHY_OTHER);
#else
    napi_complete(napi);
    hw_ops->enable_msix_one_interrupt(pdata, MSI_ID_PHY_OTHER);
#endif
    return 0;
}

static irqreturn_t fxgmac_misc_isr(int irq, void *data)
{
    struct fxgmac_pdata *pdata = data;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    u32 regval;

    regval = readreg(pdata->pAdapter, pdata->base_mem + MGMT_INT_CTRL0);
    if (!(regval & MGMT_INT_CTRL0_INT_STATUS_MISC))
        return IRQ_HANDLED;

    hw_ops->disable_msix_one_interrupt(pdata, MSI_ID_PHY_OTHER);
    hw_ops->clear_misc_int_status(pdata);

    napi_schedule_irqoff(&pdata->expansion.napi_misc);

    return IRQ_HANDLED;
}
#endif

static irqreturn_t fxgmac_isr(int irq, void *data)
{
    unsigned int dma_ch_isr, dma_isr, mac_isr;
    struct fxgmac_pdata *pdata = data;
    struct fxgmac_channel *channel;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    unsigned int i;
    u32 val;

    val = readreg(pdata->pAdapter, pdata->base_mem + MGMT_INT_CTRL0);
    if (!(val & MGMT_INT_CTRL0_INT_STATUS_RXTX_MASK))
        return IRQ_NONE;

    dma_isr = readreg(pdata->pAdapter, pdata->mac_regs + DMA_ISR);

    for (i = 0; i < pdata->channel_count; i++) {
        channel = pdata->channel_head + i;
        dma_ch_isr = readl(FXGMAC_DMA_REG(channel, DMA_CH_SR));

        if (FXGMAC_GET_REG_BITS(dma_ch_isr, DMA_CH_SR_TPS_POS,
                    DMA_CH_SR_TPS_LEN))
            pdata->stats.tx_process_stopped++;

        if (FXGMAC_GET_REG_BITS(dma_ch_isr, DMA_CH_SR_RPS_POS,
                    DMA_CH_SR_RPS_LEN))
            pdata->stats.rx_process_stopped++;

        if (FXGMAC_GET_REG_BITS(dma_ch_isr, DMA_CH_SR_TBU_POS,
                    DMA_CH_SR_TBU_LEN))
            pdata->stats.tx_buffer_unavailable++;

        if (FXGMAC_GET_REG_BITS(dma_ch_isr, DMA_CH_SR_RBU_POS,
                    DMA_CH_SR_RBU_LEN))
            pdata->stats.rx_buffer_unavailable++;

        /* Restart the device on a Fatal Bus Error */
        if (FXGMAC_GET_REG_BITS(dma_ch_isr, DMA_CH_SR_FBE_POS,
                    DMA_CH_SR_FBE_LEN)) {
            pdata->stats.fatal_bus_error++;
            schedule_work(&pdata->expansion.restart_work);
        }
        /* Clear all interrupt signals */
        writel(dma_ch_isr, FXGMAC_DMA_REG(channel, DMA_CH_SR));
    }

    if (FXGMAC_GET_REG_BITS(dma_isr, DMA_ISR_MACIS_POS,
                DMA_ISR_MACIS_LEN)) {
        mac_isr = readl(pdata->mac_regs + MAC_ISR);
#ifndef FXGMAC_MISC_NOT_ENABLED
        if (FXGMAC_GET_REG_BITS(mac_isr, MAC_ISR_MMCTXIS_POS,
                    MAC_ISR_MMCTXIS_LEN))
            hw_ops->tx_mmc_int(pdata);

        if (FXGMAC_GET_REG_BITS(mac_isr, MAC_ISR_MMCRXIS_POS,
                    MAC_ISR_MMCRXIS_LEN))
            hw_ops->rx_mmc_int(pdata);
#endif
        /* Clear all interrupt signals */
        writel(mac_isr, (pdata->mac_regs + MAC_ISR));
    }

    hw_ops->disable_mgm_interrupt(pdata);
    pdata->stats.mgmt_int_isr++;

    if (napi_schedule_prep(&pdata->expansion.napi)) {
        pdata->stats.napi_poll_isr++;
        /* Turn on polling */
        __napi_schedule_irqoff(&pdata->expansion.napi);
    }

    return IRQ_HANDLED;

}

static irqreturn_t fxgmac_dma_isr(int irq, void *data)
{
    struct fxgmac_channel *channel = data;
    struct fxgmac_pdata *pdata = channel->pdata;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    u32 regval;
    int message_id;

    if (irq == channel->expansion.dma_irq_tx) {
        message_id = MSI_ID_TXQ0;
        hw_ops->disable_msix_one_interrupt(pdata, message_id);
        regval = 0;
        regval = FXGMAC_SET_REG_BITS(regval, DMA_CH_SR_TI_POS, DMA_CH_SR_TI_LEN, 1);
        writereg(pdata->pAdapter, regval, FXGMAC_DMA_REG(channel, DMA_CH_SR));
        napi_schedule_irqoff(&channel->expansion.napi_tx);
    } else {
        message_id = channel->queue_index;
        hw_ops->disable_msix_one_interrupt(pdata, message_id);
        regval = 0;
        regval = readreg(pdata->pAdapter, FXGMAC_DMA_REG(channel, DMA_CH_SR));
        regval = FXGMAC_SET_REG_BITS(regval, DMA_CH_SR_RI_POS, DMA_CH_SR_RI_LEN, 1);
        writereg(pdata->pAdapter, regval, FXGMAC_DMA_REG(channel, DMA_CH_SR));
        napi_schedule_irqoff(&channel->expansion.napi_rx);
    }

    return IRQ_HANDLED;
}


#if FXGMAC_TX_HANG_TIMER_ENABLED
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
static void fxgmac_tx_hang_timer_handler(struct timer_list *t)
#else
static void fxgmac_tx_hang_timer_handler(unsigned long data)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
    struct fxgmac_channel *channel = from_timer(channel, t, expansion.tx_hang_timer);
#else
    struct fxgmac_channel *channel = (struct fxgmac_channel *)data;
#endif

#if FXGMAC_TX_HANG_CHECH_DIRTY
    struct fxgmac_ring *ring = channel->tx_ring;
#endif
    struct fxgmac_pdata *pdata = channel->pdata;
    struct net_device *netdev = pdata->netdev;
    unsigned int hw_reg_cur;
    unsigned int regval;

#if FXGMAC_TX_HANG_CHECH_DIRTY
    hw_reg_cur = ring->dirty;
#else
    hw_reg_cur = readl(FXGMAC_DMA_REG(channel, 0x44/* tx desc curr pointer reg */));
#endif
    if(hw_reg_cur == channel->expansion.tx_hang_hw_cur) {

        /* hw current desc still stucked */
        if(!pdata->tx_hang_restart_queuing) {
            pdata->tx_hang_restart_queuing = 1;
            DPRINTK("tx_hang_timer_handler: restart scheduled, at desc %u, queuing=%u.\n", channel->expansion.tx_hang_hw_cur, pdata->tx_hang_restart_queuing);

            netif_tx_stop_all_queues(netdev);

            /* Disable MAC Rx */
            regval = readl(pdata->mac_regs + MAC_CR);
            regval = FXGMAC_SET_REG_BITS(regval, MAC_CR_CST_POS,
                             MAC_CR_CST_LEN, 0);
            regval = FXGMAC_SET_REG_BITS(regval, MAC_CR_ACS_POS,
                             MAC_CR_ACS_LEN, 0);
            regval = FXGMAC_SET_REG_BITS(regval, MAC_CR_RE_POS,
                             MAC_CR_RE_LEN, 0);
            writel(regval, pdata->mac_regs + MAC_CR);

            schedule_work(&pdata->expansion.restart_work);
        }
    }

    channel->expansion.tx_hang_timer_active = 0;
}

static void fxgmac_tx_hang_timer_start(struct fxgmac_channel *channel)
{
    struct fxgmac_pdata *pdata = channel->pdata;

    /* Start the Tx hang timer */
    if (1 && !channel->expansion.tx_hang_timer_active) {
        channel->expansion.tx_hang_timer_active = 1;

        /* FXGMAC_INIT_DMA_TX_USECS is desc3 polling period, we give 2 more checking period */
        mod_timer(&channel->expansion.tx_hang_timer,
                jiffies + usecs_to_jiffies(FXGMAC_INIT_DMA_TX_USECS * 10));
    }
}
#endif

#if 0
static void fxgmac_init_timers(struct fxgmac_pdata *pdata)
{
    struct fxgmac_channel *channel;
    unsigned int i;

    channel = pdata->channel_head;
    for (i = 0; i < pdata->channel_count; i++, channel++) {
        if (!channel->tx_ring)
            break;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
    timer_setup(&channel->tx_timer, fxgmac_tx_timer, 0);
#else
    setup_timer(&channel->tx_timer, fxgmac_tx_timer, (unsigned long)channel);
#endif
#if FXGMAC_TX_HANG_TIMER_ENABLED
    channel->tx_hang_timer_active = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
    timer_setup(&channel->tx_hang_timer, fxgmac_tx_hang_timer_handler, 0);
#else
    setup_timer(&channel->tx_hang_timer, fxgmac_tx_hang_timer_handler, (unsigned long)channel);
#endif
#endif
    }
}

static void fxgmac_stop_timers(struct fxgmac_pdata *pdata)
{
    struct fxgmac_channel *channel;
    unsigned int i;

    channel = pdata->channel_head;
    if (channel != NULL) {
        for (i = 0; i < pdata->channel_count; i++, channel++) {
            if (!channel->tx_ring)
                break;

            del_timer_sync(&channel->tx_timer);
#if FXGMAC_TX_HANG_TIMER_ENABLED
            del_timer_sync(&channel->tx_hang_timer);
            channel->tx_hang_timer_active = 0;
#endif
        }
    }
}
#endif

static void fxgmac_napi_enable(struct fxgmac_pdata *pdata, unsigned int add)
{
    struct fxgmac_channel *channel;
    unsigned int i;
    u32 tx_napi = 0, rx_napi = 0;

#ifndef FXGMAC_MISC_NOT_ENABLED
    u32 misc_napi = 0;

    misc_napi = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MISC_NAPI_FREE_POS,
                                    FXGMAC_FLAG_MISC_NAPI_FREE_LEN);
#endif
    tx_napi = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_TX_NAPI_FREE_POS,
                                    FXGMAC_FLAG_TX_NAPI_FREE_LEN);
    rx_napi = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_RX_NAPI_FREE_POS,
                                    FXGMAC_FLAG_RX_NAPI_FREE_LEN);

    if (pdata->per_channel_irq) {
        channel = pdata->channel_head;
        for (i = 0; i < pdata->channel_count; i++, channel++) {
            if (!FXGMAC_GET_REG_BITS(rx_napi,
                                i, FXGMAC_FLAG_PER_CHAN_RX_NAPI_FREE_LEN)) {
                if (add) {
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0) ||\
      RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(9,2) )
                    netif_napi_add_weight(pdata->netdev, &channel->expansion.napi_rx,
                               fxgmac_one_poll_rx, NAPI_POLL_WEIGHT);
#else
                    netif_napi_add(pdata->netdev, &channel->expansion.napi_rx,
                               fxgmac_one_poll_rx, NAPI_POLL_WEIGHT);
#endif
                }
                napi_enable(&channel->expansion.napi_rx);
                pdata->expansion.int_flags =
                                FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_RX_NAPI_FREE_POS + i,
                                        FXGMAC_FLAG_PER_CHAN_RX_NAPI_FREE_LEN,
                                        FXGMAC_NAPI_ENABLE);
            }

            if (FXGMAC_IS_CHANNEL_WITH_TX_IRQ(i) && !tx_napi) {
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0) ||\
      RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(9,2) )
                netif_napi_add_weight(pdata->netdev, &channel->expansion.napi_tx,
                           fxgmac_one_poll_tx, NAPI_POLL_WEIGHT);
#else
                netif_napi_add(pdata->netdev, &channel->expansion.napi_tx,
                           fxgmac_one_poll_tx, NAPI_POLL_WEIGHT);
#endif
                napi_enable(&channel->expansion.napi_tx);
                pdata->expansion.int_flags =
                                FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_TX_NAPI_FREE_POS,
                                        FXGMAC_FLAG_TX_NAPI_FREE_LEN,
                                        FXGMAC_NAPI_ENABLE);
            }
            if(netif_msg_drv(pdata)) DPRINTK("napi_enable, msix ch%d napi enabled done,add=%d\n", i, add);
        }

#ifndef FXGMAC_MISC_NOT_ENABLED
        /* for misc */
        if (!misc_napi) {
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0) ||\
      RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(9,2) )
            netif_napi_add_weight(pdata->netdev, &pdata->expansion.napi_misc,
                                       fxgmac_misc_poll, NAPI_POLL_WEIGHT);
#else
            netif_napi_add(pdata->netdev, &pdata->expansion.napi_misc,
                            fxgmac_misc_poll, NAPI_POLL_WEIGHT);
#endif
            napi_enable(&pdata->expansion.napi_misc);
            pdata->expansion.int_flags =
                            FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MISC_NAPI_FREE_POS,
                                    FXGMAC_FLAG_MISC_NAPI_FREE_LEN,
                                    FXGMAC_NAPI_ENABLE);
        }
#endif
    } else {
        i = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_LEGACY_NAPI_FREE_POS,
                                    FXGMAC_FLAG_LEGACY_NAPI_FREE_LEN);
        if (!i) {
            if (add) {
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,19,0) ||\
      RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(9,2) )
                netif_napi_add_weight(pdata->netdev, &pdata->expansion.napi,
                           fxgmac_all_poll, NAPI_POLL_WEIGHT);
#else
                netif_napi_add(pdata->netdev, &pdata->expansion.napi,
                           fxgmac_all_poll, NAPI_POLL_WEIGHT);
#endif
            }

            napi_enable(&pdata->expansion.napi);
            pdata->expansion.int_flags = FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                                FXGMAC_FLAG_LEGACY_NAPI_FREE_POS,
                                                FXGMAC_FLAG_LEGACY_NAPI_FREE_LEN,
                                                FXGMAC_NAPI_ENABLE);
        }
    }
}

static void fxgmac_napi_disable(struct fxgmac_pdata *pdata, unsigned int del)
{
    struct fxgmac_channel *channel;
    unsigned int i;
    u32 tx_napi = 0, rx_napi = 0;
#ifndef FXGMAC_MISC_NOT_ENABLED
    u32 misc_napi = 0;
#endif

    if (pdata->per_channel_irq) {
#ifndef FXGMAC_MISC_NOT_ENABLED
        misc_napi = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_MISC_NAPI_FREE_POS,
                                        FXGMAC_FLAG_MISC_NAPI_FREE_LEN);
#endif
        tx_napi = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_TX_NAPI_FREE_POS,
                                        FXGMAC_FLAG_TX_NAPI_FREE_LEN);
        rx_napi = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_RX_NAPI_FREE_POS,
                                        FXGMAC_FLAG_RX_NAPI_FREE_LEN);
        channel = pdata->channel_head;
        if (channel != NULL) {
            for (i = 0; i < pdata->channel_count; i++, channel++) {
                if (FXGMAC_GET_REG_BITS(rx_napi,
                                i, FXGMAC_FLAG_PER_CHAN_RX_NAPI_FREE_LEN)) {
                    napi_disable(&channel->expansion.napi_rx);

                    if (del) {
                        netif_napi_del(&channel->expansion.napi_rx);
                    }
                    pdata->expansion.int_flags =
                                FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_RX_NAPI_FREE_POS + i,
                                        FXGMAC_FLAG_PER_CHAN_RX_NAPI_FREE_LEN,
                                        FXGMAC_NAPI_DISABLE);
                }

                if (FXGMAC_IS_CHANNEL_WITH_TX_IRQ(i) && tx_napi) {
                    napi_disable(&channel->expansion.napi_tx);
                    netif_napi_del(&channel->expansion.napi_tx);
                    pdata->expansion.int_flags =
                                FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_TX_NAPI_FREE_POS,
                                        FXGMAC_FLAG_TX_NAPI_FREE_LEN,
                                        FXGMAC_NAPI_DISABLE);
                }
                if(netif_msg_drv(pdata)) DPRINTK("napi_disable, msix ch%d napi disabled done,del=%d\n", i, del);
            }

#ifndef FXGMAC_MISC_NOT_ENABLED
            if (misc_napi) {
                napi_disable(&pdata->expansion.napi_misc);
                netif_napi_del(&pdata->expansion.napi_misc);
                pdata->expansion.int_flags =
                            FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MISC_NAPI_FREE_POS,
                                    FXGMAC_FLAG_MISC_NAPI_FREE_LEN,
                                    FXGMAC_NAPI_DISABLE);
            }
#endif
        }
    } else {
        i = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_LEGACY_NAPI_FREE_POS,
                                    FXGMAC_FLAG_LEGACY_NAPI_FREE_LEN);
        if (i) {
            napi_disable(&pdata->expansion.napi);

            if (del)
                netif_napi_del(&pdata->expansion.napi);
            pdata->expansion.int_flags = FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                                FXGMAC_FLAG_LEGACY_NAPI_FREE_POS,
                                                FXGMAC_FLAG_LEGACY_NAPI_FREE_LEN,
                                                FXGMAC_NAPI_DISABLE);

        }
    }
}

static int fxgmac_request_irqs(struct fxgmac_pdata *pdata)
{
    struct net_device *netdev = pdata->netdev;
    struct fxgmac_channel *channel;
    unsigned int i;
    int ret;
    u32 msi, msix, need_free;
    u32 tx = 0, rx = 0;
#ifndef FXGMAC_MISC_NOT_ENABLED
    u32 misc = 0;
#endif

    msi = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MSI_POS,
                                    FXGMAC_FLAG_MSI_LEN);

    msix = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MSIX_POS,
                                    FXGMAC_FLAG_MSIX_LEN);

    need_free = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_LEGACY_IRQ_FREE_POS,
                                    FXGMAC_FLAG_LEGACY_IRQ_FREE_LEN);

    if(!msix) {
        if (!need_free) {
            ret = devm_request_irq(pdata->dev, pdata->dev_irq, fxgmac_isr,
                             msi ? 0 : IRQF_SHARED,
                             netdev->name, pdata);
            if (ret) {
                netdev_alert(netdev, "error requesting irq %d, ret = %d\n", pdata->dev_irq, ret);
                return ret;
            }

            pdata->expansion.int_flags = FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                                FXGMAC_FLAG_LEGACY_IRQ_FREE_POS,
                                                FXGMAC_FLAG_LEGACY_IRQ_FREE_LEN,
                                                FXGMAC_IRQ_ENABLE);
        }
    }

    if (!pdata->per_channel_irq) {
        return 0;
    }

    tx = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_TX_IRQ_FREE_POS,
                                    FXGMAC_FLAG_TX_IRQ_FREE_LEN);
    rx = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_RX_IRQ_FREE_POS,
                                    FXGMAC_FLAG_RX_IRQ_FREE_LEN);
    channel = pdata->channel_head;
    for (i = 0; i < pdata->channel_count; i++, channel++) {
        snprintf(channel->expansion.dma_irq_name,
             sizeof(channel->expansion.dma_irq_name) - 1,
                             "%s-ch%d-Rx-%u", netdev_name(netdev), i,
             channel->queue_index);
        if(FXGMAC_IS_CHANNEL_WITH_TX_IRQ(i) && !tx) {
            snprintf(channel->expansion.dma_irq_name_tx,
                 sizeof(channel->expansion.dma_irq_name_tx) - 1,
                                 "%s-ch%d-Tx-%u", netdev_name(netdev), i,
                 channel->queue_index);

            ret = devm_request_irq(pdata->dev, channel->expansion.dma_irq_tx,
                             fxgmac_dma_isr, 0,
                             channel->expansion.dma_irq_name_tx, channel);

            if (ret) {
                netdev_alert(netdev, "fxgmac_req_irqs, err with MSIx irq \
                    request for ch %d tx, ret=%d\n", i, ret);
                goto err_irq;
            }

            pdata->expansion.int_flags = FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_TX_IRQ_FREE_POS,
                                        FXGMAC_FLAG_TX_IRQ_FREE_LEN,
                                        FXGMAC_IRQ_ENABLE);

            if(netif_msg_drv(pdata))
                DPRINTK("fxgmac_req_irqs, MSIx irq_tx request ok, ch=%d, irq=%d,%s\n",
                                    i, channel->expansion.dma_irq_tx,
                                        channel->expansion.dma_irq_name_tx);
        }

        if (!FXGMAC_GET_REG_BITS(rx, i, FXGMAC_FLAG_PER_CHAN_RX_IRQ_FREE_LEN)) {
            ret = devm_request_irq(pdata->dev, channel->dma_irq,
                               fxgmac_dma_isr, 0,
                               channel->expansion.dma_irq_name, channel);
            if (ret) {
                netdev_alert(netdev, "error requesting irq %d\n",
                         channel->dma_irq);
                goto err_irq;
            }
            pdata->expansion.int_flags = FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_RX_IRQ_FREE_POS + i,
                                        FXGMAC_FLAG_PER_CHAN_RX_IRQ_FREE_LEN,
                                        FXGMAC_IRQ_ENABLE);
        }
    }

#ifndef FXGMAC_MISC_NOT_ENABLED
    misc = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_POS,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_LEN);
    if (!misc) {
        snprintf(pdata->expansion.misc_irq_name,
             sizeof(pdata->expansion.misc_irq_name) - 1,
             "%s-misc", netdev_name(netdev));
        ret = devm_request_irq(pdata->dev,
                                pdata->expansion.misc_irq,
                                fxgmac_misc_isr,
                                0,
                                pdata->expansion.misc_irq_name,
                                pdata);
        if (ret) {
            netdev_alert(netdev,
                            "error requesting misc irq %d, ret = %d\n",
                            pdata->expansion.misc_irq,
                            ret);
            goto err_irq;
        }
        pdata->expansion.int_flags = FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_POS,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_LEN,
                                    FXGMAC_IRQ_ENABLE);
    }
#endif
    if(netif_msg_drv(pdata))
        DPRINTK("fxgmac_req_irqs, MSIx irq request ok, total=%d,%d~%d\n",
         i, (pdata->channel_head)[0].dma_irq,(pdata->channel_head)[i-1].dma_irq);
    return 0;

err_irq:
    netdev_alert(netdev, "fxgmac_req_irqs, err with MSIx irq request at %d, \
        ret=%d\n", i, ret);

    if (pdata->per_channel_irq) {
        for (i--, channel--; i < pdata->channel_count; i--, channel--) {
            if(FXGMAC_IS_CHANNEL_WITH_TX_IRQ(i) && tx) {
                pdata->expansion.int_flags=
                            FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_TX_IRQ_FREE_POS,
                                        FXGMAC_FLAG_TX_IRQ_FREE_LEN,
                                        FXGMAC_IRQ_DISABLE);
                devm_free_irq(pdata->dev, channel->expansion.dma_irq_tx, channel);
            }

            if (FXGMAC_GET_REG_BITS(rx, i, FXGMAC_FLAG_PER_CHAN_RX_IRQ_FREE_LEN)) {
                pdata->expansion.int_flags=
                            FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_RX_IRQ_FREE_POS + i,
                                    FXGMAC_FLAG_PER_CHAN_RX_IRQ_FREE_LEN,
                                    FXGMAC_IRQ_DISABLE);
                devm_free_irq(pdata->dev, channel->dma_irq, channel);
            }
        }

#ifndef FXGMAC_MISC_NOT_ENABLED
        if (misc) {
            pdata->expansion.int_flags=
                            FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_POS,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_LEN,
                                    FXGMAC_IRQ_DISABLE);
            devm_free_irq(pdata->dev, pdata->expansion.misc_irq, pdata);
        }
#endif
    }
    return ret;
}

static void fxgmac_free_irqs(struct fxgmac_pdata *pdata)
{
    struct fxgmac_channel *channel;
    unsigned int i = 0;
    u32 need_free, msix;
    u32 tx = 0, rx = 0;
#ifndef FXGMAC_MISC_NOT_ENABLED
    u32 misc = 0;
#endif

    msix = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MSIX_POS,
                                    FXGMAC_FLAG_MSIX_LEN);

    need_free = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_LEGACY_IRQ_FREE_POS,
                                    FXGMAC_FLAG_LEGACY_IRQ_FREE_LEN);

    if(!msix) {
        if (need_free) {
            devm_free_irq(pdata->dev, pdata->dev_irq, pdata);
            pdata->expansion.int_flags = FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                            FXGMAC_FLAG_LEGACY_IRQ_FREE_POS,
                                            FXGMAC_FLAG_LEGACY_IRQ_FREE_LEN,
                                            FXGMAC_IRQ_DISABLE);
        }
    }

    if (!pdata->per_channel_irq)
        return;

#ifndef FXGMAC_MISC_NOT_ENABLED
    misc = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_POS,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_LEN);
#endif
    tx = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_TX_IRQ_FREE_POS,
                                    FXGMAC_FLAG_TX_IRQ_FREE_LEN);
    rx = FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_RX_IRQ_FREE_POS,
                                    FXGMAC_FLAG_RX_IRQ_FREE_LEN);

    channel = pdata->channel_head;
    if (channel != NULL) {
        for (i = 0; i < pdata->channel_count; i++, channel++) {
            if(FXGMAC_IS_CHANNEL_WITH_TX_IRQ(i) && tx) {
                pdata->expansion.int_flags=
                                FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                        FXGMAC_FLAG_TX_IRQ_FREE_POS,
                                        FXGMAC_FLAG_TX_IRQ_FREE_LEN,
                                        FXGMAC_IRQ_DISABLE);
                devm_free_irq(pdata->dev, channel->expansion.dma_irq_tx, channel);
                if(netif_msg_drv(pdata)) DPRINTK("fxgmac_free_irqs, MSIx irq_tx clear done, ch=%d\n", i);
            }

            if (FXGMAC_GET_REG_BITS(rx, i, FXGMAC_FLAG_PER_CHAN_RX_IRQ_FREE_LEN)) {
                pdata->expansion.int_flags=
                                FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_RX_IRQ_FREE_POS + i,
                                    FXGMAC_FLAG_PER_CHAN_RX_IRQ_FREE_LEN,
                                    FXGMAC_IRQ_DISABLE);
                devm_free_irq(pdata->dev, channel->dma_irq, channel);
            }
        }

#ifndef FXGMAC_MISC_NOT_ENABLED
        if (misc) {
            pdata->expansion.int_flags=
                                FXGMAC_SET_REG_BITS(pdata->expansion.int_flags,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_POS,
                                    FXGMAC_FLAG_MISC_IRQ_FREE_LEN,
                                    FXGMAC_IRQ_DISABLE);
            devm_free_irq(pdata->dev, pdata->expansion.misc_irq, pdata);
        }
#endif
    }
    if(netif_msg_drv(pdata)) DPRINTK("fxgmac_free_irqs, MSIx rx irq clear done, total=%d\n", i);
}

void fxgmac_free_tx_data(struct fxgmac_pdata *pdata)
{
    struct fxgmac_desc_ops *desc_ops = &pdata->desc_ops;
    struct fxgmac_desc_data *desc_data;
    struct fxgmac_channel *channel;
    struct fxgmac_ring *ring;
    unsigned int i, j;

    channel = pdata->channel_head;
    if (channel != NULL) {
        for (i = 0; i < pdata->channel_count; i++, channel++) {
            ring = channel->tx_ring;
            if (!ring)
                break;

            for (j = 0; j < ring->dma_desc_count; j++) {
                desc_data = FXGMAC_GET_DESC_DATA(ring, j);
                desc_ops->unmap_desc_data(pdata, desc_data);
            }
        }
    }
}

void fxgmac_free_rx_data(struct fxgmac_pdata *pdata)
{
    struct fxgmac_desc_ops *desc_ops = &pdata->desc_ops;
    struct fxgmac_desc_data *desc_data;
    struct fxgmac_channel *channel;
    struct fxgmac_ring *ring;
    unsigned int i, j;

    channel = pdata->channel_head;
    if (channel != NULL) {
        for (i = 0; i < pdata->channel_count; i++, channel++) {
            ring = channel->rx_ring;
            if (!ring)
                break;

            for (j = 0; j < ring->dma_desc_count; j++) {
                desc_data = FXGMAC_GET_DESC_DATA(ring, j);
                desc_ops->unmap_desc_data(pdata, desc_data);
            }
        }
    }
}

/*
 * since kernel does not clear the MSI mask bits and
 * this function clear MSI mask bits when MSI is enabled.
 */
static int fxgmac_disable_pci_msi_config(struct pci_dev *pdev)
{
    u16 pcie_cap_offset = 0;
    u32 pcie_msi_mask_bits = 0;
    int ret = 0;

    pcie_cap_offset = pci_find_capability(pdev, PCI_CAP_ID_MSI);
    if (pcie_cap_offset) {
        ret = pci_read_config_dword(pdev, pcie_cap_offset, &pcie_msi_mask_bits);
        if (ret) {
            DPRINTK(KERN_ERR "read pci config space MSI cap. failed, %d\n", ret);
            ret = -EFAULT;
        }
    }

    pcie_msi_mask_bits = FXGMAC_SET_REG_BITS(pcie_msi_mask_bits,
                                            PCI_CAP_ID_MSI_ENABLE_POS,
                                            PCI_CAP_ID_MSI_ENABLE_LEN,
                                            0);
    ret = pci_write_config_dword(pdev, pcie_cap_offset, pcie_msi_mask_bits);
    if (ret) {
        DPRINTK(KERN_ERR "write pci config space MSI mask failed, %d\n", ret);
        ret = -EFAULT;
    }

    return ret;
}

static int fxgmac_disable_pci_msix_config(struct pci_dev *pdev)
{
    u16 pcie_cap_offset = 0;
    u32 pcie_msi_mask_bits = 0;
    int ret = 0;

    pcie_cap_offset = pci_find_capability(pdev, PCI_CAP_ID_MSIX);
    if (pcie_cap_offset) {
        ret = pci_read_config_dword(pdev, pcie_cap_offset, &pcie_msi_mask_bits);
        if (ret) {
            DPRINTK(KERN_ERR "read pci config space MSIX cap. failed, %d\n", ret);
            ret = -EFAULT;
        }
    }

    pcie_msi_mask_bits = FXGMAC_SET_REG_BITS(pcie_msi_mask_bits,
                                            PCI_CAP_ID_MSIX_ENABLE_POS,
                                            PCI_CAP_ID_MSIX_ENABLE_LEN,
                                            0);
    ret = pci_write_config_dword(pdev, pcie_cap_offset, pcie_msi_mask_bits);
    if (ret) {
        DPRINTK(KERN_ERR "write pci config space MSIX mask failed, %d\n", ret);
        ret = -EFAULT;
    }

    return ret;
}

int fxgmac_start(struct fxgmac_pdata *pdata)
{
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    unsigned int pcie_low_power = PCIE_LP_ASPM_LTR;
    u8 deviceid;
    int ret;

    if(netif_msg_drv(pdata)) DPRINTK("fxgmac start callin here.\n");

    if (pdata->expansion.dev_state != FXGMAC_DEV_OPEN &&
        pdata->expansion.dev_state != FXGMAC_DEV_STOP &&
        pdata->expansion.dev_state != FXGMAC_DEV_RESUME)
        return 0;

    /* must reset software again here, to avoid flushing tx queue error
     * caused by the system only run probe
     * when installing driver on the arm platform.
     */
    hw_ops->exit(pdata);

    if (FXGMAC_GET_REG_BITS(pdata->expansion.int_flags,
                            FXGMAC_FLAG_LEGACY_POS,
                            FXGMAC_FLAG_LEGACY_LEN)) {
        /*
        * we should disable msi and msix here when we use legacy interrupt,for two reasons:
        * 1. Exit will restore msi and msix config regisiter, that may enable them.
        * 2. When the driver that uses the msix interrupt by default is compiled
        *   into the OS, uninstall the driver through rmmod, and then install the
        *   driver that uses the legacy interrupt, at which time the msix enable
        *   will be turned on again by default after waking up from S4 on some platform.
        *   such as UOS platform.
        */
        ret = fxgmac_disable_pci_msi_config(pdata->pdev);
        ret |= fxgmac_disable_pci_msix_config(pdata->pdev);
        if (ret)
            return ret;
    }

    hw_ops->reset_phy(pdata);
    hw_ops->release_phy(pdata);
    cfg_r8(pdata, REG_PCI_REVID, &deviceid);
    hw_ops->pcie_init(pdata,
        pcie_low_power & PCIE_LP_ASPM_LTR,
        pcie_low_power & PCIE_LP_ASPM_L1SS,
        (FXGMAC_REV_03 == deviceid) ? true : (pcie_low_power & PCIE_LP_ASPM_L1),
        pcie_low_power & PCIE_LP_ASPM_L0S);
#ifdef FXGMAC_ASPM_ENABLED
    if (pdata->expansion.aspm_work_active || pdata->expansion.aspm_en) {
        printk("cancle aspm work.\n");
        fxgmac_cancel_aspm_config_work(pdata);
        pdata->expansion.aspm_en = false;
        pdata->expansion.aspm_work_active = false;
    }
#endif
    hw_ops->config_power_up(pdata);
    hw_ops->dismiss_all_int(pdata);

    ret = hw_ops->init(pdata);
    if (ret) {
        DPRINTK("fxgmac hw init error.\n");
        return ret;
    }
    fxgmac_napi_enable(pdata, 1);

    ret = fxgmac_request_irqs(pdata);
    if (ret)
        goto err_napi;

    hw_ops->enable_mgm_interrupt(pdata);
#if FXGMAC_INT_MODERATION_ENABLED
    hw_ops->set_interrupt_moderation(pdata);
#endif

    if (pdata->per_channel_irq)
        hw_ops->enable_msix_rxtxinterrupt(pdata);

#ifdef FXGMAC_ESD_CHECK_ENABLED
    fxgmac_schedule_esd_work(pdata);
#endif

#ifdef FXGMAC_EPHY_LOOPBACK_DETECT_ENABLED
    pdata->expansion.lb_cable_flag = 0;
    fxgmac_schedule_loopback_work(pdata);
#endif

    if (pdata->expansion.recover_phy_state)
        fxgmac_set_phy_link_ksettings(pdata);

    hw_ops->led_under_active(pdata);
    pdata->expansion.dev_state = FXGMAC_DEV_START;

    if (!pdata->expansion.recover_from_aspm) {
#ifdef FXGMAC_ASPM_ENABLED
        printk("start aspm work and phy timer.\n");
        fxgmac_schedule_aspm_config_work(pdata);
        pdata->expansion.aspm_work_active = true;
#endif
        fxgmac_phy_timer_init(pdata);
    }
    pdata->expansion.recover_from_aspm = false;

    return 0;

err_napi:
    if (!pdata->expansion.recover_from_aspm)
        fxgmac_phy_timer_destroy(pdata);

    fxgmac_napi_disable(pdata, 1);
    hw_ops->exit(pdata);
    dev_err(pdata->dev, "fxgmac start callout with irq err.\n");
    return ret;
}

void fxgmac_stop(struct fxgmac_pdata *pdata)
{
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    struct net_device *netdev = pdata->netdev;
    struct fxgmac_channel *channel;
    struct netdev_queue *txq;
    unsigned int i;

    if (pdata->expansion.dev_state != FXGMAC_DEV_START)
        return;

    pdata->expansion.dev_state = FXGMAC_DEV_STOP;
    netif_carrier_off(netdev);
    netif_tx_stop_all_queues(netdev);

    if (!pdata->expansion.recover_from_aspm)
        fxgmac_phy_timer_destroy(pdata);

#ifdef FXGMAC_EPHY_LOOPBACK_DETECT_ENABLED
    fxgmac_cancel_loopback_work(pdata);
#endif

#ifdef FXGMAC_ESD_CHECK_ENABLED
    fxgmac_cancel_esd_work(pdata);
#endif

    hw_ops->disable_tx(pdata);
    hw_ops->disable_rx(pdata);

    if (pdata->per_channel_irq) {
        hw_ops->disable_msix_interrupt(pdata);
    }
    else {
        hw_ops->disable_mgm_interrupt(pdata);
    }

    fxgmac_free_irqs(pdata);
    fxgmac_napi_disable(pdata, 1);

    channel = pdata->channel_head;
    if (channel != NULL) {
        for (i = 0; i < pdata->channel_count; i++, channel++) {
            if (!channel->tx_ring)
                continue;

            txq = netdev_get_tx_queue(netdev, channel->queue_index);
            netdev_tx_reset_queue(txq);
        }
    }
}

void fxgmac_restart_dev(struct fxgmac_pdata *pdata)
{
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    int ret;

    /* If not running, "restart" will happen on open */
    if (!netif_running(pdata->netdev) &&
        pdata->expansion.dev_state != FXGMAC_DEV_START)
        return;

    fxgmac_lock(pdata);
    fxgmac_stop(pdata);
    hw_ops->led_under_shutdown(pdata);

    fxgmac_free_tx_data(pdata);
    fxgmac_free_rx_data(pdata);

    ret = fxgmac_start(pdata);
    if (ret) {
        DPRINTK("fxgmac_restart_dev: fxgmac_start failed.\n");
    }

    fxgmac_unlock(pdata);
}

static void fxgmac_restart(struct work_struct *work)
{
    struct fxgmac_pdata *pdata = container_of(work,
                            struct fxgmac_pdata,
                            expansion.restart_work);

    rtnl_lock();

    fxgmac_restart_dev(pdata);

    rtnl_unlock();
}

void fxgmac_net_powerup(struct fxgmac_pdata *pdata)
{
    int ret;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    if(netif_msg_drv(pdata)) DPRINTK("fxgmac_net_powerup callin\n");

    /* signal that we are up now */
    pdata->expansion.powerstate = 0; //clear all bits as normal now
    if (__test_and_set_bit(FXGMAC_POWER_STATE_UP, &pdata->expansion.powerstate)) {
        return; /* do nothing if already up */
    }

    ret = fxgmac_start(pdata);
    if (ret) {
        DPRINTK("fxgmac_net_powerup: fxgmac_start error\n");
        return;
    }

    // must call it after fxgmac_start,because it will be enable in fxgmac_start
    hw_ops->disable_arp_offload(pdata);

    if(netif_msg_drv(pdata)) {
        DPRINTK("fxgmac_net_powerup callout, powerstate=%ld.\n", pdata->expansion.powerstate);
    }
}

void fxgmac_net_powerdown(struct fxgmac_pdata *pdata, unsigned int wol)
{
    struct net_device *netdev = pdata->netdev;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    u32 val;

    if(netif_msg_drv(pdata)) DPRINTK("fxgmac_net_powerdown callin here.\n");

    /* signal that we are down to the interrupt handler */
    if (__test_and_set_bit(FXGMAC_POWER_STATE_DOWN, &pdata->expansion.powerstate))
        return; /* do nothing if already down */

    if(netif_msg_drv(pdata)) DPRINTK("fxgmac_net_powerdown continue with down process.\n");
    /* phy polling timer should detect the state of fxgmac and stop link status polling accordingly */

    __clear_bit(FXGMAC_POWER_STATE_UP, &pdata->expansion.powerstate);

    /* Shut off incoming Tx traffic */
    netif_tx_stop_all_queues(netdev);

    /* call carrier off first to avoid false dev_watchdog timeouts */
    netif_carrier_off(netdev);
    netif_tx_disable(netdev);

    /* Disable Rx */
    hw_ops->disable_rx(pdata);

    /* synchronize_rcu() needed for pending XDP buffers to drain */
    //if (adapter->xdp_ring[0]) 20210709
        synchronize_rcu();

    fxgmac_stop(pdata); //some works are redundent in this call

#ifdef FXGMAC_EPHY_LOOPBACK_DETECT_ENABLED
    val = pdata->expansion.lb_cable_flag;
#else
    val = 0;
#endif
    // must call it after software reset
    hw_ops->pre_power_down(pdata, val);

    if(!test_bit(FXGMAC_POWER_STATE_DOWN, &pdata->expansion.powerstate)) {
        netdev_err(pdata->netdev,
            "fxgmac powerstate is %lu when config power to down.\n", pdata->expansion.powerstate);
    }

    /* set mac to lowpower mode and enable wol accordingly */
    hw_ops->config_power_down(pdata, wol);

#if 1
    //handle vfs if it is envolved

    //similar work as in restart() for that, we do need a resume laterly
    fxgmac_free_tx_data(pdata);
    fxgmac_free_rx_data(pdata);
#endif
    if(netif_msg_drv(pdata)) DPRINTK("fxgmac_net_powerdown callout, powerstate=%ld.\n", pdata->expansion.powerstate);
}

static int fxgmac_open(struct net_device *netdev)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_desc_ops *desc_ops;
    int ret;

    if(netif_msg_drv(pdata)) DPRINTK("fxgmac_open callin\n");

    fxgmac_lock(pdata);
    pdata->expansion.dev_state = FXGMAC_DEV_OPEN;
    desc_ops = &pdata->desc_ops;

    /* Calculate the Rx buffer size before allocating rings */
    //DPRINTK("fxgmac_open, b4 calc rx buf size, mtu,min,max=%d,%d,%d.\n", netdev->mtu, netdev->min_mtu, netdev->max_mtu);
    ret = fxgmac_calc_rx_buf_size(netdev, netdev->mtu);
    if (ret < 0)
        goto unlock;
    pdata->rx_buf_size = ret;

    /* Allocate the channels and rings */
    ret = desc_ops->alloc_channels_and_rings(pdata);
    if (ret)
        goto unlock;

    INIT_WORK(&pdata->expansion.restart_work, fxgmac_restart);

#ifdef FXGMAC_ESD_CHECK_ENABLED
    INIT_DELAYED_WORK(&pdata->expansion.esd_work, fxgmac_esd_work);
#endif

#ifdef FXGMAC_EPHY_LOOPBACK_DETECT_ENABLED
    INIT_DELAYED_WORK(&pdata->expansion.loopback_work, fxgmac_loopback_work);
#endif

#ifdef FXGMAC_ASPM_ENABLED
    INIT_DELAYED_WORK(&pdata->expansion.aspm_config_work, fxgmac_aspm_config_work);
#endif

    ret = fxgmac_start(pdata);
    if (ret)
        goto err_channels_and_rings;

    if(netif_msg_drv(pdata)) DPRINTK("fxgmac_open callout\n");

    fxgmac_unlock(pdata);

    return 0;

err_channels_and_rings:
    desc_ops->free_channels_and_rings(pdata);
    DPRINTK("fxgmac_open callout with channel alloc err\n");
unlock:
    fxgmac_unlock(pdata);
    return ret;
}

static int fxgmac_close(struct net_device *netdev)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_desc_ops *desc_ops = &pdata->desc_ops;

    if(netif_msg_drv(pdata)) DPRINTK("fxgmac_close callin\n");

    fxgmac_lock(pdata);
    /* Stop the device */
    fxgmac_stop(pdata);
    pdata->expansion.dev_state = FXGMAC_DEV_CLOSE;

    /* Free the channels and rings */
    desc_ops->free_channels_and_rings(pdata);
    pdata->hw_ops.reset_phy(pdata);
    fxgmac_phy_update_link(netdev);

#ifdef FXGMAC_ASPM_ENABLED
    fxgmac_cancel_aspm_config_work(pdata);
    pdata->expansion.aspm_work_active = false;
    pdata->expansion.recover_from_aspm = false;
    pdata->expansion.aspm_en = false;
    pdata->hw_ops.pcie_init(pdata, true, true, true, false);
#endif

    if(netif_msg_drv(pdata)) DPRINTK("fxgmac_close callout\n");

    fxgmac_unlock(pdata);
    return 0;
}


static void fxgmac_dump_state(struct fxgmac_pdata *pdata)
{
	struct fxgmac_channel *channel = pdata->channel_head;
	struct fxgmac_stats *pstats = &pdata->stats;
	struct fxgmac_ring *ring;
	u32 i;

	ring = &channel->tx_ring[0];
	DPRINTK( "Tx descriptor info:\n");
	DPRINTK( "Tx cur = 0x%x\n", ring->cur);
	DPRINTK( "Tx dirty = 0x%x\n", ring->dirty);
	DPRINTK( "Tx dma_desc_head = %pad\n", &ring->dma_desc_head);
	DPRINTK( "Tx desc_data_head = %pad\n", &ring->desc_data_head);

	for (i = 0; i < pdata->channel_count; i++, channel++) {
		ring = &channel->rx_ring[0];
		DPRINTK( "Rx[%d] descriptor info:\n", i);
		DPRINTK( "Rx cur = 0x%x\n", ring->cur);
		DPRINTK( "Rx dirty = 0x%x\n", ring->dirty);
		DPRINTK( "Rx dma_desc_head = %pad\n",
		       &ring->dma_desc_head);
		DPRINTK( "Rx desc_data_head = %pad\n",
		       &ring->desc_data_head);
	}

	DPRINTK( "Device Registers:\n");
	DPRINTK( "MAC_ISR = %08x\n", readreg(pdata->pAdapter, pdata->mac_regs + MAC_ISR));
	DPRINTK( "MAC_IER = %08x\n", readreg(pdata->pAdapter, pdata->mac_regs + MAC_IER));
	DPRINTK( "MMC_RISR = %08x\n", readreg(pdata->pAdapter, pdata->mac_regs + MMC_RISR));
	DPRINTK( "MMC_RIER = %08x\n", readreg(pdata->pAdapter, pdata->mac_regs + MMC_RIER));
	DPRINTK( "MMC_TISR = %08x\n", readreg(pdata->pAdapter, pdata->mac_regs + MMC_TISR));
	DPRINTK( "MMC_TIER = %08x\n", readreg(pdata->pAdapter, pdata->mac_regs + MMC_TIER));

	DPRINTK( "EPHY_CTRL = %04x\n", readreg(pdata->pAdapter, pdata->base_mem + MGMT_EPHY_CTRL));
	DPRINTK( "MGMT_INT_CTRL0 = %04x\n",
	       readreg(pdata->pAdapter, pdata->base_mem + MGMT_INT_CTRL0));
	DPRINTK( "LPW_CTRL = %04x\n", readreg(pdata->pAdapter, pdata->base_mem + LPW_CTRL));
	DPRINTK( "MSIX_TBL_MASK = %04x\n", readreg(pdata->pAdapter, pdata->base_mem + MSIX_TBL_BASE_ADDR + MSIX_TBL_MASK_OFFSET));

	DPRINTK( "Dump nonstick regs:\n");
	for ( i = REG_PCIE_TRIGGER; i < MSI_PBA_REG; i += 4)
		DPRINTK( "[%d] = %04x\n", i / 4, readreg(pdata->pAdapter, pdata->base_mem + i));

	pdata->hw_ops.read_mmc_stats(pdata);

	DPRINTK( "Dump TX counters:\n");
	DPRINTK( "tx_packets %lld\n", pstats->txframecount_gb);
	DPRINTK( "tx_errors %lld\n",
	       pstats->txframecount_gb - pstats->txframecount_g);
	DPRINTK( "tx_multicastframes_errors %lld\n",
	       pstats->txmulticastframes_gb - pstats->txmulticastframes_g);
	DPRINTK( "tx_broadcastframes_errors %lld\n",
	       pstats->txbroadcastframes_gb - pstats->txbroadcastframes_g);

	DPRINTK( "txunderflowerror %lld\n", pstats->txunderflowerror);
	DPRINTK( "txdeferredframes %lld\n",
	       pstats->txdeferredframes);
	DPRINTK( "txlatecollisionframes %lld\n",
	       pstats->txlatecollisionframes);
	DPRINTK( "txexcessivecollisionframes %lld\n",
	       pstats->txexcessivecollisionframes);
	DPRINTK( "txcarriererrorframes %lld\n",
	       pstats->txcarriererrorframes);
	DPRINTK( "txexcessivedeferralerror %lld\n",
	       pstats->txexcessivedeferralerror);

	DPRINTK( "txsinglecollision_g %lld\n",
	       pstats->txsinglecollision_g);
	DPRINTK( "txmultiplecollision_g %lld\n",
	       pstats->txmultiplecollision_g);
	DPRINTK( "txoversize_g %lld\n", pstats->txoversize_g);

	DPRINTK( "Dump RX counters:\n");
	DPRINTK( "rx_packets %lld\n", pstats->rxframecount_gb);
	DPRINTK( "rx_errors %lld\n",
	       pstats->rxframecount_gb - pstats->rxbroadcastframes_g -
	       pstats->rxmulticastframes_g - pstats->rxunicastframes_g);

	DPRINTK( "rx_crc_errors %lld\n", pstats->rxcrcerror);
	DPRINTK( "rxalignerror %lld\n", pstats->rxalignerror);
	DPRINTK( "rxrunterror %lld\n", pstats->rxrunterror);
	DPRINTK( "rxjabbererror %lld\n", pstats->rxjabbererror);
	DPRINTK( "rx_length_errors %lld\n", pstats->rxlengtherror);
	DPRINTK( "rxoutofrangetype %lld\n", pstats->rxoutofrangetype);
	DPRINTK( "rx_fifo_errors %lld\n", pstats->rxfifooverflow);
	DPRINTK( "rxwatchdogerror %lld\n", pstats->rxwatchdogerror);
	DPRINTK( "rxreceiveerrorframe %lld\n",
	       pstats->rxreceiveerrorframe);

	DPRINTK( "rxbroadcastframes_g %lld\n",
	       pstats->rxbroadcastframes_g);
	DPRINTK( "rxmulticastframes_g %lld\n",
	       pstats->rxmulticastframes_g);
	DPRINTK( "rxundersize_g %lld\n", pstats->rxundersize_g);
	DPRINTK( "rxoversize_g %lld\n", pstats->rxoversize_g);
	DPRINTK( "rxunicastframes_g %lld\n", pstats->rxunicastframes_g);
	DPRINTK( "rxcontrolframe_g %lld\n", pstats->rxcontrolframe_g);

	DPRINTK( "Dump Extra counters:\n");
	DPRINTK( "tx_tso_packets %lld\n", pstats->tx_tso_packets);
	DPRINTK( "rx_split_header_packets %lld\n",
	       pstats->rx_split_header_packets);
	DPRINTK( "tx_process_stopped %lld\n", pstats->tx_process_stopped);
	DPRINTK( "rx_process_stopped %lld\n", pstats->rx_process_stopped);
	DPRINTK( "tx_buffer_unavailable %lld\n",
	       pstats->tx_buffer_unavailable);
	DPRINTK( "rx_buffer_unavailable %lld\n",
	       pstats->rx_buffer_unavailable);
	DPRINTK( "fatal_bus_error %lld\n", pstats->fatal_bus_error);
	DPRINTK( "napi_poll_isr %lld\n", pstats->napi_poll_isr);
	DPRINTK( "napi_poll_txtimer %lld\n", pstats->napi_poll_txtimer);
	DPRINTK( "ephy_poll_timer_cnt %lld\n",
	       pstats->ephy_poll_timer_cnt);
	DPRINTK( "mgmt_int_isr %lld\n", pstats->mgmt_int_isr);
}

#if ((LINUX_VERSION_CODE > KERNEL_VERSION(4,0,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0) || (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7,8))))
static void fxgmac_tx_timeout(struct net_device *netdev)
#else
static void fxgmac_tx_timeout(struct net_device *netdev, unsigned int unused)
#endif
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    netdev_warn(netdev, "tx timeout, device restarting\n");
    fxgmac_dump_state(pdata);
#if FXGMAC_TX_HANG_TIMER_ENABLED
    if(!pdata->tx_hang_restart_queuing)
        schedule_work(&pdata->expansion.restart_work);
#else
    schedule_work(&pdata->expansion.restart_work);
#endif
}

static netdev_tx_t fxgmac_xmit(struct sk_buff *skb, struct net_device *netdev)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_pkt_info *tx_pkt_info;
    struct fxgmac_desc_ops *desc_ops;
    struct fxgmac_channel *channel;
    struct netdev_queue *txq;
    struct fxgmac_ring *ring;
    int ret;

    desc_ops = &pdata->desc_ops;

    //yzhang disabled
    if(netif_msg_tx_done(pdata)) DPRINTK("xmit callin, skb->len=%d,q=%d\n", skb->len, skb->queue_mapping);

    channel = pdata->channel_head + skb->queue_mapping;
    txq = netdev_get_tx_queue(netdev, channel->queue_index);
    ring = channel->tx_ring;
    tx_pkt_info = &ring->pkt_info;

    if (skb->len == 0) {
        netif_err(pdata, tx_err, netdev, "empty skb received from stack\n");
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK;
    }

    /* Prepare preliminary packet info for TX */
    memset(tx_pkt_info, 0, sizeof(*tx_pkt_info));
    fxgmac_prep_tx_pkt(pdata, ring, skb, tx_pkt_info);

    /* Check that there are enough descriptors available */
    ret = fxgmac_maybe_stop_tx_queue(channel, ring,
                         tx_pkt_info->desc_count);
    if (ret)
    {
        return ret;
    }

    ret = fxgmac_prep_tso(pdata, skb, tx_pkt_info);
    if (ret) {
        netif_err(pdata, tx_err, netdev, "error processing TSO packet\n");
        dev_kfree_skb_any(skb);
        return NETDEV_TX_OK;
    }
    fxgmac_prep_vlan(skb, tx_pkt_info);

    if (!desc_ops->map_tx_skb(channel, skb)) {
        dev_kfree_skb_any(skb);
        netif_err(pdata, tx_err, netdev, "xmit, map tx skb err\n");
        return NETDEV_TX_OK;
    }

    /* Report on the actual number of bytes (to be) sent */
    netdev_tx_sent_queue(txq, tx_pkt_info->tx_bytes);
    if(netif_msg_tx_done(pdata)) DPRINTK("xmit,before hw_xmit, byte len=%d\n", tx_pkt_info->tx_bytes);

    /* Configure required descriptor fields for transmission */
    fxgmac_dev_xmit(channel);

    if (netif_msg_pktdata(pdata))
        fxgmac_dbg_pkt(netdev, skb, true);

    /* Stop the queue in advance if there may not be enough descriptors */
    fxgmac_maybe_stop_tx_queue(channel, ring, FXGMAC_TX_MAX_DESC_NR);

    return NETDEV_TX_OK;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0) || (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7,8)))
static void fxgmac_get_stats64(struct net_device *netdev,
                    struct rtnl_link_stats64 *s)
#else
static struct rtnl_link_stats64 * fxgmac_get_stats64(struct net_device *netdev,
                    struct rtnl_link_stats64 *s)
#endif
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_stats *pstats = &pdata->stats;

#if FXGMAC_PM_FEATURE_ENABLED
    /* 20210709 for net power down */
    if(!test_bit(FXGMAC_POWER_STATE_DOWN, &pdata->expansion.powerstate))
#endif
    {
        //DPRINTK("get_stats64, ndo op, callin\n");
        pdata->hw_ops.read_mmc_stats(pdata);

        s->rx_packets = pstats->rxframecount_gb;
        s->rx_bytes = pstats->rxoctetcount_gb;
        s->rx_errors = pstats->rxframecount_gb -
               pstats->rxbroadcastframes_g -
               pstats->rxmulticastframes_g -
               pstats->rxunicastframes_g;
        s->multicast = pstats->rxmulticastframes_g;
        s->rx_length_errors = pstats->rxlengtherror;
        s->rx_crc_errors = pstats->rxcrcerror;
        s->rx_fifo_errors = pstats->rxfifooverflow;

        s->tx_packets = pstats->txframecount_gb;
        s->tx_bytes = pstats->txoctetcount_gb;
        s->tx_errors = pstats->txframecount_gb - pstats->txframecount_g;
        s->tx_dropped = netdev->stats.tx_dropped;
    }


#if (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7,8))
    return;
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0))
    return s;
#else
    return;
#endif
}

static int fxgmac_set_mac_address(struct net_device *netdev, void *addr)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    struct sockaddr *saddr = addr;

    if (!is_valid_ether_addr(saddr->sa_data))
        return -EADDRNOTAVAIL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0))
    eth_hw_addr_set(netdev, saddr->sa_data);
#else
    memcpy(netdev->dev_addr, saddr->sa_data, netdev->addr_len);
#endif
    memcpy(pdata->mac_addr, saddr->sa_data, netdev->addr_len);

    hw_ops->set_mac_address(pdata, saddr->sa_data);
    hw_ops->set_mac_hash(pdata);

    DPRINTK("fxgmac,set mac addr to %02x:%02x:%02x:%02x:%02x:%02x\n",netdev->dev_addr[0], netdev->dev_addr[1], netdev->dev_addr[2],
            netdev->dev_addr[3], netdev->dev_addr[4], netdev->dev_addr[5]);
    return 0;
}

/*
 * cmd = [0x89F0, 0x89FF]
 * When using it, we must pay attention to the thread synchronization
 * of this interface. Because it's an external call that isn't
 * initiated by the OS.
 */
static int fxgmac_ioctl(struct net_device *netdev,
            struct ifreq *ifr, int cmd)
{
    struct file f;
    int ret = FXGMAC_SUCCESS;
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    if (!netif_running(netdev) ||
        pdata->expansion.dev_state != FXGMAC_DEV_START)
        return -ENODEV;

    f.private_data = pdata;

    switch (cmd) {
        case FXGMAC_DEV_CMD:
            ret = fxgmac_netdev_ops_ioctl(&f, FXGMAC_IOCTL_DFS_COMMAND, (unsigned long)(ifr->ifr_data));
            break;
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0) )
static int fxgmac_siocdevprivate(struct net_device *dev,
                                            struct ifreq *ifr,
                                            void __user *data,
                                            int cmd)
{
    return fxgmac_ioctl(dev, ifr, cmd);
}
#endif

static int fxgmac_change_mtu(struct net_device *netdev, int mtu)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    int ret, max_mtu;
#ifdef FXGMAC_DEBUG
    int old_mtu = netdev->mtu;
#endif

    if (!netif_running(netdev)) {
        DPRINTK("Can not set mtu because NIC is not open.\n");
        return -EPERM;
    }

    /* On the Linux platform, the MTU size does not include the length
     * of the MAC address and the length of the Type, but FXGMAC_JUMBO_PACKET_MTU include them.
     */
    max_mtu = FXGMAC_JUMBO_PACKET_MTU - ETH_HLEN;
    if (mtu > max_mtu) {
        netdev_alert(netdev, "MTU exceeds maximum supported value\n");
        return -EINVAL;
    }

    fxgmac_lock(pdata);
    fxgmac_stop(pdata);
    fxgmac_free_tx_data(pdata);

    /*
     * We must unmap rx desc's dma before we change rx_buf_size.
     * Becaues the size of the unmapped DMA is set according to rx_buf_size
     */
    fxgmac_free_rx_data(pdata);

    ret = fxgmac_calc_rx_buf_size(netdev, mtu);
    if (ret < 0) {
        fxgmac_unlock(pdata);
        return ret;
    }
    pdata->rx_buf_size = ret;

    pdata->jumbo = mtu > ETH_DATA_LEN ? 1 : 0;
    netdev->mtu = mtu;

    fxgmac_start(pdata);
    netdev_update_features(netdev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0))
    DPRINTK("fxgmac,set MTU from %d to %d. min, max=(%d,%d)\n",old_mtu,
        netdev->mtu, netdev->min_mtu, netdev->max_mtu);
#else
    DPRINTK("fxgmac,set MTU from %d to %d.\n",old_mtu, netdev->mtu);
#endif
    fxgmac_unlock(pdata);

    return 0;
}

static int fxgmac_vlan_rx_add_vid(struct net_device *netdev,
                 __be16 proto,
                 u16 vid)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    set_bit(vid, pdata->active_vlans);
#if FXGMAC_FILTER_SINGLE_VLAN_ENABLED
    pdata->vlan = vid;
    hw_ops->enable_rx_vlan_filtering(pdata);
#else
    hw_ops->update_vlan_hash_table(pdata);
#endif
    DPRINTK("fxgmac,add rx vlan %d\n", vid);
    return 0;
}

static int fxgmac_vlan_rx_kill_vid(struct net_device *netdev,
                 __be16 proto,
                 u16 vid)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    clear_bit(vid, pdata->active_vlans);
#if FXGMAC_FILTER_SINGLE_VLAN_ENABLED
    pdata->vlan = 0;
    hw_ops->disable_rx_vlan_filtering(pdata);
#else
    hw_ops->update_vlan_hash_table(pdata);
#endif

    DPRINTK("fxgmac,del rx vlan %d\n", vid);
    return 0;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void fxgmac_poll_controller(struct net_device *netdev)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_channel *channel;
    unsigned int i;

    if (pdata->per_channel_irq) {
        channel = pdata->channel_head;
        for (i = 0; i < pdata->channel_count; i++, channel++)
            fxgmac_dma_isr(channel->dma_irq, channel);
    } else {
        disable_irq(pdata->dev_irq);
        fxgmac_isr(pdata->dev_irq, pdata);
        enable_irq(pdata->dev_irq);
    }
}
#endif /* CONFIG_NET_POLL_CONTROLLER */

static netdev_features_t fxgmac_fix_features(struct net_device *netdev,
                netdev_features_t features)
{
    u32 fifo_size;
    struct fxgmac_pdata* pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    fifo_size = hw_ops->calculate_max_checksum_size(pdata);

    if (netdev->mtu > fifo_size) {
        features &= ~NETIF_F_IP_CSUM;
        features &= ~NETIF_F_IPV6_CSUM;
    }

    return features;
}

static int fxgmac_set_features(struct net_device *netdev,
                 netdev_features_t features)
{
    netdev_features_t rxhash, rxcsum, rxvlan, rxvlan_filter, tso;
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    int ret = 0;

    rxhash = pdata->expansion.netdev_features & NETIF_F_RXHASH;
    rxcsum = pdata->expansion.netdev_features & NETIF_F_RXCSUM;
    rxvlan = pdata->expansion.netdev_features & NETIF_F_HW_VLAN_CTAG_RX;
    rxvlan_filter = pdata->expansion.netdev_features & NETIF_F_HW_VLAN_CTAG_FILTER;
    tso = pdata->expansion.netdev_features & (NETIF_F_TSO | NETIF_F_TSO6);

    if ((features & (NETIF_F_TSO | NETIF_F_TSO6)) && !tso) {
        DPRINTK("enable tso.\n");
        pdata->hw_feat.tso = 1;
        hw_ops->config_tso(pdata);
    } else if (!(features & (NETIF_F_TSO | NETIF_F_TSO6)) && tso) {
        DPRINTK("disable tso.\n");
        pdata->hw_feat.tso = 0;
        hw_ops->config_tso(pdata);
    }

    if ((features & NETIF_F_RXHASH) && !rxhash)
        ret = hw_ops->enable_rss(pdata);
    else if (!(features & NETIF_F_RXHASH) && rxhash)
        ret = hw_ops->disable_rss(pdata);
    if (ret)
        return ret;

    if ((features & NETIF_F_RXCSUM) && !rxcsum)
        hw_ops->enable_rx_csum(pdata);
    else if (!(features & NETIF_F_RXCSUM) && rxcsum)
        hw_ops->disable_rx_csum(pdata);

    if ((features & NETIF_F_HW_VLAN_CTAG_RX) && !rxvlan)
        hw_ops->enable_rx_vlan_stripping(pdata);
    else if (!(features & NETIF_F_HW_VLAN_CTAG_RX) && rxvlan)
        hw_ops->disable_rx_vlan_stripping(pdata);

    if ((features & NETIF_F_HW_VLAN_CTAG_FILTER) && !rxvlan_filter)
        hw_ops->enable_rx_vlan_filtering(pdata);
    else if (!(features & NETIF_F_HW_VLAN_CTAG_FILTER) && rxvlan_filter)
        hw_ops->disable_rx_vlan_filtering(pdata);

    pdata->expansion.netdev_features = features;

    DPRINTK("fxgmac,set features done,%llx\n", (u64)features);
    return 0;
}

static void fxgmac_set_rx_mode(struct net_device *netdev)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    hw_ops->config_rx_mode(pdata);
}

static const struct net_device_ops fxgmac_netdev_ops = {
    .ndo_open               = fxgmac_open,
    .ndo_stop               = fxgmac_close,
    .ndo_start_xmit         = fxgmac_xmit,
    .ndo_tx_timeout         = fxgmac_tx_timeout,
    .ndo_get_stats64        = fxgmac_get_stats64,

#if(RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(9,0))
    .ndo_change_mtu         = fxgmac_change_mtu,
#elif (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7,8))
    .ndo_change_mtu_rh74    = fxgmac_change_mtu,
#else
    .ndo_change_mtu         = fxgmac_change_mtu,
#endif

    .ndo_set_mac_address    = fxgmac_set_mac_address,
    .ndo_validate_addr      = eth_validate_addr,
    .ndo_do_ioctl           = fxgmac_ioctl,
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0) )
    .ndo_siocdevprivate     = fxgmac_siocdevprivate,
#endif
    .ndo_vlan_rx_add_vid    = fxgmac_vlan_rx_add_vid,
    .ndo_vlan_rx_kill_vid   = fxgmac_vlan_rx_kill_vid,
#ifdef CONFIG_NET_POLL_CONTROLLER
    .ndo_poll_controller    = fxgmac_poll_controller,
#endif
    .ndo_set_features       = fxgmac_set_features,
    .ndo_fix_features       = fxgmac_fix_features,
    .ndo_set_rx_mode        = fxgmac_set_rx_mode,
};

const struct net_device_ops *fxgmac_get_netdev_ops(void)
{
    return &fxgmac_netdev_ops;
}

static void fxgmac_rx_refresh(struct fxgmac_channel *channel)
{
    struct fxgmac_pdata *pdata = channel->pdata;
    struct fxgmac_ring *ring = channel->rx_ring;
    struct fxgmac_desc_data *desc_data;
    struct fxgmac_desc_ops *desc_ops = &pdata->desc_ops;
#if 0
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
#endif

    while (ring->dirty != ring->cur) {
        desc_data = FXGMAC_GET_DESC_DATA(ring, ring->dirty);
#ifdef FXGMAC_ZERO_COPY
        /* Reset desc_data values */
        desc_ops->unmap_desc_data(pdata, desc_data);

        if (desc_ops->map_rx_buffer(pdata, ring, desc_data))
            break;
#endif
        desc_ops->rx_desc_reset(pdata, desc_data, ring->dirty);
        ring->dirty = FXGMAC_GET_ENTRY(ring->dirty, ring->dma_desc_count);
    }

    /* Make sure everything is written before the register write */
    wmb();

    /* Update the Rx Tail Pointer Register with address of
     * the last cleaned entry
     */
    desc_data = FXGMAC_GET_DESC_DATA(ring, (ring->dirty - 1) & (ring->dma_desc_count - 1));
    writel(lower_32_bits(desc_data->dma_desc_addr), FXGMAC_DMA_REG(channel, DMA_CH_RDTR_LO));
}

static struct sk_buff *fxgmac_create_skb(struct fxgmac_pdata *pdata,
                     struct napi_struct *napi,
                     struct fxgmac_desc_data *desc_data,
                     unsigned int len)
{
    struct sk_buff *skb;
#ifndef FXGMAC_NOT_USE_PAGE_MAPPING
    unsigned int copy_len;
    u8 *packet;
#endif

#if (RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7,0)) && (RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(7,8))

/* just add fot compile
 * centos 7.0 not support napi_alloc_skb, and porting it is hard
 */
#define napi_alloc_skb(a,b)  NULL
    skb = __netdev_alloc_skb_ip_align(pdata->netdev, len, GFP_ATOMIC);
    if (!skb) {
        netdev_err(pdata->netdev, "%s: Rx init fails; skb is NULL\n", __func__);
        return NULL;
    }

    dma_sync_single_for_cpu(pdata->dev, desc_data->rx.buf.dma_base, len, DMA_FROM_DEVICE);
    skb_copy_to_linear_data(skb, desc_data->skb->data, len);
    skb_put(skb, len);
    dma_sync_single_for_device(pdata->dev, desc_data->rx.buf.dma_base, len, DMA_FROM_DEVICE);

    return skb;
#endif

#ifdef FXGMAC_NOT_USE_PAGE_MAPPING
#ifdef FXGMAC_ZERO_COPY
    dma_sync_single_for_cpu(pdata->dev, desc_data->rx.buf.dma_base, len, DMA_FROM_DEVICE);
    skb = desc_data->skb;
    desc_data->skb = NULL;
    skb_put(skb, len);
    dma_sync_single_for_device(pdata->dev, desc_data->rx.buf.dma_base, len, DMA_FROM_DEVICE);
#else
    skb = __netdev_alloc_skb_ip_align(pdata->netdev, len, GFP_ATOMIC);
    if (!skb) {
        netdev_err(pdata->netdev, "%s: Rx init fails; skb is NULL\n", __func__);
        return NULL;
    }

    dma_sync_single_for_cpu(pdata->dev, desc_data->rx.buf.dma_base, len, DMA_FROM_DEVICE);
    skb_copy_to_linear_data(skb, desc_data->skb->data, len);
    skb_put(skb, len);
    dma_sync_single_for_device(pdata->dev, desc_data->rx.buf.dma_base, len, DMA_FROM_DEVICE);
#endif
    return skb;
#else
    skb = napi_alloc_skb(napi, desc_data->rx.hdr.dma_len);
    if (!skb)
        return NULL;

    /* Start with the header buffer which may contain just the header
     * or the header plus data
     */
    dma_sync_single_range_for_cpu(pdata->dev, desc_data->rx.hdr.dma_base,
                        desc_data->rx.hdr.dma_off,
                        desc_data->rx.hdr.dma_len,
                        DMA_FROM_DEVICE);

    packet = page_address(desc_data->rx.hdr.pa.pages) +
         desc_data->rx.hdr.pa.pages_offset;
    copy_len = len;
    copy_len = min(desc_data->rx.hdr.dma_len, copy_len);
    skb_copy_to_linear_data(skb, packet, copy_len);
    skb_put(skb, copy_len);
    return skb;
#endif
}

static int fxgmac_tx_poll(struct fxgmac_channel *channel)
{
    struct fxgmac_pdata *pdata = channel->pdata;
    struct fxgmac_ring *ring = channel->tx_ring;
    struct net_device *netdev = pdata->netdev;
    unsigned int tx_packets = 0, tx_bytes = 0;
    struct fxgmac_desc_data *desc_data;
    struct fxgmac_dma_desc *dma_desc;
    struct fxgmac_desc_ops *desc_ops;
    struct fxgmac_hw_ops *hw_ops;
    struct netdev_queue *txq;
    int processed = 0;
    unsigned int cur;

    static int fxgmac_restart_need = 0;
    static u32 change_cnt = 0;
    static u32 reg_cur_pre = 0xffffffff;
    (void) reg_cur_pre;
    (void) change_cnt;
    (void) fxgmac_restart_need;

#if FXGMAC_TX_HANG_TIMER_ENABLED
    static u32 reg_cur = 0;
#endif

    desc_ops = &pdata->desc_ops;
    hw_ops = &pdata->hw_ops;

    /* Nothing to do if there isn't a Tx ring for this channel */
    if (!ring){
        if(netif_msg_tx_done(pdata) && (channel->queue_index < pdata->tx_q_count)) DPRINTK("tx_poll, null point to ring %d\n", channel->queue_index);
        return 0;
    }
    if((ring->cur != ring->dirty) && (netif_msg_tx_done(pdata)))
        DPRINTK("tx_poll callin, ring_cur=%d,ring_dirty=%d,qIdx=%d\n", ring->cur, ring->dirty, channel->queue_index);

    cur = ring->cur;

    /* Be sure we get ring->cur before accessing descriptor data */
    smp_rmb();

    txq = netdev_get_tx_queue(netdev, channel->queue_index);

    while (ring->dirty != cur) {
        desc_data = FXGMAC_GET_DESC_DATA(ring, ring->dirty);
        dma_desc = desc_data->dma_desc;

        if (!hw_ops->tx_complete(dma_desc)) {
#if FXGMAC_TRIGGER_TX_HANG
            struct net_device *netdev = pdata->netdev;
            #define FXGMAC_HANG_THRESHOLD       1
            //static u32 reg_tail = 0, reg_tail_pre = 0xffffffff;

            reg_cur = readl(FXGMAC_DMA_REG(channel, 0x44/* tx desc curr pointer reg */));

            if(reg_cur != reg_cur_pre){
                reg_cur_pre = reg_cur;
                change_cnt = 0;
            } else {
                change_cnt++;
            }

            if (change_cnt > 2)
            {
                //change_cnt = 0;

                DPRINTK("after complete check, cur=%d, dirty=%d,qIdx=%d, hw desc cur=%#x, pre=%#x\n", ring->cur, ring->dirty, channel->queue_index,
                    reg_cur, reg_cur_pre);

                if((ring->cur > ring->dirty) && ((ring->cur - ring->dirty) > FXGMAC_HANG_THRESHOLD) ) {
                    DPRINTK("after complete check warning..., too many TBD occupied by HW, 0xdbbb, %d.\n", (ring->cur - ring->dirty));
                    (* ((u32 *)(netdev->base_addr + 0x1000))) = 0xdbbb;

                    if(!fxgmac_restart_need ) {
                        schedule_work(&pdata->expansion.restart_work);
                        fxgmac_restart_need = 1;
                        change_cnt = 0;
                    }
                }else if((ring->cur < ring->dirty) && ((ring->cur + (ring->dma_desc_count - ring->dirty)) > FXGMAC_HANG_THRESHOLD) ) {
                    DPRINTK("after complete check warning..., too many TBD occupied by HW, 0xdb00, %d.\n", (ring->cur + (ring->dma_desc_count - ring->dirty)));
                    (* ((u32 *)(netdev->base_addr + 0x1000))) = 0xdb00;

                    if(!fxgmac_restart_need ) {
                        schedule_work(&pdata->expansion.restart_work);
                        fxgmac_restart_need = 1;
                        change_cnt = 0;
                    }
                }
            }
#endif
#if FXGMAC_TX_HANG_TIMER_ENABLED
            if((!pdata->tx_hang_restart_queuing) && (!channel->expansion.tx_hang_timer_active)) {
                reg_cur = ring->dirty;
                if(reg_cur_pre != reg_cur) {
                    reg_cur_pre = reg_cur;
                    change_cnt = 0;
                }else {
                    change_cnt++;
                }

                if (change_cnt > 4)
                {
#if FXGMAC_TX_HANG_CHECH_DIRTY
                    channel->expansion.tx_hang_hw_cur = ring->dirty;
#else
                    channel->expansion.tx_hang_hw_cur = readl(FXGMAC_DMA_REG(channel, 0x44/* tx desc curr pointer reg */));
#endif
                    /* double check for race conditione */
                    if ((!pdata->tx_hang_restart_queuing) && (!channel->expansion.tx_hang_timer_active)) {
                    DPRINTK("tx_hang polling: start timer at desc %u, timer act=%u, queuing=%u, qidx=%u.\n", reg_cur, channel->expansion.tx_hang_timer_active, pdata->tx_hang_restart_queuing, channel->queue_index);
                    fxgmac_tx_hang_timer_start(channel);
                    }
                }
            }else if (pdata->tx_hang_restart_queuing) {
                //if(netif_msg_drv(pdata)) DPRINTK("tx_hang_timer_handler: restart scheduled.\n");
            }
#endif

            break;
        }

        reg_cur_pre = 0xffffffff;
        fxgmac_restart_need = 0;
        change_cnt = 0;

        /* Make sure descriptor fields are read after reading
         * the OWN bit
         */
        dma_rmb();

        if (netif_msg_tx_done(pdata))
            fxgmac_dump_tx_desc(pdata, ring, ring->dirty, 1, 0);

        if (hw_ops->is_last_desc(dma_desc)) {
            tx_packets += desc_data->tx.packets;
            tx_bytes += desc_data->tx.bytes;
        }

        /* Free the SKB and reset the descriptor for re-use */
        desc_ops->unmap_desc_data(pdata, desc_data);
        desc_ops->tx_desc_reset(desc_data);

        processed++;
        //ring->dirty++;
        ring->dirty = FXGMAC_GET_ENTRY(ring->dirty, ring->dma_desc_count);
    }

    if (!processed)
        return 0;

    netdev_tx_completed_queue(txq, tx_packets, tx_bytes);

    smp_wmb();
    if ((ring->tx.queue_stopped == 1) &&
        (fxgmac_tx_avail_desc(ring) > FXGMAC_TX_DESC_MIN_FREE)) {
        ring->tx.queue_stopped = 0;
        netif_tx_wake_queue(txq);
    }

    //yzhang comment out to reduce print
    if(netif_msg_tx_done(pdata)){
        DPRINTK("tx_poll callout, processed=%d\n", processed);
    }

    return processed;
}

static int fxgmac_rx_poll(struct fxgmac_channel *channel, int budget)
{
    struct fxgmac_pdata *pdata = channel->pdata;
    struct fxgmac_ring *ring = channel->rx_ring;
    struct net_device *netdev = pdata->netdev;
    unsigned int len, max_len;
    unsigned int context_next, context;
    struct fxgmac_desc_data *desc_data;
    struct fxgmac_pkt_info *pkt_info;
    unsigned int incomplete;
    struct napi_struct *napi;
    struct sk_buff *skb;
    int packet_count = 0;

    /* Nothing to do if there isn't a Rx ring for this channel */
    if (!ring)
        return 0;

    incomplete = 0;
    context_next = 0;

    napi = (pdata->per_channel_irq) ? &channel->expansion.napi_rx : &pdata->expansion.napi;

    desc_data = FXGMAC_GET_DESC_DATA(ring, ring->cur);
    pkt_info = &ring->pkt_info;

    while (packet_count < budget) {
        memset(pkt_info, 0, sizeof(*pkt_info));
        skb = NULL;
        len = 0;

    read_again:
        desc_data = FXGMAC_GET_DESC_DATA(ring, ring->cur);

        if (fxgmac_rx_dirty_desc(ring) > FXGMAC_RX_DESC_MAX_DIRTY)
            fxgmac_rx_refresh(channel);

        if (fxgmac_dev_read(channel))
            break;

        ring->cur = FXGMAC_GET_ENTRY(ring->cur, ring->dma_desc_count);

        incomplete = FXGMAC_GET_REG_BITS(
                    pkt_info->attributes,
                    RX_PACKET_ATTRIBUTES_INCOMPLETE_POS,
                    RX_PACKET_ATTRIBUTES_INCOMPLETE_LEN);
        context_next = FXGMAC_GET_REG_BITS(
                    pkt_info->attributes,
                    RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_POS,
                    RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_LEN);
        context = FXGMAC_GET_REG_BITS(
                    pkt_info->attributes,
                    RX_PACKET_ATTRIBUTES_CONTEXT_POS,
                    RX_PACKET_ATTRIBUTES_CONTEXT_LEN);

        if (incomplete || context_next)
            goto read_again;

        if (pkt_info->errors) {
            netif_err(pdata, rx_err, netdev, "error in received packet\n");
            dev_kfree_skb(skb);
            pdata->netdev->stats.rx_dropped++;
            goto next_packet;
        }

        if (!context) {
            len = desc_data->rx.len;

            if (len == 0) {
                if (net_ratelimit())
                    netif_err(pdata, rx_err, netdev,
                        "A packet of length 0 was received\n");
                pdata->netdev->stats.rx_length_errors++;
                pdata->netdev->stats.rx_dropped++;
                goto next_packet;
            }

            if (len && !skb) {
                skb = fxgmac_create_skb(pdata, napi, desc_data, len);
                if (unlikely(!skb)) {
                    if (net_ratelimit())
                        netif_err(pdata, rx_err, netdev, "create skb failed\n");
                    pdata->netdev->stats.rx_dropped++;
                    goto next_packet;
                }
            }

            max_len = netdev->mtu + ETH_HLEN;
            if (!(netdev->features & NETIF_F_HW_VLAN_CTAG_RX) &&
                skb->protocol == htons(ETH_P_8021Q))
                    max_len += VLAN_HLEN;
            if (len > max_len) {
                if (net_ratelimit())
                    netif_err(pdata, rx_err, netdev,
                        "len %d larger than max size %d\n",
                            len, max_len);
                pdata->netdev->stats.rx_length_errors++;
                pdata->netdev->stats.rx_dropped++;
                dev_kfree_skb(skb);
                goto next_packet;
            }
        }

        if (!skb) {
            pdata->netdev->stats.rx_dropped++;
            goto next_packet;
        }

        if(netif_msg_pktdata(pdata))
            fxgmac_print_pkt(netdev, skb, false);

        skb_checksum_none_assert(skb);
        if (netdev->features & NETIF_F_RXCSUM)
            skb->ip_summed = CHECKSUM_UNNECESSARY;

        if (FXGMAC_GET_REG_BITS(pkt_info->attributes,
                    RX_PACKET_ATTRIBUTES_VLAN_CTAG_POS,
                    RX_PACKET_ATTRIBUTES_VLAN_CTAG_LEN)) {
            __vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q),
                           pkt_info->vlan_ctag);
            pdata->stats.rx_vlan_packets++;
        }

        if (FXGMAC_GET_REG_BITS(pkt_info->attributes,
                    RX_PACKET_ATTRIBUTES_RSS_HASH_POS,
                    RX_PACKET_ATTRIBUTES_RSS_HASH_LEN))
            skb_set_hash(skb, pkt_info->rss_hash,
                     pkt_info->rss_hash_type);

        skb->dev = netdev;
        skb->protocol = eth_type_trans(skb, netdev);
        skb_record_rx_queue(skb, channel->queue_index);

        if(pdata->expansion.fxgmac_test_tso_flag)
        {
            /* tso test */
            if(pdata->expansion.fxgmac_test_tso_seg_num == 1)
            {
                /* last segment */
                if(pdata->expansion.fxgmac_test_last_tso_len == skb->len + FXGMAC_TEST_MAC_HEAD_LEN)
                {
                    /* receive last segment, reset flag */
                    pdata->expansion.fxgmac_test_tso_flag = false;
                    pdata->expansion.fxgmac_test_tso_seg_num = 0;
                    pdata->expansion.fxgmac_test_packet_len = 0;
                    pdata->expansion.fxgmac_test_last_tso_len = 0;

                    /* process packet */
                    if((pdata->expansion.fxgmac_test_skb_arr_in_index + 1) % FXGMAC_MAX_DBG_TEST_PKT != pdata->expansion.fxgmac_test_skb_arr_out_index){
                        struct sk_buff *tmpskb = skb_copy(skb, GFP_ATOMIC);
                        skb_push(tmpskb, FXGMAC_TEST_MAC_HEAD_LEN);

                        pdata->expansion.fxgmac_test_skb_array[pdata->expansion.fxgmac_test_skb_arr_in_index] = tmpskb;
                        pdata->expansion.fxgmac_test_skb_arr_in_index = (pdata->expansion.fxgmac_test_skb_arr_in_index + 1) % FXGMAC_MAX_DBG_TEST_PKT;
                    }
                    else{
                        DPRINTK("loopback test buffer is full.");
                    }
                }
            }
            else /* non last segment */
            {
                if(pdata->expansion.fxgmac_test_packet_len == skb->len + FXGMAC_TEST_MAC_HEAD_LEN){
                    /* receive a segment */
                    pdata->expansion.fxgmac_test_tso_seg_num--;

                    /* process packet */
                    if((pdata->expansion.fxgmac_test_skb_arr_in_index + 1) % FXGMAC_MAX_DBG_TEST_PKT != pdata->expansion.fxgmac_test_skb_arr_out_index){
                        struct sk_buff *tmpskb = skb_copy(skb, GFP_ATOMIC);
                        skb_push(tmpskb, FXGMAC_TEST_MAC_HEAD_LEN);

                        pdata->expansion.fxgmac_test_skb_array[pdata->expansion.fxgmac_test_skb_arr_in_index] = tmpskb;
                        pdata->expansion.fxgmac_test_skb_arr_in_index = (pdata->expansion.fxgmac_test_skb_arr_in_index + 1) % FXGMAC_MAX_DBG_TEST_PKT;
                    }
                    else{
                        DPRINTK("loopback test buffer is full.");
                    }
                }
            }
        }
        else if(pdata->expansion.fxgmac_test_packet_len != 0)
        {
            /* xsum and phy loopback test */
            if(pdata->expansion.fxgmac_test_packet_len == skb->len + FXGMAC_TEST_MAC_HEAD_LEN)
            {
                /* reset fxg_packet_len */
                pdata->expansion.fxgmac_test_packet_len = 0;

                if((pdata->expansion.fxgmac_test_skb_arr_in_index + 1) % FXGMAC_MAX_DBG_TEST_PKT != pdata->expansion.fxgmac_test_skb_arr_out_index){
                    struct sk_buff *tmpskb = skb_copy(skb, GFP_ATOMIC);
                    skb_push(tmpskb, FXGMAC_TEST_MAC_HEAD_LEN);

                    pdata->expansion.fxgmac_test_skb_array[pdata->expansion.fxgmac_test_skb_arr_in_index] = tmpskb;
                    pdata->expansion.fxgmac_test_skb_arr_in_index = (pdata->expansion.fxgmac_test_skb_arr_in_index + 1) % FXGMAC_MAX_DBG_TEST_PKT;
                }
                else{
                    DPRINTK("loopback test buffer is full.");
                }
            }
        }
#if 1
        napi_gro_receive(napi, skb);
#else
        netif_receive_skb(skb);
#endif

next_packet:
        packet_count++;

        pdata->netdev->stats.rx_packets++;
        pdata->netdev->stats.rx_bytes += len;
    }

    //fxgmac_rx_refresh(channel);

    return packet_count;
}

static int fxgmac_one_poll_tx(struct napi_struct *napi, int budget)
{
    struct fxgmac_channel *channel = container_of(napi,
                        struct fxgmac_channel,
                        expansion.napi_tx);
    struct fxgmac_pdata *pdata = channel->pdata;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    fxgmac_tx_poll(channel);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0))
    if (napi_complete_done(napi, 0)) {
        hw_ops->enable_msix_one_interrupt(pdata, MSI_ID_TXQ0);
    }
#else
    napi_complete(napi);
    hw_ops->enable_msix_one_interrupt(pdata, MSI_ID_TXQ0);
#endif
    return 0;

}

static int fxgmac_one_poll_rx(struct napi_struct *napi, int budget)
{
    struct fxgmac_channel *channel = container_of(napi,
                        struct fxgmac_channel,
                        expansion.napi_rx);
    int processed = 0;
    struct fxgmac_pdata *pdata = channel->pdata;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    processed = fxgmac_rx_poll(channel, budget);
    if (processed < budget) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0))
        /* if there no interrupt occurred when this interrupt running,struct napi's state is NAPIF_STATE_SCHED,
        * napi_complete_done return true and we can enable irq,it will not cause unbalanced iqr issure.
        * if there more interrupt occurred when this interrupt running,struct napi's state is NAPIF_STATE_SCHED | NAPIF_STATE_MISSED
        * because napi_schedule_prep will make it. At this time napi_complete_done will return false and
        * schedule poll again because of NAPIF_STATE_MISSED,it will cause unbalanced irq issure.
        */
        if (napi_complete_done(napi, processed)) {
            hw_ops->enable_msix_one_interrupt(pdata, channel->queue_index);
        }
#else
        napi_complete(napi);
        hw_ops->enable_msix_one_interrupt(pdata, channel->queue_index);
#endif
    }

    return processed;
}

static int fxgmac_all_poll(struct napi_struct *napi, int budget)
{
    struct fxgmac_pdata *pdata = container_of(napi,
                             struct fxgmac_pdata,
                             expansion.napi);
    struct fxgmac_channel *channel;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    int processed;
    unsigned int i;

    //yzhang comment out
    if(netif_msg_rx_status(pdata)){
        DPRINTK("rx all_poll callin budget=%d\n", budget);
    }

    processed = 0;
    do {
        channel = pdata->channel_head;
        /* Cleanup Tx ring first */
        /*since only 1 tx channel supported in this version, poll ch 0 always. */
        fxgmac_tx_poll(pdata->channel_head + 0);
        for (i = 0; i < pdata->channel_count; i++, channel++) {
            processed += fxgmac_rx_poll(channel, budget);
        }
    } while (false);

    /* If we processed everything, we are done */
    if (processed < budget) {
        /* Turn off polling */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0))
        if (napi_complete_done(napi, processed))
            hw_ops->enable_mgm_interrupt(pdata);
#else
        napi_complete(napi);
        hw_ops->enable_mgm_interrupt(pdata);
#endif
    }

    if((processed) && (netif_msg_rx_status(pdata))) { //yzhang for debug
        DPRINTK("rx all_poll callout received = %d\n", processed);
    }

    return processed;
}

void fxgmac_tx_start_xmit(struct fxgmac_channel *channel,
                 struct fxgmac_ring *ring)
{
    struct fxgmac_pdata *pdata = channel->pdata;
    struct fxgmac_desc_data *desc_data;

    /* Make sure everything is written before the register write */
    wmb();

    /* Issue a poll command to Tx DMA by writing address
     * of next immediate free descriptor
     */
    desc_data = FXGMAC_GET_DESC_DATA(ring, ring->cur);

    writereg(pdata->pAdapter, lower_32_bits(desc_data->dma_desc_addr),
           FXGMAC_DMA_REG(channel, DMA_CH_TDTR_LO));

    if(netif_msg_tx_done(pdata)) {
        DPRINTK("tx_start_xmit: dump before wr reg, \
            dma base=0x%016llx,reg=0x%08x, \
            tx timer usecs=%u,tx_timer_active=%u\n",
            desc_data->dma_desc_addr,
            readreg(pdata->pAdapter, FXGMAC_DMA_REG(channel, DMA_CH_TDTR_LO)),
            pdata->tx_usecs, channel->tx_timer_active);
    }

    ring->tx.xmit_more = 0;
}

void fxgmac_dev_xmit(struct fxgmac_channel *channel)
{
    struct fxgmac_pdata *pdata = channel->pdata;
    struct fxgmac_ring *ring = channel->tx_ring;
    unsigned int tso_context, vlan_context;
    struct fxgmac_desc_data *desc_data;
    struct fxgmac_dma_desc *dma_desc;
    struct fxgmac_pkt_info *pkt_info;
    unsigned int csum, tso, vlan;
    int start_index = ring->cur;
    int cur_index = ring->cur;
    int i;

    if(netif_msg_tx_done(pdata)) DPRINTK("dev_xmit callin, desc cur=%d\n", cur_index);

    pkt_info = &ring->pkt_info;
    csum = FXGMAC_GET_REG_BITS(pkt_info->attributes,
                   TX_PACKET_ATTRIBUTES_CSUM_ENABLE_POS,
                TX_PACKET_ATTRIBUTES_CSUM_ENABLE_LEN);
    tso = FXGMAC_GET_REG_BITS(pkt_info->attributes,
                  TX_PACKET_ATTRIBUTES_TSO_ENABLE_POS,
                TX_PACKET_ATTRIBUTES_TSO_ENABLE_LEN);
    vlan = FXGMAC_GET_REG_BITS(pkt_info->attributes,
                   TX_PACKET_ATTRIBUTES_VLAN_CTAG_POS,
                TX_PACKET_ATTRIBUTES_VLAN_CTAG_LEN);

    if (tso && (pkt_info->mss != ring->tx.cur_mss))
        tso_context = 1;
    else
        tso_context = 0;

    //if(tso && (netif_msg_tx_done(pdata)))
    if((tso_context) && (netif_msg_tx_done(pdata))) {
        //tso is initialized to start...
        DPRINTK("fxgmac_dev_xmit,tso_%s tso=0x%x,pkt_mss=%d,cur_mss=%d\n",(pkt_info->mss)?"start":"stop", tso, pkt_info->mss, ring->tx.cur_mss);
    }

    if (vlan && (pkt_info->vlan_ctag != ring->tx.cur_vlan_ctag))
        vlan_context = 1;
    else
        vlan_context = 0;

    if(vlan && (netif_msg_tx_done(pdata))) DPRINTK("fxgmac_dev_xmi:pkt vlan=%d, ring vlan=%d, vlan_context=%d\n", pkt_info->vlan_ctag, ring->tx.cur_vlan_ctag, vlan_context);

    desc_data = FXGMAC_GET_DESC_DATA(ring, cur_index);
    dma_desc = desc_data->dma_desc;

    /* Create a context descriptor if this is a TSO pkt_info */
    if (tso_context || vlan_context) {
        if (tso_context) {
#if 0
            netif_dbg(pdata, tx_queued, pdata->netdev,
                  "TSO context descriptor, mss=%u\n",
                  pkt_info->mss);
#else
            if (netif_msg_tx_done(pdata)) DPRINTK("xlgamc dev xmit,construct tso context descriptor, mss=%u\n",
                  pkt_info->mss);
#endif

            /* Set the MSS size */
            dma_desc->desc2 = FXGMAC_SET_REG_BITS_LE(
                        dma_desc->desc2,
                        TX_CONTEXT_DESC2_MSS_POS,
                        TX_CONTEXT_DESC2_MSS_LEN,
                        pkt_info->mss);

            /* Mark it as a CONTEXT descriptor */
            dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                        dma_desc->desc3,
                        TX_CONTEXT_DESC3_CTXT_POS,
                        TX_CONTEXT_DESC3_CTXT_LEN,
                        1);

            /* Indicate this descriptor contains the MSS */
            dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                        dma_desc->desc3,
                        TX_CONTEXT_DESC3_TCMSSV_POS,
                        TX_CONTEXT_DESC3_TCMSSV_LEN,
                        1);

            ring->tx.cur_mss = pkt_info->mss;
        }

        if (vlan_context) {
            netif_dbg(pdata, tx_queued, pdata->netdev,
                  "VLAN context descriptor, ctag=%u\n",
                  pkt_info->vlan_ctag);

            /* Mark it as a CONTEXT descriptor */
            dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                        dma_desc->desc3,
                        TX_CONTEXT_DESC3_CTXT_POS,
                        TX_CONTEXT_DESC3_CTXT_LEN,
                        1);

            /* Set the VLAN tag */
            dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                        dma_desc->desc3,
                        TX_CONTEXT_DESC3_VT_POS,
                        TX_CONTEXT_DESC3_VT_LEN,
                        pkt_info->vlan_ctag);

            /* Indicate this descriptor contains the VLAN tag */
            dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                        dma_desc->desc3,
                        TX_CONTEXT_DESC3_VLTV_POS,
                        TX_CONTEXT_DESC3_VLTV_LEN,
                        1);

            ring->tx.cur_vlan_ctag = pkt_info->vlan_ctag;
        }

        //cur_index++;
        cur_index = FXGMAC_GET_ENTRY(cur_index, ring->dma_desc_count);
        desc_data = FXGMAC_GET_DESC_DATA(ring, cur_index);
        dma_desc = desc_data->dma_desc;
    }

    /* Update buffer address (for TSO this is the header) */
    dma_desc->desc0 =  cpu_to_le32(lower_32_bits(desc_data->skb_dma));
    dma_desc->desc1 =  cpu_to_le32(upper_32_bits(desc_data->skb_dma));

    /* Update the buffer length */
    dma_desc->desc2 = FXGMAC_SET_REG_BITS_LE(
                dma_desc->desc2,
                TX_NORMAL_DESC2_HL_B1L_POS,
                TX_NORMAL_DESC2_HL_B1L_LEN,
                desc_data->skb_dma_len);

    /* VLAN tag insertion check */
    if (vlan) {
        dma_desc->desc2 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc2,
                    TX_NORMAL_DESC2_VTIR_POS,
                    TX_NORMAL_DESC2_VTIR_LEN,
                    TX_NORMAL_DESC2_VLAN_INSERT);
        pdata->stats.tx_vlan_packets++;
    }

    /* Timestamp enablement check */
    if (FXGMAC_GET_REG_BITS(pkt_info->attributes,
                TX_PACKET_ATTRIBUTES_PTP_POS,
                TX_PACKET_ATTRIBUTES_PTP_LEN))
        dma_desc->desc2 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc2,
                    TX_NORMAL_DESC2_TTSE_POS,
                    TX_NORMAL_DESC2_TTSE_LEN,
                    1);

    /* Mark it as First Descriptor */
    dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                dma_desc->desc3,
                TX_NORMAL_DESC3_FD_POS,
                TX_NORMAL_DESC3_FD_LEN,
                1);

    /* Mark it as a NORMAL descriptor */
    dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                dma_desc->desc3,
                TX_NORMAL_DESC3_CTXT_POS,
                TX_NORMAL_DESC3_CTXT_LEN,
                0);

    /* Set OWN bit if not the first descriptor */
    if (cur_index != start_index)
        dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc3,
                    TX_NORMAL_DESC3_OWN_POS,
                    TX_NORMAL_DESC3_OWN_LEN,
                    1);

    if (tso) {
        /* Enable TSO */
        dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc3,
                    TX_NORMAL_DESC3_TSE_POS,
                    TX_NORMAL_DESC3_TSE_LEN, 1);
        dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc3,
                    TX_NORMAL_DESC3_TCPPL_POS,
                    TX_NORMAL_DESC3_TCPPL_LEN,
                    pkt_info->tcp_payload_len);
        dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc3,
                    TX_NORMAL_DESC3_TCPHDRLEN_POS,
                    TX_NORMAL_DESC3_TCPHDRLEN_LEN,
                    pkt_info->tcp_header_len / 4);

        pdata->stats.tx_tso_packets++;
    } else {
        /* Enable CRC and Pad Insertion */
        dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc3,
                    TX_NORMAL_DESC3_CPC_POS,
                    TX_NORMAL_DESC3_CPC_LEN, 0);

        /* Enable HW CSUM */
        if (csum)
            dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                        dma_desc->desc3,
                        TX_NORMAL_DESC3_CIC_POS,
                        TX_NORMAL_DESC3_CIC_LEN,
                        0x3);

        /* Set the total length to be transmitted */
        dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc3,
                    TX_NORMAL_DESC3_FL_POS,
                    TX_NORMAL_DESC3_FL_LEN,
                    pkt_info->length);
    }
    if(netif_msg_tx_done(pdata)) DPRINTK("dev_xmit before more descs, desc cur=%d, start=%d, desc=%#x,%#x,%#x,%#x\n",
        cur_index, start_index, dma_desc->desc0, dma_desc->desc1, dma_desc->desc2, dma_desc->desc3);

    if (start_index <= cur_index)
        i = cur_index - start_index + 1;
    else
        i = ring->dma_desc_count - start_index + cur_index;

    for (; i < pkt_info->desc_count; i++) {
        cur_index = FXGMAC_GET_ENTRY(cur_index, ring->dma_desc_count);

        desc_data = FXGMAC_GET_DESC_DATA(ring, cur_index);
        dma_desc = desc_data->dma_desc;

        /* Update buffer address */
        dma_desc->desc0 =
            cpu_to_le32(lower_32_bits(desc_data->skb_dma));
        dma_desc->desc1 =
            cpu_to_le32(upper_32_bits(desc_data->skb_dma));

        /* Update the buffer length */
        dma_desc->desc2 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc2,
                    TX_NORMAL_DESC2_HL_B1L_POS,
                    TX_NORMAL_DESC2_HL_B1L_LEN,
                    desc_data->skb_dma_len);

        /* Set OWN bit */
        dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc3,
                    TX_NORMAL_DESC3_OWN_POS,
                    TX_NORMAL_DESC3_OWN_LEN, 1);

        /* Mark it as NORMAL descriptor */
        dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                    dma_desc->desc3,
                    TX_NORMAL_DESC3_CTXT_POS,
                    TX_NORMAL_DESC3_CTXT_LEN, 0);

        /* Enable HW CSUM */
        if (csum)
            dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                        dma_desc->desc3,
                        TX_NORMAL_DESC3_CIC_POS,
                        TX_NORMAL_DESC3_CIC_LEN,
                        0x3);
    }

    /* Set LAST bit for the last descriptor */
    dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                dma_desc->desc3,
                TX_NORMAL_DESC3_LD_POS,
                TX_NORMAL_DESC3_LD_LEN, 1);

    dma_desc->desc2 = FXGMAC_SET_REG_BITS_LE(
                dma_desc->desc2,
                TX_NORMAL_DESC2_IC_POS,
                TX_NORMAL_DESC2_IC_LEN, 1);

    /* Save the Tx info to report back during cleanup */
    desc_data->tx.packets = pkt_info->tx_packets;
    desc_data->tx.bytes = pkt_info->tx_bytes;

    if(netif_msg_tx_done(pdata)) DPRINTK("dev_xmit last descs, desc cur=%d, desc=%#x,%#x,%#x,%#x\n",
        cur_index, dma_desc->desc0, dma_desc->desc1, dma_desc->desc2, dma_desc->desc3);

    /* In case the Tx DMA engine is running, make sure everything
     * is written to the descriptor(s) before setting the OWN bit
     * for the first descriptor
     */
    dma_wmb();

    /* Set OWN bit for the first descriptor */
    desc_data = FXGMAC_GET_DESC_DATA(ring, start_index);
    dma_desc = desc_data->dma_desc;
    dma_desc->desc3 = FXGMAC_SET_REG_BITS_LE(
                dma_desc->desc3,
                TX_NORMAL_DESC3_OWN_POS,
                TX_NORMAL_DESC3_OWN_LEN, 1);

    if(netif_msg_tx_done(pdata)) DPRINTK("dev_xmit first descs, start=%d, desc=%#x,%#x,%#x,%#x\n",
        start_index, dma_desc->desc0, dma_desc->desc1, dma_desc->desc2, dma_desc->desc3);

    if (netif_msg_tx_queued(pdata))
        fxgmac_dump_tx_desc(pdata, ring, start_index,
                    pkt_info->desc_count, 1);

    /* Make sure ownership is written to the descriptor */
    smp_wmb();

    //ring->cur = cur_index + 1;
    ring->cur = FXGMAC_GET_ENTRY(cur_index, ring->dma_desc_count);

#if 0
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0) )
    if (!ring->tx.xmit_more || netif_xmit_stopped(netdev_get_tx_queue(pdata->netdev,
                           channel->queue_index)))
#elif ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,1) )
    if (!pkt_info->skb->xmit_more ||
        netif_xmit_stopped(netdev_get_tx_queue(pdata->netdev,
                           channel->queue_index)))
#else
    if (netif_xmit_stopped(netdev_get_tx_queue(pdata->netdev,
                           channel->queue_index)))
#endif
    {
        fxgmac_tx_start_xmit(channel, ring);
    }
    else
        ring->tx.xmit_more = 1;
#endif

    fxgmac_tx_start_xmit(channel, ring);

    /* yzhang for reduce debug output */
    if(netif_msg_tx_done(pdata)){
        DPRINTK("dev_xmit callout %s: descriptors %u to %u written\n",
          channel->name, start_index & (ring->dma_desc_count - 1),
          (ring->cur - 1) & (ring->dma_desc_count - 1));
    }
}

extern void fxgmac_diag_get_rx_info(struct fxgmac_channel *channel);

static void fxgmac_get_rx_tstamp(struct fxgmac_pkt_info *pkt_info,
                 struct fxgmac_dma_desc *dma_desc)
{
    //u32 tsa, tsd;
    u64 nsec;
#if 0
    tsa = FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                     RX_CONTEXT_DESC3_TSA_POS,
                RX_CONTEXT_DESC3_TSA_LEN);
    tsd = FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                     RX_CONTEXT_DESC3_TSD_POS,
                RX_CONTEXT_DESC3_TSD_LEN);
    if (tsa && !tsd) {
#endif
        nsec = le32_to_cpu(dma_desc->desc1);
        nsec <<= 32;
        nsec |= le32_to_cpu(dma_desc->desc0);
        if (nsec != 0xffffffffffffffffULL) {
            pkt_info->rx_tstamp = nsec;
            pkt_info->attributes = FXGMAC_SET_REG_BITS(
                    pkt_info->attributes,
                    RX_PACKET_ATTRIBUTES_RX_TSTAMP_POS,
                    RX_PACKET_ATTRIBUTES_RX_TSTAMP_LEN,
                    1);
        }
    //}
}

static int fxgmac_dev_read(struct fxgmac_channel *channel)
{
    struct fxgmac_pdata *pdata = channel->pdata;
    struct fxgmac_ring *ring = channel->rx_ring;
    struct net_device *netdev = pdata->netdev;
    struct fxgmac_desc_data *desc_data;
    struct fxgmac_dma_desc *dma_desc;
    struct fxgmac_pkt_info *pkt_info;
    u32 ipce, iphe, rxparser;
    unsigned int err, etlt;

    //unsigned int i;
    static unsigned int cnt_incomplete = 0;

    desc_data = FXGMAC_GET_DESC_DATA(ring, ring->cur);
    dma_desc = desc_data->dma_desc;
    pkt_info = &ring->pkt_info;

    /* Check for data availability */
    if (FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                   RX_NORMAL_DESC3_OWN_POS,
                   RX_NORMAL_DESC3_OWN_LEN))
        return 1;

    /* Make sure descriptor fields are read after reading the OWN bit */
    dma_rmb();

    if (netif_msg_rx_status(pdata))
        fxgmac_dump_rx_desc(pdata, ring, ring->cur);

    if (FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                   RX_NORMAL_DESC3_CTXT_POS,
                   RX_NORMAL_DESC3_CTXT_LEN)) {
        /* Timestamp Context Descriptor */
        fxgmac_get_rx_tstamp(pkt_info, dma_desc);

        pkt_info->attributes = FXGMAC_SET_REG_BITS(
                    pkt_info->attributes,
                    RX_PACKET_ATTRIBUTES_CONTEXT_POS,
                    RX_PACKET_ATTRIBUTES_CONTEXT_LEN,
                    1);
        pkt_info->attributes = FXGMAC_SET_REG_BITS(
                pkt_info->attributes,
                RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_POS,
                RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_LEN,
                0);
        if(netif_msg_rx_status(pdata)) DPRINTK("dev_read context desc,ch=%s\n",channel->name);
        return 0;
    }

    /* Normal Descriptor, be sure Context Descriptor bit is off */
    pkt_info->attributes = FXGMAC_SET_REG_BITS(
                pkt_info->attributes,
                RX_PACKET_ATTRIBUTES_CONTEXT_POS,
                RX_PACKET_ATTRIBUTES_CONTEXT_LEN,
                0);

    /* Indicate if a Context Descriptor is next */
#if 0
    if (FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                   RX_NORMAL_DESC3_CDA_POS,
                   RX_NORMAL_DESC3_CDA_LEN))
        pkt_info->attributes = FXGMAC_SET_REG_BITS(
                pkt_info->attributes,
                RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_POS,
                RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_LEN,
                1);
#endif
    /* Get the header length */
    if (FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                   RX_NORMAL_DESC3_FD_POS,
                   RX_NORMAL_DESC3_FD_LEN)) {
        desc_data->rx.hdr_len = FXGMAC_GET_REG_BITS_LE(dma_desc->desc2,
                            RX_NORMAL_DESC2_HL_POS,
                            RX_NORMAL_DESC2_HL_LEN);
        if (desc_data->rx.hdr_len)
            pdata->stats.rx_split_header_packets++;
    }

#if 0 //(FXGMAC_RSS_FEATURE_ENABLED) //20210608
    /* Get the RSS hash */
    if (FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                   RX_NORMAL_DESC3_RSV_POS,
                   RX_NORMAL_DESC3_RSV_LEN)) {
        pkt_info->attributes = FXGMAC_SET_REG_BITS(
                pkt_info->attributes,
                RX_PACKET_ATTRIBUTES_RSS_HASH_POS,
                RX_PACKET_ATTRIBUTES_RSS_HASH_LEN,
                1);

        pkt_info->rss_hash = le32_to_cpu(dma_desc->desc1);

        l34t = FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                          RX_NORMAL_DESC3_L34T_POS,
                      RX_NORMAL_DESC3_L34T_LEN);
        switch (l34t) {
        case RX_DESC3_L34T_IPV4_TCP:
        case RX_DESC3_L34T_IPV4_UDP:
        case RX_DESC3_L34T_IPV6_TCP:
        case RX_DESC3_L34T_IPV6_UDP:
            pkt_info->rss_hash_type = PKT_HASH_TYPE_L4;
            break;
        default:
            pkt_info->rss_hash_type = PKT_HASH_TYPE_L3;
        }
    }
#endif
    /* Get the pkt_info length */
    desc_data->rx.len = FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                    RX_NORMAL_DESC3_PL_POS,
                    RX_NORMAL_DESC3_PL_LEN);
    //DPRINTK("dev_read upon FD=1, pkt_len=%u\n",desc_data->rx.len);

    if (!FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                    RX_NORMAL_DESC3_LD_POS,
                    RX_NORMAL_DESC3_LD_LEN)) {
        /* Not all the data has been transferred for this pkt_info */
        pkt_info->attributes = FXGMAC_SET_REG_BITS(
                pkt_info->attributes,
                RX_PACKET_ATTRIBUTES_INCOMPLETE_POS,
                RX_PACKET_ATTRIBUTES_INCOMPLETE_LEN,
                1);
        cnt_incomplete++;
        if ((cnt_incomplete < 2) && netif_msg_rx_status(pdata))
            DPRINTK("dev_read NOT last desc,pkt incomplete yet,%u\n", cnt_incomplete);

        return 0;
    }
    if ((cnt_incomplete) && netif_msg_rx_status(pdata))
        DPRINTK("dev_read rx back to normal and incomplete cnt=%u\n", cnt_incomplete);
    cnt_incomplete = 0; //when back to normal, reset cnt

    /* This is the last of the data for this pkt_info */
    pkt_info->attributes = FXGMAC_SET_REG_BITS(
            pkt_info->attributes,
            RX_PACKET_ATTRIBUTES_INCOMPLETE_POS,
            RX_PACKET_ATTRIBUTES_INCOMPLETE_LEN,
            0);

    /* Set checksum done indicator as appropriate */
    if (netdev->features & NETIF_F_RXCSUM) {
        ipce = FXGMAC_GET_REG_BITS_LE(desc_data->dma_desc->desc1,
                                    RX_NORMAL_DESC1_WB_IPCE_POS,
                                    RX_NORMAL_DESC1_WB_IPCE_LEN);
        iphe = FXGMAC_GET_REG_BITS_LE(desc_data->dma_desc->desc1,
                                    RX_NORMAL_DESC1_WB_IPHE_POS,
                                    RX_NORMAL_DESC1_WB_IPHE_LEN);
        if (!ipce && !iphe)
            pkt_info->attributes = FXGMAC_SET_REG_BITS(
                    pkt_info->attributes,
                    RX_PACKET_ATTRIBUTES_CSUM_DONE_POS,
                    RX_PACKET_ATTRIBUTES_CSUM_DONE_LEN,
                    1);
        else
            return 0;
    }

    /* Check for errors (only valid in last descriptor) */
    err = FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                     RX_NORMAL_DESC3_ES_POS,
                     RX_NORMAL_DESC3_ES_LEN);
    /* b111: Incomplete parsing due to ECC error */
    rxparser = FXGMAC_GET_REG_BITS_LE(desc_data->dma_desc->desc2,
                                RX_NORMAL_DESC2_WB_RAPARSER_POS,
                                RX_NORMAL_DESC2_WB_RAPARSER_LEN);
    if (err || rxparser == 0x7) {
        pkt_info->errors = FXGMAC_SET_REG_BITS(pkt_info->errors,
                                RX_PACKET_ERRORS_FRAME_POS,
                                RX_PACKET_ERRORS_FRAME_LEN, 1);
        return 0;
    }

    etlt = FXGMAC_GET_REG_BITS_LE(dma_desc->desc3,
                      RX_NORMAL_DESC3_ETLT_POS,
                      RX_NORMAL_DESC3_ETLT_LEN);
        if ((etlt == 0x4) &&
            (netdev->features & NETIF_F_HW_VLAN_CTAG_RX)) {
                pkt_info->attributes = FXGMAC_SET_REG_BITS(
                        pkt_info->attributes,
                        RX_PACKET_ATTRIBUTES_VLAN_CTAG_POS,
                        RX_PACKET_ATTRIBUTES_VLAN_CTAG_LEN, 1);
                pkt_info->vlan_ctag =
                        FXGMAC_GET_REG_BITS_LE(dma_desc->desc0,
                                               RX_NORMAL_DESC0_OVT_POS,
                                               RX_NORMAL_DESC0_OVT_LEN);
                netif_dbg(pdata, rx_status, netdev, "vlan-ctag=%#06x\n",
                          pkt_info->vlan_ctag);
        }

    return 0;
}
