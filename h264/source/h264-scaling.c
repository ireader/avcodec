// Rec. ITU-T H.264 (02/2016)
// 7.3.2.1.1.1 Scaling list syntax (p67)

#include "h264-scaling.h"

void h264_scaling_list_4x4(bitstream_t* stream, int32_t ScalingList4x4[16], uint8_t *useDefaultScalingMatrixFlag)
{
	int i;
	int lastScale = 8;
	int nextScale = 8;
	for (i = 0; i < 16; i++)
	{
		if (nextScale != 0)
		{
			int delta_scale = bitstream_read_se(stream);
			nextScale = (lastScale + delta_scale + 256) % 256;
			*useDefaultScalingMatrixFlag = (i == 0 && nextScale == 0) ? 1 : 0;
		}
		ScalingList4x4[i] = 0 == nextScale ? lastScale : nextScale;
		lastScale = ScalingList4x4[i];
	}
}

void h264_scaling_list_8x8(bitstream_t* stream, int32_t ScalingList8x8[64], uint8_t *useDefaultScalingMatrixFlag)
{
	int i;
	int lastScale = 8;
	int nextScale = 8;
	for (i = 0; i < 64; i++)
	{
		if (nextScale != 0)
		{
			int delta_scale = bitstream_read_se(stream);
			nextScale = (lastScale + delta_scale + 256) % 256;
			*useDefaultScalingMatrixFlag = (i == 0 && nextScale == 0) ? 1 : 0;
		}
		ScalingList8x8[i] = 0 == nextScale ? lastScale : nextScale;
		lastScale = ScalingList8x8[i];
	}
}
