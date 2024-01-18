// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Phytium JPEG Encoder Engine
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include "phytium_jpeg_reg.h"
#include "phytium_jpeg_core.h"
#include <linux/arm-smccc.h>

static u32 phytium_jpeg_header[PHYTIUM_JPEG_HEADER_SIZE] = {
	0xe0ffd8ff, 0x464a0100, 0x01004649, 0x01000001,
	0x00000100, 0x4300dbff, 0x0c0b1000, 0x100a0c0e,
	0x120e0d0e, 0x18131011, 0x16181a28, 0x23311816,
	0x3a281d25, 0x393c3d33, 0x40373833, 0x404e5c48,
	0x37455744, 0x516d5038, 0x67625f57, 0x4d3e6768,
	0x64707971, 0x67655c78, 0x00dbff63, 0x12110143,
	0x18151812, 0x2f1a1a2f, 0x42384263, 0x63636363,
	0x63636363, 0x63636363, 0x63636363, 0x63636363,
	0x63636363, 0x63636363, 0x63636363, 0x63636363,
	0x63636363, 0x63636363, 0x63636363, 0xc0ff6363,
	/* h_index(40) indicates high 8 bits of the height
	 * w_index(41) contains the low 8 bits of the height,
	 * and the width. For example, height 480(0x01e0)
	 * locates at 0x<01> 081100 and 0x038002 <e0>.
	 * width 640 (0x0280) locates at 0x03 <80> <02> e0.
	 */

	/* 0x0200 <11> 01 is a field marks YUV mode */
	0x01081100, 0x038002e0, 0x02001101, 0x11030111,
	0x00c4ff01, 0x0100001f, 0x01010105, 0x00010101,
	0x00000000, 0x01000000, 0x05040302, 0x09080706,
	0xc4ff0b0a, 0x00011f00, 0x01010103, 0x01010101,
	0x00000101, 0x00000000, 0x04030201, 0x08070605,
	0xff0b0a09, 0x10b500c4, 0x03010200, 0x03040203,
	0x04040505, 0x7d010000, 0x00030201, 0x12051104,
	0x06413121, 0x07615113, 0x32147122, 0x08a19181,
	0xc1b14223, 0xf0d15215, 0x72623324, 0x160a0982,
	0x1a191817, 0x28272625, 0x35342a29, 0x39383736,
	0x4544433a, 0x49484746, 0x5554534a, 0x59585756,
	0x6564635a, 0x69686766, 0x7574736a, 0x79787776,
	0x8584837a, 0x89888786, 0x9493928a, 0x98979695,
	0xa3a29a99, 0xa7a6a5a4, 0xb2aaa9a8, 0xb6b5b4b3,
	0xbab9b8b7, 0xc5c4c3c2, 0xc9c8c7c6, 0xd4d3d2ca,
	0xd8d7d6d5, 0xe2e1dad9, 0xe6e5e4e3, 0xeae9e8e7,
	0xf4f3f2f1, 0xf8f7f6f5, 0xc4fffaf9, 0x0011b500,
	0x04020102, 0x07040304, 0x00040405, 0x00770201,
	0x11030201, 0x31210504, 0x51411206, 0x13716107,
	0x08813222, 0xa1914214, 0x2309c1b1, 0x15f05233,
	0x0ad17262, 0xe1342416, 0x1817f125, 0x27261a19,
	0x352a2928, 0x39383736, 0x4544433a, 0x49484746,
	0x5554534a, 0x59585756, 0x6564635a, 0x69686766,
	0x7574736a, 0x79787776, 0x8483827a, 0x88878685,
	0x93928a89, 0x97969594, 0xa29a9998, 0xa6a5a4a3,
	0xaaa9a8a7, 0xb5b4b3b2, 0xb9b8b7b6, 0xc4c3c2ba,
	0xc8c7c6c5, 0xd3d2cac9, 0xd7d6d5d4, 0xe2dad9d8,
	0xe6e5e4e3, 0xeae9e8e7, 0xf5f4f3f2, 0xf9f8f7f6,
	0x00fefffa, 0x0000008f, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0xdaff0000, 0x01030c00, 0x03110200, 0x003f0011
};

static char yuv_mode_str[YUV_MODE_STR_LEN] = { "yuv444" };

module_param_string(yuv_mode, yuv_mode_str, sizeof(yuv_mode_str), 0444);
MODULE_PARM_DESC(yuv_mode, "Users select one mode from such modes as 'yuv444', or 'yuv422', or 'yuv420'. If no mode is set, the driver adapts defaults mode 'yuv444'.");

static u32 phytium_jpeg_read(struct phytium_jpeg_dev *jpeg_dev, u32 reg)
{
	u32 reg_val = readl(jpeg_dev->base_addr + reg);

	dev_dbg(jpeg_dev->dev, "read 0x%p + 0x%x -->val[0x%x]\n",
		  jpeg_dev->base_addr, reg, reg_val);

	return reg_val;
}

static void phytium_jpeg_write(struct phytium_jpeg_dev *jpeg_dev,
				u32 reg, u32 val)
{
	writel(val, jpeg_dev->base_addr + reg);
	dev_dbg(jpeg_dev->dev, "write 0x%x to addr 0x%p + 0x%x\n",
		  val, jpeg_dev->base_addr, reg);
}

static void phytium_jpeg_update(struct phytium_jpeg_dev *jpeg_dev, u32 reg,
				u32 clear, u32 bits)
{
	u32 reg_val = readl(jpeg_dev->base_addr + reg);
	u32 tmp = reg_val;

	reg_val &= ~clear;
	reg_val |= bits;
	writel(reg_val, jpeg_dev->base_addr + reg);

	dev_dbg(jpeg_dev->dev, "the val of addr 0x%p + 0x%x, from 0x%x to 0x%x\n",
		  jpeg_dev->base_addr, reg, tmp, readl(jpeg_dev->base_addr + reg));
}

static void phytium_jpeg_init_regs(struct phytium_jpeg_dev *jpeg_dev)
{
	u32 transform_info = 0;
	u32 disable_all_interrupt = 0;
	u32 clear_all_interrupt = INT_FIFO_OVERFLOW | INT_OCM_BUF_OVERFLOW |
			INT_JPEG_ENCODE_COMPLETE | INT_VIDEO_FORMAT_CHANGE;
	u32 rate_to_reg = 0;

	/* First, disable the JPEG engine, set bit0 = 0*/
	phytium_jpeg_write(jpeg_dev, TRANSFORM_INFO_REG, transform_info);

	/* Second, set VGAvideo_source_information. bit1 = 0 marks VGA */
	transform_info |= 0;

	/* Third, set AXI burst length bit[16:22]= 0xf , default value*/
	transform_info |= (0xF << TRANS_AXI_LEN_SHIFT) & TRANSINFO_AXI_LEN;

	/* Fourth, the default sampling format is YUV422, set bit13 to 0 */
	/* ignore setting sampling interval */
	phytium_jpeg_write(jpeg_dev, TRANSFORM_INFO_REG, transform_info);
	udelay(5);

	/* Fifth, setting frame rate.
	 *   Linux driver prohibit float point operations. So use the
	 *   format: reg_val = (1 second * 10^8 / frame_rate / 134 *100)
	 *   write reg_val to register. then enable Highest bit31 = 1
	 */
	if (jpeg_dev->frame_rate) {
		rate_to_reg = 100000000 / jpeg_dev->frame_rate / 134 * 100;
		rate_to_reg |= FRAME_SAMPLE_CTRL_EN;
		phytium_jpeg_write(jpeg_dev, FRAME_SAMPLE_CTRL, rate_to_reg);
	}
	/* Sixth, HUFF_MODE, driver needn't to configure, ignore */

	/* disable all interrupts  and then clear all interrupts */
	phytium_jpeg_write(jpeg_dev, INT_STATUS_CTRL_REG,
			   disable_all_interrupt);
	udelay(5);
	phytium_jpeg_write(jpeg_dev, INT_STATUS_CTRL_REG, clear_all_interrupt);

	/* Seventh, Sample_mode, hardware default is yuv444 */
	 jpeg_dev->yuv420 = false;
}

