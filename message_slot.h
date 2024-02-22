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
#define MAJOR_NUM 236

// Set the message of the device driver
#define IOCTL_SET_ENC _IOW(MAJOR_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "char_dev"
#define BUF_LEN 80
#define DEVICE_FILE_NAME "simple_char_dev"
#define SUCCESS 0
#define MAX_DEVICE_ID 256
#define MAX_CHANNEL_MESSAGE_LEN 128
#define MSG_SLOT_CHANNEL 0x0
#define MSG_SLOT_BAD_COPY 2
#define MSG_SLOT_BAD_ACCESS 3
#define MAX_CHANNEL_BITS 20
#define MAX_CHANNEL_ID (1ll << MAX_CHANNEL_BITS)
#define BTREE_CHILD_BITS 5
#define BTREE_CHILD_COUNT (1ull << BTREE_CHILD_BITS)
#define BTREE_CHILD_MASK (BTREE_CHILD_COUNT - 1)
#define BTREE_NUM_LEVELS (MAX_CHANNEL_BITS / BTREE_CHILD_BITS)
#define BTREE_LEVEL_MASK(level) ((BTREE_CHILD_MASK << ((BTREE_NUM_LEVELS - 1) * BTREE_CHILD_BITS)) >> ((level) * BTREE_CHILD_BITS))
typedef struct _btree_layer {
    void * children[BTREE_CHILD_COUNT];
} btree_layer, *pbtree_layer;
typedef struct _channel {
    char message[MAX_CHANNEL_MESSAGE_LEN];
    size_t len;
} channel, *pchannel;
#endif
