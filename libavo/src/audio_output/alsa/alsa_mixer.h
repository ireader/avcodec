#ifndef _alsa_mixer_h_
#define _alsa_mixer_h_

static int alsa_mixer_load(const char* device, snd_mixer_t** p)
{
	snd_mixer_t* mixer = NULL;
	int r = snd_mixer_open(&mixer, 0);
	if (0 == r)
	{
		r = snd_mixer_attach(mixer, device);
		if (0 == r)
		{
			r = snd_mixer_selem_register(mixer, NULL, NULL);
			if (0 == r)
			{
				r = snd_mixer_load(mixer);
				if (0 == r)
				{
					*p = mixer;
					return 0;
				}
			}
		}
	}
	return r;
}

static int alsa_mixer_set_volume(snd_mixer_t* mixer, int volume)
{
	long min = 0;
	long max = 0;
	snd_mixer_elem_t* e;

	for (e = snd_mixer_first_elem(mixer); e; e = snd_mixer_elem_next(e))
	{
		if (SND_MIXER_ELEM_SIMPLE == snd_mixer_elem_get_type(e)
			&& snd_mixer_selem_is_active(e)
			&& 0 != snd_mixer_selem_has_playback_volume(e))
		{
			snd_mixer_selem_get_playback_volume_range(e, &min, &max);

			volume = ((long long)(volume & 0xFFFF) * (max - min) + 0xFFFF / 2) / 0xFFFF; // map (0-0xFFFF) to volume range
			//printf("mixer volume range: %ld-%ld, set: %d\n", min, max, volume);

			snd_mixer_selem_set_playback_volume_all(e, volume);
			return 0;
		}
	}
	return -1;
}

static int alsa_mixer_get_volume(snd_mixer_t* mixer, int* volume)
{
	long v;
	long min = 0;
	long max = 0;
	snd_mixer_elem_t* e;

	for (e = snd_mixer_first_elem(mixer); e; e = snd_mixer_elem_next(e))
	{
		if (SND_MIXER_ELEM_SIMPLE == snd_mixer_elem_get_type(e)
			&& snd_mixer_selem_is_active(e)
			&& 0 != snd_mixer_selem_has_playback_volume(e))
		{
			snd_mixer_selem_get_playback_volume_range(e, &min, &max);

			v = 0;
			snd_mixer_selem_get_playback_volume(e, SND_MIXER_SCHN_FRONT_LEFT, &v);
			//printf("mixer volume range[%ld-%ld], current: %ld", min, max, v);

			*volume = ((long long)v * 0xFFFF + (max - min) / 2) / (max - min); // map volume range to (0-0xFFFF)
			//printf(" map: %ld\n", *volume);
			return 0;
		}
	}
	return -1;
}

#endif /* !_alsa_mixer_h_ */
