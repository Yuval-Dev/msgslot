#include<linux/fs.h>
#include<stdio.h>
#include<linux/ioctl.h>
#include<fcntl.h>
#include<errno.h>

int main(int argc, char ** argv) {
    if(argc != 4) {
        fprintf(stderr, "invalid arguments... \nusage: message_write <channel_file> <channel_id> <message>");
        return -1;
    }
    char * fname = argv[1];
    char * cid_end;
    int channel_id = strtol(argv[2], &cid_end, 10);
    if(cid_end == argv[2]) {
        fprintf(stderr, "could not parse the ID string: %s are you sure it is a valid integer?\n", argv[2]);
        return -1;
    }
    int fd = open(fname, O_RDWR);
    if(fd == -1) {
        perror("in open()");
        return -1;
    }
    if(ioctl(fd, 0, channel_id) < 0) {
        perror("in ioctl()");
        return -1;
    }
    int len = strlen(argv[3]);
    if((len = write(fd, argv[3], len)) < 0) {
        perror("in write()");
        return -1;
    }
}