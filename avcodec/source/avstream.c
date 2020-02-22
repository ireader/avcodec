#include "avstream.h"
#include "avbuffer.h"
#include <string.h>
#include <assert.h>

struct avstream_t* avstream_alloc(int size)
{
	struct avbuffer_t* buf;
	struct avstream_t* stream;
	buf = avbuffer_alloc(size + sizeof(struct avstream_t));
	if (buf)
	{
		stream = (struct avstream_t*)buf->data;
		memset(stream, 0, sizeof(struct avstream_t));
		stream->extra = (uint8_t*)(stream + 1);
		stream->bytes = size;
		stream->opaque = buf;
		return stream;
	}
	return NULL;
}

int32_t avstream_addref(struct avstream_t* stream)
{
	struct avbuffer_t* buf;
	if (NULL == stream || NULL == stream->opaque)
		return -1;

	buf = (struct avbuffer_t*)stream->opaque;
	return avbuffer_addref(buf);
}

int32_t avstream_release(struct avstream_t* stream)
{
	struct avbuffer_t* buf;
	if (NULL == stream || NULL == stream->opaque)
		return -1;

	buf = (struct avbuffer_t*)stream->opaque;
	return avbuffer_release(buf);
}