/* Turn on the clock of the jpeg engine */
static void phytium_jpeg_on(struct phytium_jpeg_dev *jpeg_dev)
{
	if (test_bit(VIDEO_CLOCKS_ON, &jpeg_dev->status))
		return;

	/* Turn on the relevant clocks */
	set_bit(VIDEO_CLOCKS_ON, &jpeg_dev->status);
}

/* Disable the jpeg engine */
static void phytium_jpeg_off(struct phytium_jpeg_dev *jpeg_dev)
{
	u32 disable_all_interrupt = 0;
	u32 clear_all_interrupt = INT_FIFO_OVERFLOW | INT_OCM_BUF_OVERFLOW |
			INT_JPEG_ENCODE_COMPLETE | INT_VIDEO_FORMAT_CHANGE;

	if (!test_bit(VIDEO_CLOCKS_ON, &jpeg_dev->status)) {
		dev_info(jpeg_dev->dev, "JPEG Engine is already off.\n");
		return;
	}

	/* disable all interrupt */
	phytium_jpeg_write(jpeg_dev, INT_STATUS_CTRL_REG, disable_all_interrupt);
	/* clear all interrupt */
	phytium_jpeg_write(jpeg_dev, INT_STATUS_CTRL_REG, clear_all_interrupt);
	/* disable JPEG engine */
	phytium_jpeg_update(jpeg_dev, TRANSFORM_INFO_REG, TRANSINFO_ENABLE_ENGINE, 0);

	clear_bit(VIDEO_CLOCKS_ON, &jpeg_dev->status);
	/* wait 50 ms */
	mdelay(50);
	/* C08 bit7 1:busy */
}

static inline void phytium_jpeg_enable_source_detecting(struct phytium_jpeg_dev *jpeg_dev)
{
	/*
	 * Enable the dectection to discovery
	 * the source resolution is changed
	 */
	//phytium_jpeg_update(jpeg_dev, INT_STATUS_CTRL_REG, 0, DETECT_RESOLUTION_CHANGE_EN);
	phytium_jpeg_update(jpeg_dev, TRANSFORM_INFO_REG, 0, TRANSINFO_SRC_SELECT);
}

#define res_check(val) \
	test_and_clear_bit(VIDEO_MODE_DETECT_DONE, &(val)->status)

static void phytium_jpeg_get_resolution(struct phytium_jpeg_dev *jpeg_dev)
{
	u32 source_info;
	u32 width;
	u32 height;
	struct v4l2_bt_timings *detected_timings = &jpeg_dev->detected_timings;

	/* Before get a new resolution, maybe need to wait 10 us */
	detected_timings->width = MIN_WIDTH;
	detected_timings->height = MIN_HEIGHT;
	jpeg_dev->v4l2_input_status = V4L2_IN_ST_NO_SIGNAL;


	phytium_jpeg_enable_source_detecting(jpeg_dev);
	source_info = phytium_jpeg_read(jpeg_dev, SRC_VGA_INFO_REG);
	width = (source_info & SRC_HOR_PIXELS) >> SRC_WIDTH_SHIFT;
	height = (source_info & SRC_VER_PIXELS) >> SRC_HEIGHT_SHIFT;

	if (width * height != 0) {
		detected_timings->width = width;
		detected_timings->height = height;
	}

	jpeg_dev->v4l2_input_status = 0;

	/*
	 * Resolution is changed will trigger an interrupt, resolution detecting
	 * also is disable during process interrupt. So re-enable.
	 */
	phytium_jpeg_enable_source_detecting(jpeg_dev);
	dev_info(jpeg_dev->dev, "Change resolution: %uX%u\n", width, height);
}

static void phytium_jpeg_set_resolution(struct phytium_jpeg_dev *jpeg_dev)
{
	struct v4l2_bt_timings  *active_timings = &jpeg_dev->active_timings;
	int i;
	int src_addrs[OCM_BUF_NUM];
	/*
	 * The OCM address space is 0x30C0_0000 ~ 0x30C7_FFFF, JPEG Engine uses the
	 * high-bottom address. src_0 uses 0x30C4_0000 ~ 0x30c6_0000 (total capacity is
	 * 128KB, greater than the requirements of the largest resolution). src_1 uses
	 * 0x30C6_0000 ~ 0x30C7_FFFF.
	 */

	/* The OCM address should shift right 8 bits */
	for (i = 0; i < OCM_BUF_NUM; i++)
		src_addrs[i] = jpeg_dev->src_addrs[i].dma_addr >> OCM_BUF_SHIFT;

	phytium_jpeg_write(jpeg_dev, OCM_BUF0_ADDR, src_addrs[0]);
	phytium_jpeg_write(jpeg_dev, OCM_BUF1_ADDR, src_addrs[1]);

	/*
	 * In the worst case, the size of one image will be compressed to 25% the
	 * raw image's size. When a pixel is 4-byte, no need to divide 4.
	 */
	jpeg_dev->max_compressed_size = active_timings->width * active_timings->height;
}

/* The below functions is implemented for various v4l2 ioctl operations */
static int phytium_jpeg_querycap(struct file *file, void *priv,
				 struct v4l2_capability *cap)
{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	strscpy(cap->driver, PHYTIUM_JPEG_NAME, sizeof(cap->driver));
	strscpy(cap->card, "Phytium JPEG Engine", sizeof(cap->card));
	snprintf(cap->bus_info, sizeof(cap->bus_info), "platform:%s", dev_name(jpeg_dev->dev));

	return 0;
}

static int phytium_jpeg_enum_format(struct file *file, void *priv,
				    struct v4l2_fmtdesc *f)
{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	if (f->index) {
		dev_err(jpeg_dev->dev, "Failed to enum format\n");
		return -EINVAL;
	}

	f->pixelformat = V4L2_PIX_FMT_JPEG;

	return 0;
}

static int phytium_jpeg_get_format(struct file *file, void *priv,
				   struct v4l2_format *f)
{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	f->fmt.pix = jpeg_dev->pix_fmt;

	return 0;
}

static int phytium_jpeg_enum_input(struct file *file, void *priv,
				struct v4l2_input *input)
{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	if (input->index) {
		dev_err(jpeg_dev->dev, "failed to enum input\n");
		return -EINVAL;
	}

	strscpy(input->name, "Host DC Capture", sizeof(input->name));
	input->type = V4L2_INPUT_TYPE_CAMERA;
	input->capabilities = V4L2_IN_CAP_DV_TIMINGS;
	input->status = jpeg_dev->v4l2_input_status;

	return 0;
}

