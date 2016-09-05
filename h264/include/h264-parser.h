#ifndef _h264_parser_h_
#define _h264_parser_h_

#include <stddef.h>

void* h264_parser_create();
void h264_parser_destroy(void* parser);

int h264_parser_input(void* parser, const void* nalu, size_t bytes);

int h264_parser_getflags(void* parser);

#endif /* !_h264_parser_h_ */
