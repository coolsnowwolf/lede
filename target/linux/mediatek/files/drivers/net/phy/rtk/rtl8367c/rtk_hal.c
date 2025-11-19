#include <linux/kernel.h>
#include <linux/delay.h>
#include  "./include/rtk_switch.h"
#include  "./include/vlan.h"
#include  "./include/port.h"
#include  "./include/rate.h"
#include  "./include/rtk_hal.h"
#include  "./include/l2.h"
#include  "./include/stat.h"
#include  "./include/igmp.h"
#include  "./include/trap.h"
#include  "./include/leaky.h"
#include  "./include/mirror.h"
#include  "./include/rtl8367c_asicdrv_port.h"
#include  "./include/rtl8367c_asicdrv_mib.h"
#include  "./include/smi.h"
#include  "./include/qos.h"
#include  "./include/trunk.h"

void rtk_hal_switch_init(void)
{
	if(rtk_switch_init() != 0)
        printk("rtk_switch_init failed\n");
	mdelay(500);
	/*vlan init */
	if (rtk_vlan_init() != 0)
        printk("rtk_vlan_init failed\n");
}

void rtk_hal_dump_full_mib(void)
{
	rtk_port_t port;
	rtk_stat_counter_t Cntr;
	rtk_stat_port_type_t cntr_idx;

	for (port = UTP_PORT0; port < (UTP_PORT0 + 5); port++) {
		printk("\nPort%d\n", port);
		for (cntr_idx = STAT_IfInOctets; cntr_idx < STAT_PORT_CNTR_END; cntr_idx ++) {
			rtk_stat_port_get(port, cntr_idx, &Cntr);
			printk("%8llu ", Cntr);
			if (((cntr_idx%10) == 9))
				printk("\n");
		}
	}

	for (port = EXT_PORT0; port < (EXT_PORT0 + 2); port++) {
		printk("\nPort%d\n", port);
		for (cntr_idx = STAT_IfInOctets; cntr_idx < STAT_PORT_CNTR_END; cntr_idx ++) {
			rtk_stat_port_get(port, cntr_idx, &Cntr);
			printk("%8llu ", Cntr);
			if (((cntr_idx%10) == 9))
				printk("\n");
		}
	}
	rtk_stat_global_reset();
}
void rtk_dump_mib_type(rtk_stat_port_type_t cntr_idx)
{
	rtk_port_t port;
	rtk_stat_counter_t Cntr;

	for (port = UTP_PORT0; port < (UTP_PORT0 + 5); port++) {
		rtk_stat_port_get(port, cntr_idx, &Cntr);
		printk("%8llu", Cntr);
	}
	for (port = EXT_PORT0; port < (EXT_PORT0 + 2); port++) {
		rtk_stat_port_get(port, cntr_idx, &Cntr);
		printk("%8llu", Cntr);
	}
	printk("\n");
}

