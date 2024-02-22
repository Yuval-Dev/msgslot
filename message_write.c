#include<linux/fs.h>
#include<stdio.h>
#include<linux/ioctl.h>
#include<fcntl.h>
#include<errno.h>

int main(int argc, char ** argv) {
    if(argc != 4) {
        fprintf(stderr, "invalid arguments... \nusage: message_write <channel_file> <channel_id> <message>\n");
        return -1;
    }
    char * fname = argv[1];
    int channel_id = atoi(argv[2]);
    int fd = open(fname, O_RDWR);
    if(fd == -1) {
        printf("bad open(), %d\n", errno);
        return 0;
    }
    char buf[129];
    int len = strlen(argv[3]);
    memcpy(buf, argv[3], len);
    buf[len] = 0;
    if(ioctl(fd, 0, channel_id) < 0) {
        fprintf(stderr, "error in ioctl: %d\n", errno);
        return -1;
    }
    errno = 0;
    int res;
    if((res = write(fd, buf, len)) != len) {
        fprintf(stderr, "error writing to channel: %d, %d, %d\n", errno, res, len);
        return -1;
    }
}