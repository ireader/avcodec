#include "avplayer-live.h"
#include "AVLivePlayer.h"

void* avplayer_live_create(void* window)
{
	return new AVLivePlayer(window);
}

void avplayer_live_destroy(void* player)
{
	delete (AVLivePlayer*)player;
}

int avplayer_live_input(void* player, struct avpacket_t* pkt, int video)
{
	return ((AVLivePlayer*)player)->Input(pkt, 0 != video);
}
