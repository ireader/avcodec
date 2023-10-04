#include "ffhelper.h"

static void avbuffer_free_AVPacket(void* opaque, void* data)
{
	(void)data;
	av_packet_unref((AVPacket*)opaque);
}

static void avbuffer_free_AVFrame(void* opaque, void* data)
{
	(void)data;
	av_frame_unref((AVFrame*)opaque);
}

struct avpacket_t* ffmpeg_to_avpacket(AVPacket* ff)
{
	AVPacket* ref;
	struct avbuffer_t* buf;
	struct avpacket_t* pkt = avpacket_alloc(sizeof(AVPacket));
	if (pkt)
	{
		ref = (AVPacket*)pkt->data;
		buf = (struct avbuffer_t*)pkt->opaque;
		buf->free = avbuffer_free_AVPacket;
		buf->opaque = ref;

		pkt->data = ff->data;
		pkt->size = ff->size;
		pkt->pts = ff->pts;
		pkt->dts = ff->dts;
		//pkt->codecid = ffmpeg_to_avpacket_codecid(codecid);
		pkt->flags = (ff->flags & AV_PKT_FLAG_KEY) ? AVPACKET_FLAG_KEY : 0;

		av_packet_move_ref(ref, ff);
	}

	return pkt;
}

int avpacket_to_ffmpeg(const struct avpacket_t* pkt, int stream_index, AVPacket* ff)
{
	//av_init_packet(ff);
	memset(ff, 0, sizeof(*ff));
	ff->data = pkt->data;
	ff->size = pkt->size;
	ff->pts = pkt->pts;
	ff->dts = pkt->dts;
	ff->flags = (ff->flags & AVPACKET_FLAG_KEY) ? AV_PKT_FLAG_KEY : 0;
	ff->stream_index = stream_index;
	return 0;
}

struct avframe_t* ffmpeg_to_avframe(AVFrame* ff)
{
	size_t i;
	AVFrame* ref;
	struct avbuffer_t* buf;
	struct avframe_t* frame = avframe_alloc(sizeof(AVFrame));
	if (frame)
	{
		ref = (AVFrame*)frame->data[0];
		buf = (struct avbuffer_t*)frame->opaque;
		buf->free = avbuffer_free_AVFrame;
		buf->opaque = ref;

		av_frame_move_ref(ref, ff);

		if (AV_PIX_FMT_YUVJ420P == ref->format)
			ref->format = AV_PIX_FMT_YUV420P;
		frame->format = ref->nb_samples > 0 ? ffmpeg_to_avpacket_audio_format(ref->format) : ref->format; //AV_PIX_FMT_YUV420P;
		frame->pts = AV_NOPTS_VALUE == ref->pts ? ref->pkt_dts : ref->pts;
		frame->dts = ref->pkt_dts;
		frame->flags = ref->flags;
		frame->width = ref->width;
		frame->height = ref->height;
#if LIBAVCODEC_VERSION_MAJOR < 59
		frame->channels = ref->channels;
#else
		frame->channels = ref->ch_layout.nb_channels;
#endif	
		frame->samples = ref->nb_samples;
		frame->sample_bits = 8 * av_get_bytes_per_sample((enum AVSampleFormat)ref->format);
		frame->sample_rate = ref->sample_rate;
		for (i = 0; i < sizeof(frame->data) / sizeof(frame->data[0]); i++)
		{
			frame->data[i] = ref->data[i];
			frame->linesize[i] = ref->linesize[i];
		}
	}

	return frame;
}

void avframe_to_ffmpeg(struct avframe_t* frame, AVFrame* ff)
{
	size_t i;
	memset(ff, 0, sizeof(AVFrame));
	ff->format = frame->samples > 0 ? avpacket_to_ffmpeg_audio_format(frame->format) : frame->format;
	ff->pts = frame->pts;
	//ff->pkt_pts = frame->pts;
	ff->pkt_dts = frame->dts;
	ff->flags = frame->flags;
	ff->width = frame->width;
	ff->height = frame->height;
#if LIBAVCODEC_VERSION_MAJOR < 59
	ff->channels = pkt->stream->channels;
	ff->channel_layout = av_get_default_channel_layout(codecpar.channels);
#else
	av_channel_layout_default(&ff->ch_layout, frame->channels);
#endif
	ff->nb_samples = frame->samples;
	ff->sample_rate = frame->sample_rate;
	for (i = 0; i < sizeof(frame->data) / sizeof(frame->data[0]); i++)
	{
		ff->data[i] = frame->data[i];
		ff->linesize[i] = frame->linesize[i];
	}
}

struct avstream_t* ffmpeg_to_avstream(const AVCodecParameters* codecpar)
{
	struct avstream_t* stream;
	stream = avstream_alloc(codecpar->extradata_size);
	if (!stream)
		return stream;

	if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		stream->width = codecpar->width;
		stream->height = codecpar->height;
	}
	else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
	{
#if LIBAVCODEC_VERSION_MAJOR < 59
		stream->channels = codecpar->channels;
#else
		stream->channels = codecpar->ch_layout.nb_channels;
#endif
		stream->sample_bits = codecpar->bits_per_raw_sample;
		stream->sample_rate = codecpar->sample_rate;
	}
	else
	{
		avstream_release(stream);
		return NULL;
	}

	stream->codecid = ffmpeg_to_avpacket_codecid(codecpar->codec_id);
	memcpy(stream->extra, codecpar->extradata, codecpar->extradata_size);
	stream->bytes = codecpar->extradata_size;
	return stream;
}