void rtk_hal_dump_mib(void)
{

	printk("==================%8s%8s%8s%8s%8s%8s%8s\n", "Port0", "Port1",
	       "Port2", "Port3", "Port4", "Port16", "Port17");
	/* Get TX Unicast Pkts */
	printk("TX Unicast Pkts  :");
	rtk_dump_mib_type(STAT_IfOutUcastPkts);
	/* Get TX Multicast Pkts */
	printk("TX Multicast Pkts:");
	rtk_dump_mib_type(STAT_IfOutMulticastPkts);
	/* Get TX BroadCast Pkts */
	printk("TX BroadCast Pkts:");
	rtk_dump_mib_type(STAT_IfOutBroadcastPkts);
	/* Get TX Collisions */
	/* Get TX Puase Frames */
	printk("TX Pause Frames  :");
	rtk_dump_mib_type(STAT_Dot3OutPauseFrames);
	/* Get TX Drop Events */
	/* Get RX Unicast Pkts */
	printk("RX Unicast Pkts  :");
	rtk_dump_mib_type(STAT_IfInUcastPkts);
	/* Get RX Multicast Pkts */
	printk("RX Multicast Pkts:");
	rtk_dump_mib_type(STAT_IfInMulticastPkts);
	/* Get RX Broadcast Pkts */
	printk("RX Broadcast Pkts:");
	rtk_dump_mib_type(STAT_IfInBroadcastPkts);
	/* Get RX FCS Erros */
	printk("RX FCS Errors    :");
	rtk_dump_mib_type(STAT_Dot3StatsFCSErrors);
	/* Get RX Undersize Pkts */
	printk("RX Undersize Pkts:");
	rtk_dump_mib_type(STAT_EtherStatsUnderSizePkts);
	/* Get RX Discard Pkts */
	printk("RX Discard Pkts  :");
	rtk_dump_mib_type(STAT_Dot1dTpPortInDiscards);
	/* Get RX Fragments */
	printk("RX Fragments     :");
	rtk_dump_mib_type(STAT_EtherStatsFragments);
	/* Get RX Oversize Pkts */
	printk("RX Oversize Pkts :");
	rtk_dump_mib_type(STAT_EtherOversizeStats);
	/* Get RX Jabbers */
	printk("RX Jabbers       :");
	rtk_dump_mib_type(STAT_EtherStatsJabbers);
	/* Get RX Pause Frames */
	printk("RX Pause Frames  :");
	rtk_dump_mib_type(STAT_Dot3InPauseFrames);
	/* clear MIB */
	rtk_stat_global_reset();
}
EXPORT_SYMBOL(rtk_hal_dump_mib);

int rtk_hal_dump_vlan(void)
{
	rtk_vlan_cfg_t vlan;
	int i;

	printk("vid    portmap\n");
	for (i = 0; i < RTK_SW_VID_RANGE; i++) {
		rtk_vlan_get(i, &vlan);
		printk("%3d    ", i);
		printk("%c",
		       RTK_PORTMASK_IS_PORT_SET(vlan.mbr,
						UTP_PORT0) ? '1' : '-');
		printk("%c",
		       RTK_PORTMASK_IS_PORT_SET(vlan.mbr,
						UTP_PORT1) ? '1' : '-');
		printk("%c",
		       RTK_PORTMASK_IS_PORT_SET(vlan.mbr,
						UTP_PORT2) ? '1' : '-');
		printk("%c",
		       RTK_PORTMASK_IS_PORT_SET(vlan.mbr,
						UTP_PORT3) ? '1' : '-');
		printk("%c",
		       RTK_PORTMASK_IS_PORT_SET(vlan.mbr,
						UTP_PORT4) ? '1' : '-');
		printk("%c",
		       RTK_PORTMASK_IS_PORT_SET(vlan.mbr,
						EXT_PORT0) ? '1' : '-');
		printk("%c",
		       RTK_PORTMASK_IS_PORT_SET(vlan.mbr,
						EXT_PORT1) ? '1' : '-');
		printk("\n");
	}
	return 0;
}

void rtk_hal_clear_vlan(void)
{
	rtk_api_ret_t ret;

	ret =  rtk_vlan_reset();
    if (ret != RT_ERR_OK)
        printk("rtk_vlan_reset failed\n");
}

int rtk_hal_set_vlan(struct ra_switch_ioctl_data *data)
{
	rtk_vlan_cfg_t vlan;
	rtk_api_ret_t ret;
	int i;

	/* clear vlan entry first */
	memset(&vlan, 0x00, sizeof(rtk_vlan_cfg_t));
	RTK_PORTMASK_CLEAR(vlan.mbr);
	RTK_PORTMASK_CLEAR(vlan.untag);
	rtk_vlan_set(data->vid, &vlan);

	memset(&vlan, 0x00, sizeof(rtk_vlan_cfg_t));
	for (i = 0; i < 5; i++) {
		if (data->port_map & (1 << i)) {
			RTK_PORTMASK_PORT_SET(vlan.mbr, i);
			RTK_PORTMASK_PORT_SET(vlan.untag, i);
			rtk_vlan_portPvid_set(i, data->vid, 0);
		}
	}
	for (i = 0; i < 2; i++) {
		if (data->port_map & (1 << (i + 5))) {
			RTK_PORTMASK_PORT_SET(vlan.mbr, (i + EXT_PORT0));
			RTK_PORTMASK_PORT_SET(vlan.untag, (i + EXT_PORT0));
			rtk_vlan_portPvid_set((i + EXT_PORT0), data->vid, 0);
		}
	}
	vlan.ivl_en = 1;
	ret = rtk_vlan_set(data->vid, &vlan);

	return 0;
}

