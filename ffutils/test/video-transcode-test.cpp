#include <Windows.h>
#include <iostream>
#include <assert.h>
#include "sys/system.h"
#include "mov-buffer.h"
#include "mov-format.h"
#include "mov-reader.h"
#include "mov-writer.h"
#include "../test/mov-file-buffer.h"
#include "ffdecoder.h"
#include "ffencoder.h"
#include "avdecoder.h"
#include "avencoder.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

struct video_transcode_test_t
{
	AVFormatContext* ic;
	int video;
	int codec;
	AVStream* s;

	void* decoder;
	void* encoder;
	mov_writer_t* mov;
	int track;
};

static uint8_t s_buffer[2 * 1024 * 1024];

inline const char* ftimestamp(uint32_t t, char* buf)
{
	sprintf(buf, "%02u:%02u:%02u.%03u", t / 3600000, (t / 60000) % 60, (t / 1000) % 60, t % 1000);
	return buf;
}

static int video_transcode_encode_frame(video_transcode_test_t* ctx, AVFrame* frame)
{
	if (!ctx->encoder)
	{
		ctx->encoder = avencoder_create_h264("fast", "baseline", NULL, 50, frame->width, frame->height, 2000000000);
		if (!ctx->encoder)
		{
			printf("create encoder failed.\n");
			return -1;
		}

		AVCodecParameters* par = ffencoder_getcodecpar(ctx->encoder);
		ctx->track = mov_writer_add_video(ctx->mov, MOV_OBJECT_H264, frame->width, frame->height, par->extradata, par->extradata_size);
	}

	frame->pict_type = AV_PICTURE_TYPE_NONE;
	frame->pts = frame->pkt_dts;
	int r = ffencoder_input(ctx->encoder, frame);
	if (r < 0)
	{
		printf("encode frame error: %d\n", r);
	}

	AVPacket pkt2;
	memset(&pkt2, 0, sizeof(pkt2));
	r = ffencoder_getpacket(ctx->encoder, &pkt2);
	if (r >= 0)
	{
		printf("pts: %lld, dts: %lld, flags: %x\n", pkt2.pts, pkt2.dts, pkt2.flags);
		mov_writer_write(ctx->mov, ctx->track, pkt2.data, pkt2.size, pkt2.pts, pkt2.dts, (pkt2.flags & AV_PKT_FLAG_KEY) ? MOV_AV_FLAG_KEYFREAME : 0);
		av_packet_unref(&pkt2);
	}

	return r;
}

static int video_transcode_decode_packet(video_transcode_test_t* ctx, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags)
{
	AVPacket pkt;
	memset(&pkt, 0, sizeof(pkt));
	av_new_packet(&pkt, bytes);
	memcpy(pkt.data, buffer, bytes);
	pkt.size = bytes;
	pkt.pts = pts;
	pkt.dts = dts;
	pkt.flags = (flags & MOV_AV_FLAG_KEYFREAME) ? AV_PKT_FLAG_KEY : 0;
	pkt.stream_index = 0;

	int r = ffdecoder_input(ctx->decoder, &pkt);
	if (0 != r)
	{
		printf("ffdecoder_input ==> %d\n", r);
	}

	AVFrame frame;
	memset(&frame, 0, sizeof(frame));
	r = ffdecoder_getframe(ctx->decoder, &frame);
	if (r < 0)
	{
		printf("decode packet failed: %d\n", r);
	}
	else if (r >= 0)
	{
		video_transcode_encode_frame(ctx, &frame);
		av_frame_unref(&frame);
	}

	av_packet_unref(&pkt);
	return r;
}

static void mov_onread(void* param, uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags)
{
	static char s_pts[64], s_dts[64];
	static int64_t v_pts, v_dts;
	static int64_t a_pts, a_dts;
	static int64_t x_pts, x_dts;

	struct video_transcode_test_t* ctx = (struct video_transcode_test_t*)param;
	if (ctx->video != track)
		return; // ignore

	if (MOV_OBJECT_H264 == ctx->codec)
	{
		printf("[H264] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u%s\n", ftimestamp(pts, s_pts), ftimestamp(dts, s_dts), (int)(pts - v_pts), (int)(dts - v_dts), (unsigned int)bytes, flags ? " [I]" : "");
		v_pts = pts;
		v_dts = dts;
	}
	else if (MOV_OBJECT_H265 == ctx->codec)
	{
		uint8_t nalu_type = (((const uint8_t*)buffer)[4] >> 1) & 0x3F;
		uint8_t irap = 16 <= nalu_type && nalu_type <= 23;

		printf("[H265] pts: %s, dts: %s, diff: %03d/%03d, bytes: %u%s,%d\n", ftimestamp(pts, s_pts), ftimestamp(dts, s_dts), (int)(pts - v_pts), (int)(dts - v_dts), (unsigned int)bytes, flags ? " [I]" : "", (unsigned int)nalu_type);
		v_pts = pts;
		v_dts = dts;
		video_transcode_decode_packet(ctx, buffer, bytes, pts + 100000, dts + 100000, flags);
	}
	else
	{
	}
}

static void mov_video_info(void* param, uint32_t track, uint8_t object, int /*width*/, int /*height*/, const void* extra, size_t bytes)
{
	struct video_transcode_test_t* ctx = (struct video_transcode_test_t*)param;
	ctx->codec = object;
	if (MOV_OBJECT_H264 == object)
	{
		ctx->video = track;
		ctx->decoder = avdecoder_create_h264(extra, bytes);
	}
	else if (MOV_OBJECT_HEVC == object)
	{
		ctx->video = track;
		ctx->decoder = avdecoder_create_h265(extra, bytes);
	}
	else
	{
		assert(0);
	}
}

static void mov_audio_info(void* /*param*/, uint32_t /*track*/, uint8_t /*object*/, int /*channel_count*/, int /*bit_per_sample*/, int /*sample_rate*/, const void* /*extra*/, size_t /*bytes*/)
{
}

static void mov_subtitle_info(void* /*param*/, uint32_t /*track*/, uint8_t /*object*/, const void* /*extra*/, size_t /*bytes*/)
{
}

void video_transcode_test(const char* mp4)
{
	struct video_transcode_test_t ctx;
	memset(&ctx, 0, sizeof(ctx));

	FILE* fp = fopen(mp4, "rb");
	mov_reader_t* mov = mov_reader_create(mov_file_buffer(), fp);
	uint64_t duration = mov_reader_getduration(mov);

	FILE* wfp = fopen("1.mp4", "wb");
	ctx.mov = mov_writer_create(mov_file_buffer(), wfp, 0);

	struct mov_reader_trackinfo_t info = { mov_video_info, mov_audio_info, mov_subtitle_info };
	mov_reader_getinfo(mov, &info, &ctx);
	for (int i = 0; i < 500 && mov_reader_read(mov, s_buffer, sizeof(s_buffer), mov_onread, &ctx) > 0; i++)
	{
	}

	duration /= 2;
	mov_reader_seek(mov, (int64_t*)&duration);

	mov_reader_destroy(mov);
	mov_writer_destroy(ctx.mov);
	fclose(wfp);
	fclose(fp);
}
