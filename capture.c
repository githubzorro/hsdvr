/*
 * Copyright 2004-2013 Freescale Semiconductor, Inc.
 *
 * Copyright (c) 2006, Chips & Media.  All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <malloc.h>
#include <linux/videodev2.h>
#include <linux/mxcfb.h>
#include "vpu_test.h"
#include <linux/ipu.h>
#include <linux/mxc_v4l2.h>
#include "g2d.h"


struct ipu_pos {
	u32 x;
	u32 y;
};

struct ipu_crop {
	struct ipu_pos pos;
	u32 w;
	u32 h;
};

struct ipu_deinterlace {
	bool	enable;
	u8	motion; /*see ipu_motion_sel*/
#define IPU_DEINTERLACE_FIELD_TOP	0
#define IPU_DEINTERLACE_FIELD_BOTTOM	1
#define IPU_DEINTERLACE_FIELD_MASK	\
		(IPU_DEINTERLACE_FIELD_TOP | IPU_DEINTERLACE_FIELD_BOTTOM)
	/* deinterlace frame rate double flags */
#define IPU_DEINTERLACE_RATE_EN		0x80
#define IPU_DEINTERLACE_RATE_FRAME1	0x40
#define IPU_DEINTERLACE_RATE_MASK	\
		(IPU_DEINTERLACE_RATE_EN | IPU_DEINTERLACE_RATE_FRAME1)
#define IPU_DEINTERLACE_MAX_FRAME	2
	u8	field_fmt;
};

struct ipu_input {
	u32 width;
	u32 height;
	u32 format;
	struct ipu_crop crop;
	dma_addr_t paddr;

	struct ipu_deinterlace deinterlace;
	dma_addr_t paddr_n; /*valid when deinterlace enable*/
};

struct ipu_alpha {
#define IPU_ALPHA_MODE_GLOBAL	0
#define IPU_ALPHA_MODE_LOCAL	1
	u8 mode;
	u8 gvalue; /* 0~255 */
	dma_addr_t loc_alp_paddr;
};

struct ipu_colorkey {
	bool enable;
	u32 value; /* RGB 24bit */
};

struct ipu_overlay {
	u32	width;
	u32	height;
	u32	format;
	struct ipu_crop crop;
	struct ipu_alpha alpha;
	struct ipu_colorkey colorkey;
	dma_addr_t paddr;
};

struct ipu_output {
	u32	width;
	u32	height;
	u32	format;
	u8	rotate;
	struct ipu_crop crop;
	dma_addr_t paddr;
};

struct ipu_task {
	struct ipu_input input;
	struct ipu_output output;

	bool overlay_en;
	struct ipu_overlay overlay;

#define IPU_TASK_PRIORITY_NORMAL 0
#define IPU_TASK_PRIORITY_HIGH	1
	u8	priority;

#define	IPU_TASK_ID_ANY	0
#define	IPU_TASK_ID_VF	1
#define	IPU_TASK_ID_PP	2
#define	IPU_TASK_ID_MAX 3
	u8	task_id;

	int	timeout;
};

enum {
	IPU_CHECK_OK = 0,
	IPU_CHECK_WARN_INPUT_OFFS_NOT8ALIGN = 0x1,
	IPU_CHECK_WARN_OUTPUT_OFFS_NOT8ALIGN = 0x2,
	IPU_CHECK_WARN_OVERLAY_OFFS_NOT8ALIGN = 0x4,
	IPU_CHECK_ERR_MIN,
	IPU_CHECK_ERR_INPUT_CROP,
	IPU_CHECK_ERR_OUTPUT_CROP,
	IPU_CHECK_ERR_OVERLAY_CROP,
	IPU_CHECK_ERR_INPUT_OVER_LIMIT,
	IPU_CHECK_ERR_OV_OUT_NO_FIT,
	IPU_CHECK_ERR_OVERLAY_WITH_VDI,
	IPU_CHECK_ERR_PROC_NO_NEED,
	IPU_CHECK_ERR_SPLIT_INPUTW_OVER,
	IPU_CHECK_ERR_SPLIT_INPUTH_OVER,
	IPU_CHECK_ERR_SPLIT_OUTPUTW_OVER,
	IPU_CHECK_ERR_SPLIT_OUTPUTH_OVER,
	IPU_CHECK_ERR_SPLIT_WITH_ROT,
	IPU_CHECK_ERR_NOT_SUPPORT,
	IPU_CHECK_ERR_NOT16ALIGN,
};

