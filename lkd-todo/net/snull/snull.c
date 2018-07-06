/*
 * snull.c --  the Simple Network Utility
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.
 * based on snull from
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": %s: " fmt, __func__

#include <linux/etherdevice.h>
#include <linux/errno.h>
#include <linux/if_ether.h>
#include <linux/in.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netdevice.h>
#include <linux/ratelimit.h>
#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/tcp.h>
#include <linux/types.h>

#include "snull.h"

#define DEVSNULL_DEVICE_NUM	2

/* Transmitter lockup simulation, normally disabled. */
static unsigned long lockup;
module_param(lockup, ulong, 0);

static int timeout = SNULL_TIMEOUT;
module_param(timeout, int, 0);

/* Do we run in NAPI mode? */
static int use_napi;
module_param(use_napi, int, 0);

/* A structure representing an in-flight packet. */
struct snull_packet {
	struct snull_packet *next;
	struct net_device *dev;
	unsigned int datalen;
	u8 data[ETH_DATA_LEN];
};

static int pool_size = 8;
module_param(pool_size, int, 0);

/* This structure is private to each device. It is used to pass
 * packets in and out, so there is place for a packet
 */

struct snull_priv {
	struct net_device_stats stats;
	int status;
	struct snull_packet *ppool;
	struct snull_packet *rx_queue;  /* List of incoming packets */
	bool rx_ints_flag;
	unsigned int tx_packetlen;
	u8 *tx_packetdata;
	struct sk_buff *skb;
	spinlock_t lock;	/* spinlock to protect netdev data e.g. pool */
	struct net_device *dev;
	struct napi_struct napi;
};

/* The devices */
static struct net_device *snull_devs[DEVSNULL_DEVICE_NUM];

static void (*snull_interrupt)(struct net_device *);

/* Set up a device's packet pool. */
static int snull_setup_pool(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	int i;
	struct snull_packet *pkt;

	printk(KERN_INFO "%s\n", __func__);
	priv->ppool = NULL;
	for (i = 0; i < pool_size; i++) {
		pkt = kmalloc(sizeof(*pkt), GFP_KERNEL);
		if (!pkt) {
			netdev_notice(dev, "allocating packet pool failed\n");
			return -ENOMEM;
		}
		pkt->dev = dev;
		pkt->next = priv->ppool;
		priv->ppool = pkt;
	}

	return 0;
}

static void snull_teardown_pool(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	struct snull_packet *pkt;

	printk(KERN_INFO "%s\n", __func__);
	while ((pkt = priv->ppool)) {
		priv->ppool = pkt->next;
		kfree(pkt);
		/* FIXME - in-flight packets ? */
	}
}

/* Buffer/pool management. */
struct snull_packet *snull_get_tx_buffer(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	unsigned long flags;
	struct snull_packet *pkt;
    
	printk(KERN_INFO "%s\n", __func__);
	spin_lock_irqsave(&priv->lock, flags);
	pkt = priv->ppool;
	priv->ppool = pkt->next;
	if (priv->ppool == NULL) {
		printk (KERN_INFO "Pool empty\n");
		netif_stop_queue(dev);
	}
	spin_unlock_irqrestore(&priv->lock, flags);
	return pkt;
}

static void snull_release_buffer(struct snull_packet *pkt)
{
	unsigned long flags;
	struct snull_priv *priv = netdev_priv(pkt->dev);

	printk(KERN_INFO "%s\n", __func__);
	spin_lock_irqsave(&priv->lock, flags);
	pkt->next = priv->ppool;
	priv->ppool = pkt;
	spin_unlock_irqrestore(&priv->lock, flags);
	if (netif_queue_stopped(pkt->dev) && pkt->next == NULL)
		netif_wake_queue(pkt->dev);
}

static void snull_enqueue_buf(struct net_device *dev, struct snull_packet *pkt)
{
	unsigned long flags;
	struct snull_priv *priv = netdev_priv(dev);

	printk(KERN_INFO "%s\n", __func__);
	spin_lock_irqsave(&priv->lock, flags);
	pkt->next = priv->rx_queue;  /* FIXME - misorders packets */
	priv->rx_queue = pkt;
	spin_unlock_irqrestore(&priv->lock, flags);
}

