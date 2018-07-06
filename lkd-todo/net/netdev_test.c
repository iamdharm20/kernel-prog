/*
 * Building a Transmitting Network Driver
 *
 * Extend your stub network device driver to include a transmission
 * function, which means supplying a method for
 * ndo_start_xmit().
 *
 *   insmod module/netdev_test.ko
 *   ifconfig netdev up 192.168.0.10
 *   ping -I netdev localhost
 *       or
 *   ping -bI netdev 192.168..0.10
 * 
 * make client_lo and make server_lo
 * run executable server_lo and client_lo for testing.
 *
 * Make sure your chosen address is not being used by anything else.
*/

#include <linux/module.h>     /* Needed by all modules */
#include <linux/kernel.h>     /* Needed for KERN_INFO */
#include <linux/init.h>       /* Needed for the macros */
#include <linux/etherdevice.h>/* eth_change_mtu */
#include <linux/errno.h>
#include <linux/moduleparam.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>

static int timeout = 5;
module_param(timeout, int, 0);

/* The devices */
static struct net_device *ndevs;

struct netdev_packet {
        struct net_device *dev;
        unsigned int datalen;
        u8 data[ETH_DATA_LEN];
};

/* This structure is private to each device. It is used to pass
 * packets in and out, so there is place for a packet
 */

struct netdev_priv {
        struct net_device_stats stats;
        struct sk_buff *skb;
        struct net_device *dev;
};