static int phytium_jpeg_get_input(struct file *file, void *priv,
				  unsigned int *i)
{
	*i = 0;
	return 0;
}

static int phytium_jpeg_set_input(struct file *file, void *priv,
				  unsigned int i)
{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	if (i != 0) {
		dev_err(jpeg_dev->dev, "Failed to set input\n");
		return -EINVAL;
	}

	return 0;
}

static int phytium_jpeg_get_parm(struct file *file, void *priv,
			struct v4l2_streamparm *stream)
{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);
	/* Readbuffers num is 3 */
	stream->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	stream->parm.capture.readbuffers = CAPTURE_BUF_NUMBER;
	stream->parm.capture.timeperframe.denominator = 1;

	if (jpeg_dev->frame_rate == 0)
		stream->parm.capture.timeperframe.denominator = MAX_FRAME_RATE;
	else
		stream->parm.capture.timeperframe.denominator = jpeg_dev->frame_rate;

	return 0;
}

static int phytium_jpeg_set_parm(struct file *file, void *priv,
				struct v4l2_streamparm *stream)
{
	unsigned int frame_rate = 0;
	u32 rate_to_reg = 0;
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);
	 /* Readbuffers num is 3 */
	stream->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	stream->parm.capture.readbuffers = CAPTURE_BUF_NUMBER;

	if (stream->parm.capture.timeperframe.numerator)
		frame_rate = stream->parm.capture.timeperframe.denominator /
			stream->parm.capture.timeperframe.numerator;

	if (frame_rate == 0 || frame_rate > MAX_FRAME_RATE) {
		frame_rate = MAX_FRAME_RATE;
		stream->parm.capture.timeperframe.denominator = MAX_FRAME_RATE;
		stream->parm.capture.timeperframe.numerator = 1;
	}
	/*
	 *   reg_val = (1 second * 10^9 / frame_rate / 13.4)
	 *   Linux driver prohibit float point operations. So use the
	 *   format: reg_val = (1 second * 10^8 / frame_rate / 134 *100)
	 *   write reg_val to register. then enable Highest bit31 = 1
	 */
	if (jpeg_dev->frame_rate != frame_rate) {
		jpeg_dev->frame_rate = frame_rate;
		rate_to_reg = 100000000 / jpeg_dev->frame_rate / 134 * 100;
		rate_to_reg |= FRAME_SAMPLE_CTRL_EN;
		phytium_jpeg_write(jpeg_dev, FRAME_SAMPLE_CTRL, rate_to_reg);
	}

	return 0;
}

static int phytium_jpeg_enum_framesizes(struct file *file, void *priv,
					struct v4l2_frmsizeenum *fsize)

{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	if (fsize->index != 0) {
		dev_err(jpeg_dev->dev, "Failed to enum framesize.\n");
		return -EINVAL;
	}

	if (fsize->pixel_format != V4L2_PIX_FMT_JPEG) {
		dev_err(jpeg_dev->dev, "enum framesize pixel_format is not JPEG");
		return -EINVAL;
	}

	fsize->discrete.width = jpeg_dev->pix_fmt.width;
	fsize->discrete.height = jpeg_dev->pix_fmt.height;
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;

	return 0;

}

static int phytium_jpeg_enum_frameintervals(struct file *file, void *priv,
					    struct v4l2_frmivalenum *fival)
{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	if (fival->index != 0) {
		dev_err(jpeg_dev->dev, "enum frame intervals failed\n");
		return -EINVAL;
	}

	if (fival->width != jpeg_dev->detected_timings.width ||
		fival->height != jpeg_dev->detected_timings.height) {
		dev_err(jpeg_dev->dev, "interval isn't same with the detected_timings.\n");
		return -EINVAL;
	}

	if (fival->pixel_format != V4L2_PIX_FMT_JPEG) {
		dev_err(jpeg_dev->dev, "enum frame interval pixel fomat is incorrect.\n");
		return -EINVAL;
	}

	fival->type = V4L2_FRMIVAL_TYPE_CONTINUOUS;
	fival->stepwise.min.denominator = MAX_FRAME_RATE;
	fival->stepwise.min.numerator = 1;
	fival->stepwise.max.denominator = 1;
	fival->stepwise.max.numerator = 1;
	fival->stepwise.step = fival->stepwise.max;

	return 0;
}

static int phytium_jpeg_set_dv_timings(struct file *file, void *priv,
				       struct v4l2_dv_timings *timings)
{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	/* the params are passed from user space are same with hardware's params */
	if (timings->bt.width == jpeg_dev->active_timings.width &&
	    timings->bt.height == jpeg_dev->active_timings.height)
		return 0;

	if (vb2_is_busy(&jpeg_dev->queue)) {
		dev_err(jpeg_dev->dev, "queue is busy during setting dv timings.\n");
		return -EBUSY;
	}

	jpeg_dev->active_timings = timings->bt;
	phytium_jpeg_set_resolution(jpeg_dev);
	jpeg_dev->pix_fmt.width = timings->bt.width;
	jpeg_dev->pix_fmt.height = timings->bt.height;
	jpeg_dev->pix_fmt.sizeimage = jpeg_dev->max_compressed_size;
	timings->type = V4L2_DV_BT_656_1120;

	return 0;
}

static int phytium_jpeg_get_dv_timings(struct file *file, void *priv,
				       struct v4l2_dv_timings *timings)
{
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	timings->type = V4L2_DV_BT_656_1120;
	timings->bt = jpeg_dev->active_timings;

	return 0;
}

static int phytium_jpeg_query_dv_timings(struct file *file, void *priv,
					 struct v4l2_dv_timings *timings)
{
	int ret;
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	/*
	 * This blocks only if the driver is currently in the process of
	 * detecting a new resolution; in the event of no signal or timeout
	 * this function is woken up.
	 */
	if ((file->f_flags & O_NONBLOCK) &&
	    test_bit(VIDEO_RES_CHANGE, &jpeg_dev->status))
		return -EAGAIN;

	ret = wait_event_interruptible(jpeg_dev->wait, !test_bit(VIDEO_RES_CHANGE,
				       &jpeg_dev->status));
	if (ret) {
		dev_err(jpeg_dev->dev, "Failed to query dv timing\n");
		return -EINTR;
	}

	timings->type = V4L2_DV_BT_656_1120;
	timings->bt = jpeg_dev->detected_timings;

	return jpeg_dev->v4l2_input_status ? -ENOLINK : 0;
}

static const struct v4l2_dv_timings_cap phytium_jpeg_timings_cap = {
	.type = V4L2_DV_BT_656_1120,
	.bt = {
		.min_width = MIN_WIDTH,
		.max_width = MAX_WIDTH,
		.min_height = MIN_HEIGHT,
		.max_height = MAX_HEIGHT,
		.min_pixelclock = 6574080, /* 640 x 480 x 24Hz */
		.max_pixelclock = 1244160000, /* 1920 x 1080 x 60Hz */
		.standards = V4L2_DV_BT_STD_CEA861 | V4L2_DV_BT_STD_DMT |
			V4L2_DV_BT_STD_CVT | V4L2_DV_BT_STD_GTF,
		.capabilities = V4L2_DV_BT_CAP_PROGRESSIVE |
			V4L2_DV_BT_CAP_REDUCED_BLANKING |
			V4L2_DV_BT_CAP_CUSTOM,
	},
};

