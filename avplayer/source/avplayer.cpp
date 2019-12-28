#include "avplayer.h"
#include "AVPlayerCore.h"

void* avplayer_create(avplayer_onrender avrender, void* param)
{
	AVPlayerCore *player = new AVPlayerCore(avrender, param);
	return player;
}

void avplayer_destroy(void* p)
{
	AVPlayerCore *player = (AVPlayerCore *)p;
	delete player;
}

int avplayer_process(void* p, uint64_t clock)
{
	AVPlayerCore *player = (AVPlayerCore *)p;
	return player->Process(clock);
}

void avplayer_play(void* p)
{
	AVPlayerCore *player = (AVPlayerCore *)p;
	player->Play();
}

void avplayer_stop(void* p)
{
	AVPlayerCore *player = (AVPlayerCore *)p;
	player->Stop();
}

void avplayer_pause(void* p)
{
	AVPlayerCore *player = (AVPlayerCore *)p;
	player->Pause();
}

int avplayer_input_audio(void* p, const void* pcm, int64_t pts, uint64_t durationMS, int serial)
{
	AVPlayerCore *player = (AVPlayerCore *)p;
	player->Input(pcm, pts, durationMS, serial);
	return 0;
}

int avplayer_input_video(void* p, const void* frame, int64_t pts, int serial)
{
	AVPlayerCore *player = (AVPlayerCore *)p;
	player->Input(frame, pts, serial);
	return 0;
}

int64_t avplayer_get_audio_duration(void* p)
{
	AVPlayerCore *player = (AVPlayerCore *)p;
	return player->GetAudioDuration(0);
}
