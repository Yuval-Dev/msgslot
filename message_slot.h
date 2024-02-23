#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h>
#include <stddef.h>
#include <linux/types.h>
// The major device number.
// We don't rely on dynamic registration
// any more. We want ioctls to know this
// number at compile time.
//#define MAJOR_NUM 244
#define MAJOR_NUM 235

// Set the message of the device driver
#define IOCTL_SET_ENC _IOW(MAJOR_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FILE_NAME "message_slot_driver"
#define SUCCESS 0
#define MAX_DEVICE_ID 256
#define MAX_CHANNEL_MESSAGE_LEN 128
#define MSG_SLOT_CHANNEL 0x0
#define MAX_CHANNEL_BITS 28
#define MAX_CHANNEL_ID (1ull << MAX_CHANNEL_BITS)
#define BTREE_CHILD_BITS 7
#define BTREE_CHILD_COUNT (1ull << BTREE_CHILD_BITS)
#define BTREE_CHILD_MASK (BTREE_CHILD_COUNT - 1)
#define BTREE_NUM_LEVELS (MAX_CHANNEL_BITS / BTREE_CHILD_BITS)
typedef struct _btree_layer {
    void * children[BTREE_CHILD_COUNT];
} btree_layer, *pbtree_layer;
typedef struct _channel {
    char message[MAX_CHANNEL_MESSAGE_LEN];
    size_t len;
} channel, *pchannel;
#endif
