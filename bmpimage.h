#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "splash.h"

typedef struct
{
	uint16_t fileType;
	uint32_t fileSize;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t imageOffset;
}__attribute__((packed)) BmpHead;

typedef struct
{
	uint32_t infoSize;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bitsPerPixel;
	uint32_t compressionType;
	uint32_t size;
	uint32_t xPixelsPerMeter;
	uint32_t yPixelsPerMeter;
	uint32_t colorUsed;
	uint32_t colorImportant;
}__attribute__((packed)) BmpInfo;

typedef struct
{
	uint8_t   bytesPerPixel;
	uint32_t  imageSize;
	uint8_t   imageDataStart;
	uint32_t  imageDataBytes;
}__attribute__((packed)) BmpExtInfo;


BmpHead bmpHead;
BmpInfo bmpInfo;
BmpExtInfo bmpExtInfo;

typedef struct
{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t reserved;
}__attribute__((packed)) RGBQuad;

int load_bmp(char *buffer, const char *fileName, struct fb_var_screeninfo *fb1_var)
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

	if((ret = read(bmp_fd, &bmpHead, sizeof(BmpHead))) != sizeof(BmpHead)) {
		fprintf(stderr, "Read BMP Head Failed!\n");
		return EXIT_FAILURE;
	}

	if((ret = read(bmp_fd, &bmpInfo, sizeof(BmpInfo))) != sizeof(BmpInfo)) {
		fprintf(stderr, "Read BMP Info Failed!\n");
		return EXIT_FAILURE;
	}

	if ((bmp24_buf = (char*)malloc(bmpInfo.width*bmpInfo.height*3)) == 0) {
		fprintf(stderr, "Malloc Failed!\n");
		return EXIT_FAILURE;
	}

	bmpExtInfo.bytesPerPixel = bmpInfo.bitsPerPixel/8;
	bmpExtInfo.imageSize = bmpInfo.width*bmpInfo.height;
	bmpExtInfo.imageDataStart = sizeof(BmpHead)+sizeof(BmpInfo);
	bmpExtInfo.imageDataBytes = bmpExtInfo.imageSize*bmpExtInfo.bytesPerPixel;

//	fprintf(stdout, "ImageOffset: %d\n", bmpHead.imageOffset);
//	fprintf(stdout, "CompressionType: %d\n", bmpInfo.compressionType);
//	fprintf(stdout, "BytesPerPixel: %d\n", bmpExtInfo.bytesPerPixel);

	lseek(bmp_fd, bmpExtInfo.imageDataStart, SEEK_SET);
	if((ret = read(bmp_fd, bmp24_buf, bmpExtInfo.imageDataBytes)) != bmpExtInfo.imageDataBytes) {
		fprintf(stderr, "Read 24bit BMP Failed!\n");
		return EXIT_FAILURE;
	}

	int half_height = (fb1_var->yres-bmpInfo.height)/2;
	int half_width = (fb1_var->xres-bmpInfo.width)/2;

	for(y = bmpInfo.height+half_height-1; y >= half_height; --y)
		for(x = half_width*4; x < (half_width+bmpInfo.width)*4; x+=4)
		{
			*(buffer_tmp+y*fb1_var->xres*4+x) = bmp24_buf[i];
			*(buffer_tmp+y*fb1_var->xres*4+x+1) = bmp24_buf[i+1];
			*(buffer_tmp+y*fb1_var->xres*4+x+2) = bmp24_buf[i+2];
			*(buffer_tmp+y*fb1_var->xres*4+x+3) = 255;
			i += bmpExtInfo.bytesPerPixel;
		}

	free(bmp24_buf);
	return EXIT_SUCCESS;
}