void rtk_hal_vlan_portpvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority)
{
	rtk_vlan_portPvid_set(port, pvid, priority);
}

int rtk_hal_set_ingress_rate(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;

	if (data->on_off == 1)
		ret =
		    rtk_rate_igrBandwidthCtrlRate_set(data->port, data->bw, 0,
						      1);
	else
		ret =
		    rtk_rate_igrBandwidthCtrlRate_set(data->port, 1048568, 0,
						      1);

	return ret;
}

int rtk_hal_set_egress_rate(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;

	if (data->on_off == 1)
		ret =
		    rtk_rate_egrBandwidthCtrlRate_set(data->port, data->bw, 1);
	else
		ret = rtk_rate_egrBandwidthCtrlRate_set(data->port, 1048568, 1);

	return ret;
}

void rtk_hal_dump_table(void)
{
	rtk_uint32 i;
	rtk_uint32 address = 0;
	rtk_l2_ucastAddr_t l2_data;
	rtk_l2_ipMcastAddr_t ipMcastAddr;

	printk("hash  port(0:17)   fid   vid  mac-address\n");
	while (1) {
		if (rtk_l2_addr_next_get(READMETHOD_NEXT_L2UC, UTP_PORT0, &address, &l2_data) != RT_ERR_OK) {
			break;
		} else {
			printk("%03x   ", l2_data.address);
			for (i = 0; i < 5; i++)
				if ( l2_data.port == i)
					printk("1");
				else
					printk("-");
			for (i = 16; i < 18; i++)
				if ( l2_data.port == i)
					printk("1");
				else
					printk("-");

			printk("      %2d", l2_data.fid);
			printk("  %4d", l2_data.cvid);
			printk("  %02x%02x%02x%02x%02x%02x\n", l2_data.mac.octet[0],
			l2_data.mac.octet[1], l2_data.mac.octet[2], l2_data.mac.octet[3],
			l2_data.mac.octet[4], l2_data.mac.octet[5]);
			address ++;
			}
	}

	address = 0;
	while (1) {
        if (rtk_l2_ipMcastAddr_next_get(&address, &ipMcastAddr) != RT_ERR_OK) {
            break;
        } else {
            printk("%03x   ", ipMcastAddr.address);
            for (i = 0; i < 5; i++)
                printk("%c", RTK_PORTMASK_IS_PORT_SET(ipMcastAddr.portmask, i) ? '1' : '-');
            for (i = 16; i < 18; i++)
                printk("%c", RTK_PORTMASK_IS_PORT_SET(ipMcastAddr.portmask, i) ? '1' : '-');
			printk("                ");
			printk("01005E%06x\n", (ipMcastAddr.dip & 0xefffff));
            address ++;
            }
    }
}

void rtk_hal_clear_table(void)
{
	rtk_api_ret_t ret;

	ret = rtk_l2_table_clear();
	if (ret != RT_ERR_OK)
		printk("rtk_l2_table_clear failed\n");
}

