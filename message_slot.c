// Declare what kind of code we want
// from the header files. Defining __KERNEL__
// and MODULE allows us to access kernel-level
// code not usually available to userspace programs.
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>

MODULE_LICENSE("GPL");

//Our custom definitions of IOCTL operations
#include "message_slot.h"

static btree_layer btree;

pchannel channel_open(int index) {
  printk("channel_open(%d)\n", index);
  pbtree_layer cur_node, new_child;
  int level, i;
  pchannel channel;
  cur_node = &btree;
  for(level = 0; level < BTREE_NUM_LEVELS - 1; level++) {
    i = index & BTREE_LEVEL_MASK(level);
    if(cur_node->children[i] == 0) {
      new_child = kmalloc(sizeof(btree_layer), GFP_KERNEL);
      if(new_child == 0) {
        printk("in channel_open(): bad kmalloc()\n");
        return 0;
      }
      cur_node->children[i] = new_child;
      memset(new_child, 0, sizeof(btree_layer));
      new_child->parent = cur_node;
    }
    cur_node = cur_node->children[i];
    cur_node->ref_cnt++;
  }
  i = index & BTREE_LEVEL_MASK(BTREE_NUM_LEVELS - 1);
  if(cur_node->children[i] == 0) {
    void * addr = kmalloc(sizeof(struct _channel), GFP_KERNEL);
    if(addr == 0) {
      printk("in channel_open(): bad kmalloc() 2\n");
      return 0;
    }
    cur_node->children[i] = addr;
    memset(addr, 0, sizeof(struct _channel));
    channel = (pchannel)addr;
    channel->parent = cur_node;
    channel->index = index;
  } else {
    channel = (pchannel)cur_node->children[i];
  }
  channel->ref_cnt++;
  return channel;
}


void channel_close(pchannel channel) {
  int index = channel->index;
  int i;
  pbtree_layer next, cur = channel->parent;
  channel->ref_cnt--;
  for(i = BTREE_NUM_LEVELS - 2; i >= 0; i--) {
    next = cur->parent;
    cur->ref_cnt--;
  }
}

void free_all(pbtree_layer ptr, int depth) {
  if(depth == BTREE_NUM_LEVELS) {
    return;
  }
  int i;
  for(i = 0; i < BTREE_CHILD_COUNT; i++) {
    free_all(ptr->children[i], depth + 1);
    kfree(ptr->children[i]);
  }
}

struct chardev_info {
  spinlock_t lock;
};

// used to prevent concurent access into the same device
static int dev_open_flag = 0;

static struct chardev_info device_info;


//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file )
{
  printk("Invoking device_open(%p)\n", file);
  file->private_data = (void *)0;
  return SUCCESS;
}

//---------------------------------------------------------------
static int device_release( struct inode* inode,
                           struct file*  file)
{
  pchannel cur_channel;
  cur_channel = (pchannel)file->private_data;
  printk("Invoking device_release(%p,%p)\n", inode, file);
  if(cur_channel != 0) {
    channel_close(cur_channel);
  }
  return SUCCESS;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset )
{
  printk("Invoking device_read(%p)\n", file);
  pchannel channel;
  channel = (pchannel)file->private_data;
  if(channel == 0) {
    return -EINVAL;
  }
  // read doesnt really do anything (for now)
  //printk( "Invocing device_read(%p,%ld) - "
  //        "operation not supported yet\n"
  //        "(last written - %s)\n",
  //        file, length, the_message );
  if(channel->len == 0) {
    return -EWOULDBLOCK;
  }
  if(channel->len > length) {
    return -ENOSPC;
  }
  if(access_ok(buffer, length) == 0) {
    return -MSG_SLOT_BAD_ACCESS;
  }
  if(copy_to_user(buffer, channel->message, channel->len) != 0) {
    return -MSG_SLOT_BAD_COPY;
  }
  return channel->len;
}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset)
{
  printk("Invoking device_write(%p)\n\r\n", file);
  pchannel channel;
  channel = (pchannel)file->private_data;
  if(channel == 0) {
    return -EINVAL;
  }
  if(length > MAX_CHANNEL_MESSAGE_LEN) {
    return -EMSGSIZE;
  }
  if(length == 0) {
    return -EINVAL;
  }
  if(access_ok(buffer, length) == 0) {
    return -EFAULT;
  }
  if(copy_from_user(channel->message, buffer, length) != 0) {
    channel->len = 0;
    return -EACCES;
  }
  channel->len = length;
  return length;
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
  printk("Invoking device_ioctl(%p)\n", file);
  pchannel channel = (pchannel)file->private_data;
  if(ioctl_command_id != MSG_SLOT_CHANNEL) {
    return -EINVAL;
  }
  if(ioctl_param > MAX_CHANNEL_ID) {
    return -EINVAL;
  }
  if(channel != 0) {
    channel_close(channel);
    channel = 0;
  }
  file->private_data = channel_open(ioctl_param);
  return SUCCESS;
}

//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  .release        = device_release,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init simple_init(void)
{
  memset(&btree, 0, sizeof(btree_layer));
  int rc = -1;
  // init dev struct
  memset( &device_info, 0, sizeof(struct chardev_info) );
  spin_lock_init( &device_info.lock );

  // Register driver capabilities. Obtain major num
  rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );

  // Negative values signify an error
  if( rc < 0 ) {
    printk( KERN_ALERT "%s registraion failed for  %d\n",
                       DEVICE_FILE_NAME, MAJOR_NUM );
    return rc;
  }

  printk( "Registeration is successful!! ");
  printk( "If you want to talk to the device driver,\n" );
  printk( "you have to create a device file:\n" );
  printk( "mknod /dev/%s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM );
  printk( "You can echo/cat to/from the device file.\n" );
  printk( "Dont forget to rm the device file and "
          "rmmod when you're done\n" );

  return 0;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void)
{
  free_all(&btree, 0);
  // Unregister the device
  // Should always succeed
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================
