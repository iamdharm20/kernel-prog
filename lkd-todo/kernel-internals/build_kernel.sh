#!/bin/bash

set -ex
export CROSS_COMPILE=arm-linux-gnueabihf-
export ARCH=arm

#export KERNEL_RELEASE=4.4.113-SF_PLT_REL_0.1-g5f4270c
#make artik530_raptor_defconfig

#How to avoid dirty
# step 1 modify scripts/setlocalversion and comment echo +
# run uname -r on target and get the kernel version
# Edit .config 
	#CONFIG_LOCALVERSION="-SF_PLT_REL_0.1-g5f4270c"
	#CONFIG_LOCALVERSION_AUTO=n

#make zImage -j4 EXTRAVERSION=-SF_PLT_REL_0.1
make zImage -j4

#make dtbs EXTRAVERSION=-SF_PLT_REL_0.1
make dtbs

mkdir -p ~/gitweb/modules-test/
make modules_install INSTALL_MOD_PATH=~/gitweb/modules-test/

# Cleaning is done on three levels.
# make clean     Delete most generated files
#                Leave enough to build external modules
# make mrproper  Delete the current configuration, and all generated files
# make distclean Remove editor backup files, patch leftover files and the like