void rtk_hal_add_table(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_l2_ucastAddr_t l2_entry;
	rtk_mac_t mac;

	mac.octet[0] =data->mac[0];
	mac.octet[1] =data->mac[1];
	mac.octet[2] =data->mac[2];
	mac.octet[3] =data->mac[3];
	mac.octet[4] =data->mac[4];
	mac.octet[5] =data->mac[5];

	memset(&l2_entry, 0x00, sizeof(rtk_l2_ucastAddr_t));
	l2_entry.port = data->port;
	l2_entry.ivl = 1;
	l2_entry.cvid = data->vid;
	l2_entry.fid = 0;
	l2_entry.efid = 0;
	l2_entry.is_static = 1;
	ret = rtk_l2_addr_add(&mac, &l2_entry);
	if (ret != RT_ERR_OK)
		printk("rtk_hal_add_table failed\n");
}

void rtk_hal_del_table(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_l2_ucastAddr_t l2_entry;
	rtk_mac_t mac;

	mac.octet[0] =data->mac[0];
	mac.octet[1] =data->mac[1];
	mac.octet[2] =data->mac[2];
	mac.octet[3] =data->mac[3];
	mac.octet[4] =data->mac[4];
	mac.octet[5] =data->mac[5];

	memset(&l2_entry, 0x00, sizeof(rtk_l2_ucastAddr_t));
	l2_entry.port = data->port;
	l2_entry.ivl = 1;
	l2_entry.cvid = data->vid;
	l2_entry.fid = 0;
	l2_entry.efid = 0;
	ret = rtk_l2_addr_del(&mac, &l2_entry);
	if (ret != RT_ERR_OK)
		printk("rtk_hal_add_table failed\n");
}
void rtk_hal_get_phy_status(struct ra_switch_ioctl_data *data)
{
	rtk_port_linkStatus_t linkStatus;
	rtk_port_speed_t speed;
	rtk_port_duplex_t duplex;

    rtk_port_phyStatus_get(data->port, &linkStatus, &speed, &duplex);
    printk("Port%d Status:\n", data->port);
	if (linkStatus == 1) {
        printk("Link Up");
        if (speed == 0)
			printk(" 10M");
		else if (speed == 1)
			printk(" 100M");
		else if (speed == 2)
            printk(" 1000M");
        if (duplex == 0)
			printk(" Half Duplex\n");
		else
			printk(" Full Duplex\n");
	} else
		printk("Link Down\n");

}

void rtk_hal_set_port_mirror(struct ra_switch_ioctl_data *data)
{
	rtk_portmask_t rx_portmask;
	rtk_portmask_t tx_portmask;
	rtk_api_ret_t ret;
	int i;

	rtk_mirror_portIso_set(ENABLED);
	RTK_PORTMASK_CLEAR(rx_portmask);
	RTK_PORTMASK_CLEAR(tx_portmask);
    for (i = 0; i < 5; i++)
		if (data->rx_port_map & (1 << i))
			RTK_PORTMASK_PORT_SET(rx_portmask, i);
	for (i = 0; i < 2; i++)
		if (data->rx_port_map & (1 << (i + 5)))
			RTK_PORTMASK_PORT_SET(rx_portmask, (i + EXT_PORT0));

	RTK_PORTMASK_CLEAR(tx_portmask);
    for (i = 0; i < 5; i++)
        if (data->tx_port_map & (1 << i))
            RTK_PORTMASK_PORT_SET(tx_portmask, i);
    for (i = 0; i < 2; i++)
        if (data->tx_port_map & (1 << (i + 5)))
            RTK_PORTMASK_PORT_SET(tx_portmask, (i + EXT_PORT0));

    ret = rtk_mirror_portBased_set(data->port, &rx_portmask, &tx_portmask);
	if (!ret)
		printk("rtk_mirror_portBased_set success\n");
}

void rtk_hal_read_reg(struct ra_switch_ioctl_data *data)
{
	ret_t retVal;

	retVal = smi_read(data->reg_addr, &data->reg_val);
	if(retVal != RT_ERR_OK)
		printk("switch reg read failed\n");
	else
		printk("reg0x%x = 0x%x\n", data->reg_addr, data->reg_val);
}

