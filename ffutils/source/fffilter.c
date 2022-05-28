#include "fffilter.h"
#include <stdio.h>
#include <stdlib.h>

int fffilter_create_source(AVFilterContext** filter, AVFilterGraph* graph, const AVBufferSrcParameters* param, const char* name)
{
	char args[128] = { 0 };
	snprintf(args, sizeof(args)-1,
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d:frame_rate=%d/%d",
		param->width, param->height, param->format, //frame->width, frame->height, frame->format,
		param->time_base.num, param->time_base.den,
		param->sample_aspect_ratio.num, FFMAX(param->sample_aspect_ratio.den, 1),
		param->frame_rate.num, param->frame_rate.den);

	return avfilter_graph_create_filter(filter, avfilter_get_by_name("buffer"), name, args, NULL, graph);
}
