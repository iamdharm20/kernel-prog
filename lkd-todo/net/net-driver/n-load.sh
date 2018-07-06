#!/bin/bash

insmod modules/network-driver.ko
sudo ifconfig eth0 192.168.1.1 up && sudo ifconfig eth1 192.168.2.1 up
ping 192.168.1.2
