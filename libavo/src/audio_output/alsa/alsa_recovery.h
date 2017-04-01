#ifndef _alsa_recovery_h_
#define _alsa_recovery_h_

static int alsa_recovery(snd_pcm_t* handle, int err)
{
	if (err == -EPIPE)
	{
		err = snd_pcm_prepare(handle);
		if (err < 0)
			printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
		return 0;
	}
	else if (err == -ESTRPIPE)
	{
		while ((err == snd_pcm_resume(handle)) == -EAGAIN)
			sleep(1);
		if (err < 0)
		{
			err = snd_pcm_prepare(handle);
			if (err < 0)
				printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
		}
		return 0;
	}
	return err;
}

#endif /* !_alsa_recovery_h_ */
