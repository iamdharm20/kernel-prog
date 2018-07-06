tcpSockHack

    include/linux/skbuff.h
    net/socket.c
    net/core/sock.c
    drivers/net/loopback.c
    security/selinux/hooks.c


http://haifux.org/lectures/177/netLec.pdf
http://users.encs.concordia.ca/~bill/hspl/xtplinux/programflow.html
https://github.com/bgmerrell/driver-samples/tree/master/s_22
https://people.freedesktop.org/~narmstrong/meson_drm_doc/index.html

NAPI
Netfilter
iptables

The two most important structures of linux kernel network layer :
	– sk_buff (defined in include/linux/skbuff.h)
		sk_buff represents data and headers.

		sk_buff allocation is done with alloc_skb() or dev_alloc_skb();
		drivers use dev_alloc_skb();. (free by kfree_skb() and dev_kfree_skb().

		unsigned char* data : points to the current header.

		skb_pull(int len) – removes data from the start of a buffer by
		advancing data to data+len and by decreasing len.

		sk_buff includes 3 unions:
			transport_header (previously called h) – for layer 4, the transport
				layer (can include tcp header or udp header or icmp header, and  more)
			network_header – (previously called nh) for layer 3, the network
				layer (can include ip header or ipv6 header or arp header).
			mac_header – (previously called mac) for layer 2, the link layer.
				ETH_HLEN is 14, the size of ethernet header.

		How to get header using skb:
				skb_network_header(skb)
				skb_transport_header(skb)
				skb_mac_header(skb) return pointer to the header.

	– netdevice (defined in include/linux/netdevice.h)
		net_device represents a network interface card.
			Many times this is implemented using the private data of the
				device (the void *priv member of net_device);
			unsigned int mtu – Maximum Transmission Unit: the maximum
				size of frame the device can handle.
				Each protocol has mtu of its own; the default is 1500 for Ethernet.
			unsigned char dev_addr[MAX_ADDR_LEN] : the MAC address
				of the device (6 bytes).
			
when we create a network device the below file will be created:
~/gitweb/LKD/driver/net$ ls /sys/class/net/netdev/
addr_assign_type  carrier_changes  flags              link_mode         operstate       proto_down  tx_queue_len
address           dev_id           gro_flush_timeout  mtu               phys_port_id    queues      type
addr_len          dev_port         ifalias            name_assign_type  phys_port_name  speed       uevent
broadcast         dormant          ifindex            netdev_group      phys_switch_id  statistics
carrier           duplex           iflink             netdev_testfile   power           subsystem

netif_start_queue() to tell the kernel that our interface is ready to operate packets
netif_stop_queue() to tell the kernel that our interface no longer accepts packets

netif_{start|stop|wake}_subqueue() functions to manage each queue while the
device is still operational.

https://github.com/torvalds/linux/blob/master/Documentation/networking/netdevices.txt

dev_queue_xmit() and netif_rx().

netif_rx () put data into the processing queue and then return.
net_rx () is a subroutines of data reception, it is called by interrupt service routine.

struct monitoring_hdr {
    u8      version;
    u8      type;
    u8      reserved;
    u8      haddr_len;

    u32     clock;
} __packed;

Sender side:

    ...
    skb = alloc_skb(mtu, GFP_ATOMIC);
    if (unlikely(!skb))
            return;
    skb_reserve(skb, ll_hlen);
    skb_reset_network_header(skb);
    nwp = (struct monitoring_hdr *)skb_put(skb, hdr_len);
    /* ... Set up fields in struct monitoring_hdr ... */
    memcpy(skb_put(skb, dev->addr_len), src, dev->addr_len);
    memcpy(skb_put(skb, dev->addr_len), dst, dev->addr_len);
    ...

Receiver side:

   ...
   skb_reset_network_header(skb);
   nwp = (struct monitoring_hdr *)skb_network_header(skb);

   src = skb_pull(skb, nwp->haddr_len);
   dst = skb_pull(skb, nwp->haddr_len);

   skb_pull(skb, offsetof(struct monitoring_hdr, haddrs_begin));
   src = skb->data;
   dst = skb_pull(skb, nwp->haddr_len);

   printk("%02x:%02x...", src[0], src[1]...)

Networking kernel API's:
	struct socket *sock;

	of_find_node_by_path
	of_get_property
	inet_addr, htons,
	
	sock_create_kern
	kernel_bind
	kernel_connect

	htons(ulen);
	kernel_sendmsg

	ntohs()	
	memcpy, memmove
	kernel_recvmsg

	kernel_sock_shutdown(sock, SHUT_RDWR);
	sock_release(sock);

Socket Buffer Functions
        skb_queue_empty — check if a queue is empty
        kfree_skb — free an sk_buff
        skb_share_check — check if buffer is shared and if so clone it
        skb_queue_len — get queue length
        skb_queue_head — queue a buffer at the list head
        skb_queue_tail — queue a buffer at the list tail
        skb_dequeue — remove from the head of the queue
        skb_insert — insert a buffer
        skb_append — append a buffer
        skb_unlink — remove a buffer from a list
        skb_dequeue_tail — remove from the head of the queue
        skb_put — add data to a buffer
        skb_push — add data to the start of a buffer
        skb_pull — remove data from the start of a buffer
        skb_headroom — bytes at buffer head
        skb_tailroom — bytes at buffer end
        skb_reserve — adjust headroom
        skb_trim — remove end from a buffer
        skb_queue_purge — empty a list
        dev_alloc_skb — allocate an skbuff for sending
        alloc_skb — allocate a network buffer

	skb_reset_mac_header
	skb_network_header(skb)
	skb_transport_header(skb)
	skb_mac_header(skb) return pointer to the header.
	
Driver Support

        init_etherdev — Register ethernet device
        alloc_etherdev — Allocates and sets up an ethernet device
        dev_add_pack — add packet handler
        dev_remove_pack — remove packet handler
        dev_get_by_name — find a device by its name
        dev_get — test if a device exists
        dev_get_by_index — find a device by its ifindex
        dev_alloc_name — allocate a name for a device
        dev_alloc — allocate a network device and name
        netdev_state_change — device changes state
        dev_load — load a network module
        dev_open — prepare an interface for use.
        dev_close — shutdown an interface.
        register_netdevice_notifier — register a network notifier block
        unregister_netdevice_notifier — unregister a network notifier block
        dev_queue_xmit — transmit a buffer
        netif_rx — post buffer to the network code
        dev_ioctl — network device ioctl
        dev_new_index — allocate an ifindex
        netdev_finish_unregister — complete unregistration
        unregister_netdevice — remove device from the kernel

	
	netif_start_queue() to tell the kernel that our interface is ready to operate packets
	netif_stop_queue() to tell the kernel that our interface no longer accepts packets
	netdev_priv(struct net_device *ndev);
	
API's

	struct sockaddr_in;
	struct addrinfo;
	

	#include <netinet/in.h>
	
	uint32_t htonl(uint32_t hostlong);
	uint16_t htons(uint16_t hostshort);
	uint32_t ntohl(uint32_t netlong);
	uint16_t ntohs(uint16_t netshort);

inet_ntoa(), inet_aton(), inet_addr(), inet_ntop(), inet_pton():

	char *inet_ntoa(struct in_addr in);
	int inet_aton(const char *cp, struct in_addr *inp);
	in_addr_t inet_addr(const char *cp);

socket(), bind(), connect(), listen(), accept(), send, recv, close, poll, epoll, select, fcntl, 
getaddrinfo, freeaddrinfo, gethostbyname(), gethostbyaddr(), setsockopt(), getsockopt().
errno, perror, strerror
memset,memcpy, memmove, malloc.