#define IPU_CHECK_TASK		_IOWR('I', 0x1, struct ipu_task)
#define IPU_QUEUE_TASK		_IOW('I', 0x2, struct ipu_task)
#define IPU_ALLOC		_IOWR('I', 0x3, int)
#define IPU_FREE		_IOW('I', 0x4, int)

//#define TEST_BUFFER_NUM 5	move to vpu_test.h 
//#define G2D_CACHEABLE    0
//#define INSTANCE_NUM 4

struct instance_priv ins_priv[INSTANCE_NUM]  = {0};

/*----move to struct instance_priv-----------------
struct capture_testbuffer cap_buffers[TEST_BUFFER_NUM];
struct g2d_buf *g2d_buffers[TEST_BUFFER_NUM];

int fd_capture_v4l = 0;
---------------------------------------------*/
char fb_dev[100] = "/dev/fb0";
int fd_fb = 0;
int fd_ipu = 0;
int g_fb_phys;
unsigned short * g_fb;
int g_fb_size;
struct fb_var_screeninfo g_screen_info;
int g_num_buffers = TEST_BUFFER_NUM;
int g_fmt = V4L2_PIX_FMT_UYVY;
int g_rotate = 0;
int g_vdi_enable = 1;
int g_vdi_motion = 2;
int g_tb = 0;
int g_in_width = 720;
int g_in_height = 480;
int g_display_width = 720;
int g_display_height = 480;
/*--move to struct instance_priv
int g_display_top = 0;
int g_display_left = 0;
-------------------*/
int g_frame_size;
int g_g2d_fmt = G2D_NV12;
int g_mem_type = V4L2_MEMORY_USERPTR;
v4l2_std_id g_current_std = V4L2_STD_NTSC;
static struct ipu_task task;


static void draw_image_to_framebuffer(struct g2d_buf *buf, int img_width, int img_height, int img_format, 
		 struct fb_var_screeninfo *screen_info, int left, int top, int to_width, int to_height, int set_alpha, int rotation)
{
	int i;
	struct g2d_surface src,dst;
	void *g2dHandle;

	if ( ( (left+to_width) > (int)screen_info->xres ) || ( (top+to_height) > (int)screen_info->yres ) )  {
		printf("Bad display image dimensions!\n");
		return;
	}

#if CACHEABLE
        g2d_cache_op(buf, G2D_CACHE_FLUSH);
#endif

	if(g2d_open(&g2dHandle) == -1 || g2dHandle == NULL) {
		printf("Fail to open g2d device!\n");
		g2d_free(buf);
		return;
	}

/*
	NOTE: in this example, all the test image data meet with the alignment requirement.
	Thus, in your code, you need to pay attention on that.

	Pixel buffer address alignment requirement,
	RGB/BGR:  pixel data in planes [0] with 16bytes alignment,
	NV12/NV16:  Y in planes [0], UV in planes [1], with 64bytes alignment,
	I420:    Y in planes [0], U in planes [1], V in planes [2], with 64 bytes alignment,
	YV12:  Y in planes [0], V in planes [1], U in planes [2], with 64 bytes alignment,
	NV21/NV61:  Y in planes [0], VU in planes [1], with 64bytes alignment,
	YUYV/YVYU/UYVY/VYUY:  in planes[0], buffer address is with 16bytes alignment.
*/

	src.format = img_format;
	switch (src.format) {
	case G2D_RGB565:
	case G2D_RGBA8888:
	case G2D_RGBX8888:
	case G2D_BGRA8888:
	case G2D_BGRX8888:
	case G2D_BGR565:
	case G2D_YUYV:
	case G2D_UYVY:
		src.planes[0] = buf->buf_paddr;
		break;
	case G2D_NV12:
		src.planes[0] = buf->buf_paddr;
		src.planes[1] = buf->buf_paddr + img_width * img_height;
		break;
	case G2D_I420:
		src.planes[0] = buf->buf_paddr;
		src.planes[1] = buf->buf_paddr + img_width * img_height;
		src.planes[2] = src.planes[1]  + img_width * img_height / 4;
		break;
	case G2D_NV16:
		src.planes[0] = buf->buf_paddr;
		src.planes[1] = buf->buf_paddr + img_width * img_height;
                break;
	default:
		printf("Unsupport image format in the example code\n");
		return;
	}