static struct snull_packet *snull_dequeue_buf(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	struct snull_packet *pkt;
	unsigned long flags;

	printk(KERN_INFO "%s\n", __func__);
	spin_lock_irqsave(&priv->lock, flags);
	pkt = priv->rx_queue;
	if (pkt != NULL)
		priv->rx_queue = pkt->next;
	spin_unlock_irqrestore(&priv->lock, flags);
	return pkt;
}

/*This function is called to fill up an eth header, since arp is not
 * available on the interface
 */
static int snull_header(struct sk_buff *skb, struct net_device *dev,
                unsigned short type, const void *daddr, const void *saddr,
                unsigned len)
{
	struct ethhdr *eth = (struct ethhdr *)skb_push(skb,ETH_HLEN);

	printk(KERN_INFO "%s\n", __func__);
	eth->h_proto = htons(type);
	memcpy(eth->h_source, saddr ? saddr : dev->dev_addr, dev->addr_len);
	memcpy(eth->h_dest,   daddr ? daddr : dev->dev_addr, dev->addr_len);
	eth->h_dest[ETH_ALEN-1]   ^= 0x01;   /* dest is us xor 1 */
	
	/*Documentation/printk-formats.txt*/
	printk(KERN_INFO "hdr->h_dest 0x%pM\n", eth->h_dest);	
	printk(KERN_INFO "eth->h_source 0x%pM\n", eth->h_source);
	printk(KERN_INFO "MACPROTO=%04x\n", eth->h_proto);
	return (dev->hard_header_len);
}

/* Open and close */
static int snull_open(struct net_device *dev)
{
	printk(KERN_INFO "%s\n", __func__);
	/* request_region(), request_irq(), ....  (like fops->open) */

	/* 
	 * Assign the hardware address of the board: use "\0SNULx", where
	 * x is 0 or 1. The first byte is '\0' to avoid being a multicast
	 * address (the first byte of multicast addrs is odd).
	 */
	memcpy(dev->dev_addr, "\0SNUL0", ETH_ALEN);
	if (dev == snull_devs[1])
		dev->dev_addr[ETH_ALEN-1]++; /* \0SNUL1 */
	netif_start_queue(dev);
	return 0;
}

static int snull_release(struct net_device *dev)
{
    /* release ports, irq and such -- like fops->close */

	printk(KERN_INFO "%s\n", __func__);
	netif_stop_queue(dev); /* can't transmit any more */
	return 0;
}

/* Configuration changes (passed on by ifconfig) */
static int snull_config(struct net_device *dev, struct ifmap *map)
{
	if (dev->flags & IFF_UP) /* can't act on a running interface */
		return -EBUSY;

	printk(KERN_INFO "%s\n", __func__);
	/* Don't allow changing the I/O address */
	if (map->base_addr != dev->base_addr) {
		netdev_warn(dev, "snull: Can't change I/O address\n");
		return -EOPNOTSUPP;
	}

	/* Allow changing the IRQ */
	if (map->irq != dev->irq)
		dev->irq = map->irq;

	/* ignore other fields */
	return 0;
}

/* Receive a packet: retrieve, encapsulate and pass over to upper levels */
static void snull_rx(struct net_device *dev, struct snull_packet *pkt)
{
	struct sk_buff *skb;
	struct snull_priv *priv = netdev_priv(dev);

	printk(KERN_INFO "%s\n", __func__);
	/*
	 * The packet has been retrieved from the transmission
	 * medium. Build an skb around it, so upper layers can handle it
	 */
	skb = dev_alloc_skb(pkt->datalen + 2);
	if (!skb) {
		if (printk_ratelimit())
			printk(KERN_NOTICE "snull rx: low on mem - packet dropped\n");
		priv->stats.rx_dropped++;
		goto out;
	}
	skb_reserve(skb, 2); /* align IP on 16B boundary */
	memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);

	/* Write metadata, and then pass to the receive level */
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
	priv->stats.rx_packets++;
	priv->stats.rx_bytes += pkt->datalen;
	pr_debug("%s: %s: skb->protocol: 0x%X; len:%d\n", __func__,
		 dev->name, ntohs(skb->protocol), skb->len);
	netif_rx(skb);
  out:
	return;
}