static unsigned int inet_addr(char *str)
{
	int a, b, c, d;
	char arr[4];

	if (sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
		return 0;

	arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d;
	return *(unsigned int *)arr;
}

static int netdev_configure(struct net_device *ndev, __be32 addr)
{
	printk(KERN_INFO "Hit: %s: %s\n", __func__, ndev->name);
	#if 0
        struct ifreq ir;
        struct sockaddr_in *sin = (void *)&ir.ifr_ifru.ifru_addr;
        mm_segment_t oldfs = get_fs();
        struct net *net = dev_net(ndev);
        int res;

        memset(&ir, 0, sizeof(ir));
        strcpy(ir.ifr_ifrn.ifrn_name, ndev->name);
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = addr;
        sin->sin_port = 0;

        set_fs(get_ds());
        res = devinet_ioctl(net, SIOCSIFADDR, (struct ifreq __user *)&ir);
        set_fs(oldfs);

        if (res) {
                dev_err(&ndev->dev, "SIOCSIFADDR failed on %s\n", ndev->name);
                return res;
        }

        ir.ifr_ifru.ifru_flags = ndev->flags | IFF_UP;

        set_fs(get_ds());
        res = devinet_ioctl(net, SIOCSIFFLAGS, (struct ifreq __user *)&ir);
        set_fs(oldfs);

        if (res < 0) {
                dev_err(&ndev->dev, "Failed to activate %s\n", ndev->name);
                return res;
        }

        dev_info(&ndev->dev, "%pI4 up\n", &addr);

	#endif
        return 0;
}

/* Open and close */
static int netdev_open(struct net_device *dev)
{
	printk(KERN_INFO "Hit: %s: %s\n", __func__, dev->name);
	/* request_region(), request_irq(), ....  (like fops->open) */
	/* netif_start_queue() to tell the kernel that our interface is ready to operate packets*/
	/* start up the transmission queue */
	netif_start_queue(dev);

        return 0;
}

static int netdev_release(struct net_device *dev)
{
	printk(KERN_INFO "Hit: %s: %s\n", __func__, dev->name);
	/* release ports, irq and such -- like fops->close */
	/* netif_stop_queue() to tell the kernel that our interface no longer accepts packets */
	/* shutdown the transmission queue */
	netif_stop_queue(dev); /* can't transmit any more */

	return 0;
}

static void netdev_rx(struct sk_buff *skb, struct net_device *dev)
{
	struct netdev_priv *priv = netdev_priv(dev);
	/* just a loopback, already has the skb */
	printk(KERN_INFO "%s: I'm receiving a packet\n", __func__);

	#if 0
	/* https://stackoverflow.com/questions/29553990/print-tcp-packet-data */
	/* ----- Print all needed information from received TCP packet ------ */
	struct iphdr *iph;          /* IPv4 header */
	struct tcphdr *tcph;        /* TCP header */
	u16 sport, dport;           /* Source and destination ports */
	u32 saddr, daddr;           /* Source and destination addresses */
	unsigned char *user_data;   /* TCP data begin pointer */
	unsigned char *tail;        /* TCP data end pointer */
	unsigned char *it;          /* TCP data iterator */
	
	/* Network packet is empty, seems like some problem occurred. Skip it */
	if (!skb)
        	return NF_ACCEPT;

	iph = ip_hdr(skb);          /* get IP header */

	/* Skip if it's not TCP packet */
	if (iph->protocol != IPPROTO_TCP)
        	return NF_ACCEPT;

	tcph = tcp_hdr(skb);        /* get TCP header */

	/* Convert network endianness to host endiannes */
	saddr = ntohl(iph->saddr);
	daddr = ntohl(iph->daddr);
	sport = ntohs(tcph->source);
	dport = ntohs(tcph->dest);

	/* Watch only port of interest */
	if (sport != PTCP_WATCH_PORT)
        	return NF_ACCEPT;

	/* Calculate pointers for begin and end of TCP packet data */
	user_data = (unsigned char *)((unsigned char *)tcph + (tcph->doff * 4));
	tail = skb_tail_pointer(skb);

	/* ----- Print all needed information from received TCP packet ------ */
	/* Show only HTTP packets */
	if (user_data[0] != 'H' || user_data[1] != 'T' || user_data[2] != 'T' ||
		user_data[3] != 'P') {
		return NF_ACCEPT;
	}

	/* Print packet route */
	pr_debug("print_tcp: %pI4h:%d -> %pI4h:%d\n", &saddr, sport, &daddr, dport);

	/* Print TCP packet data (payload) */
	pr_debug("print_tcp: data:\n");
	for (it = user_data; it != tail; ++it) {
		char c = *(char *)it;
		if (c == '\0')
			break;

		printk("%c", c);
	}
	printk("\n\n");

	#endif
	printk("skb len: %d\n", skb->len);
	printk("user data: %s\n", skb->data);
	priv->stats.rx_packets++;
	//netif_rx(skb);
}

/* Transmit a packet (called by the kernel) */
static int netdev_tx(struct sk_buff *skb, struct net_device *dev)
{
/*
	Synchronization: __netif_tx_lock spinlock.

	When the driver sets NETIF_F_LLTX in dev->features this will be
	called without holding netif_tx_lock. In this case the driver
	has to lock by itself when needed.
	The locking there should also properly protect against
	set_rx_mode. WARNING: use of NETIF_F_LLTX is deprecated.
	Don't use it for new drivers.

	Context: Process with BHs disabled or BH (timer),
	         will be called with interrupts disabled by netconsole.

	Return codes: 
	o NETDEV_TX_OK everything ok. 
	o NETDEV_TX_BUSY Cannot transmit packet, try later 
	  Usually a bug, means queue start/stop flow control is broken in
	the driver. Note: the driver must NOT put the skb in its DMA ring.
	*/

        struct netdev_priv *priv = netdev_priv(dev);
	printk(KERN_INFO "Hit: %s: %s\n", __func__, dev->name);

	/* Remember the skb, so we can free it at interrupt time */
	priv->skb = skb;

	dev->trans_start = jiffies;
	printk(KERN_INFO "Sending packet :\n");

	printk("skb len: %d\n", skb->len);
	printk("user data: %s\n", priv->skb->data);

	#if 0
	int i = 0;
	/* print out 16 bytes per line */
	for (i = 0; i < skb->len; ++i) {
		#if 0
		if ((i & 0xf) == 0)
			printk(KERN_INFO "\n  ");
		printk(KERN_INFO "%02x ", skb->data[i]);
		#endif
	}
	printk(KERN_INFO "\n");
	#endif

	priv->stats.tx_packets++;

	/* loopback it */
	netdev_rx(skb, dev);

	//dev_kfree_skb(skb);
        
	return 0; /* Our simple device can not fail */
}

/* Deal with a transmit timeout. */
static void netdev_tx_timeout(struct net_device *dev)
{
	/* Synchronization: netif_tx_lock spinlock; all TX queues frozen.
	Context: BHs disabled
	Notes: netif_queue_stopped() is guaranteed true
	*/
	printk(KERN_INFO "Hit: %s: %s\n", __func__, dev->name);

}

/* Ioctl commands */
static int netdev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	printk(KERN_INFO "Hit: %s: %s\n", __func__, dev->name);
        return 0;
}