void rtk_hal_write_reg(struct ra_switch_ioctl_data *data)
{
	ret_t retVal;

    retVal = smi_write(data->reg_addr, data->reg_val);
    if(retVal != RT_ERR_OK)
        printk("switch reg write failed\n");
    else
        printk("write switch reg0x%x 0x%x success\n", data->reg_addr, data->reg_val);
}

void rtk_hal_get_phy_reg(struct ra_switch_ioctl_data *data)
{
	ret_t retVal;
	rtk_port_phy_data_t Data;

	retVal = rtk_port_phyReg_get(data->port, data->reg_addr, &Data);
	if (retVal == RT_ERR_OK)
		printk("Get: phy[%d].reg[%d] = 0x%04x\n", data->port, data->reg_addr, Data);
	else
		printk("read phy reg failed\n");
}

void rtk_hal_set_phy_reg(struct ra_switch_ioctl_data *data)
{
	ret_t retVal;

	retVal = rtk_port_phyReg_set(data->port, data->reg_addr, data->reg_val);
	if (retVal == RT_ERR_OK)
		printk("Set: phy[%d].reg[%d] = 0x%04x\n", data->port, data->reg_addr, data->reg_val);
	else
		printk("write phy reg failed\n");
}
void rtk_hal_qos_en(struct ra_switch_ioctl_data *data)
{

	if (data->on_off == 1) {
		if (rtk_qos_init(8) != 0)
			printk("rtk_qos_init(8) failed\n");
	}
	else {
		if (rtk_qos_init(1) != 0)
            printk("rtk_qos_init(1) failed\n");
	}
}

void rtk_hal_qos_set_table2type(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_priority_select_t PriDec;

	/* write all pri to 0 */
	PriDec.port_pri = 0;
    PriDec.dot1q_pri = 0;
    PriDec.acl_pri = 0;
    PriDec.cvlan_pri = 0;
    PriDec.svlan_pri = 0;
    PriDec.dscp_pri = 0;
    PriDec.dmac_pri = 0;
    PriDec.smac_pri = 0;

	if (data->qos_type == 0)
		PriDec.port_pri = 1;
	else if (data->qos_type == 1)
		PriDec.dot1q_pri = 1;
	else if (data->qos_type == 2)
		PriDec.acl_pri = 1;
	else if (data->qos_type == 3)
		PriDec.dscp_pri = 1;
	else if (data->qos_type == 4)
		PriDec.cvlan_pri = 1;
	else if (data->qos_type == 5)
		PriDec.svlan_pri = 1;
	else if (data->qos_type == 6)
		PriDec.dmac_pri = 1;
	else if (data->qos_type == 7)
		PriDec.smac_pri = 1;

	if (data->qos_table_idx == 0)
		ret = rtk_qos_priSel_set(PRIDECTBL_IDX0, &PriDec);
	else
		ret = rtk_qos_priSel_set(PRIDECTBL_IDX1, &PriDec);

	if (ret != 0)
		printk("rtk_qos_priSel_set failed\n");

}

void rtk_hal_qos_get_table2type(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_priority_select_t PriDec;

	if (data->qos_table_idx == 0)
        ret = rtk_qos_priSel_get(PRIDECTBL_IDX0, &PriDec);
    else
        ret = rtk_qos_priSel_get(PRIDECTBL_IDX1, &PriDec);

	if (ret != 0)
        printk("rtk_qos_priSel_set failed\n");
    else {
		printk("port_pri  = %d\n", PriDec.port_pri);
		printk("dot1q_pri = %d\n", PriDec.dot1q_pri);
		printk("acl_pri   = %d\n", PriDec.acl_pri);
		printk("dscp_pri  = %d\n", PriDec.dscp_pri);
		printk("cvlan_pri = %d\n", PriDec.cvlan_pri);
		printk("svlan_pri = %d\n", PriDec.svlan_pri);
		printk("dmac_pri  = %d\n", PriDec.dmac_pri);
		printk("smac_pri  = %d\n", PriDec.smac_pri);
	}
}