/* The poll implementation. */
static int snull_poll(struct napi_struct *napi, int budget)
{
	int npackets = 0;
	struct sk_buff *skb;
	struct snull_priv *priv = container_of(napi, struct snull_priv, napi);
	struct net_device *dev = priv->dev;
	struct snull_packet *pkt;

	printk(KERN_INFO "%s\n", __func__);
	while (npackets < budget && priv->rx_queue) {
		pkt = snull_dequeue_buf(dev);
		skb = dev_alloc_skb(pkt->datalen + 2);
		if (!skb) {
			printk_ratelimited(KERN_NOTICE "snull: packet dropped\n");
			priv->stats.rx_dropped++;
			snull_release_buffer(pkt);
			continue;
		}
		skb_reserve(skb, 2); /* align IP on 16B boundary */  
		memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);
		skb->dev = dev;
		skb->protocol = eth_type_trans(skb, dev);
		skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
		netif_receive_skb(skb);

		/* Maintain stats */
		npackets++;
		priv->stats.rx_packets++;
		priv->stats.rx_bytes += pkt->datalen;
		snull_release_buffer(pkt);
	}
	/* If we processed all packets, we're done; tell the kernel and reenable ints */
	if (!priv->rx_queue) {
		napi_complete(napi);
		priv->rx_ints_flag = true;	/* enable receive interrupts */
		return 0;
	}
	/* We couldn't process everything. */
	return npackets;
}

/* The typical interrupt entry point */
static void snull_regular_interrupt(struct net_device *dev)
{
	unsigned int statusword;
	struct snull_packet *pkt = NULL;
	struct snull_priv *priv = netdev_priv(dev);

	printk(KERN_INFO "%s\n", __func__);
	/* Lock the device */
	spin_lock(&priv->lock);
	/* retrieve statusword: real netdevices use I/O instructions */
	statusword = priv->status;
	priv->status = 0;
	if (statusword & SNULL_RX_INTR) {
		/* send it to snull_rx for handling */
		pkt = priv->rx_queue;
		if (pkt) {
			priv->rx_queue = pkt->next;
			snull_rx(dev, pkt);
		}
	}
	if (statusword & SNULL_TX_INTR) {
		/* a transmission is over: free the skb */
		priv->stats.tx_packets++;
		priv->stats.tx_bytes += priv->tx_packetlen;
		dev_kfree_skb(priv->skb);
	}

	/* Unlock the device and we are done */
	spin_unlock(&priv->lock);
	if (pkt)
		snull_release_buffer(pkt); /* Do this outside the lock! */
	return;
}

/* A NAPI interrupt handler. */
static void snull_napi_interrupt(struct net_device *dev)
{
	unsigned int statusword;
	struct snull_priv *priv = netdev_priv(dev);

	printk(KERN_INFO "%s\n", __func__);
	/* Lock the device */
	spin_lock(&priv->lock);
	/* retrieve statusword: real netdevices use I/O instructions */
	statusword = priv->status;
	priv->status = 0;
	if (statusword & SNULL_RX_INTR) {
		priv->rx_ints_flag = false;  /* Disable further interrupts */
		napi_schedule(&priv->napi);
	}
	if (statusword & SNULL_TX_INTR) {
        	/* a transmission is over: free the skb */
		priv->stats.tx_packets++;
		priv->stats.tx_bytes += priv->tx_packetlen;
		dev_kfree_skb(priv->skb);
	}

	/* Unlock the device and we are done */
	spin_unlock(&priv->lock);
}

