http://nano-chicken.blogspot.com/search/label/Linux%20-%20kernel

ndo_open() and ndo_validate_addr() are called, when the NIC is bring up.
ndo_stop() is called, when the NIC is shut down.
ndo_start_xmit() is called, when a packet is sent from the NIC.
ndo_change_mtu() is called, when the MTU of the NIC is changed.
ndo_set_mac_address() is called, when the MAC address of the NIC is changed.

modeprobe <name>.ko
ifconfig eth0 192.168.1.1 up && ifconfig eth1 192.168.2.1 up
ping 192.168.1.2