static int phytium_jpeg_enum_dv_timings(struct file *file, void *priv,
				struct v4l2_enum_dv_timings *timings)
{
	return v4l2_enum_dv_timings_cap(timings, &phytium_jpeg_timings_cap,
					NULL, NULL);
}

static int phytium_jpeg_dv_timings_cap(struct file *file, void *priv,
				struct v4l2_dv_timings_cap *cap)
{
	*cap = phytium_jpeg_timings_cap;

	return 0;
}

/* The function is used to notify DV that video resolution is altered */
static int phytium_jpeg_sub_event(struct v4l2_fh *fh,
				  const struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_SOURCE_CHANGE:
		return v4l2_src_change_event_subscribe(fh, sub);
	default:
		break;
	}

	return v4l2_ctrl_subscribe_event(fh, sub);
}

static const struct v4l2_ioctl_ops phytium_jpeg_ioctl_ops = {
	.vidioc_querycap = phytium_jpeg_querycap,
	.vidioc_enum_fmt_vid_cap = phytium_jpeg_enum_format,
	.vidioc_g_fmt_vid_cap = phytium_jpeg_get_format,
	.vidioc_s_fmt_vid_cap = phytium_jpeg_get_format,
	.vidioc_try_fmt_vid_cap = phytium_jpeg_get_format,
	.vidioc_reqbufs = vb2_ioctl_reqbufs,
	.vidioc_querybuf = vb2_ioctl_querybuf,
	.vidioc_qbuf = vb2_ioctl_qbuf,
	.vidioc_expbuf = vb2_ioctl_expbuf,
	.vidioc_dqbuf = vb2_ioctl_dqbuf,
	.vidioc_create_bufs = vb2_ioctl_create_bufs,
	.vidioc_prepare_buf = vb2_ioctl_prepare_buf,
	.vidioc_streamon = vb2_ioctl_streamon,
	.vidioc_streamoff = vb2_ioctl_streamoff,
	.vidioc_enum_input = phytium_jpeg_enum_input,
	.vidioc_g_input = phytium_jpeg_get_input,
	.vidioc_s_input = phytium_jpeg_set_input,
	.vidioc_g_parm = phytium_jpeg_get_parm,
	.vidioc_s_parm = phytium_jpeg_set_parm,
	.vidioc_enum_framesizes = phytium_jpeg_enum_framesizes,
	.vidioc_enum_frameintervals = phytium_jpeg_enum_frameintervals,
	.vidioc_s_dv_timings = phytium_jpeg_set_dv_timings,
	.vidioc_g_dv_timings = phytium_jpeg_get_dv_timings,
	.vidioc_query_dv_timings = phytium_jpeg_query_dv_timings,
	.vidioc_enum_dv_timings = phytium_jpeg_enum_dv_timings,
	.vidioc_dv_timings_cap = phytium_jpeg_dv_timings_cap,
	.vidioc_subscribe_event = phytium_jpeg_sub_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
};

static void phytium_jpeg_init_jpeg_quant(struct phytium_jpeg_dev *jpeg_dev)
{
	const u32 y_quant_table[QUANT_REG_NUM] = {
	    0x08000000, 0x0ba2e8ba, 0x0aaaaaab, 0x09249249, 0x0aaaaaab,
	    0x0ccccccc, 0x08000000, 0x09249249, 0x09d89d8a, 0x09249249,
	    0x071c71c7, 0x07878788, 0x08000000, 0x06bca1af, 0x05555555,
	    0x03333333, 0x04ec4ec5, 0x05555555, 0x05d1745d, 0x05d1745d,
	    0x05555555, 0x029cbc15, 0x03a83a84, 0x03759f23, 0x0469ee58,
	    0x03333333, 0x0234f72c, 0x02828283, 0x02192e2a, 0x02222222,
	    0x023ee090, 0x02828283, 0x02492492, 0x0253c825, 0x02000000,
	    0x01c71c72, 0x01642c86, 0x01a41a42, 0x02000000, 0x01e1e1e2,
	    0x0178a4c8, 0x01dae607, 0x0253c825, 0x02492492, 0x0199999a,
	    0x012c9fb5, 0x01948b10, 0x0178a4c8, 0x0158ed23, 0x014e5e0a,
	    0x013e22cc, 0x013b13b1, 0x013e22cc, 0x02108421, 0x01a98ef6,
	    0x0121fb78, 0x010ecf57, 0x01249249, 0x013e22cc, 0x01111111,
	    0x01642c86, 0x01446f86, 0x013e22cc, 0x014afd6a
	};

	const u32 c_quant_table[QUANT_REG_NUM] = {
	    0x07878788, 0x071c71c7, 0x071c71c7, 0x05555555, 0x06186186,
	    0x05555555, 0x02b93105, 0x04ec4ec5, 0x04ec4ec5, 0x02b93105,
	    0x014afd6a, 0x01f07c1f, 0x02492492, 0x01f07c1f, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a,
	    0x014afd6a, 0x014afd6a, 0x014afd6a, 0x014afd6a
	};
	int i;

	for (i = 0; i < QUANT_REG_NUM; i++) {
		phytium_jpeg_write(jpeg_dev, Y_QUANT_INDEX_ADDR_REG(i), y_quant_table[i]);
		phytium_jpeg_write(jpeg_dev, C_QUANT_INDEX_ADDR_REG(i), c_quant_table[i]);
	}

}

static void phytium_jpeg_start(struct phytium_jpeg_dev *jpeg_dev)
{
	phytium_jpeg_on(jpeg_dev);
	phytium_jpeg_init_regs(jpeg_dev);

	/* Resolution set to 640x480 if no signal is found */
	phytium_jpeg_get_resolution(jpeg_dev);

	/* Set timings since the device is being opened for the first tiime */
	jpeg_dev->active_timings =  jpeg_dev->detected_timings;
	phytium_jpeg_set_resolution(jpeg_dev);

	jpeg_dev->pix_fmt.width = jpeg_dev->active_timings.width;
	jpeg_dev->pix_fmt.height = jpeg_dev->active_timings.height;
	jpeg_dev->pix_fmt.sizeimage = jpeg_dev->max_compressed_size;
}

static void phytium_jpeg_stop(struct phytium_jpeg_dev *jpeg_dev)
{
	set_bit(VIDEO_STOPPED, &jpeg_dev->status);
	cancel_delayed_work_sync(&jpeg_dev->res_work);

	phytium_jpeg_off(jpeg_dev);

	jpeg_dev->v4l2_input_status = V4L2_IN_ST_NO_SIGNAL;
	jpeg_dev->status = 0;
}

static int phytium_jpeg_open(struct file *file)
{
	int ret;
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	mutex_lock(&jpeg_dev->video_lock);

	ret = v4l2_fh_open(file);
	if (ret != 0) {
		mutex_unlock(&jpeg_dev->video_lock);
		dev_err(jpeg_dev->dev, "Failed to open the phytium jpeg device.\n");
		return ret;
	}

	if (v4l2_fh_is_singular_file(file))
		phytium_jpeg_start(jpeg_dev);

	mutex_unlock(&jpeg_dev->video_lock);

	return 0;
}

static int phytium_jpeg_release(struct file *file)
{
	int ret;
	struct phytium_jpeg_dev *jpeg_dev = video_drvdata(file);

	mutex_lock(&jpeg_dev->video_lock);

	if (v4l2_fh_is_singular_file(file))
		phytium_jpeg_stop(jpeg_dev);

	ret = _vb2_fop_release(file, NULL);
	mutex_unlock(&jpeg_dev->video_lock);

	return ret;
}


