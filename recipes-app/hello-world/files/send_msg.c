#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int set_baud_rate(int fd, int baudrate)
{
    struct termios options;
    // printf("setting baudrate as %d \n",baudrate);
    // read current setting on uart
    if (tcgetattr(fd, &options) < 0) {
        perror("tcgetattr");
        return -1;
    }
    // printf("set flag In: %d Out: %d\n",options.c_ispeed,options.c_ospeed);
    // setting baud rate
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    // printf("set flag In: %d Out: %d\n",options.c_ispeed,options.c_ospeed);

    options.c_cflag &= ~PARENB; // no parity check
    options.c_cflag &= ~CSTOPB; // setting stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     // 8 bit data

    options.c_cflag &= ~CRTSCTS; // no hardware flow control
    options.c_cflag |= (CLOCAL | CREAD); // local connection

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // default input mode
    options.c_iflag &= ~(IXON | IXOFF | IXANY);         // no SW flow control
    options.c_oflag &= ~OPOST;                          // default output mode

    // apply setting 
    // printf("Apply setting\n");
    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }
    // printf("Setting applied\n");
    return 0;
}
int main(void)
{
    int fd_btn = open("/dev/button_detect", O_RDONLY);
    if(fd_btn<0){
        perror("open");
        return 0;
    }
    int fd_tty = open("/dev/ttySTM1", O_RDWR | O_NOCTTY);
    // printf("fdtty: %d \n",fd_tty);
    if(fd_tty<0){
        perror("open");
        return 0;
    }
    // printf("setting baudrate\n");
    if (set_baud_rate(fd_tty, 115200) < 0) {
        close(fd_tty);
        return 0;
    }
    char buf[128];
    int len;
    // printf("listening for signal\n");

    while (1) {
        len = read(fd_btn, buf, sizeof(buf));
        if (len > 0) {
            // printf("[APP] read string from /dev/button_detect %d %s",len,buf);
            ssize_t bytes_written = write(fd_tty, buf, len);
            // printf("send result: %zd\n",bytes_written);
            if (bytes_written == -1) {
                perror("write");
            }
        }
    }
}