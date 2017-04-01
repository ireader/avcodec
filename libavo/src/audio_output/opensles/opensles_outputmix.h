#ifndef _opensles_outputmix_h_
#define _opensles_outputmix_h_

#include "opensles_output.h"

static int opensles_outputmix_create(struct opensles_player_t* player)
{
	int ret;
	const SLInterfaceID ids1[] = { SL_IID_VOLUME };
	const SLboolean req1[] = { SL_BOOLEAN_FALSE };

	ret = (*player->engine)->CreateOutputMix(player->engine, &player->outputObject, 1, ids1, req1);
	CHECK_OPENSL_ERROR(ret, "%s: slEngine->CreateOutputMix() failed", __FUNCTION__);

	ret = (*player->outputObject)->Realize(player->outputObject, SL_BOOLEAN_FALSE);
	CHECK_OPENSL_ERROR(ret, "%s: slOutputMixObject->Realize() failed", __FUNCTION__);

	return 0;
}

static int opensles_outputmix_destroy(struct opensles_player_t* player)
{
	if (player->outputObject) {
		(*player->outputObject)->Destroy(player->outputObject);
		player->outputObject = NULL;
	}
	return 0;
}

#endif /* !_opensles_outputmix_h_ */
