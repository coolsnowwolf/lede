#include <linux/uaccess.h>
#include <linux/trace_seq.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/u64_stats_sync.h>

#include  "./rtl8367c/include/rtk_switch.h"
#include  "./rtl8367c/include/port.h"
#include  "./rtl8367c/include/vlan.h"
#include  "./rtl8367c/include/rtl8367c_asicdrv_port.h"
#include  "./rtl8367c/include/stat.h"
#include  "./rtl8367c/include/l2.h"
#include  "./rtl8367c/include/smi.h"
#include  "./rtl8367c/include/mirror.h"
#include  "./rtl8367c/include/igmp.h"
#include  "./rtl8367c/include/leaky.h"

static struct proc_dir_entry *proc_reg_dir;
static struct proc_dir_entry *proc_esw_cnt;
static struct proc_dir_entry *proc_vlan_cnt;
static struct proc_dir_entry *proc_mac_tbl;
static struct proc_dir_entry *proc_reg;
static struct proc_dir_entry *proc_phyreg;
static struct proc_dir_entry *proc_mirror;
static struct proc_dir_entry *proc_igmp;

#define PROCREG_ESW_CNT         "esw_cnt"
#define PROCREG_VLAN            "vlan"
#define PROCREG_MAC_TBL            "mac_tbl"
#define PROCREG_REG            "reg"
#define PROCREG_PHYREG            "phyreg"
#define PROCREG_MIRROR            "mirror"
#define PROCREG_IGMP            "igmp"
#define PROCREG_DIR             "rtk_gsw"

#define RTK_SW_VID_RANGE        16

static void rtk_dump_mib_type(rtk_stat_port_type_t cntr_idx)
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
static void rtk_hal_dump_mib(void)
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

static int rtk_hal_dump_vlan(void)
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

