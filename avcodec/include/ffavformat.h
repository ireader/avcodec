#ifndef _ffavformat_h_
#define _ffavformat_h_

//#include "picture.h"
//#include "libavutil/pixfmt.h"

// PICTURE_YUV420 => AV_PIX_FMT_YUV420P
enum AVPixelFormat ffavforamt_from(int format);

// AV_PIX_FMT_YUV420P => PICTURE_YUV420
int ffavforamt_to(enum AVPixelFormat format);

#endif /*! _ffavformat_h_ */
