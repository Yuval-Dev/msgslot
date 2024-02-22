rm /dev/slot0
rm /dev/slot1
rmmod ./message_slot.ko
make all && insmod ./message_slot.ko
sleep 10
mknod /dev/slot1 c 236 1 && gcc message_write.c -o message_write && ./message_write /dev/slot1 10 helloworld