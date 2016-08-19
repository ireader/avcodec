#include "h264-scaling.h"

void h264_scaling_list(bitstream_t* stream, int *scalingList, int sizeOfScalingList, int *useDefaultScalingMatrixFlag)
{
	int lastScale = 8;
	int nextScale = 8;
	for (int j = 0; j < sizeOfScalingList; j++)
	{
		if (nextScale != 0)
		{
			int delta_scale = bitstream_read_se(stream);
			nextScale = (lastScale + delta_scale + 256) % 256;
			*useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0) ? 1 : 0;
		}
		scalingList[j] = 0 == nextScale ? lastScale : nextScale;
		lastScale = scalingList[j];
	}
}