static const struct v4l2_file_operations phytium_jpeg_fops = {
	.owner = THIS_MODULE,
	.read = vb2_fop_read,
	.poll = vb2_fop_poll,
	.unlocked_ioctl = video_ioctl2,
	.mmap = vb2_fop_mmap,
	.open = phytium_jpeg_open,
	.release = phytium_jpeg_release,
};

static void phytium_jpeg_update_jpeg_header(u32 width, u32 height)
{
	const int h_index = PHYTIUM_JPEG_HEADER_H_INDEX;
	const int w_index = PHYTIUM_JPEG_HEADER_W_INDEX;

	/* the high 8 bits of the height locates at bit24~bit31 */
	phytium_jpeg_header[h_index] = phytium_jpeg_header[h_index] & 0x00FFFFFF;
	phytium_jpeg_header[h_index] |= ((height >> 8) & 0xFF) << 24;

	/* the low 8 bits of the height locates at bit0~bit7 */
	phytium_jpeg_header[w_index] = phytium_jpeg_header[w_index] & 0xFF000000;
	phytium_jpeg_header[w_index] |= height & 0xFF;

	/* the high 8 bits of the width locates at bit8~bit15 */
	phytium_jpeg_header[w_index] |= ((width >> 8) & 0xFF) << 8;
	/* the low 8 bits of the width locates at bit16~bit24 */
	phytium_jpeg_header[w_index] |= (width & 0xFF) << 16;
}

static void phytium_jpeg_fill_header(struct phytium_jpeg_dev *jpeg_dev,
				     struct phytium_jpeg_buffer *jpeg_buf)
{
	void *vbuf = vb2_plane_vaddr(&jpeg_buf->vb.vb2_buf, 0);
	u32 width = jpeg_dev->active_timings.width;
	u32 height = jpeg_dev->active_timings.height;

	/* update the contents of the phytium jpeg header according to the resolution */
	phytium_jpeg_update_jpeg_header(width, height);

	/* replenish the contents of the JPEG header */
	memcpy(vbuf, phytium_jpeg_header, PHYTIUM_JPEG_HEADER_LEN);
}

static int phytium_jpeg_start_frame(struct phytium_jpeg_dev *jpeg_dev)
{
	dma_addr_t dst_addr;
	unsigned long status;
	struct phytium_jpeg_buffer *jpeg_buf;

	if (jpeg_dev->v4l2_input_status) {
		dev_err(jpeg_dev->dev, "No signal; needn't start frame\n");
		return 0;
	}

	spin_lock_irqsave(&jpeg_dev->hw_lock, status);
	jpeg_buf = list_first_entry_or_null(&jpeg_dev->buffers,
					    struct phytium_jpeg_buffer, link);
	if (jpeg_buf == NULL) {
		spin_unlock_irqrestore(&jpeg_dev->hw_lock, status);
		dev_err(jpeg_dev->dev, "No buffers; doesn't start frame\n");
		return -EPROTO;
	}

	set_bit(VIDEO_FRAME_INPRG, &jpeg_dev->status);
	dst_addr = vb2_dma_contig_plane_dma_addr(&jpeg_buf->vb.vb2_buf, 0);
	spin_unlock_irqrestore(&jpeg_dev->hw_lock, status);

	/*
	 * Because the JPEG Engine is unable to add a JPEG header, the phytium
	 * jpeg driver is required to fill the contents of a JPEG header before
	 * the jpeg engine write datas to the dma address.
	 */
	phytium_jpeg_fill_header(jpeg_dev, jpeg_buf);
	dst_addr += PHYTIUM_JPEG_HEADER_LEN;
	/*
	 * The ikvm application only using the last frame, so the driver replenish
	 * one output register with a dma address.
	 */
	dst_addr >>= JPEG_DST_ADDR_SHIFT;
	phytium_jpeg_write(jpeg_dev, BUF_LIST_INDEX_ADDR(VB_BUF_NO), dst_addr);
	/* Enable the validilty of the buffer marked with index */
	phytium_jpeg_write(jpeg_dev, BUF_LIST_INDEX_CTRL_STS_ADDR(VB_BUF_NO),
						STS_JPEG_BUF_HIGH_LEVEL_VALID);
	/* Enable the interruption which is used to identify an image was compressed */
	phytium_jpeg_update(jpeg_dev, INT_STATUS_CTRL_REG, 0, STS_VE_JPEG_CODE_COMP_EN);
	/* Enable JPEG, start to capture and compress */
	phytium_jpeg_update(jpeg_dev, TRANSFORM_INFO_REG, TRANSINFO_ENABLE_ENGINE, 1);

	return 0;
}

static void phytium_jpeg_resolution_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct phytium_jpeg_dev *jpeg_dev = container_of(dwork,
						struct phytium_jpeg_dev, res_work);
	u32 input_status = jpeg_dev->v4l2_input_status;

	phytium_jpeg_on(jpeg_dev);

	/* Exit early in the case no clients remain */
	if (test_bit(VIDEO_STOPPED, &jpeg_dev->status))
		goto done;

	phytium_jpeg_init_regs(jpeg_dev);
	phytium_jpeg_get_resolution(jpeg_dev);

	/* if source's resolution is changed, the event should be enqueued */
	if (jpeg_dev->detected_timings.width != jpeg_dev->active_timings.width ||
		jpeg_dev->detected_timings.height != jpeg_dev->active_timings.height ||
		input_status != jpeg_dev->v4l2_input_status) {

		static const struct v4l2_event event = {
			.type = V4L2_EVENT_SOURCE_CHANGE,
			.u.src_change.changes = V4L2_EVENT_SRC_CH_RESOLUTION,
		};
		v4l2_event_queue(&jpeg_dev->vdev, &event);
		clear_bit(VIDEO_FRAME_INPRG, &jpeg_dev->status);
		dev_info(jpeg_dev->dev, "event notifies changing resolution\n");
	} else if (test_bit(VIDEO_STREAMING, &jpeg_dev->status)) {
		/* No resolution change so just restart streaming */
		dev_info(jpeg_dev->dev, "resolution doesn't change\n");
		phytium_jpeg_set_resolution(jpeg_dev);
		phytium_jpeg_start_frame(jpeg_dev);
	}

done:
	clear_bit(VIDEO_RES_CHANGE, &jpeg_dev->status);
	wake_up_interruptible_all(&jpeg_dev->wait);
}

static int phytium_jpeg_queue_setup(struct vb2_queue *q,
				    unsigned int *num_buffers,
				    unsigned int *num_planes,
				    unsigned int sizes[],
				    struct device *alloc_devs[])
{
	struct phytium_jpeg_dev *jpeg_dev = vb2_get_drv_priv(q);

	if (*num_planes) {
		if (sizes[0] < jpeg_dev->max_compressed_size) {
			v4l2_err(&jpeg_dev->v4l2_dev, "queue v4l2_buf's size is invalid\n");
			return -EINVAL;
		}
	}

	*num_planes = 1;
	sizes[0] = jpeg_dev->max_compressed_size;
	return 0;
}

