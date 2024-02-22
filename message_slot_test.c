#include<stdio.h>
#include "message_slot.h"

static btree_layer btree;

pchannel channel_open(int index) {
  printf("channel_open(%d)\n", index);
  pbtree_layer cur_node, new_child;
  int level, i;
  pchannel channel;
  cur_node = &btree;
  for(level = 0; level < BTREE_NUM_LEVELS - 1; level++) {
    i = index & BTREE_LEVEL_MASK(level);
    if(cur_node->children[i] == 0) {
      new_child = malloc(sizeof(btree_layer));
      printf("kmalloc() -> %p\n", new_child);
      if(new_child == 0) {
        printf("in channel_open(): bad kmalloc()\n");
        return 0;
      }
      cur_node->children[i] = new_child;
      memset(new_child, 0, sizeof(btree_layer));
    }
    cur_node = cur_node->children[i];
  }
  i = index & BTREE_LEVEL_MASK(BTREE_NUM_LEVELS - 1);
  if(cur_node->children[i] == 0) {
    void * addr = malloc(sizeof(struct _channel));
    printf("kmalloc() -> %p\n", addr);
    if(addr == 0) {
      printf("in channel_open(): bad kmalloc() 2\n");
      return 0;
    }
    cur_node->children[i] = addr;
    memset(addr, 0, sizeof(struct _channel));
    channel = (pchannel)addr;
  } else {
    channel = (pchannel)cur_node->children[i];
  }
  return channel;
}

void free_all(pbtree_layer ptr, int depth) {
  int i;
  for(i = 0; i < BTREE_CHILD_COUNT; i++) {
    if(ptr->children[i] != 0) {
      if(depth != BTREE_NUM_LEVELS - 1) {
        free_all(ptr->children[i], depth + 1);
      }
      free(ptr->children[i]);
      printf("kfree(%p)\n", ptr->children[i]);
    }
  }
}


int main() {
    while(1) {
        char type;
        scanf(" %c", &type);
        if(type == 'r') {
            int id;
            scanf("%d", &id);
            pchannel c = channel_open(id);
            printf("%s\n", c->message);
        } else if(type == 'w') {
            int id;
            scanf("%d", &id);
            pchannel c = channel_open(id);
            scanf(" %[^\n]s", c->message);
            printf("new_value: %s\n", c->message);
        } else break;
    }
    free_all(&btree, 0);
}