/* Transmit a packet (low level interface) */
static void snull_hw_tx(char *buf, int len, struct net_device *dev)
{
	/*
	 * This function deals with hw details. This interface loops
	 * back the packet to the other snull interface (if any).
	 * In other words, this function implements the snull behaviour,
	 * while all other procedures are rather device-independent
	 */
	struct iphdr *ih;
	struct net_device *dest;
	struct snull_priv *priv;
	u32 *saddr, *daddr;
	struct snull_packet *tx_buffer;
    
	printk(KERN_INFO "%s: Transmit a packet (low level interface)\n", __func__);
	/* I am paranoid. Ain't I? */
	if (len < sizeof(struct ethhdr) + sizeof(struct iphdr)) {
		printk("snull: Hmm... packet too short (%i octets)\n",
				len);
		return;
	}

	if (0) { /* enable this conditional to look at the data */
		int i;
		printk("len is %i\n" KERN_DEBUG "data:",len);
		for (i=14 ; i<len; i++)
			printk(" %02x",buf[i]&0xff);
		printk("\n");
	}
	/*
	 * Ethhdr is 14 bytes, but the kernel arranges for iphdr
	 * to be aligned (i.e., ethhdr is unaligned)
	 */
	ih = (struct iphdr *)(buf+sizeof(struct ethhdr));
	saddr = &ih->saddr;
	daddr = &ih->daddr;

	((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
	((u8 *)daddr)[2] ^= 1;

	ih->check = 0;         /* and rebuild the checksum (ip needs it) */
	ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
	
	printk("%08x:%05i --> %08x:%05i\n", ntohl(ih->saddr),
		ntohs(((struct tcphdr *)(ih+1))->source),
		 ntohl(ih->daddr),ntohs(((struct tcphdr *)(ih+1))->dest));

	printk("%08x:%05i <-- %08x:%05i\n", ntohl(ih->daddr),
		ntohs(((struct tcphdr *)(ih+1))->dest), 
		ntohl(ih->saddr),ntohs(((struct tcphdr *)(ih+1))->source));

	if (dev == snull_devs[0])
		printk("%08x:%05i --> %08x:%05i\n",
				ntohl(ih->saddr),ntohs(((struct tcphdr *)(ih+1))->source),
				ntohl(ih->daddr),ntohs(((struct tcphdr *)(ih+1))->dest));
	else
		printk("%08x:%05i <-- %08x:%05i\n",
				ntohl(ih->daddr),ntohs(((struct tcphdr *)(ih+1))->dest),
				ntohl(ih->saddr),ntohs(((struct tcphdr *)(ih+1))->source));

	/*
	 * Ok, now the packet is ready for transmission: first simulate a
	 * receive interrupt on the twin device, then  a
	 * transmission-done on the transmitting device
	 */
	dest = snull_devs[dev == snull_devs[0] ? 1 : 0];

	priv = netdev_priv(dest);
	tx_buffer = snull_get_tx_buffer(dev);
	tx_buffer->datalen = len;
	memcpy(tx_buffer->data, buf, len);
	snull_enqueue_buf(dest, tx_buffer);
	if (priv->rx_ints_flag) {
		priv->status |= SNULL_RX_INTR;
		snull_interrupt(dest);
	}

	priv = netdev_priv(dev);
	priv->tx_packetlen = len;
	priv->tx_packetdata = buf;
	priv->status |= SNULL_TX_INTR;
	if (lockup && ((priv->stats.tx_packets + 1) % lockup) == 0) {
        	/* Simulate a dropped transmit interrupt */
		netif_stop_queue(dev);
		PDEBUG("Simulate lockup at %ld, txp %ld\n", jiffies,
				(unsigned long) priv->stats.tx_packets);
	}
	else
		snull_interrupt(dev);
}

/* Transmit a packet (called by the kernel) */
static int snull_tx(struct sk_buff *skb, struct net_device *dev)
{
	int len;
	char *data, shortpkt[ETH_ZLEN];
	struct snull_priv *priv = netdev_priv(dev);
	
	printk(KERN_INFO "%s:Transmit a packet (called by the kernel)\n", __func__);
	data = skb->data;
	len = skb->len;
	if (len < ETH_ZLEN) {
		memset(shortpkt, 0, ETH_ZLEN);
		memcpy(shortpkt, skb->data, skb->len);
		len = ETH_ZLEN;
		data = shortpkt;
	}
	dev->trans_start = jiffies; /* save the timestamp */

	/* Remember the skb, so we can free it at interrupt time */
	priv->skb = skb;

	/* actual deliver of data is device-specific, and not shown here */
	snull_hw_tx(data, len, dev);

	return 0; /* Our simple device can not fail */
}

/* Deal with a transmit timeout. */
static void snull_tx_timeout (struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);

	printk("Transmit timeout at %ld, latency %ld\n", jiffies,
		jiffies - dev->trans_start);
        /* Simulate a transmission interrupt to get things moving */
	priv->status = SNULL_TX_INTR;
	snull_interrupt(dev);
	priv->stats.tx_errors++;
	netif_wake_queue(dev);
	return;
}

/* Ioctl commands */
static int snull_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	PDEBUG("ioctl\n");
	return 0;
}

