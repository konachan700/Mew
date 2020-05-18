#include <linux/types.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <hidapi/hidapi.h>

int main(int argc, char **argv) {
    int res_hid, fd_fw, res;
    hid_device *handle_hid;
    
    char buf[65];
    
    if (argc != 2) {
        printf("Bad params count %d; Usage [mew-fw /home/user/firmware.bin]\n", argc - 1);
        return 1;
    }
    
    fd_fw = open(argv[1], O_RDWR|O_NONBLOCK);
	if (fd_fw < 0) {
		printf("Cannot open firmware %s\n", argv[1]);
		return 3;
	}
	
	res_hid = hid_init();
	//printf("res_hid = %d\n", res_hid);
	
	handle_hid = hid_open(0x1234, 0x4321, NULL);
	//printf("res_hid = %d\n", (handle_hid != NULL));
	
	buf[0] = 0x01;
	memset(buf, 0, 65);
	res = hid_write(handle_hid, buf, 64);
	if (res < 0) {
		printf("Cannot write to hidraw device %d\n", res);
		//return 4;
	} else {
		printf("Written %d bytes\n", res);
	}
	
	
    
//    
//    buf[0] = 0x01;
//    res = write(fd_hid, buf, 65);
//    if (res < 0) {
//    	printf("Cannot write to hidraw device %s\n", argv[1]);
//    	return 4;
//    } else {
//    	printf("Written %d bytes\n", res);
//    }
//    
	
	close(fd_fw);
	hid_close(handle_hid);
	res_hid = hid_exit();
	
    return 0;
}














