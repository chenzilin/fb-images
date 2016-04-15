#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>  // arm-linux-gnueabihf-gcc -std=gnu11 splash.c -o splash
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>


struct mxcfb_gbl_alpha {
        int enable;
        int alpha;
};

#define MXCFB_SET_GBL_ALPHA     _IOW('F', 0x21, struct mxcfb_gbl_alpha)

struct mxcfb_gbl_alpha alpha;

char lcd_buf[1440*540*4];
struct fb_var_screeninfo fb1_var;
unsigned int* fb_mem;


int load_bmp(int *buffer, const char *fileName, struct fb_var_screeninfo &fb1_var)
{
	int ret;
	int bmp_fd;
	int i = 0, x, y;
	int width,height;
	char *bmp24_buf = 0;
	char *buffer_tmp = (char*)buffer;

	bmp_fd = open(fileName, O_RDWR);
	if(bmp_fd < 0) {
		fprintf(stderr, "Open BMP Failed!\n");
		return EXIT_FAILURE;
	}

	lseek(bmp_fd,18,SEEK_SET);
	if((ret = read(bmp_fd,&width,4))!= 4) {
		fprintf(stderr, "Read BMP Width Failed!\n");
		return EXIT_FAILURE;
	}

	lseek(bmp_fd,22,SEEK_SET);
	if((ret = read(bmp_fd,&height,4)) != 4) {
		fprintf(stderr, "Read BMP Height Failed!\n");
		return EXIT_FAILURE;
	}

	if ((bmp24_buf = (char*)malloc(width*height*3)) == 0) {
		fprintf(stderr, "Malloc Failed!\n");
		return EXIT_FAILURE;
	}

	lseek(bmp_fd,54,SEEK_SET);
	if((ret = read(bmp_fd,bmp24_buf,width*height*3)) != width*height*3) {
		fprintf(stderr, "Read 24bit BMP Failed!\n");
		return EXIT_FAILURE;
	}

	for(y = height-1; y >= 0; --y)
		for(x = 0; x < width*4; x+=4)
		{
			*(buffer_tmp+y*1440*4+x) = bmp24_buf[i];
			*(buffer_tmp+y*1440*4+x+1) = bmp24_buf[i+1];
			*(buffer_tmp+y*1440*4+x+2) = bmp24_buf[i+2];
			*(buffer_tmp+y*1440*4+x+3) = 255;
			i+=3;
		}

	free(bmp24_buf);
	return EXIT_SUCCESS;
}

int main(int argc,char **argv)
{
	int i,ret,x,y;
	int lcd_fd,bmp_fd;
	unsigned char *pScr;

	// 获取资源许可
	lcd_fd =  open("/dev/fb1", O_RDWR);
	if(lcd_fd < 0)
		{
			printf("open LCD Failed!\n");
			return -1;
		}

	// Disable global alpha since we need Pixel Alpha
	alpha.enable = 0;
	alpha.alpha = 0xff;
	ioctl(lcd_fd, MXCFB_SET_GBL_ALPHA, &alpha);
 
	// enable FB1
	ioctl(lcd_fd, FBIOBLANK, FB_BLANK_UNBLANK);

	ioctl(lcd_fd, FBIOGET_VSCREENINFO, &fb1_var);
	fprintf(stdout, "RES: %d, %d\n", fb1_var.xres, fb1_var.yres);

	fb_mem = (unsigned int*)mmap(NULL, 1440*540*4*2, PROT_READ| PROT_WRITE, MAP_SHARED,lcd_fd, 0);
	if(fb_mem == MAP_FAILED)
		{
			printf("mmap Failed!\n");
			return -1;
		}
	memset(fb_mem, 0x0,1440*540*4*2*sizeof(char));

	pScr = (unsigned char*)fb_mem;
	load_bmp((int*)fb_mem, "/home/root/test.bmp", fb1_var);

	// 撤销显存映射
	munmap(fb_mem, 1440*540*4*2);

	//释放资源
	close(lcd_fd);
	close(bmp_fd);
	return 0;
}