/* Return statistics to the caller */
static struct net_device_stats *snull_stats(struct net_device *dev)
{
	struct snull_priv *priv = netdev_priv(dev);
	printk(KERN_INFO "%s\n", __func__);
	return &priv->stats;
}

static const struct header_ops snull_header_ops = {
        .create  = snull_header
};

static const struct net_device_ops snull_netdev_ops = {
	.ndo_open            = snull_open,
	.ndo_stop            = snull_release,
	.ndo_start_xmit      = snull_tx,
	.ndo_do_ioctl        = snull_ioctl,
	.ndo_set_config      = snull_config,
	.ndo_get_stats       = snull_stats,
	.ndo_change_mtu      = eth_change_mtu,
	.ndo_tx_timeout      = snull_tx_timeout
};

/* The init function (sometimes called probe). It is invoked by register_netdev() */
static void snull_init(struct net_device *dev)
{
	/*
	 * initialize the priv field. This encloses the statistics and a few private fields.
	 */
	struct snull_priv *priv = netdev_priv(dev);

	printk(KERN_INFO "%s\n", __func__);
    	/* 
	 * Then, assign other fields in dev, using ether_setup() and some
	 * hand assignments
	 */
	ether_setup(dev); /* assign some of the fields */
	dev->watchdog_timeo = timeout;
	dev->netdev_ops = &snull_netdev_ops;
	dev->header_ops = &snull_header_ops;
	/* keep the default flags, just add NOARP */
	dev->flags           |= IFF_NOARP;
	dev->features        |= NETIF_F_HW_CSUM;

	memset(priv, 0, sizeof(*priv));
	if (use_napi) {
		printk(KERN_INFO "%s: netif_napi_add done\n", __func__);
		netif_napi_add(dev, &priv->napi, snull_poll,2);
	}
	spin_lock_init(&priv->lock);
	printk(KERN_INFO "%s: enable recv interrupts\n", __func__);
	priv->rx_ints_flag = true;		/* enable receive interrupts */
}

/* Finally, the module stuff */
static void snull_cleanup(void)
{
	int i;

	printk(KERN_INFO "%s\n", __func__);
	for (i = 0; i < DEVSNULL_DEVICE_NUM; i++) {
		if (snull_devs[i]) {
			if (snull_devs[i]->ifindex)
				unregister_netdev(snull_devs[i]);

			snull_teardown_pool(snull_devs[i]);
			free_netdev(snull_devs[i]);
		}
	}
	return;
}

static int snull_init_module(void)
{
	int i, ret;

	printk(KERN_INFO "%s\n", __func__);
	snull_interrupt = use_napi ? snull_napi_interrupt : snull_regular_interrupt;

	for (i = 0; i < DEVSNULL_DEVICE_NUM; i++) {
		/* Allocate the devices */
		snull_devs[i] = alloc_netdev(sizeof(struct snull_priv),
					      "sn%d", NET_NAME_UNKNOWN,
					      snull_init);
		if (!snull_devs[i]) {
			ret = -ENOMEM;
			break;
		}

		ret = snull_setup_pool(snull_devs[i]);
		if (ret < 0)
			break;

		ret = register_netdev(snull_devs[i]);
		if (ret) {
			netdev_err(snull_devs[i],
				   "inc: error %i registering device \"%s\"\n",
				    ret, snull_devs[i]->name);
			/* Make forcibly ifindex zero */
			if (snull_devs[i]->ifindex)
				snull_devs[i]->ifindex = 0;
			break;
		}
	}

	if (ret)
		snull_cleanup();
	return ret;
}

module_init(snull_init_module);
module_exit(snull_cleanup);

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");