void rtk_hal_qos_set_port2table(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;

	ret = rtk_qos_portPriSelIndex_set(data->port, data->qos_table_idx);
	if (ret != 0)
		printk("rtk_qos_portPriSelIndex_set failed\n");
}

void rtk_hal_qos_get_port2table(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_qos_priDecTbl_t Index;

	ret = rtk_qos_portPriSelIndex_get(data->port, &Index);
	if (ret != 0)
		printk("rtk_qos_portPriSelIndex_set failed\n");
	else
		printk("port%d belongs to table%d\n", data->port, Index);
}

void rtk_hal_qos_set_port2pri(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;

	ret = rtk_qos_portPri_set(data->port, data->qos_pri);
	if (ret != 0)
		printk("rtk_qos_portPri_set failed\n");
}

void rtk_hal_qos_get_port2pri(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_pri_t Int_pri;

	ret = rtk_qos_portPri_get(data->port, &Int_pri);
	if (ret != 0)
		printk("rtk_qos_portPri_set failed\n");
	else
		printk("port%d priority = %d\n", data->port, Int_pri);
}

void rtk_hal_qos_set_dscp2pri(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;

	ret = rtk_qos_dscpPriRemap_set(data->qos_dscp, data->qos_pri);
	if (ret != 0)
		printk("rtk_qos_dscpPriRemap_set failed\n");
}

void rtk_hal_qos_get_dscp2pri(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_pri_t Int_pri;

	ret = rtk_qos_dscpPriRemap_get(data->qos_dscp, &Int_pri);
	if (ret != 0)
		printk("rtk_qos_dscpPriRemap_set failed\n");
	else
		printk("dscp%d priority is %d\n", data->qos_dscp, Int_pri);
}

void rtk_hal_qos_set_pri2queue(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_qos_pri2queue_t pri2qid;

	ret = rtk_qos_priMap_get(8, &pri2qid);
	pri2qid.pri2queue[data->qos_queue_num] = data->qos_pri;
	ret = rtk_qos_priMap_set(8, &pri2qid);
	if (ret != 0)
		printk("rtk_qos_priMap_set failed\n");
}

void rtk_hal_qos_get_pri2queue(struct ra_switch_ioctl_data *data)
{
	int i;
	rtk_api_ret_t ret;
	rtk_qos_pri2queue_t pri2qid;

	ret = rtk_qos_priMap_get(8, &pri2qid);
	if (ret != 0)
		printk("rtk_qos_priMap_get failed\n");
	else {
		for (i = 0; i < 8; i++)
			printk("pri2qid.pri2queue[%d] = %d\n", i, pri2qid.pri2queue[i]);
	}
}

void rtk_hal_qos_set_queue_weight(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_qos_queue_weights_t qweights;

	ret = rtk_qos_schedulingQueue_get(data->port, &qweights);
	qweights.weights[data->qos_queue_num] = data->qos_weight;
	ret = rtk_qos_schedulingQueue_set(data->port, &qweights);
	if (ret != 0)
		printk("rtk_qos_schedulingQueue_set failed\n");
}

void rtk_hal_qos_get_queue_weight(struct ra_switch_ioctl_data *data)
{
	int i;
	rtk_api_ret_t ret;
	rtk_qos_queue_weights_t qweights;

	ret = rtk_qos_schedulingQueue_get(data->port, &qweights);
	if (ret != 0)
		printk("rtk_qos_schedulingQueue_get failed\n");
	else {
		printk("=== Port%d queue weight ===\n", data->port);
		for (i = 0; i < 8; i++)
			printk("qweights.weights[%d] = %d\n",i ,qweights.weights[i]);
	}
}