static int phytium_jpeg_buf_prepare(struct vb2_buffer *vb)
{
	struct phytium_jpeg_dev *jpeg_dev = vb2_get_drv_priv(vb->vb2_queue);

	if (vb2_plane_size(vb, 0) < jpeg_dev->max_compressed_size) {
		v4l2_err(&jpeg_dev->v4l2_dev, "failed to prepare buffer\n");
		return -EINVAL;
	}

	return 0;
}

static inline struct phytium_jpeg_buffer *
phytium_vb2buf_to_dstbuf(struct vb2_v4l2_buffer *buf)
{
	return  container_of(buf, struct phytium_jpeg_buffer, vb);
}

static void phytium_jpeg_buf_queue(struct vb2_buffer *vb)
{
	bool empty;
	struct phytium_jpeg_dev *jpeg_dev = vb2_get_drv_priv(vb->vb2_queue);
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct phytium_jpeg_buffer *jpeg_buf = phytium_vb2buf_to_dstbuf(vbuf);
	unsigned long status;

	spin_lock_irqsave(&jpeg_dev->hw_lock, status);
	empty = list_empty(&jpeg_dev->buffers);
	list_add_tail(&jpeg_buf->link, &jpeg_dev->buffers);
	spin_unlock_irqrestore(&jpeg_dev->hw_lock, status);

	/* the empty ensures the address of the first node's vb2_v4l2_buf
	 * in the list is written to output register
	 */
	if (test_bit(VIDEO_STREAMING, &jpeg_dev->status) &&
	    (!test_bit(VIDEO_FRAME_INPRG, &jpeg_dev->status)) &&
	    empty)
		phytium_jpeg_start_frame(jpeg_dev);
}

static void phytium_jpeg_bufs_done(struct phytium_jpeg_dev *jpeg_dev,
				   enum vb2_buffer_state state)
{
	unsigned long flags;
	struct phytium_jpeg_buffer *buf;

	spin_lock_irqsave(&jpeg_dev->hw_lock, flags);

	list_for_each_entry(buf, &jpeg_dev->buffers, link)
		vb2_buffer_done(&buf->vb.vb2_buf, state);

	INIT_LIST_HEAD(&jpeg_dev->buffers);

	spin_unlock_irqrestore(&jpeg_dev->hw_lock, flags);
}

static void phytium_jpeg_irq_res_change(struct phytium_jpeg_dev *jpeg_dev,
					ulong delay)
{
	dev_info(jpeg_dev->dev, "Source resolution is changed, resetting\n");
	set_bit(VIDEO_RES_CHANGE, &jpeg_dev->status);

	phytium_jpeg_off(jpeg_dev);

	schedule_delayed_work(&jpeg_dev->res_work, delay);
}

static irqreturn_t phytium_jpeg_irq(int irq, void *arg)
{
	struct phytium_jpeg_dev *jpeg_dev = arg;
	u32 status;
	struct phytium_jpeg_buffer *buf;
	u32 frame_size;

	if (test_bit(VIDEO_POWEROFF, &jpeg_dev->status)) {
		dev_info(jpeg_dev->dev, "jpeg engine is requested to poweroff\n");
		return IRQ_HANDLED;
	}

	status = phytium_jpeg_read(jpeg_dev, INT_STATUS_CTRL_REG);

	if (status & INT_VIDEO_FORMAT_CHANGE) {
		dev_info(jpeg_dev->dev, "receive resolution changed interrupt\n");
		phytium_jpeg_update(jpeg_dev, INT_STATUS_CTRL_REG,
				DETECT_RESOLUTION_CHANGE_EN, 0);
		phytium_jpeg_write(jpeg_dev, INT_STATUS_CTRL_REG, INT_VIDEO_FORMAT_CHANGE);
		phytium_jpeg_irq_res_change(jpeg_dev, RESOLUTION_CHANGE_DELAY);
		return IRQ_HANDLED;
	}

	/*
	 * JPEG engine finish compressing a image JPEG encoding to trigger
	 * a interruption. the status identifies the buffer number. Currently,
	 * the driver uses one buffer.
	 *
	 * Note: Because the JPEG doesn't support adding a JPEG header, and
	 * driver is also unable to add a JPEG header to vb2_buffers. One
	 * solution is that a JPEG header is added by an application.
	 */
	if (status & INT_JPEG_COMP_BUF_LIST_NO) {
		frame_size = phytium_jpeg_read(jpeg_dev, jpeg_dev->comp_size_read);
		frame_size &= JPEG_BUF_CAPACITY_SIZE;
		frame_size >>= JPEG_BUF_CAPACITY_SIZE_SHIFT;
		spin_lock(&jpeg_dev->hw_lock);
		clear_bit(VIDEO_FRAME_INPRG, &jpeg_dev->status);
		/* Delete first node from the queue */
		buf = list_first_entry_or_null(&jpeg_dev->buffers,
					       struct phytium_jpeg_buffer, link);
		if (buf != NULL) {
			frame_size += PHYTIUM_JPEG_HEADER_LEN;
			vb2_set_plane_payload(&buf->vb.vb2_buf, 0, frame_size);
			if (!list_is_last(&buf->link, &jpeg_dev->buffers)) {
				buf->vb.vb2_buf.timestamp = ktime_get_ns();
				buf->vb.sequence = jpeg_dev->sequence++;
				buf->vb.field = V4L2_FIELD_NONE;
				vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_DONE);
				list_del(&buf->link);
			}
		}

		spin_unlock(&jpeg_dev->hw_lock);
		/* Disable JPEG engine */
		phytium_jpeg_update(jpeg_dev, TRANSFORM_INFO_REG, TRANSINFO_ENABLE_ENGINE, 0);
		/* Disable interruption */
		phytium_jpeg_update(jpeg_dev, INT_STATUS_CTRL_REG, STS_VE_JPEG_CODE_COMP_EN, 0);
		/* clear all interruption of the hardware's buffers */
		phytium_jpeg_update(jpeg_dev, INT_STATUS_CTRL_REG, INT_JPEG_ENCODE_COMPLETE, 1);

		status &= ~INT_JPEG_COMP_BUF_LIST_NO;
		if (test_bit(VIDEO_STREAMING,  &jpeg_dev->status) && buf)
			phytium_jpeg_start_frame(jpeg_dev);
	}

	return IRQ_HANDLED;
}

/* VIDIOC_STREAMON, all vb2_v4l2_buf' states are queue */
static int phytium_jpeg_start_streaming(struct vb2_queue *q, unsigned int count)
{
	int ret;
	struct phytium_jpeg_dev *jpeg_dev = vb2_get_drv_priv(q);

	jpeg_dev->sequence = 0;
	ret = phytium_jpeg_start_frame(jpeg_dev);
	if (ret != 0) {
		phytium_jpeg_bufs_done(jpeg_dev, VB2_BUF_STATE_QUEUED);
		return ret;
	}

	/* set the states of the jpeg engine */
	set_bit(VIDEO_STREAMING, &jpeg_dev->status);
	return ret;
}

