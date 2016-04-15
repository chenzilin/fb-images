#ifndef BMPIMAGE_H_
#define BMPIMAGE_H_

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

int load_bmp(int *buffer, const char *fileName, struct fb_var_screeninfo &fb1_var)
{
	int ret;
	int bmp_fd;
	int i = 0, x, y;
	int width,height;
	char *bmp24_buf = 0;
	char *buffer_tmp = (char*)buffer;

	if((bmp_fd = open(fileName, O_RDWR)) < 0) {
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

	int half_height = (fb1_var.yres-height)/2;
	int half_width = (fb1_var.xres-width)/2;

	for(y = height+half_height-1; y >= half_height; --y)
		for(x = half_width*4; x < (half_width+width)*4; x+=4)
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

#endif