/* Configuration changes (passed on by ifconfig) */
static int netdev_config(struct net_device *dev, struct ifmap *map)
{
	printk(KERN_INFO "Hit: %s: %s\n", __func__, dev->name);

	if (dev->flags & IFF_UP) /* can't act on a running interface */
		return -EBUSY;

	/* Don't allow changing the I/O address */
	if (map->base_addr != dev->base_addr) {
		netdev_warn(dev, "devinc: Can't change I/O address\n");
		return -EOPNOTSUPP;
	}

	/* Allow changing the IRQ */
	if (map->irq != dev->irq)
		dev->irq = map->irq;

	/* ignore other fields */
	return 0;
}

/* Return statistics to the caller */
static struct net_device_stats *netdev_stats(struct net_device *dev)
{
        struct netdev_priv *priv = netdev_priv(dev);
	printk(KERN_INFO "Hit: %s: %s\n", __func__, dev->name);

        return &priv->stats;
}
/*
 * int (*ndo_open)(struct net_device *dev);
 *     This function is called when a network device transitions to the up
 *     state.
 * int (*ndo_stop)(struct net_device *dev);
 *     This function is called when a network device transitions to the down
 *     state.
 *
 * netdev_tx_t (*ndo_start_xmit)(struct sk_buff *skb,
 *                               struct net_device *dev);
 *	Called when a packet needs to be transmitted.
 *	Returns NETDEV_TX_OK.  Can return NETDEV_TX_BUSY, but you should stop
 *	the queue before that can happen; it's for obsolete devices and weird
 *	corner cases, but the stack really does a non-trivial amount
 *	of useless work if you return NETDEV_TX_BUSY.
 *	Required; cannot be NULL.
 *
 * int (*ndo_do_ioctl)(struct net_device *dev, struct ifreq *ifr, int cmd);
 *	Called when a user requests an ioctl which can't be handled by
 *	the generic interface code. If not defined ioctls return
 *	not supported error code.
 *
 * int (*ndo_set_config)(struct net_device *dev, struct ifmap *map);
 *	Used to set network devices bus interface parameters. This interface
 *	is retained for legacy reasons; new devices should use the bus
 *	interface (PCI) for low level management.
 *
 * int (*ndo_change_mtu)(struct net_device *dev, int new_mtu);
 *	Called when a user wants to change the Maximum Transfer Unit
 *	of a device. If not defined, any request to change MTU will
 *	will return an error.
 *
 * void (*ndo_tx_timeout)(struct net_device *dev);
 *	Callback used when the transmitter has not made any progress
 *	for dev->watchdog ticks.
 * * struct net_device_stats* (*ndo_get_stats)(struct net_device *dev);
 *	Called when a user wants to get the network device usage
 *	statistics. Drivers must do one of the following:
 *	1. Define @ndo_get_stats64 to fill in a zero-initialised
 *	   rtnl_link_stats64 structure passed by the caller.
 *	2. Define @ndo_get_stats to update a net_device_stats structure
 *	   (which should normally be dev->stats) and return a pointer to
 *	   it. The structure may be changed asynchronously only if each
 *	   field is written atomically.
 *	3. Update dev->stats asynchronously and atomically, and define
 *	   neither operation.
 *
*/
static const struct net_device_ops netdev_ops = {
        .ndo_open            = netdev_open,
        .ndo_stop            = netdev_release,
        .ndo_start_xmit      = netdev_tx,
        .ndo_tx_timeout      = netdev_tx_timeout,
	.ndo_do_ioctl        = netdev_ioctl,
        .ndo_set_config      = netdev_config,
        .ndo_get_stats       = netdev_stats,
        .ndo_change_mtu      = eth_change_mtu,
};

