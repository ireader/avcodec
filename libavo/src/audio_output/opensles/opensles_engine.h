#ifndef _opensles_engine_h_
#define _opensles_engine_h_

#include "opensles_output.h"

static int opensles_engine_create(struct opensles_player_t* player)
{
	int ret;

	ret = slCreateEngine(&player->engineObject, 0, NULL, 0, NULL, NULL);
	CHECK_OPENSL_ERROR(ret, "%s: slCreateEngine() failed", __FUNCTION__);

	ret = (*player->engineObject)->Realize(player->engineObject, SL_BOOLEAN_FALSE);
	CHECK_OPENSL_ERROR(ret, "%s: slObject->Realize() failed", __FUNCTION__);

	ret = (*player->engineObject)->GetInterface(player->engineObject, SL_IID_ENGINE, &player->engine);
	CHECK_OPENSL_ERROR(ret, "%s: slObject->GetInterface() failed", __FUNCTION__);

	return 0;
}

static int opensles_engine_destroy(struct opensles_player_t* player)
{
	player->engine = NULL;
	if (player->engineObject) {
		(*player->engineObject)->Destroy(player->engineObject);
		player->engineObject = NULL;
	}
	return 0;
}

#endif /* !_opensles_engine_h_ */
