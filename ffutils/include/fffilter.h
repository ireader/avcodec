#ifndef _fffilter_h_
#define _fffilter_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersrc.h"

/// Create Buffer Source Filter(0-input, 1-output)
/// @param[out] filter
/// @param[in] graph from avfilter_graph_alloc()
/// @param[in] stream source parameter
/// @param[in] name source buffer filter name
/// @return >=0-ok, <0-error
int fffilter_create_source(AVFilterContext** filter, AVFilterGraph* graph, const AVBufferSrcParameters* param, const char* name);

#ifdef __cplusplus
}
#endif
#endif /* !_fffilter_h_ */
