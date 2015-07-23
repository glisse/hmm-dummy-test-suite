#!/bin/sh
#
# Simple script to load/reload the HMM dummy driver and create appropriate
# device files.

sync
rmmod hmm_dummy
modprobe hmm_dummy || exit 1
rm .tmp_v_*

# device0
rm -f /dev/hmm_dummy_device0[0-3]
major=$(awk "\$2==\"hmm_dummy_device0\" {print \$1}" /proc/devices)
echo hmm_dummy_device0 major is $major
mknod /dev/hmm_dummy_device00 c $major 0
mknod /dev/hmm_dummy_device01 c $major 1
mknod /dev/hmm_dummy_device02 c $major 2
mknod /dev/hmm_dummy_device03 c $major 3
chgrp wheel /dev/hmm_dummy_device0[0-3]
chmod 664 /dev/hmm_dummy_device0[0-3]

# device1
rm -f /dev/hmm_dummy_device1[0-3]
major=$(awk "\$2==\"hmm_dummy_device1\" {print \$1}" /proc/devices)
echo hmm_dummy_device1 major is $major
mknod /dev/hmm_dummy_device10 c $major 0
mknod /dev/hmm_dummy_device11 c $major 1
mknod /dev/hmm_dummy_device12 c $major 2
mknod /dev/hmm_dummy_device13 c $major 3
chgrp wheel /dev/hmm_dummy_device1[0-3]
chmod 664 /dev/hmm_dummy_device1[0-3]
