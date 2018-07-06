http://lkw.readthedocs.io/en/latest/doc/07_kernel_data_stuctures.html
echo "print" > /proc/linked_list
dmesg

Let us delete a non existent node.
echo "delete 0" > /proc/linked_list

Let us delete some nodes.
echo "delete 30" > /proc/linked_list

Let us print again.
echo "print" > /proc/linked_list
