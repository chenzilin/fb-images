#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#include "tgaimage.h"
#include "bmpimage.h"

using namespace std;

struct fb_var_screeninfo fb_var;
struct fb_fix_screeninfo fb_fix;

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

void flipPingPong(int &fd_fb, struct fb_var_screeninfo &vinfo, bool &pingPong)
{
	ioctl(fd_fb, FBIO_WAITFORVSYNC, NULL);
	vinfo.yoffset = pingPong * vinfo.yres;
	ioctl(fd_fb, FBIOPAN_DISPLAY, &vinfo);
	pingPong != pingPong;
}

int main(int argc, char **argv)
{
	int fd_fb;
	bool pingPong;
	size_t overlay_sz;
	int *overlay_buf;
	struct mxcfb_gbl_alpha alpha;

	if (argc != 3) {
		fprintf(stderr, "Usage: ./splash </dev/fb0|1> <image file dir path>\n");
		return EXIT_FAILURE;
	}

	signal(SIGINT, signal_handle);

	if ((fd_fb = open(argv[1], O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open /dev/fb!\n");
		return EXIT_FAILURE;
	}

	ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var);
	fprintf(stdout, "RES: %d, %d\n", fb_var.xres, fb_var.yres);

	overlay_sz = fb_var.xres * fb_var.yres * 4;
	if (imageFileBuffer.image_file_buffer == 0) {
		if ((imageFileBuffer.image_file_buffer = (char*)malloc(overlay_sz)) == 0) {
			fprintf(stderr, "Cannot malloc image file buffer!\n");
			close(fd_fb);
		}
		imageFileBuffer.image_file_buffer_len = overlay_sz;
	}
	overlay_buf = (int *)mmap(0, overlay_sz, PROT_READ | PROT_WRITE,MAP_SHARED, fd_fb, 0);
	fprintf(stdout, "SharedMem Ptr: %p\n", overlay_buf);

	// first clear fb
	memset(overlay_buf, 0x00, overlay_sz);

	// Disable global alpha since we need Pixel Alpha
	alpha.enable = 0;
	alpha.alpha = 0xff;
	ioctl(fd_fb, MXCFB_SET_GBL_ALPHA, &alpha);

	// enable FB
	ioctl(fd_fb, FBIOBLANK, FB_BLANK_UNBLANK);

	pingPong = fb_var.yoffset;
	list<string> image_list;
	getimagefromdir(argv[2], image_list);

	string suffix;
	auto iter = image_list.begin();
	while (animation_running) {

		suffix = (*iter).substr((*iter).size()-4, 4);
		if (suffix == ".tga") {
			if (EXIT_SUCCESS != load_tga(overlay_buf+pingPong*overlay_sz/2, (*iter).c_str(), &fb_var)) {
				fprintf(stderr, "Failed to load tga images!\n");
				break ;
			}
		}
		else if (suffix == ".bmp") {
			if (EXIT_SUCCESS != load_bmp(overlay_buf+pingPong*overlay_sz/2, (*iter).c_str(), &fb_var)) {
				fprintf(stderr, "Failed to load bmp images!\n");
				break;
			}
		}
		else continue;

		flipPingPong(fd_fb, fb_var, pingPong);

		++iter;
		if (iter == image_list.end()) iter = image_list.begin();

		usleep(66000);
	}

	if (imageFileBuffer.image_file_buffer != 0) free(imageFileBuffer.image_file_buffer);

	// disable FB
//	ioctl(fd_fb, FBIOBLANK, FB_BLANK_POWERDOWN);
	munmap(overlay_buf, overlay_sz);

	close(fd_fb);
	return EXIT_SUCCESS;
}