static ssize_t netdev_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk(KERN_INFO "%s\n", __func__);
	return 0;
}

static ssize_t netdev_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	printk(KERN_INFO "%s\n", __func__);
	return 0;
}
static DEVICE_ATTR(netdev_testfile, S_IRUGO | S_IWUSR, netdev_show, netdev_store);

/* The init function (sometimes called probe).
 * It is invoked by register_netdev()
 */
static void netdev_init(struct net_device *dev)
{
        /**
         * initialize the priv field. This encloses the statistics
         * and a few private fields.
         */
        struct netdev_priv *priv = netdev_priv(dev);
	char *local_addr = "192.168.0.10";		/* IP addr */
	int ret = 0;

	printk(KERN_INFO "%s: %s\n",__func__, dev->name);

	#if 0
	/* Fill in the MAC address with a phoney */
	for (j = 0; j < ETH_ALEN; ++j) {
		dev->dev_addr[j] = (char)j;
	}
	#endif

        /**
         * Then, assign other fields in dev, some
         * hand assignments
         */
	ether_setup(dev); /* assign some of the fields */

        dev->mtu                = ETH_DATA_LEN;
        dev->addr_len           = 2;
        dev->tx_queue_len       = 10;   /* Ethernet: 1000 */
	
	/* this method for calllback is also possible: dev->do_ioctl= netdev_ioctl;*/

        dev->watchdog_timeo = timeout;
        dev->netdev_ops = &netdev_ops;

        /* keep the default flags, just add NOARP */
        dev->flags           |= IFF_NOARP;
        dev->features		|= NETIF_F_HW_CSUM;

        memset(priv, 0, sizeof(*priv));

	if(local_addr) {
		ret = netdev_configure(priv->dev, inet_addr(local_addr));
		if (!ret)
			printk("netdev with ip :sudo ifconfig netdev 192.168.0.10\n");
	}

	/* Just for laughs, let's claim that we've seen 50 collisions. */
	priv->stats.collisions = 50;

}

/* Finally, the module stuff */
static void netdev_cleanup(void)
{

	printk(KERN_INFO "%s: Unloading transmitting network module\n", __func__);
	device_remove_file(&ndevs->dev, &dev_attr_netdev_testfile);

	if (ndevs) {
		unregister_netdev(ndevs);
		free_netdev(ndevs);
	}

}

static int __init netdev_init_module(void)
{
	int ret;

	printk(KERN_INFO "%s: loading transmitting network module\n", __func__);
	
	/* Allocate the devices */
	ndevs = alloc_netdev(sizeof(struct netdev_priv), "netdev", NET_NAME_UNKNOWN, netdev_init);
	if (!ndevs) {
		return -ENOMEM;
	}
	printk("alloc_netdev: done\n");

	/* If device has registered successfully, it will be freed on last use
	by free_netdev(). This is required to handle the pathologic case cleanly
	(example: rmmod netdev_test </sys/class/net/myeth/mtu )
	
	alloc_netdev_mqs()/alloc_netdev() reserve extra space for driver
	private data which gets freed when the network device is freed. If
	separately allocated data is attached to the network device
	(netdev_priv(dev)) then it is up to the module exit handler to free that.	
	*/
        ret = register_netdev(ndevs);
        if (ret) {
        	netdev_err(ndevs,"netdev: error %i registering device \"%s\"\n",ret, ndevs->name);
		goto cleanup;
        }
	printk(KERN_INFO "Succeeded in loading %s\n", dev_name(&ndevs->dev));

	ret = device_create_file(&ndevs->dev, &dev_attr_netdev_testfile);
	if (ret) {
		pr_err("netdevice attr create error: %d.\n", ret);
	}
	printk("/sys/class/net/netdev/netdev_testfile created\n");
	
cleanup:
	if(ret)
		netdev_cleanup();

	return ret;
}


module_init(netdev_init_module);
module_exit(netdev_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dharmender sharma");
/* The description -- see modinfo */
MODULE_DESCRIPTION("A simple Network device LKM!");
