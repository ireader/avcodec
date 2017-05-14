#include "direct_capture_8.h"
#include "av_register.h"
#include "avframe.h"
#include <stdio.h>
#include <assert.h>
#include <mmreg.h>

#define MIN(a, b) ((a)<(b) ? (a) : (b))
#define MAX(a, b) ((a)>(b) ? (a) : (b))

//#pragma comment(lib, "dsound.lib")
typedef HRESULT (WINAPI *fpDirectSoundCaptureCreate8)(_In_opt_ LPCGUID pcGuidDevice, _Outptr_ LPDIRECTSOUNDCAPTURE8 *ppDSC8, _Pre_null_ LPUNKNOWN pUnkOuter);
static fpDirectSoundCaptureCreate8 pDirectSoundCaptureCreate8;
static GUID s_IID_IDirectSoundCaptureBuffer8 = { 0x990df4, 0xdbb, 0x4872,{ 0x83, 0x3e, 0x6d, 0x30, 0x3e, 0x80, 0xae, 0xb6 } };
static GUID s_IID_IDirectSoundNotify8 = { 0xb0210783, 0x89cd, 0x11d0, { 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16 } };
static GUID s_DSDEVID_DefaultCapture = { 0xdef00001, 0x9c6d, 0x47ed, { 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03 } };

static DWORD WINAPI direct_capture_worker(LPVOID param)
{
	struct direct_capture_t* ds;
	ds = (struct direct_capture_t*)param;

	while(1)
	{
		DWORD r = WaitForMultipleObjects(MAX_EVENTS+1, ds->events, FALSE, INFINITE);
		if(WAIT_OBJECT_0 + MAX_EVENTS == r)
			break;

		if(WAIT_OBJECT_0 <= r && r < WAIT_OBJECT_0 + MAX_EVENTS)
		{
			int step = ds->samples * ds->bytes_per_sample;

			DWORD len1, len2;
			LPVOID ptr1, ptr2;
			HRESULT hr = ds->dsb->Lock(step * (r - WAIT_OBJECT_0), step, &ptr1, &len1, &ptr2, &len2, 0);
			if(SUCCEEDED(hr))
			{
				assert(0 == len2);
				assert(0 == (len1 + len2) % ds->bytes_per_sample);
				ds->cb(ds->param, ptr1, len1 / ds->bytes_per_sample);
				ds->dsb->Unlock(ptr1, len1, ptr2, len2);
			}
		}
	}

	return 0;
}

static int direct_capture_start(void* ai)
{
	struct direct_capture_t* ds;
	ds = (struct direct_capture_t*)ai;

	LPDIRECTSOUNDNOTIFY8 notify = NULL;
	DSBPOSITIONNOTIFY pos[MAX_EVENTS];
	if (FAILED(ds->dsb->QueryInterface(s_IID_IDirectSoundNotify8, (LPVOID*)&notify)))
		return -1;

	DWORD step = ds->samples * ds->bytes_per_sample;
	for (int i = 0; i < MAX_EVENTS; i++)
	{
		assert(NULL == ds->events[i]);
		ds->events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		pos[i].dwOffset = step * (i + 1) - 1;
		pos[i].hEventNotify = ds->events[i];
	}
	notify->SetNotificationPositions(MAX_EVENTS, pos);
	notify->Release();

	ds->thread = CreateThread(NULL, 0, direct_capture_worker, ds, 0, NULL);
	return SUCCEEDED(ds->dsb->Start(DSCBSTART_LOOPING)) ? 0 : -1;
}

static int direct_capture_stop(void* ai)
{
	struct direct_capture_t* ds;
	ds = (struct direct_capture_t*)ai;
	if (ds->thread)
	{
		SetEvent(ds->events[MAX_EVENTS]);
		WaitForSingleObject(ds->thread, INFINITE);
		CloseHandle(ds->thread);
		ds->thread = NULL;
	}

	for (int i = 0; i < MAX_EVENTS; i++)
	{
		if (ds->events[i])
		{
			CloseHandle(ds->events[i]);
			ds->events[i] = 0;
		}
	}

	return SUCCEEDED(ds->dsb->Stop()) ? 0 : -1;
}

static int direct_capture_close(void* ai)
{
	struct direct_capture_t* ds;
	ds = (struct direct_capture_t*)ai;

	direct_capture_stop(ds);

	CloseHandle(ds->events[MAX_EVENTS]);

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

static void* direct_capture_open(int channels, int samples_per_second, int fmt, int samples, audio_input_callback cb, void* param)
{
	struct direct_capture_t* ds;
	ds = (struct direct_capture_t*)malloc(sizeof(*ds));
	if (NULL == ds)
		return NULL;

	memset(ds, 0, sizeof(*ds));
	ds->cb = cb;
	ds->param = param;
	ds->samples = MAX(samples, samples_per_second / 100); // at least 10ms;
	ds->bytes_per_sample = channels * PCM_SAMPLE_BITS(fmt) / 8;

	assert(NULL == ds->sound && NULL == ds->dsb);
	if (FAILED(pDirectSoundCaptureCreate8(&s_DSDEVID_DefaultCapture, &ds->sound, NULL)))
	{
		direct_capture_close(ds);
		return NULL;
	}

	WAVEFORMATEX wave;
	memset(&wave, 0, sizeof(wave));
	wave.cbSize = 0;
	wave.wFormatTag = PCM_SAMPLE_FLOAT(fmt) ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
	wave.nChannels = (WORD)channels;
	wave.nSamplesPerSec = samples_per_second;
	wave.wBitsPerSample = (WORD)(ds->bytes_per_sample / channels * 8);
	wave.nBlockAlign = (WORD)ds->bytes_per_sample;
	wave.nAvgBytesPerSec = wave.nSamplesPerSec * wave.nBlockAlign;

	DSCBUFFERDESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = 0;
	desc.dwBufferBytes = ds->samples * ds->bytes_per_sample * MAX_EVENTS;
	desc.lpwfxFormat = &wave;

	LPDIRECTSOUNDCAPTUREBUFFER lpBuffer = NULL;
	if (FAILED(ds->sound->CreateCaptureBuffer(&desc, &lpBuffer, NULL)))
	{
		direct_capture_close(ds);
		return NULL;
	}

	if (FAILED(lpBuffer->QueryInterface(s_IID_IDirectSoundCaptureBuffer8, (LPVOID*)&ds->dsb)))
	{
		lpBuffer->Release();
		direct_capture_close(ds);
		return NULL;
	}
	lpBuffer->Release();

	ds->events[MAX_EVENTS] = CreateEvent(NULL, FALSE, FALSE, NULL);
	return ds;
}

extern "C" int directsound8_recorder_register()
{
	HMODULE hDSound = LoadLibraryEx("dsound.dll", NULL, 0);
	pDirectSoundCaptureCreate8 = (fpDirectSoundCaptureCreate8)GetProcAddress(hDSound, "DirectSoundCaptureCreate8");
	if (NULL == pDirectSoundCaptureCreate8)
		return -1;

	static audio_input_t ai;
	memset(&ai, 0, sizeof(ai));
	ai.open = direct_capture_open;
	ai.close = direct_capture_close;
	ai.start = direct_capture_start;
	ai.stop = direct_capture_stop;
	return av_set_class(AV_AUDIO_RECORDER, "directsound8", &ai);
}