void rtk_hal_enable_igmpsnoop(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_portmask_t pmask;


	ret = rtk_igmp_init();
	if (data->on_off == 1) {
		RTK_PORTMASK_CLEAR(pmask);
		RTK_PORTMASK_PORT_SET(pmask, EXT_PORT0);
		ret |= rtk_igmp_static_router_port_set(&pmask);
		ret |= rtk_igmp_protocol_set(UTP_PORT4, PROTOCOL_IGMPv1, IGMP_ACTION_FORWARD);
		ret |= rtk_igmp_protocol_set(UTP_PORT4, PROTOCOL_IGMPv2, IGMP_ACTION_FORWARD);
		ret |= rtk_igmp_protocol_set(UTP_PORT4, PROTOCOL_MLDv1, IGMP_ACTION_FORWARD);
		ret |= rtk_igmp_protocol_set(EXT_PORT1, PROTOCOL_IGMPv1, IGMP_ACTION_FORWARD);
		ret |= rtk_igmp_protocol_set(EXT_PORT1, PROTOCOL_IGMPv2, IGMP_ACTION_FORWARD);
		ret |= rtk_igmp_protocol_set(EXT_PORT1, PROTOCOL_MLDv1, IGMP_ACTION_FORWARD);
		ret |= rtk_igmp_protocol_set(UTP_PORT0, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);
		ret |= rtk_igmp_protocol_set(UTP_PORT1, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);
		ret |= rtk_igmp_protocol_set(UTP_PORT2, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);
		ret |= rtk_igmp_protocol_set(UTP_PORT3, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);
		ret |= rtk_igmp_protocol_set(EXT_PORT0, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);

		ret |= rtk_leaky_vlan_set(LEAKY_IPMULTICAST, ENABLED);
		ret |= rtk_l2_ipMcastForwardRouterPort_set(DISABLED);
		/* drop unknown multicast packets*/
		/* ret |= rtk_trap_unknownMcastPktAction_set(UTP_PORT4, MCAST_IPV4, MCAST_ACTION_DROP);*/
	} else {
		RTK_PORTMASK_CLEAR(pmask);
        RTK_PORTMASK_PORT_SET(pmask, EXT_PORT0);
        RTK_PORTMASK_PORT_SET(pmask, EXT_PORT1);
		ret |= rtk_igmp_protocol_set(UTP_PORT0, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);
		ret |= rtk_igmp_protocol_set(UTP_PORT1, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);
		ret |= rtk_igmp_protocol_set(UTP_PORT2, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);
		ret |= rtk_igmp_protocol_set(UTP_PORT3, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);
		ret |= rtk_igmp_protocol_set(EXT_PORT0, PROTOCOL_IGMPv3, IGMP_ACTION_ASIC);

		ret |= rtk_igmp_static_router_port_set(&pmask);
	}
	if(ret != RT_ERR_OK)
		printk("enable switch igmpsnoop failed\n");
}

void rtk_hal_disable_igmpsnoop(void)
{
	if (rtk_igmp_state_set(DISABLED) != RT_ERR_OK)
		printk("Disable IGMP SNOOPING failed\n");
}

rtk_api_ret_t rtk_port_phyTestMode_set(rtk_port_t port, rtk_port_phy_test_mode_t mode)
{
    rtk_uint32          data, regData, i;
    rtk_api_ret_t       retVal;

    RTK_CHK_PORT_IS_UTP(port);

    if(mode >= PHY_TEST_MODE_END)
        return RT_ERR_INPUT;

    if( (mode == PHY_TEST_MODE_2) || (mode == PHY_TEST_MODE_3) )
        return RT_ERR_INPUT;

    if (PHY_TEST_MODE_NORMAL != mode)
    {
        /* Other port should be Normal mode */
        RTK_SCAN_ALL_LOG_PORT(i)
        {
            if(rtk_switch_isUtpPort(i) == RT_ERR_OK)
            {
                if(i != port)
                {
                    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(i), 9, &data)) != RT_ERR_OK)
                        return retVal;

                    if((data & 0xE000) != 0)
                        return RT_ERR_NOT_ALLOWED;
                }
            }
        }
    }

    if ((retVal = rtl8367c_getAsicPHYReg(rtk_switch_port_L2P_get(port), 9, &data)) != RT_ERR_OK)
        return retVal;

    data &= ~0xE000;
    data |= (mode << 13);
    if ((retVal = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(port), 9, data)) != RT_ERR_OK)
        return retVal;

    if (PHY_TEST_MODE_4 == mode)
    {
        if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
            return retVal;

        if((retVal = rtl8367c_getAsicReg(0x1300, &regData)) != RT_ERR_OK)
            return retVal;

        if( (regData == 0x0276) || (regData == 0x0597) )
        {
            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xbcc2, 0xF4F4)) != RT_ERR_OK)
                return retVal;
        }

        if( (regData == 0x6367) )
        {
            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xa436, 0x80c1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl8367c_setAsicPHYOCPReg(rtk_switch_port_L2P_get(port), 0xa438, 0xfe00)) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;
}

