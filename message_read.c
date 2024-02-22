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
    int channel_id = atoi(argv[2]);
    int fd = open(fname, O_RDONLY);
    char buf[129];
    ioctl(fd, 0, channel_id);
    int len;
    if((len = read(fd, buf, sizeof(buf) - 1)) < 0) {
        fprintf(stderr, "error reading from channel%d\n", errno);
        return 0;
    }
    buf[len] = 0;
    printf("%s\n", buf);
}