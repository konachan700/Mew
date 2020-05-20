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
#include <openssl/sha.h>

#define MEW_DFU_MODE_COMMAND					0x09
#define MEW_DFU_MODE_WRITE_FW					0x43
#define MEW_DFU_MODE_VERIFY						0x71
#define MEW_DFU_CALC_CRC(a)						(a[0] ^ a[1] ^ a[2] ^ a[3] ^ a[4] ^ a[5] ^ a[6])

#define HID_REPORT_SIZE_OUT 					65
#define SHA256_SIZE 							32

int main(int argc, char **argv) {
	char crc;
    int res_hid, fd_fw, res, i, len, file_size;
    hid_device *handle_hid;
    struct stat st;
    SHA256_CTX ctx;
    
    char *bufp;
    char buf[HID_REPORT_SIZE_OUT], r_buf[64], sha256[SHA256_SIZE], file_buf[1024];
    
    if (argc != 2) {
        printf("Bad params count %d; Usage [mew-fw /home/user/firmware.bin]\n", argc - 1);
        return 1;
    }
    
    fd_fw = open(argv[1], O_RDWR|O_NONBLOCK);
	if (fd_fw < 0) {
		printf("Cannot open firmware %s\n", argv[1]);
		return 3;
	}
	
	SHA256_Init(&ctx);

	stat(argv[1], &st);
	file_size = st.st_size;
	printf("Firmware file size is %d\n", file_size);

	while (1) {
		len = read(fd_fw, file_buf, 1024);
		if (len > 0) {
			SHA256_Update(&ctx, file_buf, len);
			if (len < 1024) {
				break;
			}
		}
	}

	SHA256_Final(sha256, &ctx);
	printf("\nLocal SHA-256: ");
	for (i=0; i<SHA256_SIZE; i++) printf("%02lX", (unsigned char) sha256[i]);
	printf("\n");

	res_hid = hid_init();
	handle_hid = hid_open(0x6666, 0x4300, NULL);
	
	memset(buf, 0x00, HID_REPORT_SIZE_OUT);

	bufp = (char*)(buf + 1);
	bufp[0] = MEW_DFU_MODE_VERIFY;
	bufp[1] = ((file_size >> 16) & 0xFF);
	bufp[2] = ((file_size >>  8) & 0xFF);
	bufp[3] = ((file_size      ) & 0xFF);
	bufp[7] = (char) MEW_DFU_CALC_CRC(bufp);

	printf("Verifing...\n");
	res = hid_write(handle_hid, buf, HID_REPORT_SIZE_OUT);
	if (res < 0) {
		printf("Cannot write to hidraw device %d\n", res);
		goto close_all;
	} else {
		memset(r_buf, 0x00, 64);
		res = hid_read_timeout(handle_hid, r_buf, 64, 60000);
		printf("%s\n", r_buf);
	}
	
	memset(buf, 0x00, HID_REPORT_SIZE_OUT);
	memcpy(bufp, sha256, SHA256_SIZE);
	res = hid_write(handle_hid, buf, HID_REPORT_SIZE_OUT);
	if (res < 0) {
		printf("Cannot write to hidraw device %d\n", res);
		goto close_all;
	} else {
		memset(r_buf, 0x00, 64);
		res = hid_read_timeout(handle_hid, r_buf, 64, 60000);
		printf("%s\n", r_buf);

		memset(r_buf, 0x00, 64);
		res = hid_read_timeout(handle_hid, r_buf, 64, 60000);
		for (i=0; i<SHA256_SIZE; i++) printf("%02lX", (unsigned char) r_buf[i+1]);
		printf("\n\n");
	}

close_all:
	close(fd_fw);
	hid_close(handle_hid);
	res_hid = hid_exit();
	
    return 0;
}














