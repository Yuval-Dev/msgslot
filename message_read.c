#include<linux/fs.h>
#include<stdio.h>
#include<linux/ioctl.h>
#include<fcntl.h>
#include<errno.h>

int main(int argc, char ** argv) {
    if(argc != 3) {
        fprintf(stderr, "invalid arguments... \nusage: message_read <channel_file> <channel_id>");
    }
    char * fname = argv[1];
    char * cid_end;
    int channel_id = strtol(argv[2], &cid_end, 10);
    if(cid_end == argv[2]) {
        fprintf(stderr, "could not parse the ID string: %s are you sure it is a valid integer?\n", argv[2]);
        return -1;
    }
    int fd = open(fname, O_RDONLY);
    if(fd == -1) {
        perror("in open()");
        return -1;
    }
    char buf[129];
    if(ioctl(fd, 0, channel_id) < 0) {
        perror("in ioctl()");
        return -1;
    }
    int len;
    if((len = read(fd, buf, sizeof(buf) - 1)) < 0) {
        perror("in read()");
        return -1;
    }
    buf[len] = 0;
    printf("%s\n", buf);
}