	src.left = 0;
	src.top = 0;
	src.right = img_width;
	src.bottom = img_height;
	src.stride = img_width;
	src.width  = img_width;
	src.height = img_height;
	src.rot  = G2D_ROTATION_0;

	dst.planes[0] = g_fb_phys;
	dst.left = left;
	dst.top = top;
	dst.right = left + to_width;
	dst.bottom = top + to_height;
	dst.stride = screen_info->xres;
	dst.width  = screen_info->xres;
	dst.height = screen_info->yres;
	dst.rot    = rotation;
	dst.format = screen_info->bits_per_pixel == 16 ? G2D_RGB565 : (screen_info->red.offset == 0 ? G2D_RGBA8888 : G2D_BGRA8888);

	if (set_alpha)
	{
		src.blendfunc = G2D_ONE;
		dst.blendfunc = G2D_ONE_MINUS_SRC_ALPHA;
	
		src.global_alpha = 0x80;
		dst.global_alpha = 0xff;
	
		g2d_enable(g2dHandle, G2D_BLEND);
		g2d_enable(g2dHandle, G2D_GLOBAL_ALPHA);
	}

	g2d_blit(g2dHandle, &src, &dst);
	g2d_finish(g2dHandle);

	if (set_alpha)
	{
		g2d_disable(g2dHandle, G2D_GLOBAL_ALPHA);
		g2d_disable(g2dHandle, G2D_BLEND);
	}

	g2d_close(g2dHandle);
}

int prepare_g2d(int instns)
{
	int i;

	for (i = 0; i < g_num_buffers; i++) {
#if CACHEABLE
		ins_priv[instns].g2d_buffers[i] = g2d_alloc(g_frame_size, 1);//alloc physical contiguous memory for source image data with cacheable attribute
#else
		ins_priv[instns].g2d_buffers[i] = g2d_alloc(g_frame_size, 0);//alloc physical contiguous memory for source image data
#endif
		if(!ins_priv[instns].g2d_buffers[i]) {
			printf("Fail to allocate physical memory for image buffer!\n");
			return -1;
		}
		//printf("g2d_buffers[%d].buf_paddr = 0x%x.\r\n", i, g2d_buffers[i]->buf_paddr);
	}

	return 0;
}

void memfree(int buf_size, int buf_cnt, int instns)
{
	int i;
        unsigned int page_size;

	page_size = getpagesize();
	buf_size = (buf_size + page_size - 1) & ~(page_size - 1);

	for (i = 0; i < buf_cnt; i++) {
		if (ins_priv[instns].cap_buffers[i].start)
			munmap(ins_priv[instns].cap_buffers[i].start, buf_size);
		if (ins_priv[instns].cap_buffers[i].offset)
			ioctl(fd_ipu, IPU_FREE, &ins_priv[instns].cap_buffers[i].offset);
	}
}

int memalloc(int buf_size, int buf_cnt, int instns)
{
	int i, ret = 0;
        unsigned int page_size;

	for (i = 0; i < buf_cnt; i++) {
		page_size = getpagesize();
		buf_size = (buf_size + page_size - 1) & ~(page_size - 1);
		ins_priv[instns].cap_buffers[i].length = ins_priv[instns].cap_buffers[i].offset = buf_size;
		ret = ioctl(fd_ipu, IPU_ALLOC, &ins_priv[instns].cap_buffers[i].offset);
		if (ret < 0) {
			printf("ioctl IPU_ALLOC fail\n");
			ret = -1;
			goto err;
		}
		ins_priv[instns].cap_buffers[i].start = mmap(0, buf_size, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd_ipu, ins_priv[instns].cap_buffers[i].offset);
		if (!ins_priv[instns].cap_buffers[i].start) {
			printf("mmap fail\n");
			ret = -1;
			goto err;
		}
		printf("USRP: alloc bufs offset 0x%x, start 0x%x, size %d\r\n", ins_priv[instns].cap_buffers[i].offset, ins_priv[instns].cap_buffers[i].start, buf_size);
	}

	return ret;
err:
	memfree(buf_size, buf_cnt, instns);
	return ret;
}

