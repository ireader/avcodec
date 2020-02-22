#include "avplayer-file.h"
#include "AVFilePlayer.h"

void* avplayer_file_create(void* window, avplayer_file_read reader, void* param)
{
	return new AVFilePlayer(window, reader, param);
}

void avplayer_file_destroy(void* player)
{
	delete (AVFilePlayer*)player;
}

int avplayer_file_process(void* player, uint64_t clock)
{
	return ((AVFilePlayer*)player)->Process(clock);
}

void avplayer_file_play(void* player)
{
	((AVFilePlayer*)player)->Play();
}

void avplayer_file_pause(void* player)
{
	((AVFilePlayer*)player)->Pause();
}

void avplayer_file_reset(void* player)
{
	((AVFilePlayer*)player)->Reset();
}

uint64_t avplayer_file_getpos(void* player)
{
	return ((AVFilePlayer*)player)->GetPosition();
}

int avplayer_file_setspeed(void* player, int speed)
{
	return ((AVFilePlayer*)player)->SetSpeed(speed);
}

int avplayer_file_getspeed(void* player)
{
	return ((AVFilePlayer*)player)->GetSpeed();
}
