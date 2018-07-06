http://amsekharkernel.blogspot.in/2012/08/nat-implementation-using-netfilter-in.html
https://github.com/mbaez/mbsniffer/blob/master/Sniffer/src/Modulo/Sniffer.c
https://www.cs.cornell.edu/courses/cs5413/2014fa/labs/lab3.htm
ihttps://github.com/bloomark/kernel-sniffer

iptables:
https://wiki.openwrt.org/doc/howto/netfilter


simple Netfilter module as a kernel module that simply drops all packets and logs dropped packets to /var/log/messages. 
We can add filters in 5 points: input flow before and after routing, output flow before and after routing and while 
forwarding a packat from one adapter to another.After adding the module (insmod) it will drop any package from ip 192.168.0.2
Note that you need to enable netfilter suppot in the kernel configuration.

iph = ip_hdr(skb); tcph=tcp_hdr(skb); udph=udp_hdr(skb); ethh = eth_hdr(skb);
 or
struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);
struct tcphdr *tcp_header = (struct tcphdr *)skb_transport_header(skb);
struct udphdr *udp_header = (struct udphdr *)skb_transport_header(skb);
struct ethhdr * ethh = eth_hdr(skb);
        
Hooks:
[1]  NF_IP_PRE_ROUTING  called after sanity check, before routing decisions.
[2]  NF_IP_LOCAL_IN     after routing decisions if packet is for this host
[3]  NF_IP_FORWARD      if the packet is designed for another host
[4]  NF_IP_POST_ROUTING just before outbound "hit the wire"     
[5]  NF_IP_LOCAL_OUT    for packet commings from local processess on therie way out

NF_ACCEPT: accept the packet (continue network stack trip)
NF_DROP: drop the packet (don't continue trip)
NF_REPEAT: repeat the hook function
NF_STOLEN: hook steals the packet (don't continue trip)
NF_QUEUE: queue the packet to userspace.
#endif

