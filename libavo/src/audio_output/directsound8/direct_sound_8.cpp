#include "direct_sound_8.h"
#include "audio_output.h"
#include "av_register.h"
#include "avframe.h"
#include <mmreg.h>
//#include <audiodefs.h>
//#include <Ks.h>
//#include <KsProxy.h>
#include <assert.h>
#include <stdio.h>

#define MIN(a, b) ((a)<(b) ? (a) : (b))
#define MAX(a, b) ((a)>(b) ? (a) : (b))

static GUID s_IID_IDirectSoundBuffer8 = { 0x6825a449, 0x7524, 0x4d82, { 0x92, 0x0f, 0x50, 0xe3, 0x6a, 0xb3, 0xab, 0x1e } };
static pDirectSoundCreate8 fpDirectSoundCreate8;

static int direct_sound_close(void* ao)
{
	struct direct_sound_t* ds;
	ds = (struct direct_sound_t*)ao;

	if (ds->dsb)
	{
		ds->dsb->Stop();
		ds->dsb->Release();
		ds->dsb = NULL;
	}

	if (ds->sound)
	{
		ds->sound->Release();
		ds->sound = NULL;
	}
	
	free(ds);
	return 0;
}

static void* direct_sound_open(int channels, int samples_per_second, int fmt, int samples)
{
	struct direct_sound_t* ds;
	ds = (struct direct_sound_t*)malloc(sizeof(*ds));
	if (NULL == ds)
		return NULL;

	memset(ds, 0, sizeof(*ds));
	ds->samples = MAX(samples, samples_per_second / 2); // at least 500ms;
	ds->bytes_per_sample = channels * PCM_SAMPLE_BITS(fmt) / 8;
	ds->N = ds->samples * ds->bytes_per_sample;

	if (FAILED(fpDirectSoundCreate8(NULL, &ds->sound, NULL)))
	{
		direct_sound_close(ds);
		return NULL;
	}

	if (FAILED(ds->sound->SetCooperativeLevel(::GetTopWindow(NULL), DSSCL_PRIORITY)))
	{
		direct_sound_close(ds);
		return NULL;
	}

	WAVEFORMATEX wave;
	memset(&wave, 0, sizeof(wave));
	wave.cbSize = 0;
	wave.wFormatTag = PCM_SAMPLE_FLOAT(fmt) ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
	wave.nChannels = (WORD)channels;
	wave.nSamplesPerSec = (DWORD)samples_per_second;
	wave.wBitsPerSample = (WORD)(ds->bytes_per_sample / channels * 8);
	wave.nBlockAlign = (WORD)ds->bytes_per_sample;
	wave.nAvgBytesPerSec = wave.nSamplesPerSec * wave.nBlockAlign;
	//WAVEFORMATEXTENSIBLE wave;
	//memset(&wave, 0, sizeof(wave));
	//wave.Format.cbSize = sizeof(wave);
	//wave.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	//wave.Format.nChannels = (WORD)channels;
	//wave.Format.nSamplesPerSec = samplesPerSec;
	//wave.Format.wBitsPerSample = (WORD)bitsPerSamples;
	//wave.Format.nBlockAlign = (WORD)(channels*bitsPerSamples/8);
	//wave.Format.nAvgBytesPerSec = samplesPerSec*channels*bitsPerSamples/8;
	//wave.Samples.wValidBitsPerSample = (WORD)bitsPerSamples;
	//wave.dwChannelMask = SPEAKER_FRONT_LEFT;
	//wave.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	DSBUFFERDESC desc;
	ZeroMemory(&desc, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLFX | DSBCAPS_GETCURRENTPOSITION2/* | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY*/;
	desc.dwBufferBytes = ds->N;
	desc.lpwfxFormat = (LPWAVEFORMATEX)&wave;

	LPDIRECTSOUNDBUFFER lpBuffer = NULL;
	if (FAILED(ds->sound->CreateSoundBuffer(&desc, &lpBuffer, NULL)))
	{
		direct_sound_close(ds);
		return NULL;
	}
	//lpBuffer->SetFormat(&wave);

	if (FAILED(lpBuffer->QueryInterface(s_IID_IDirectSoundBuffer8, (LPVOID*)&ds->dsb)))
	{
		lpBuffer->Release();
		direct_sound_close(ds);
		return NULL;
	}
	lpBuffer->Release();

	return ds;
}

BOOL direct_sound_update(struct direct_sound_t* ds)
{
	DWORD dwPlay = 0;
	DWORD dwWrite = 0;
	assert(ds->rb.size <= ds->N);

	if (FAILED(ds->dsb->GetCurrentPosition(&dwPlay, &dwWrite)))
		return FALSE;

	assert(0 == dwPlay % ds->bytes_per_sample);
	assert(0 == dwWrite % ds->bytes_per_sample);
	assert(0 == ds->rb.read % ds->bytes_per_sample);
	assert(0 == ds->rb.size % ds->bytes_per_sample);

	DWORD size = (dwWrite + ds->N - ds->rb.read) % ds->N;
	if (size >= ds->rb.size)
	{
		ds->rb.size = 0;
		ds->rb.read = dwWrite;
	}
	else
	{
		ds->rb.read = (ds->rb.read + size) % ds->N;
		ds->rb.size -= size;
	}

	return TRUE;
}

DWORD direct_sound_lockwrite(struct direct_sound_t* ds, DWORD offset, const void* samples, DWORD bytes)
{
	void* pLockedBuffer1 = NULL;
	void* pLockedBuffer2 = NULL;
	DWORD dwLockedBuffer1Len = 0;
	DWORD dwLockedBuffer2Len = 0;

	HRESULT hr = ds->dsb->Lock(offset, bytes, &pLockedBuffer1, &dwLockedBuffer1Len, &pLockedBuffer2, &dwLockedBuffer2Len, 0);
	if (hr == DSERR_BUFFERLOST)
	{
		ds->dsb->Restore();
		hr = ds->dsb->Lock(offset, bytes, &pLockedBuffer1, &dwLockedBuffer1Len, &pLockedBuffer2, &dwLockedBuffer2Len, 0);
	}
	if (FAILED(hr))
		return 0;

	assert((dwLockedBuffer1Len + dwLockedBuffer2Len) <= (DWORD)bytes);
	assert(0 == (dwLockedBuffer1Len + dwLockedBuffer2Len) % ds->bytes_per_sample);
	if (dwLockedBuffer1Len > 0) memcpy(pLockedBuffer1, samples, dwLockedBuffer1Len);
	if (dwLockedBuffer2Len > 0) memcpy(pLockedBuffer2, (BYTE*)samples + dwLockedBuffer1Len, dwLockedBuffer2Len);

	hr = ds->dsb->Unlock(pLockedBuffer1, dwLockedBuffer1Len, pLockedBuffer2, dwLockedBuffer2Len);
	return FAILED(hr) ? 0 : (dwLockedBuffer1Len + dwLockedBuffer2Len);
}

static int direct_sound_write(void* ao, const void* pcm, int count)
{
	struct direct_sound_t* ds;
	ds = (struct direct_sound_t*)ao;
	assert(pcm && count > 0);
	assert((DWORD)count <= ds->samples);

	direct_sound_update(ds);
	DWORD offset = (ds->rb.read + ds->rb.size) % ds->N;
	DWORD bytes = direct_sound_lockwrite(ds, offset, pcm, count * ds->bytes_per_sample);
	assert(0 == bytes % ds->bytes_per_sample);
	ds->rb.size += bytes;
	assert(ds->rb.size <= ds->N);

	assert(count * ds->bytes_per_sample == bytes);
	return bytes / ds->bytes_per_sample;
}

static int direct_sound_play(void* ao)
{
	struct direct_sound_t* ds;
	ds = (struct direct_sound_t*)ao;

	HRESULT hr = ds->dsb->Play(0, 0, DSBPLAY_LOOPING);
	if (DSERR_BUFFERLOST == hr)
	{
		//load data
		hr = ds->dsb->Play(0, 0, DSBPLAY_LOOPING);
	}
	return SUCCEEDED(hr) ? 0 : -1;
}

static int direct_sound_pause(void* ao)
{
	struct direct_sound_t* ds;
	ds = (struct direct_sound_t*)ao;

	// save and stop
	direct_sound_update(ds);
	return SUCCEEDED(ds->dsb->Stop()) ? 0 : -1;
}

static int direct_sound_reset(void* ao)
{
	struct direct_sound_t* ds;
	ds = (struct direct_sound_t*)ao;

	// fill silently data
	void* pLockedBuffer1 = NULL;
	void* pLockedBuffer2 = NULL;
	DWORD dwLockedBuffer1Len = 0;
	DWORD dwLockedBuffer2Len = 0;
	HRESULT hr = ds->dsb->Lock(0, 0, &pLockedBuffer1, &dwLockedBuffer1Len, &pLockedBuffer2, &dwLockedBuffer2Len, DSBLOCK_ENTIREBUFFER);
	if (SUCCEEDED(hr))
	{
		if (dwLockedBuffer1Len > 0) memset(pLockedBuffer1, 0, dwLockedBuffer1Len);
		if (dwLockedBuffer2Len > 0) memset(pLockedBuffer2, 0, dwLockedBuffer2Len);
		hr = ds->dsb->Unlock(pLockedBuffer1, dwLockedBuffer1Len, pLockedBuffer2, dwLockedBuffer2Len);
	}

	ds->rb.read = ds->rb.size = 0; // clear buffer
	return SUCCEEDED(hr) ? 0 : -1;
}

static int direct_sound_getsamples(void* ao)
{
	struct direct_sound_t* ds;
	ds = (struct direct_sound_t*)ao;
	direct_sound_update(ds);
	return ds->rb.size / ds->bytes_per_sample;
}

extern "C" int directsound8_player_register()
{
	HMODULE hDSound = LoadLibraryEx("dsound.dll", NULL, 0);
	fpDirectSoundCreate8 = (pDirectSoundCreate8)GetProcAddress(hDSound, "DirectSoundCreate8");
	if (NULL == fpDirectSoundCreate8)
		return -1;

	static audio_output_t ao;
	memset(&ao, 0, sizeof(ao));
	ao.open = direct_sound_open;
	ao.close = direct_sound_close;
	ao.write = direct_sound_write;
	ao.play = direct_sound_play;
	ao.pause = direct_sound_pause;
	ao.reset = direct_sound_reset;
	ao.get_frames = direct_sound_getsamples;
	return av_set_class(AV_AUDIO_PLAYER, "directsound8", &ao);
}
