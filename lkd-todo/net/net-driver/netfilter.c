#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>          
#include <net/sock.h>
#include <linux/inet.h>
#include <linux/netfilter.h>
#include <uapi/linux/netfilter_ipv4.h> 
#include <linux/ip.h>             
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netdevice.h>

/*
http://amsekharkernel.blogspot.in/2012/08/nat-implementation-using-netfilter-in.html
https://github.com/mbaez/mbsniffer/blob/master/Sniffer/src/Modulo/Sniffer.c
https://www.cs.cornell.edu/courses/cs5413/2014fa/labs/lab3.htm
ihttps://github.com/bloomark/kernel-sniffer
simple Netfilter module as a kernel module that simply drops all packets and logs dropped packets to /var/log/messages. 
We can add filters in 5 points: input flow before and after routing, output flow before and after routing and while 
forwarding a packat from one adapter to another.After adding the module (insmod) it will drop any package from ip 192.168.0.2
Note that you need to enable netfilter suppot in the kernel configuration.
*/

unsigned int test_nf_hookfn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	#if 0
	iph = ip_hdr(skb); tcph=tcp_hdr(skb); udph=udp_hdr(skb); ethh = eth_hdr(skb);
	or
	struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);
	struct tcphdr *tcp_header = (struct tcphdr *)skb_transport_header(skb);
	struct udphdr *udp_header = (struct udphdr *)skb_transport_header(skb);
	struct ethhdr * ethh = eth_hdr(skb);
	
	Hooks:
	[1]  NF_IP_PRE_ROUTING	called after sanity check, before routing decisions.
	[2]  NF_IP_LOCAL_IN	after routing decisions if packet is for this host
	[3]  NF_IP_FORWARD	if the packet is designed for another host
	[4]  NF_IP_POST_ROUTING	just before outbound "hit the wire"	
	[5]  NF_IP_LOCAL_OUT	for packet commings from local processess on therie way out

	NF_ACCEPT: accept the packet (continue network stack trip)
	NF_DROP: drop the packet (don't continue trip)
	NF_REPEAT: repeat the hook function
	NF_STOLEN: hook steals the packet (don't continue trip)
	NF_QUEUE: queue the packet to userspace.
	#endif	
	
	struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb); 
	//ip_header = ip_hdr(skb);
	struct tcphdr *tcp_header;
	struct udphdr *udp_header;

	/* https://stackoverflow.com/questions/14242338/linux-kernelregister-a-handler-for-a-specific-udp-port-traffic */
	if(ip_header->saddr == in_aton("192.168.1.1")) { 
		
		if (ip_header->protocol == IPPROTO_UDP) {
			udp_header = (struct udphdr *)skb_transport_header(skb);
			
			printk(KERN_INFO "protocol: %d S Port: %d D port: %u\n", ip_header->protocol, ntohs(udp_header->source), ntohs(udp_header->dest));
			printk(" IN: %s\n",skb->dev->name);
			printk(" Protocol: UDP\n");
			printk(" Length: %d\n",skb->len);
			printk(" TTL: %d\n", ip_header->ttl);
			printk(" ID: %d\n", ip_header->id);
			printk(" Ip address: %pI4 : %pI4\n", &ip_header->saddr, &ip_header->daddr);
			printk(KERN_INFO "UDP packet dropped\n");
			return NF_DROP;
		}

		if (ip_header->protocol == IPPROTO_TCP) {
			//tcp_header = (struct tcphdr *)skb_transport_header(skb); 
			tcp_header = (struct tcphdr *)( skb_transport_header(skb) + 20); /*Note: +20 is only for incoming packets*/
			printk(" IN: %s\n", skb->dev->name);
			printk(" Protocol: TCP\n");
			printk(" Length: %d\n", skb->len);
			printk(" TTL: %d\n", ip_header->ttl);
			printk(" ID: %d\n", ip_header->id);
			printk(" Ip address: %pI4 : %pI4\n", &ip_header->saddr, &ip_header->daddr);
			printk(KERN_INFO "protocol: %d S Port: %u\n", ip_header->protocol, tcp_header->source);
			printk(KERN_INFO "TCP loopback packet dropped\n");		/* log to var/log/messages */
			return NF_DROP; 
		}

	}

	return NF_ACCEPT;
}

static struct nf_hook_ops nfho;				/* struct holding set of hook function options */ 

static int __init netfilter_test_init(void)
{
	printk(KERN_INFO "Loading Netfilter\n");
	nfho.hook              =       test_nf_hookfn;		/* function to call when conditions below met */
	nfho.pf                =       PF_INET;			/* protocol family IPV4 PF_INET packets */
	//nfho.hooknum 	   = 	   NF_IP_PRE_ROUTING;		/* called right after packet recieved, first hook in Netfilter */
	nfho.hooknum           =       0;			/* Type of NetFilter hook */
	nfho.priority          =       NF_IP_PRI_FIRST;		/* set to highest priority over all other hook functions */
	nf_register_hook(&nfho);				/* register hook */

	return 0;
}
static void __exit netfilter_test_exit(void) 
{
	printk(KERN_INFO "Unloading Netfilter\n"); 
	nf_unregister_hook(&nfho);				/* unregister hook */ 
}
 
module_init(netfilter_test_init);
module_exit(netfilter_test_exit);
MODULE_LICENSE("GPL");

