#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "tgaimage.h"
#include "bmpimage.h"

using namespace std;

struct fb_var_screeninfo fb1_var;
struct fb_fix_screeninfo fb1_fix;

struct mxcfb_gbl_alpha {
	int enable;
	int alpha;
};
#define MXCFB_SET_GBL_ALPHA     _IOW('F', 0x21, struct mxcfb_gbl_alpha)

static int animation_running = 1;
void signal_handle(int)
{
	fprintf(stdout, "I Got Singnal SIGINT\n");
	animation_running = 0;
	signal(SIGINT, SIG_DFL);
}

void flipPingPong(int &fd_fb1, struct fb_var_screeninfo &vinfo, bool &pingPong)
{
	ioctl(fd_fb1, FBIO_WAITFORVSYNC, NULL);
	vinfo.yoffset = pingPong * vinfo.yres;
	ioctl(fd_fb1, FBIOPAN_DISPLAY, &vinfo);
	pingPong != pingPong;
}

int main(int argc, char **argv)
{
	int fd_fb1;
	bool pingPong;
	size_t overlay_sz;
	int *overlay_buf;
	struct mxcfb_gbl_alpha alpha;

	if (argc != 2) {
		fprintf(stderr, "Usage: ./splash <image file dir path>\n");
		return EXIT_FAILURE;
	}

	signal(SIGINT, signal_handle);

	if ((fd_fb1 = open("/dev/fb1", O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open /dev/fb1!\n");
		return EXIT_FAILURE;
	}

	ioctl(fd_fb1, FBIOGET_VSCREENINFO, &fb1_var);
	fprintf(stdout, "RES: %d, %d\n", fb1_var.xres, fb1_var.yres);

	overlay_sz = fb1_var.xres * fb1_var.yres * 4;
	if (imageFileBuffer.image_file_buffer == 0) {
		if ((imageFileBuffer.image_file_buffer = (char*)malloc(overlay_sz)) == 0) {
			fprintf(stderr, "Cannot malloc image file buffer!\n");
			close(fd_fb1);
		}
		imageFileBuffer.image_file_buffer_len = overlay_sz;
	}
	overlay_buf = (int *)mmap(0, overlay_sz, PROT_READ | PROT_WRITE,MAP_SHARED, fd_fb1, 0);
	fprintf(stdout, "SharedMem Ptr: %p\n", overlay_buf);

	// first clear fb1
	memset(overlay_buf, 0x00, overlay_sz);

	// Disable global alpha since we need Pixel Alpha
	alpha.enable = 0;
	alpha.alpha = 0xff;
	ioctl(fd_fb1, MXCFB_SET_GBL_ALPHA, &alpha);

	// enable FB1
	ioctl(fd_fb1, FBIOBLANK, FB_BLANK_UNBLANK);

	pingPong = fb1_var.yoffset;
	list<string> image_list;
	getimagefromdir(argv[1], image_list);

	string suffix;
	auto iter = image_list.begin();
	while (animation_running) {

		suffix = (*iter).substr((*iter).size()-4, 4);
		if (suffix == ".tga") {
			if (EXIT_SUCCESS != load_tga(overlay_buf+pingPong*overlay_sz/2, (*iter).c_str(), &fb1_var)) {
				fprintf(stderr, "Failed to load tga images!\n");
				break ;
			}
		}
		else if (suffix == ".bmp") {
			if (EXIT_SUCCESS != load_bmp(overlay_buf+pingPong*overlay_sz/2, (*iter).c_str(), &fb1_var)) {
				fprintf(stderr, "Failed to load bmp images!\n");
				break;
			}
		}
		else continue;

		flipPingPong(fd_fb1, fb1_var, pingPong);

		++iter;
		if (iter == image_list.end()) iter = image_list.begin();

		usleep(66000);
	}

	if (imageFileBuffer.image_file_buffer != 0) free(imageFileBuffer.image_file_buffer);

	// disable FB1
//	ioctl(fd_fb1, FBIOBLANK, FB_BLANK_POWERDOWN);
	munmap(overlay_buf, overlay_sz);

	close(fd_fb1);
	return EXIT_SUCCESS;
}