static void rtk_hal_dump_table(void)
{
	rtk_uint32 i;
	rtk_uint32 address = 0;
	rtk_l2_ucastAddr_t l2_data;
	rtk_l2_ipMcastAddr_t ipMcastAddr;
	rtk_l2_age_time_t age_timout;

	rtk_l2_aging_get(&age_timout);
	printk("Mac table age timeout =%d\n",(unsigned int)age_timout);

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

static void rtk_hal_clear_table(void)
{
        rtk_api_ret_t ret;

        ret = rtk_l2_table_clear();
        if (ret != RT_ERR_OK)
                printk("rtk_l2_table_clear failed\n");
}

static void rtk_hal_read_reg(unsigned int reg_addr)
{
        ret_t retVal;
	 unsigned int reg_val;

        retVal = smi_read(reg_addr, &reg_val);

        if(retVal != RT_ERR_OK)
                printk("switch reg read failed\n");
        else
                printk("reg0x%x = 0x%x\n", reg_addr, reg_val);
}

static void rtk_hal_write_reg(unsigned int reg_addr , unsigned int reg_val)
{
        ret_t retVal;

    retVal = smi_write(reg_addr, reg_val);

    if(retVal != RT_ERR_OK)
        printk("switch reg write failed\n");
    else
        printk("write switch reg0x%x 0x%x success\n", reg_addr, reg_val);
}

static void rtk_hal_get_phy_reg(unsigned int port ,unsigned int reg_addr )
{
        ret_t retVal;
        rtk_port_phy_data_t Data;

        retVal = rtk_port_phyReg_get(port, reg_addr, &Data);
        if (retVal == RT_ERR_OK)
                printk("Get: phy[%d].reg[%d] = 0x%04x\n", port, reg_addr, Data);
        else
                printk("read phy reg failed\n");
}

static void rtk_hal_set_phy_reg(unsigned int port ,unsigned int reg_addr,unsigned int reg_val)
{
        ret_t retVal;

        retVal = rtk_port_phyReg_set(port, reg_addr, reg_val);
        if (retVal == RT_ERR_OK)
                printk("Set: phy[%d].reg[%d] = 0x%04x\n", port, reg_addr, reg_val);
        else
                printk("write phy reg failed\n");
}

static void rtk_hal_set_port_mirror(unsigned int port ,unsigned int rx_port_map,unsigned int tx_port_map)
{
        rtk_portmask_t rx_portmask;
        rtk_portmask_t tx_portmask;
        rtk_api_ret_t ret;
        int i;

        rtk_mirror_portIso_set(ENABLED);
        RTK_PORTMASK_CLEAR(rx_portmask);
        RTK_PORTMASK_CLEAR(tx_portmask);

	for (i = 0; i < 5; i++)
                if (rx_port_map & (1 << i))
                        RTK_PORTMASK_PORT_SET(rx_portmask, i);

        for (i = 0; i < 2; i++)
                if (rx_port_map & (1 << (i + 5)))
                        RTK_PORTMASK_PORT_SET(rx_portmask, (i + EXT_PORT0));

        RTK_PORTMASK_CLEAR(tx_portmask);

	for (i = 0; i < 5; i++)
		if (tx_port_map & (1 << i))
		           RTK_PORTMASK_PORT_SET(tx_portmask, i);

	for (i = 0; i < 2; i++)
		if (tx_port_map & (1 << (i + 5)))
			RTK_PORTMASK_PORT_SET(tx_portmask, (i + EXT_PORT0));

	ret = rtk_mirror_portBased_set(port, &rx_portmask, &tx_portmask);

        if (!ret)
                printk("rtk_mirror_portBased_set success\n");

}

static void rtk_hal_enable_igmpsnoop(int hw_on)
{
        rtk_api_ret_t ret;
        rtk_portmask_t pmask;

        ret = rtk_igmp_init();
        if (hw_on == 1) {
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

static void rtk_hal_disable_igmpsnoop(void)
{
        if (rtk_igmp_state_set(DISABLED) != RT_ERR_OK)
                printk("Disable IGMP SNOOPING failed\n");
}

static ssize_t mac_tbl_write(struct file *file,
                            const char __user *buffer, size_t count,
                            loff_t *data)
{
	rtk_hal_clear_table();

        return count;
}


static ssize_t phyreg_ops(struct file *file,
                            const char __user *buffer, size_t count,
                            loff_t *data)
{
        char buf[64];
	 unsigned int port;
        unsigned int offset;
        unsigned int val;

        memset(buf, 0, 64);

        if (copy_from_user(buf, buffer, count))
                return -EFAULT;


        if(buf[0] == 'w') {

                if(sscanf(buf, "w %d %x %x", &port,&offset,&val) == -1)
                        return -EFAULT;
                else
                        rtk_hal_set_phy_reg(port,offset,val);

        } else {

		if(sscanf(buf, "r %d %x",&port, &offset) == -1)
                        return -EFAULT;
                else
                        rtk_hal_get_phy_reg(port,offset);
        }

        return count;
}

static ssize_t reg_ops(struct file *file,
                            const char __user *buffer, size_t count,
                            loff_t *data)
{
        char buf[64];
        unsigned int offset;
        unsigned int val;

        memset(buf, 0, 64);

        if (copy_from_user(buf, buffer, count))
                return -EFAULT;


        if(buf[0] == 'w') {

                if(sscanf(buf, "w %x %x", &offset,&val) == -1)
                        return -EFAULT;
                else
                        rtk_hal_write_reg(offset,val);

        } else {

                if(sscanf(buf, "r %x", &offset) == -1)
                        return -EFAULT;
                else
                        rtk_hal_read_reg(offset);
        }

        return count;
}

static ssize_t mirror_ops(struct file *file,
                            const char __user *buffer, size_t count,
                            loff_t *data)
{
        char buf[64];
	 unsigned int port;
        unsigned int tx_map,rx_map;

        memset(buf, 0, 64);

        if (copy_from_user(buf, buffer, count))
                return -EFAULT;

	if(sscanf(buf, "%d %x %x", &port,&rx_map,&tx_map) == -1)
		return -EFAULT;
	else
		rtk_hal_set_port_mirror(port,rx_map,tx_map);

        return count;
}


static ssize_t igmp_ops(struct file *file,
                            const char __user *buffer, size_t count,
                            loff_t *data)
{
        char buf[8];
        unsigned int ops;

        if (copy_from_user(buf, buffer, count))
                return -EFAULT;

	if(sscanf(buf, "%d", &ops) == -1)
		return -EFAULT;

        if(ops == 0)
                rtk_hal_disable_igmpsnoop();
	else if (ops == 1)
		rtk_hal_enable_igmpsnoop(0);
	else //hw igmp
		rtk_hal_enable_igmpsnoop(1);

        return count;
}


static int esw_cnt_read(struct seq_file *seq, void *v)
{
	rtk_hal_dump_mib();
	return 0;
}

static int vlan_read(struct seq_file *seq, void *v)
{
	rtk_hal_dump_vlan();
	return 0;
}

static int mac_tbl_read(struct seq_file *seq, void *v)
{
	rtk_hal_dump_table();
	return 0;
}

static int reg_show(struct seq_file *seq, void *v)
{
	return 0;
}

static int phyreg_show(struct seq_file *seq, void *v)
{
	return 0;
}

static int mirror_show(struct seq_file *seq, void *v)
{
	return 0;
}

static int igmp_show(struct seq_file *seq, void *v)
{
	return 0;
}

static int switch_count_open(struct inode *inode, struct file *file)
{
	return single_open(file, esw_cnt_read, 0);
}

static int switch_vlan_open(struct inode *inode, struct file *file)
{
	return single_open(file, vlan_read, 0);
}

static int mac_tbl_open(struct inode *inode, struct file *file)
{
	return single_open(file, mac_tbl_read, 0);
}

static int reg_open(struct inode *inode, struct file *file)
{
	return single_open(file, reg_show, 0);
}

static int phyreg_open(struct inode *inode, struct file *file)
{
	return single_open(file, phyreg_show, 0);
}

static int mirror_open(struct inode *inode, struct file *file)
{
	return single_open(file, mirror_show, 0);
}

static int igmp_open(struct inode *inode, struct file *file)
{
	return single_open(file, igmp_show, 0);
}


static const struct proc_ops switch_count_fops = {
	.proc_open = switch_count_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release
};

static const struct proc_ops switch_vlan_fops = {
	.proc_open = switch_vlan_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release
};

static const struct proc_ops mac_tbl_fops = {
	.proc_open = mac_tbl_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_write = mac_tbl_write,
	.proc_release = single_release
};

static const struct proc_ops reg_fops = {
	.proc_open = reg_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_write = reg_ops,
	.proc_release = single_release
};

static const struct proc_ops phyreg_fops = {
	.proc_open = phyreg_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_write = phyreg_ops,
	.proc_release = single_release
};

static const struct proc_ops mirror_fops = {
	.proc_open = mirror_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_write = mirror_ops,
	.proc_release = single_release
};

static const struct proc_ops igmp_fops = {
	.proc_open = igmp_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_write = igmp_ops,
	.proc_release = single_release
};

int gsw_debug_proc_init(void)
{

	if (!proc_reg_dir)
		proc_reg_dir = proc_mkdir(PROCREG_DIR, NULL);

	proc_esw_cnt =
	proc_create(PROCREG_ESW_CNT, 0, proc_reg_dir, &switch_count_fops);

	if (!proc_esw_cnt)
		pr_err("!! FAIL to create %s PROC !!\n", PROCREG_ESW_CNT);

	proc_vlan_cnt =
	proc_create(PROCREG_VLAN, 0, proc_reg_dir, &switch_vlan_fops);

	if (!proc_vlan_cnt)
		pr_err("!! FAIL to create %s PROC !!\n", PROCREG_VLAN);

	proc_mac_tbl =
	proc_create(PROCREG_MAC_TBL, 0, proc_reg_dir, &mac_tbl_fops);

	if (!proc_mac_tbl)
		pr_err("!! FAIL to create %s PROC !!\n", PROCREG_MAC_TBL);

	proc_reg =
	proc_create(PROCREG_REG, 0, proc_reg_dir, &reg_fops);

	if (!proc_reg)
		pr_err("!! FAIL to create %s PROC !!\n", PROCREG_REG);

	proc_phyreg =
	proc_create(PROCREG_PHYREG, 0, proc_reg_dir, &phyreg_fops);

	if (!proc_phyreg)
		pr_err("!! FAIL to create %s PROC !!\n", PROCREG_PHYREG);

	proc_mirror =
	proc_create(PROCREG_MIRROR, 0, proc_reg_dir, &mirror_fops);

	if (!proc_mirror)
		pr_err("!! FAIL to create %s PROC !!\n", PROCREG_MIRROR);

	proc_igmp =
	proc_create(PROCREG_IGMP, 0, proc_reg_dir, &igmp_fops);

	if (!proc_igmp)
		pr_err("!! FAIL to create %s PROC !!\n", PROCREG_IGMP);

	return 0;
}

void gsw_debug_proc_exit(void)
{
	if (proc_esw_cnt)
		remove_proc_entry(PROCREG_ESW_CNT, proc_reg_dir);
}


