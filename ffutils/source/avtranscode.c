#include "avtranscode.h"
#include "fftranscode.h"
#include "ffhelper.h"
#include "avpbs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

struct avtranscode_t
{
	union
	{
		struct 
		{
			char preset[64];
			char profile[16];
			char tune[16];
			int gop;
			int width;
			int height;
			int bitrate;
		} v;
	} u;

	AVPACKET_CODEC_ID codecid;
	struct avpbs_t* pbs;
	void* h264;
	void* transcode;
	int (*create_transcoder)(struct avtranscode_t* avt, const struct avpacket_t* pkt);

	struct avpacket_t* pkt;
};

static int avtranscode_create_h264_encoder(struct avtranscode_t* avt, const struct avpacket_t* pkt);

struct avtranscode_t* avtranscode_create_h264(const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate)
{
	struct avtranscode_t* avt;
	avt = (struct avtranscode_t*)calloc(1, sizeof(*avt));
	if (avt)
	{
		snprintf(avt->u.v.preset, sizeof(avt->u.v.preset), "%s", preset);
		snprintf(avt->u.v.profile, sizeof(avt->u.v.profile), "%s", profile);
		snprintf(avt->u.v.tune, sizeof(avt->u.v.tune), "%s", tune);
		avt->u.v.gop = gop;
		avt->u.v.width = width;
		avt->u.v.height = height;
		avt->u.v.bitrate = bitrate;
		avt->create_transcoder = avtranscode_create_h264_encoder;
	}

	return avt;
}

//void* avtranscode_create_h265(const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate);

struct avtranscode_t* avtranscode_create_aac(int sample_rate, int channel, int bitrate)
{
	return NULL;
}

struct avtranscode_t* avtranscode_create_opus(int sample_rate, int channel, int bitrate)
{
	return NULL;
}

void avtranscode_destroy(struct avtranscode_t* avt)
{
	if (avt->transcode)
	{
		fftranscode_destroy(avt->transcode);
		avt->transcode = NULL;
	}
	
	if (avt->pbs && avt->h264)
		avt->pbs->destroy(&avt->h264);
}

static int avtranscode_onpacket(void* param, struct avpacket_t* pkt)
{
	struct avtranscode_t* avt;
	avt = (struct avtranscode_t*)param;
	avpacket_addref(pkt);
	avt->pkt = pkt;
	return 0;
}

int avtranscode_input(struct avtranscode_t* avt, const struct avpacket_t* pkt)
{
	int r;
	AVPacket ff;
	
	if (!avt->transcode || avt->codecid != pkt->stream->codecid)
	{
		if (avt->transcode)
		{
			fftranscode_destroy(avt->transcode);
			avt->transcode = NULL;
		}

		r = avt->create_transcoder(avt, pkt);
		if (0 != r)
			return r;
	}

	memset(&ff, 0, sizeof(ff));
	avpacket_to_ffmpeg(pkt, pkt->stream->stream, &ff);
	return fftranscode_input(avt->transcode, &ff);
}

int avtranscode_getpacket(struct avtranscode_t* avt, struct avpacket_t** pkt)
{
	int r;
	AVPacket out;
	memset(&out, 0, sizeof(out));
	r = fftranscode_getpacket(avt->transcode, &out);
	if (r < 0)
		return r;

	if (avt->pbs && avt->h264)
	{
		r = avt->pbs->input(avt->h264, out.pts, out.dts, out.data, out.size, (out.flags & AV_PKT_FLAG_KEY) ? AVPACKET_FLAG_KEY : 0);
		if (0 == r)
			*pkt = avt->pkt; // temp pkt
	}

	av_packet_unref(&out);
	return r;
}

static int avtranscode_create_h264_encoder(struct avtranscode_t* avt, const struct avpacket_t* pkt)
{
	switch (pkt->stream->codecid)
	{
	case AVCODEC_VIDEO_H265:
	{
		AVCodecParameters h265;
		AVCodecParameters* h264;
		memset(&h265, 0, sizeof(AVCodecParameters));
		h265.codec_type = AVMEDIA_TYPE_VIDEO;
		h265.codec_id = AV_CODEC_ID_H265;
		h265.extradata = (uint8_t*)pkt->stream->extra;
		h265.extradata_size = pkt->stream->bytes;
		avt->transcode = fftranscode_create_h264(&h265, avt->u.v.preset, avt->u.v.profile, avt->u.v.tune, avt->u.v.gop, avt->u.v.width, avt->u.v.height, avt->u.v.bitrate);
		if (avt->transcode)
		{
			avt->pbs = avpbs_find(pkt->stream->codecid);
			h264 = fftranscode_getcodecpar(avt->transcode);
			avt->h264 = avt->pbs->create(pkt->stream->stream, AVCODEC_VIDEO_H264, h264->extradata, h264->extradata_size, avtranscode_onpacket, avt);
			avcodec_parameters_free(&h264);
		}
		return 0;
	}

	default:
		return -1;
	}
}