int
v4l_start_capturing(int instns)
{
	unsigned int i;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;

	for (i = 0; i < g_num_buffers; i++) {
		memset(&buf, 0, sizeof (buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = g_mem_type;
		buf.index = i;
		if (g_mem_type == V4L2_MEMORY_USERPTR) {
			buf.length = ins_priv[instns].cap_buffers[i].length;
//			buf.m.userptr = (unsigned long) cap_buffers[i].offset;
			buf.m.userptr = (unsigned long) ins_priv[instns].cap_buffers[i].start;
		}
		if (ioctl(ins_priv[instns].fd_capture_v4l, VIDIOC_QUERYBUF, &buf) < 0) {
				printf("VIDIOC_QUERYBUF error\n");
				return -1;
		}

		if (g_mem_type == V4L2_MEMORY_MMAP) {
					ins_priv[instns].cap_buffers[i].length = buf.length;
					ins_priv[instns].cap_buffers[i].offset = (size_t) buf.m.offset;
					ins_priv[instns].cap_buffers[i].start = mmap (NULL, ins_priv[instns].cap_buffers[i].length,
						PROT_READ | PROT_WRITE, MAP_SHARED,
						ins_priv[instns].fd_capture_v4l, ins_priv[instns].cap_buffers[i].offset);
			memset(ins_priv[instns].cap_buffers[i].start, 0xFF, ins_priv[instns].cap_buffers[i].length);
		}
		printf("cap_buffers[%d].offset = 0x%x.\r\n", i, ins_priv[instns].cap_buffers[i].offset);
		printf("cap_buffers[%d].start = 0x%x.\r\n", i, ins_priv[instns].cap_buffers[i].start);
	}

	for (i = 0; i < g_num_buffers; i++) {
		memset(&buf, 0, sizeof (buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = g_mem_type;
		buf.index = i;
		if (g_mem_type == V4L2_MEMORY_USERPTR)
			buf.m.offset = (unsigned int)ins_priv[instns].cap_buffers[i].start;
		else
			buf.m.offset = ins_priv[instns].cap_buffers[i].offset;
		buf.length = ins_priv[instns].cap_buffers[i].length;
		if (ioctl (ins_priv[instns].fd_capture_v4l, VIDIOC_QBUF, &buf) < 0) {
			printf("VIDIOC_QBUF error\n");
			return -1;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl (ins_priv[instns].fd_capture_v4l, VIDIOC_STREAMON, &type) < 0) {
		printf("VIDIOC_STREAMON error\n");
		return -1;
	}
	return 0;
}

void
v4l_stop_capturing(int instns)
{
	int i;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(ins_priv[instns].fd_capture_v4l, VIDIOC_STREAMOFF, &type);

	for (i = 0; i < g_num_buffers; i++)
	{
		g2d_free(ins_priv[instns].g2d_buffers[i]);
	}

	if (g_mem_type == V4L2_MEMORY_USERPTR) {
		memfree(g_frame_size, g_num_buffers, instns);
	} else {
		for (i = 0; i < g_num_buffers; i++)
		{
			munmap(ins_priv[instns].cap_buffers[i].start, ins_priv[instns].cap_buffers[i].length);
		}
	}

	munmap(g_fb, g_fb_size);
	close(fd_fb);
	close(fd_ipu);
	close(ins_priv[instns].fd_capture_v4l);
	ins_priv[instns].fd_capture_v4l = -1;
}

int
v4l_capture_setup(struct encode *enc, int width, int height, int fps)
{
	char v4l_capture_dev[80], node[8];
	struct v4l2_input input;
	struct v4l2_fmtdesc fmtdesc;
	struct v4l2_capability cap;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;
	v4l2_std_id id;
	unsigned int min, i;
	int instns = enc->cmdl->instns;
	struct fb_fix_screeninfo fb_info;

	if (ins_priv[instns].fd_capture_v4l > 0) {
		warn_msg("capture device already opened\n");
		return -1;
	}

	g_display_width = enc->cmdl->display_width;
	g_display_height = enc->cmdl->display_height;
	ins_priv[instns].g_display_top = enc->cmdl->display_top;
	ins_priv[instns].g_display_left = enc->cmdl->display_left;
	g_frame_size = width * height * 2;
	printf("v4l_capture_setup: display left = %d, top = %d, width = %d, height = %d.\r\n", ins_priv[instns].g_display_left, ins_priv[instns].g_display_top, g_display_width, g_display_height);

	sprintf(node, "%d", enc->cmdl->video_node_capture);
	strcpy(v4l_capture_dev, "/dev/video");
	strcat(v4l_capture_dev, node);

	if ((ins_priv[instns].fd_capture_v4l = open(v4l_capture_dev, O_RDWR, 0)) < 0) {
		err_msg("Unable to open %s\n", v4l_capture_dev);
		return -1;
	}

	if ((fd_ipu = open("/dev/mxc_ipu", O_RDWR, 0)) < 0) {
		printf("open ipu dev fail\n");
		return -1;
	}

	if ((fd_fb = open(fb_dev, O_RDWR, 0)) < 0) {
		printf("Unable to open %s\n", fb_dev);
		return -1;
	}

	/* Get fix screen info. */
	if ((ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_info)) < 0) {
		printf("%s FBIOGET_FSCREENINFO failed.\n", fb_dev);
		return -1;
	}
	g_fb_phys = fb_info.smem_start;

	/* Get variable screen info. */
	if ((ioctl(fd_fb, FBIOGET_VSCREENINFO, &g_screen_info)) < 0) {
		printf("%s FBIOGET_VSCREENINFO failed.\n", fb_dev);
		return -1;
	}

	g_fb_phys += (g_screen_info.xres_virtual * g_screen_info.yoffset * g_screen_info.bits_per_pixel / 8);

	g_fb_size = g_screen_info.xres_virtual * g_screen_info.yres_virtual * g_screen_info.bits_per_pixel / 8;

	/* Map the device to memory*/
	g_fb = (unsigned short *) mmap(0, g_fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
	if ((int)g_fb <= 0) {
		printf("\nError: failed to map framebuffer device to memory.\n");
		return -1;
	}

	if (g_mem_type == V4L2_MEMORY_USERPTR)
		if (memalloc(g_frame_size, g_num_buffers, instns) < 0)
			return -1;

	if (prepare_g2d(instns) < 0) {
		printf("prepare_output failed\n");
		return -1;
	}

	memset(&task, 0, sizeof(struct ipu_task));
	task.output.width = g_in_width;
	task.output.height = g_in_height;
	task.output.crop.w = g_in_width;
	task.output.crop.h = g_in_height;
	task.output.format = V4L2_PIX_FMT_NV12;

	task.input.width  = g_in_width;
	task.input.height = g_in_height;
	task.input.crop.w = g_in_width;
	task.input.crop.h = g_in_height;
	task.input.format = g_fmt;
	task.input.deinterlace.enable = g_vdi_enable;
	task.input.deinterlace.motion = g_vdi_motion;

	task.input.paddr = ins_priv[instns].cap_buffers[0].offset;
	task.output.paddr = ins_priv[instns].g2d_buffers[0]->buf_paddr;

	if (ioctl(fd_ipu, IPU_CHECK_TASK, &task) != IPU_CHECK_OK) {
		printf("IPU_CHECK_TASK failed.\r\n");
		return -1;
	}

	if (ioctl (ins_priv[instns].fd_capture_v4l, VIDIOC_QUERYCAP, &cap) < 0) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s is no V4L2 device\n",
					v4l_capture_dev);
			return -1;
		} else {
			fprintf (stderr, "%s isn not V4L device,unknow error\n",
			v4l_capture_dev);
			return -1;
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf (stderr, "%s is no video capture device\n",
			v4l_capture_dev);
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf (stderr, "%s does not support streaming i/o\n",
			v4l_capture_dev);
		return -1;
	}

	input.index = 0;
	while (ioctl(ins_priv[instns].fd_capture_v4l, VIDIOC_ENUMINPUT, &input) >= 0)
	{
		input.index += 1;
	}

	if (ioctl(ins_priv[instns].fd_capture_v4l, VIDIOC_G_STD, &id) < 0)
	{
		printf("VIDIOC_G_STD failed\n");
		close(ins_priv[instns].fd_capture_v4l);
		return -1;
	}
	g_current_std = id;

	fmtdesc.index = 0;
	while (ioctl(ins_priv[instns].fd_capture_v4l, VIDIOC_ENUM_FMT, &fmtdesc) >= 0)
	{
		fmtdesc.index += 1;
	}

	/* Select video input, video standard and tune here. */
	memset(&fmt, 0, sizeof(fmt));

	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = g_in_width;
	fmt.fmt.pix.height      = g_in_height;
	fmt.fmt.pix.pixelformat = g_fmt;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	if (ioctl (ins_priv[instns].fd_capture_v4l, VIDIOC_S_FMT, &fmt) < 0){
		fprintf (stderr, "%s iformat not supported \n",
			v4l_capture_dev);
		return -1;
	}

	/* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;

	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	if (ioctl(ins_priv[instns].fd_capture_v4l, VIDIOC_G_FMT, &fmt) < 0)
	{
		printf("VIDIOC_G_FMT failed\n");
		close(ins_priv[instns].fd_capture_v4l);
		return -1;
	}

	g_in_width = fmt.fmt.pix.width;
	g_in_height = fmt.fmt.pix.height;

	memset(&req, 0, sizeof (req));

	req.count = g_num_buffers;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = g_mem_type;

	if (ioctl (ins_priv[instns].fd_capture_v4l, VIDIOC_REQBUFS, &req) < 0) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s does not support "
					 "memory mapping\n", v4l_capture_dev);
			return -1;
		} else {
			fprintf (stderr, "%s does not support "
					 "memory mapping, unknow error\n", v4l_capture_dev);
			return -1;
		}
	}

	if (req.count < 2) {
		fprintf (stderr, "Insufficient buffer memory on %s\n",
			 v4l_capture_dev);
		return -1;
	}

	return 0;
}

int
v4l_get_capture_data(struct v4l2_buffer *buf, int instns)
{
	memset(buf, 0, sizeof(struct v4l2_buffer));
	buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf->memory = g_mem_type;
	if (ioctl(ins_priv[instns].fd_capture_v4l, VIDIOC_DQBUF, buf) < 0) {
		err_msg("VIDIOC_DQBUF failed\n");
		return -1;
	}

	return 0;
}

void
v4l_put_capture_data(struct v4l2_buffer *buf, int instns)
{
	ioctl(ins_priv[instns].fd_capture_v4l, VIDIOC_QBUF, buf);
}

void v4l_deinterlace_capture_data(struct v4l2_buffer *buf, int instns)
{
//qiang_debug add start
/*
	int total_time;
	struct timeval tv_start, tv_current;

	gettimeofday(&tv_start, 0);
*/
//qiang_debug add end

	task.input.paddr = ins_priv[instns].cap_buffers[buf->index].offset;
	task.output.paddr = ins_priv[instns].g2d_buffers[buf->index]->buf_paddr;
	if ((task.input.paddr != 0) && (task.output.paddr != 0)) {
		if (ioctl(fd_ipu, IPU_QUEUE_TASK, &task) < 0) {
			printf("IPU_QUEUE_TASK failed\n");		/*zorro err, some times, software pend this err when enc*/
		}
	}

//qiang_debug add start
/*
	gettimeofday(&tv_current, 0);
	total_time = (tv_current.tv_sec - tv_start.tv_sec) * 1000000L;
	total_time += tv_current.tv_usec - tv_start.tv_usec;
	printf("ipu time = %u us\n", total_time);
*/
//qiang_debug add end
}

void v4l_render_capture_data(struct encode *enc, int index)
{
//qiang_debug add start
/*
	int total_time;
	struct timeval tv_start, tv_current;

	gettimeofday(&tv_start, 0);
*/
//qiang_debug add end
	int instns = enc->cmdl->instns;

	if (enc->cmdl->disp_mode == 1)
		draw_image_to_framebuffer(ins_priv[instns].g2d_buffers[index], g_in_width, g_in_height, g_g2d_fmt, &g_screen_info, ins_priv[instns].g_display_left, ins_priv[instns].g_display_top, g_display_width, g_display_height, 0, G2D_ROTATION_0);
	else if (enc->cmdl->disp_mode == 2)
		draw_image_to_framebuffer(ins_priv[instns].g2d_buffers[index], g_in_width, g_in_height, g_g2d_fmt, &g_screen_info, 0, 0, 1920, 1080, 0, G2D_ROTATION_0);
//qiang_debug add start
/*
	gettimeofday(&tv_current, 0);
	total_time = (tv_current.tv_sec - tv_start.tv_sec) * 1000000L;
	total_time += tv_current.tv_usec - tv_start.tv_usec;
	printf("g2d time = %u us\n", total_time);
*/
//qiang_debug add end
}