static void phytium_jpeg_stop_streaming(struct vb2_queue *q)
{
	int ret;
	struct phytium_jpeg_dev *jpeg_dev = vb2_get_drv_priv(q);

	clear_bit(VIDEO_STREAMING, &jpeg_dev->status);
	ret = wait_event_timeout(jpeg_dev->wait,
				 !test_bit(VIDEO_FRAME_INPRG, &jpeg_dev->status),
				 STOP_TIMEOUT);

	/* time out */
	if (ret == 0) {
		dev_err(jpeg_dev->dev, "Timed out when stopping streaming.\n");
		/*
		 * Need to force stop any DMA and try and get HW into a good states
		 * for future calls to start streaming again.
		 */
		phytium_jpeg_off(jpeg_dev);
		phytium_jpeg_on(jpeg_dev);
		phytium_jpeg_init_regs(jpeg_dev);
		phytium_jpeg_get_resolution(jpeg_dev);
	}
	/* first stop jpeg, wait, the free buffer */
	phytium_jpeg_bufs_done(jpeg_dev, VB2_BUF_STATE_ERROR);
}

static const struct vb2_ops phytium_jpeg_vb2_ops = {
	.queue_setup = phytium_jpeg_queue_setup,
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish = vb2_ops_wait_finish,
	.buf_prepare = phytium_jpeg_buf_prepare,
	.buf_queue = phytium_jpeg_buf_queue,
	.start_streaming = phytium_jpeg_start_streaming,
	.stop_streaming = phytium_jpeg_stop_streaming,
};

static void phytium_jpeg_set_yuv_mode(struct phytium_jpeg_dev *jpeg_dev)
{
	const char *mode = yuv_mode_str;
	enum jpeg_yuv_mode yuv_mode;

	if (strstr(mode, "yuv422") != NULL)
		yuv_mode = YUV422;
	else if (strstr(mode, "yuv420") != NULL)
		yuv_mode = YUV420;
	else
		yuv_mode = YUV444;

	/* set the yuv mode register */
	phytium_jpeg_write(jpeg_dev, SAMPLE_MODE_REG, yuv_mode);

	/* update the field which indicates YUV mode locates in the JPEG header. */
	phytium_jpeg_header[YUVID] &= 0xFFFF00FF;
	if (yuv_mode == YUV422)
		phytium_jpeg_header[YUVID] |= 0x2100;
	else if (yuv_mode == YUV420)
		phytium_jpeg_header[YUVID] |= 0x2200;
	else
		phytium_jpeg_header[YUVID] |= 0x1100;

}

static irqreturn_t phytium_jpeg_timer31_irq(int irq, void *arg)
{
	struct phytium_jpeg_dev *jpeg_dev = arg;

	/* disable timer interrupt */
	writel(0, jpeg_dev->timer31_addr);

	/* clear timer interrupt status */
	writel(0x8, jpeg_dev->timer31_addr + 0x2c);

	/* clear JPEG Engine's  poweroff status */
	clear_bit(VIDEO_POWEROFF, &jpeg_dev->status);
	dev_info(jpeg_dev->dev, "timer31 set jpeg status 0x%lx\n", jpeg_dev->status);

	/* JPEG Engine is poweron, reconfig quntization table and YUV mode */
	phytium_jpeg_init_jpeg_quant(jpeg_dev);
	phytium_jpeg_set_yuv_mode(jpeg_dev);
	phytium_jpeg_update(jpeg_dev, INT_STATUS_CTRL_REG, 0, DETECT_RESOLUTION_CHANGE_EN);
	phytium_jpeg_update(jpeg_dev, TRANSFORM_INFO_REG, 0, TRANSINFO_SRC_SELECT);

	dev_info(jpeg_dev->dev, "reconfigure quant table and yuv mode\n");

	return IRQ_HANDLED;
}

static int phytium_jpeg_parser_timer31_irq(struct phytium_jpeg_dev *jpeg_dev)
{
	int irq;
	int ret;
	struct device *dev = jpeg_dev->dev;

	irq = irq_of_parse_and_map(dev->of_node, 2);
	if (!irq) {
		dev_err(dev, "Failed to get timer31 IRQ\n");
		return -ENODEV;
	}

	ret = devm_request_threaded_irq(dev, irq, NULL, phytium_jpeg_timer31_irq,
			IRQF_ONESHOT, PHYTIUM_JPEG_NAME, jpeg_dev);
	if (ret < 0)
		dev_err(dev, "Failed to request timer31 IRQ %d\n", irq);

	return ret;
}

static irqreturn_t phytium_jpeg_timer30_irq(int irq, void *arg)
{
	struct phytium_jpeg_dev *jpeg_dev = arg;
	struct arm_smccc_res res;

	/* disable timer interrupt */
	writel(0, jpeg_dev->timer30_addr);
	/* clear timer interrupt status */
	writel(0x8, jpeg_dev->timer30_addr + 0x2c);

	/* Disable interruption */
	phytium_jpeg_update(jpeg_dev, INT_STATUS_CTRL_REG, STS_VE_JPEG_CODE_COMP_EN, 0);

	/* call SE to poweroff JPEG Engine */
	arm_smccc_smc(0xc300fff4, 0x9, 0x2, 0x80000020, 0, 0, 0, 0, &res);

	/* set JPEG Engine's status is poweroff */
	set_bit(VIDEO_POWEROFF, &jpeg_dev->status);
	dev_info(jpeg_dev->dev, "timer30 set jpeg status 0x%lx\n", jpeg_dev->status);

	return IRQ_HANDLED;
}

static int phytium_jpeg_parser_timer30_irq(struct phytium_jpeg_dev *jpeg_dev)
{
	int irq;
	int ret;
	struct device *dev = jpeg_dev->dev;

	irq = irq_of_parse_and_map(dev->of_node, 1);
	if (!irq) {
		dev_err(dev, "Failed to get timer30 IRQ\n");
		return -ENODEV;
	}

	ret = devm_request_threaded_irq(dev, irq, NULL, phytium_jpeg_timer30_irq,
			IRQF_ONESHOT, PHYTIUM_JPEG_NAME, jpeg_dev);
	if (ret < 0)
		dev_err(dev, "Failed to request timer30 IRQ %d\n", irq);

	return ret;
}

static int phytium_jpeg_init(struct phytium_jpeg_dev *jpeg_dev)
{
	int irq;
	int ret;
	struct device *dev = jpeg_dev->dev;
	u32 ocm_buf_addr[OCM_BUF_NUM];
	int i;

	irq = irq_of_parse_and_map(dev->of_node, 0);
	if (!irq) {
		dev_err(dev, "Failed to get IRQ\n");
		return -ENODEV;
	}

	ret = devm_request_threaded_irq(dev, irq, NULL, phytium_jpeg_irq,
				IRQF_ONESHOT, PHYTIUM_JPEG_NAME, jpeg_dev);
	if (ret < 0) {
		dev_err(dev, "Failed to request IRQ %d\n", irq);
		return ret;
	}

	ret = phytium_jpeg_parser_timer30_irq(jpeg_dev);
	if (ret < 0)
		return ret;

	ret = phytium_jpeg_parser_timer31_irq(jpeg_dev);
	if (ret < 0)
		return ret;

	ret = of_property_read_u32_array(dev->of_node, "phytium,ocm-buf-addr",
					ocm_buf_addr, OCM_BUF_NUM);
	if (ret != 0) {
		dev_err(dev, "Failed to get the OCM address from device tree node.\n");
		return ret;
	}

	for (i = 0; i < OCM_BUF_NUM; i++)
		jpeg_dev->src_addrs[i].dma_addr = ocm_buf_addr[i];

	/* CMA memory for JPEG device */
	of_reserved_mem_device_init(dev);
	ret = dma_set_mask_and_coherent(dev, DMA_BIT_MASK(32));
	if (ret != 0) {
		dev_err(dev, "Failed to set DMA mask\n");
		return ret;
	}

	/* Initializing JPEG Y and CbCr quantization table */
	phytium_jpeg_init_jpeg_quant(jpeg_dev);

	/* Select YUV mode */
	phytium_jpeg_set_yuv_mode(jpeg_dev);
	dev_info(dev, "successfully initialize jpeg engine\n");
	return 0;

}


