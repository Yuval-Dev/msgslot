#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, off_t *);
static ssize_t device_write(struct file *, const char *, size_t, off_t *);


#endif