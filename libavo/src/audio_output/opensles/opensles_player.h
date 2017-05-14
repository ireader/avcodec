#ifndef _opensles_player_h_
#define _opensles_player_h_

#include "opensles_output.h"

static int opensles_player_create(struct opensles_player_t* player, SLAndroidDataFormat_PCM_EX* format)
{
	int ret;
	const SLboolean req2[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
	const SLInterfaceID ids2[] = { SL_IID_PLAY, SL_IID_VOLUME, SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, player->buffer_count };

	SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, player->outputObject };
	SLDataSink audio_sink = { &loc_outmix, NULL };
	SLDataSource audio_source = { &loc_bufq, format };

	ret = (*player->engine)->CreateAudioPlayer(player->engine, &player->playerObject, &audio_source, &audio_sink, sizeof(ids2) / sizeof(*ids2), ids2, req2);
	CHECK_OPENSL_ERROR(ret, "%s: SLEngine->CreateAudioPlayer() failed", __FUNCTION__);

	ret = (*player->playerObject)->Realize(player->playerObject, SL_BOOLEAN_FALSE);
	CHECK_OPENSL_ERROR(ret, "%s: SLPlayerObject->Realize() failed", __FUNCTION__);

	ret = (*player->playerObject)->GetInterface(player->playerObject, SL_IID_PLAY, &player->player);
	CHECK_OPENSL_ERROR(ret, "%s: SLPlayerObject->GetInterface(SL_IID_PLAY) failed", __FUNCTION__);

	ret = (*player->playerObject)->GetInterface(player->playerObject, SL_IID_VOLUME, &player->volume);
	CHECK_OPENSL_ERROR(ret, "%s: SLPlayerObject->GetInterface(SL_IID_VOLUME) failed", __FUNCTION__);

	ret = (*player->playerObject)->GetInterface(player->playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &player->bufferQ);
	CHECK_OPENSL_ERROR(ret, "%s: SLPlayerObject->GetInterface(SL_IID_ANDROIDSIMPLEBUFFERQUEUE) failed", __FUNCTION__);

	return 0;
}

static int opensles_player_destroy(struct opensles_player_t* player)
{
	if (player->player)
	{
		(*player->player)->SetPlayState(player->player, SL_PLAYSTATE_STOPPED);
		player->player = NULL;
	}

	if (player->bufferQ)
	{
		(*player->bufferQ)->Clear(player->bufferQ);
		player->bufferQ = NULL;
	}

	player->volume = NULL;

	if (player->playerObject) {
		(*player->playerObject)->Destroy(player->playerObject);
		player->playerObject = NULL;
	}

	return 0;
}

#endif /* !_opensles_player_h_ */
