#ifndef TGAIMAGE_H_
#define TGAIMAGE_H_

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

using namespace std;

int load_tga(int *buffer, const char *fileName, struct fb_var_screeninfo &fb1_var)
{
	FILE *fp;
	char* ptr;
	int width;
	int height;
	unsigned int imagesize;
	unsigned char tgaheader[12];
	unsigned char attributes[6];
	int yoffset;

	if (buffer == NULL) return EXIT_FAILURE;

	if ((fp = fopen(fileName, "rb")) == NULL) return EXIT_FAILURE;

	if(fread(&tgaheader, sizeof(tgaheader), 1, fp) == 0) {
		fclose(fp);
		return EXIT_FAILURE;
	}

	if(fread(attributes, sizeof(attributes), 1, fp) == 0) {
		fclose(fp);
		return EXIT_FAILURE;
	}

	width = attributes[1] * 256 + attributes[0];
	height = attributes[3] * 256 + attributes[2];
	imagesize = attributes[4] / 8 * width * height;
//	fprintf(stdout, "TGA bits: %d\n", attributes[5] & 030);
//	fprintf(stdout, "TGA Pixel depth: %d\n", attributes[4]);
//	fprintf(stdout, "Image buffer ptr: 0x%p\n", buffer);

	yoffset = (fb1_var.yres-height)/2;
	ptr = (char*)buffer + (fb1_var.xres-width)/2*4 + yoffset*fb1_var.xres*4;
	ptr += height * fb1_var.xres*4;

	int n = 0;
	while (n < imagesize)
	{
		fread(ptr, width * 4, 1, fp);
		n += width * 4;
		ptr -= fb1_var.xres * 4;
	}
	fclose(fp);
	return EXIT_SUCCESS;
}

#endif