void rtk_hal_set_phy_test_mode(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;

    ret = rtk_port_phyTestMode_set(data->port, data->mode);
	if (ret != RT_ERR_OK)
		printk("rtk_port_phyTestMode_set failed\n");
	else
		printk("set port%d in test mode %d.\n", data->port, data->mode);
}

void rtk_hal_set_port_trunk(struct ra_switch_ioctl_data *data)
{

	rtk_api_ret_t ret;
	rtk_portmask_t member;
	int i;

	RTK_PORTMASK_CLEAR(member);
	for (i = 0; i < 4; i++) {
		if (data->port_map & (1 << i))
			RTK_PORTMASK_PORT_SET(member, i);
    }

	ret = rtk_trunk_port_set(TRUNK_GROUP0, &member);
	if (ret != RT_ERR_OK)
		printk("rtk_trunk_port_set failed\n");

	ret = rtk_trunk_distributionAlgorithm_set(RTK_WHOLE_SYSTEM, 0x7F);
	if (ret != RT_ERR_OK)
		printk("rtk_trunk_distributionAlgorithm_set failed\n");
}

void rtk_hal_vlan_tag(struct ra_switch_ioctl_data *data)
{
	rtk_api_ret_t ret;
	rtk_vlan_cfg_t vlan;

    ret = rtk_vlan_get(data->vid, &vlan);
	if (ret != RT_ERR_OK)
		printk("rtk_vlan_get failed\n");
	else {
		if (data->on_off == 0)
			RTK_PORTMASK_PORT_SET(vlan.untag, data->port);
		else
			RTK_PORTMASK_PORT_CLEAR(vlan.untag, data->port);

		ret = rtk_vlan_set(data->vid, &vlan);
		if (ret != RT_ERR_OK)
			printk("rtk_vlan_set failed\n");
	}
}

void rtk_hal_vlan_mode(struct ra_switch_ioctl_data *data)
{
	rtk_vlan_cfg_t vlan1, vlan2;
	rtk_api_ret_t ret;

	ret = rtk_vlan_get(1, &vlan1);
	if (ret != RT_ERR_OK)
		printk("rtk_vlan_get failed\n");

	ret = rtk_vlan_get(2, &vlan2);
	if (ret != RT_ERR_OK)
		printk("rtk_vlan_get failed\n");

	if (data->mode == 0) { //ivl
		vlan1.ivl_en = 1;
		vlan1.fid_msti = 0;
		rtk_vlan_set(1, &vlan1);
		vlan2.ivl_en = 1;
		vlan2.fid_msti = 0;
		rtk_vlan_set(2, &vlan2);
	} else if(data->mode == 1) {//svl
		vlan1.ivl_en = 0;
		vlan1.fid_msti = 0;
		rtk_vlan_set(1, &vlan1);
		vlan2.ivl_en = 0;
		vlan2.fid_msti = 1;
		rtk_vlan_set(2, &vlan2);
	} else
		printk("mode not supported\n");
}