static int phytium_jpeg_setup_video(struct phytium_jpeg_dev *jpeg_dev)
{
	struct v4l2_device *v4l2_dev = &jpeg_dev->v4l2_dev;
	struct vb2_queue *dst_vq = &jpeg_dev->queue;
	struct video_device *vdev = &jpeg_dev->vdev;
	int ret;

	jpeg_dev->pix_fmt.pixelformat = V4L2_PIX_FMT_JPEG;
	jpeg_dev->pix_fmt.field = V4L2_FIELD_NONE;
	jpeg_dev->pix_fmt.colorspace = V4L2_COLORSPACE_SRGB; /* maybe ARGB */
	jpeg_dev->pix_fmt.quantization =  V4L2_QUANTIZATION_FULL_RANGE;
	jpeg_dev->v4l2_input_status = V4L2_IN_ST_NO_SIGNAL;

	ret = v4l2_device_register(jpeg_dev->dev, v4l2_dev);
	if (ret != 0) {
		dev_err(jpeg_dev->dev, "Failed to register v4l2 device\n");
		return ret;
	}

	/* Register how many v4l2 controls to a handler */
	dst_vq->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dst_vq->io_modes = VB2_MMAP | VB2_READ | VB2_DMABUF;
	dst_vq->dev = v4l2_dev->dev;
	dst_vq->lock = &jpeg_dev->video_lock;
	dst_vq->ops = &phytium_jpeg_vb2_ops;
	dst_vq->mem_ops = &vb2_dma_contig_memops;
	dst_vq->drv_priv = jpeg_dev;
	dst_vq->buf_struct_size = sizeof(struct phytium_jpeg_buffer);
	dst_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	dst_vq->min_buffers_needed = CAPTURE_BUF_NUMBER;
	ret = vb2_queue_init(dst_vq);
	if (ret) {
		dev_err(jpeg_dev->dev, "Failed to init vb2 queue\n");
		goto err_v4l2_register;
	}

	vdev->queue = dst_vq;
	vdev->fops = &phytium_jpeg_fops;
	vdev->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_READWRITE |
			    V4L2_CAP_STREAMING;
	vdev->v4l2_dev = v4l2_dev;
	strscpy(vdev->name, PHYTIUM_JPEG_NAME, sizeof(vdev->name));
	vdev->vfl_type = VFL_TYPE_VIDEO;
	vdev->vfl_dir = VFL_DIR_RX;
	vdev->release = video_device_release_empty;
	vdev->ioctl_ops = &phytium_jpeg_ioctl_ops;
	vdev->lock = &jpeg_dev->video_lock;

	video_set_drvdata(vdev, jpeg_dev);
	ret = video_register_device(vdev, VFL_TYPE_VIDEO, 0);
	if (ret != 0) {
		dev_err(jpeg_dev->dev, "Failed to register video device\n");
		goto err_video_register;
	}

	v4l2_info(v4l2_dev, "phytium JPEG registered as /dev/video%d (%d, %d)\n",
			jpeg_dev->vdev.num, VIDEO_MAJOR, jpeg_dev->vdev.minor);
	return ret;

err_video_register:
	vb2_queue_release(dst_vq);

err_v4l2_register:
	v4l2_device_unregister(v4l2_dev);
	return ret;
}

static const struct phytium_jpeg_config phytium_jpeg_config = {
	.comp_size_read = BUF_LIST_INDEX_CTRL_STS_ADDR(VB_BUF_NO),
};

static const struct of_device_id phytium_jpeg_match[] = {
	{
	    .compatible = "phytium,jpeg",
	    .data = &phytium_jpeg_config,
	},
	{},
};

MODULE_DEVICE_TABLE(of, phytium_jpeg_match);

static int phytium_jpeg_probe(struct platform_device *pdev)
{
	struct phytium_jpeg_dev *jpeg_dev;
	const struct of_device_id *match;
	const struct phytium_jpeg_config *config;
	struct resource *res;
	int ret;

	jpeg_dev = devm_kzalloc(&pdev->dev, sizeof(*jpeg_dev), GFP_KERNEL);
	if (jpeg_dev == NULL)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	jpeg_dev->base_addr = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(jpeg_dev->base_addr)) {
		dev_err(jpeg_dev->dev, "Failed to ioremap.\n");
		return PTR_ERR(jpeg_dev->base_addr);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	jpeg_dev->timer30_addr = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(jpeg_dev->base_addr)) {
		dev_err(jpeg_dev->dev, "Failed to parse timer30.\n");
		return PTR_ERR(jpeg_dev->timer30_addr);
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	jpeg_dev->timer31_addr = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(jpeg_dev->base_addr)) {
		dev_err(jpeg_dev->dev, "Failed to parse timer30.\n");
		return PTR_ERR(jpeg_dev->timer31_addr);
	}
	match = of_match_node(phytium_jpeg_match, pdev->dev.of_node);
	if (match == NULL) {
		dev_err(jpeg_dev->dev, "Failed to match.\n");
		return -EINVAL;
	}

	config = match->data;
	jpeg_dev->comp_size_read = config->comp_size_read;

	jpeg_dev->frame_rate = 30;
	jpeg_dev->dev = &pdev->dev;
	spin_lock_init(&jpeg_dev->hw_lock);
	mutex_init(&jpeg_dev->video_lock);
	init_waitqueue_head(&jpeg_dev->wait);
	INIT_DELAYED_WORK(&jpeg_dev->res_work, phytium_jpeg_resolution_work);
	INIT_LIST_HEAD(&jpeg_dev->buffers);

	ret = phytium_jpeg_init(jpeg_dev);
	if (ret != 0) {
		dev_err(jpeg_dev->dev, "Failed to initialize the JPEG engine.\n");
		return ret;
	}

	ret = phytium_jpeg_setup_video(jpeg_dev);

	return ret;
}

#define to_phytium_jpeg(x) container_of((x), struct phytium_jpeg_dev, v4l2_dev)
static int phytium_jpeg_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct v4l2_device *v4l2_dev = dev_get_drvdata(dev);
	struct phytium_jpeg_dev *jpeg_dev = to_phytium_jpeg(v4l2_dev);

	phytium_jpeg_off(jpeg_dev);

	video_unregister_device(&jpeg_dev->vdev);

	vb2_queue_release(&jpeg_dev->queue);

	v4l2_device_unregister(v4l2_dev);

	of_reserved_mem_device_release(dev);

	return 0;
}

static struct platform_driver phytium_jpeg_driver = {
	.probe = phytium_jpeg_probe,
	.remove =  phytium_jpeg_remove,
	.driver =  {
	    .name = PHYTIUM_JPEG_NAME,
	    .of_match_table = phytium_jpeg_match,
	},
};

module_platform_driver(phytium_jpeg_driver);

MODULE_DESCRIPTION("Phytium JPEG Encoder driver");
MODULE_AUTHOR("Wang Min <wangmin@phytium.com.cn>");
MODULE_LICENSE("GPL v2");
