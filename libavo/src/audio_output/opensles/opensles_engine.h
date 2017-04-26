#ifndef _opensles_engine_h_
#define _opensles_engine_h_

static int opensles_engine_create(SLObjectItf* slEngineObject, SLEngineItf* slEnginItf)
{
	int ret;

	ret = slCreateEngine(slEngineObject, 0, NULL, 0, NULL, NULL);
	CHECK_OPENSL_ERROR(ret, "%s: slCreateEngine() failed", __FUNCTION__);

	ret = (**slEngineObject)->Realize(*slEngineObject, SL_BOOLEAN_FALSE);
	CHECK_OPENSL_ERROR(ret, "%s: SLObjectItf->Realize() failed", __FUNCTION__);

	ret = (**slEngineObject)->GetInterface(*slEngineObject, SL_IID_ENGINE, slEnginItf);
	CHECK_OPENSL_ERROR(ret, "%s: slObject->GetInterface(SL_IID_ENGINE) failed", __FUNCTION__);

	return 0;
}

static int opensles_engine_destroy(SLObjectItf* slEngineObject, SLEngineItf* slEnginItf)
{
	*slEnginItf = NULL;
	if (*slEngineObject) {
		(**slEngineObject)->Destroy(*slEngineObject);
		*slEngineObject = NULL;
	}
	return 0;
}

#endif /* !_opensles_engine_h_ */
