#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "splash.h"

typedef struct
{
	uint8_t   descLen;
	uint8_t   cmapType;
	uint8_t   imageType;
	uint16_t  cmapStart;
	uint16_t  cmapLen;
	uint8_t   cmapBits;
	uint16_t  xOffset;
	uint16_t  yOffset;
}__attribute__((packed)) TgaHead;

typedef struct
{
	uint16_t  width;
	uint16_t  height;
	uint8_t   bitsPerPixel;
	uint8_t   attrib;
}__attribute__((packed)) TgaInfo;

typedef struct
{
	uint8_t   bytesPerPixel;
	uint32_t  imageSize;
	uint8_t   imageDataStart;
	uint32_t  imageDataBytes;
}__attribute__((packed)) TgaExtInfo;

TgaHead tgaHead;
TgaInfo tgaInfo;
TgaExtInfo tgaExtInfo;

bool decode_tga(FILE *fp)
{
	char head;
	char data[5];
	int currPixel = 0;
	int buffer_index = 0;

	fseek(fp, tgaExtInfo.imageDataStart, SEEK_SET);
	do{
		if(fread(&head, sizeof(head), 1, fp) == 0) {
			fprintf(stdout, "Read encode head failed!\n");
			return EXIT_FAILURE;
		}

		if (head & 0x80) {
			int icnt = (head & 0x7f) + 1;
			fread(&data, tgaExtInfo.bytesPerPixel, 1, fp);
			for (int i = 0; i < icnt; ++i) {
				imageFileBuffer.image_file_buffer[buffer_index+0] = data[0];
				imageFileBuffer.image_file_buffer[buffer_index+1] = data[1];
				imageFileBuffer.image_file_buffer[buffer_index+2] = data[2];
				if (tgaExtInfo.bytesPerPixel == 4)
					imageFileBuffer.image_file_buffer[buffer_index+3] = data[3];
				++currPixel;
				buffer_index += 4;
			}
		}
		else {
			int icnt = head + 1;
			for (int i = 0; i < icnt; ++i) {
			fread(&data, tgaExtInfo.bytesPerPixel, 1, fp);
				imageFileBuffer.image_file_buffer[buffer_index+0] = data[0];
				imageFileBuffer.image_file_buffer[buffer_index+1] = data[1];
				imageFileBuffer.image_file_buffer[buffer_index+2] = data[2];
				if (tgaExtInfo.bytesPerPixel == 4)
					imageFileBuffer.image_file_buffer[buffer_index+3] = data[3];
				++currPixel;
				buffer_index += 4;
			}
		}
	} while(!feof(fp) && currPixel < tgaExtInfo.imageSize);

	return EXIT_SUCCESS;
}

int load_tga(char *buffer, const char *fileName, struct fb_var_screeninfo *fb1_var)
{
	FILE *fp;
	char* ptr;
	int yoffset;

	if ((fp = fopen(fileName, "rb")) == NULL) {
		fprintf(stderr, "Open tga image failed!\n");
		return EXIT_FAILURE;
	}

	if(fread(&tgaHead, sizeof(TgaHead), 1, fp) == 0) {
		fclose(fp);
		return EXIT_FAILURE;
	}

	if(fread(&tgaInfo, sizeof(TgaInfo), 1, fp) == 0) {
		fclose(fp);
		return EXIT_FAILURE;
	}

	tgaExtInfo.bytesPerPixel = tgaInfo.bitsPerPixel/8;
	tgaExtInfo.imageSize = tgaInfo.width*tgaInfo.height;
	tgaExtInfo.imageDataStart = sizeof(TgaHead)+sizeof(TgaInfo)+tgaHead.descLen;
	tgaExtInfo.imageDataBytes = tgaExtInfo.imageSize*tgaExtInfo.bytesPerPixel;

//	fprintf(stdout, "TGA width: %d\n", tgaInfo.width);
//	fprintf(stdout, "TGA height: %d\n", tgaInfo.height);
//	fprintf(stdout, "IGA bytesPerPixel: %d\n", tgaExtInfo.bytesPerPixel);
//	fprintf(stdout, "TGA imageSize: %d\n", tgaExtInfo.imageSize);
//	fprintf(stdout, "TGA imageDataStart: %d\n", tgaExtInfo.imageDataStart);
//	fprintf(stdout, "TGA imageDataBytes: %d\n", tgaExtInfo.imageDataBytes);
//	fprintf(stdout, "Image buffer ptr: %p\n", buffer);

//PRINT_TIME(AA);
	if (tgaHead.imageType == 10) { // compressed tga image
		decode_tga(fp);
	}
//PRINT_TIME(BB);

	yoffset = (fb1_var->yres-tgaInfo.height)/2;
	ptr = (char*)buffer + (fb1_var->xres-tgaInfo.width)/2*4 + yoffset*fb1_var->xres*4;
	ptr += tgaInfo.height * fb1_var->xres*4;

	int i = 0;
	while (i < tgaExtInfo.imageDataBytes) {
		if (tgaHead.imageType == 10) { // compressed tga image
			memcpy(ptr, imageFileBuffer.image_file_buffer+i, tgaInfo.width*4);
		}
		else {
			fread(ptr, tgaInfo.width * 4, 1, fp);
		}

		i += tgaInfo.width * 4;
		ptr -= fb1_var->xres * 4;
	}
	fclose(fp);
	return EXIT_SUCCESS;
}
