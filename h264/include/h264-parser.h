#ifndef _h264_parser_h_
#define _h264_parser_h_

#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
struct h264_parser_t;
struct h264_parser_t* h264_parser_create(void);
void h264_parser_destroy(struct h264_parser_t* parser);

int h264_parser_input(struct h264_parser_t* parser, const void* annexb, size_t bytes);

int h264_parser_getflags(struct h264_parser_t* parser);

int h264_parser_iskeyframe(struct h264_parser_t* parser);
#ifdef __cplusplus
}
#endif
#endif /* !_h264_parser_h_ */

