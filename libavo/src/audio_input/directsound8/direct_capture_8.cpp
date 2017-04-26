#include "direct_capture_8.h"
#include "av_register.h"
#include <stdio.h>
#include <assert.h>

//#pragma comment(lib, "dsound.lib")
typedef HRESULT (WINAPI *fpDirectSoundCaptureCreate8)(_In_opt_ LPCGUID pcGuidDevice, _Outptr_ LPDIRECTSOUNDCAPTURE8 *ppDSC8, _Pre_null_ LPUNKNOWN pUnkOuter);
static fpDirectSoundCaptureCreate8 pDirectSoundCaptureCreate8;
static GUID s_IID_IDirectSoundCaptureBuffer8 = { 0x990df4, 0xdbb, 0x4872,{ 0x83, 0x3e, 0x6d, 0x30, 0x3e, 0x80, 0xae, 0xb6 } };
static GUID s_IID_IDirectSoundNotify8 = { 0xb0210783, 0x89cd, 0x11d0, { 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16 } };
static GUID s_DSDEVID_DefaultCapture = { 0xdef00001, 0x9c6d, 0x47ed, { 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03 } };

inline void DxLog(const char* format, ...)
{
	static TCHAR msg[2*1024] = {0};

	va_list vl;
	va_start(vl, format);
	vsnprintf(msg, sizeof(msg)-1, format, vl);
	va_end(vl);

	OutputDebugString(msg);
}

DxSound8In::DxSound8In(audio_input_callback cb, void* param)
{
	m_cb = cb;
	m_param = param;
	m_thread = NULL;
	m_sound = NULL;
	m_dsb = NULL;
	memset(m_events, 0, sizeof(m_events));
	m_events[MAX_EVENTS] = CreateEvent(NULL, FALSE, FALSE, NULL);
}

DxSound8In::~DxSound8In()
{
	Close();
	CloseHandle(m_events[MAX_EVENTS]);
}

int DxSound8In::Open(int channels, int bitsPerSamples, int samplesPerSec)
{
	m_channels = channels;
	m_bitsPerSample = bitsPerSamples;
	m_samplesPerSec = samplesPerSec;

	assert(NULL==m_sound && NULL==m_dsb);
	HRESULT hr = pDirectSoundCaptureCreate8(&s_DSDEVID_DefaultCapture, &m_sound, NULL);
	if(FAILED(hr))
		return hr;

	WAVEFORMATEX format;
	memset(&format, 0, sizeof(format));
	format.cbSize = 0;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = (WORD)channels;
	format.nSamplesPerSec = samplesPerSec;
	format.wBitsPerSample = (WORD)bitsPerSamples;
	format.nBlockAlign = (WORD)(channels*bitsPerSamples/8);
	format.nAvgBytesPerSec = (samplesPerSec/MAX_EVENTS)*MAX_EVENTS*channels*bitsPerSamples/8;

	DSCBUFFERDESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = 0;
	desc.dwBufferBytes = format.nAvgBytesPerSec;
	desc.lpwfxFormat = &format;

	LPDIRECTSOUNDCAPTUREBUFFER lpBuffer = NULL;
	if(FAILED(hr = m_sound->CreateCaptureBuffer(&desc, &lpBuffer, NULL)))
	{
		Close();
		return hr;
	}

	if(FAILED(hr = lpBuffer->QueryInterface(s_IID_IDirectSoundCaptureBuffer8, (LPVOID*)&m_dsb)))
	{
		lpBuffer->Release();
		Close();
		return hr;
	}
	lpBuffer->Release();

	LPDIRECTSOUNDNOTIFY8 notify = NULL;
	DSBPOSITIONNOTIFY pos[MAX_EVENTS];
	if(FAILED(hr = m_dsb->QueryInterface(s_IID_IDirectSoundNotify8, (LPVOID*)&notify)))
	{
		Close();
		return hr;
	}

	DWORD step = format.nAvgBytesPerSec*8/(MAX_EVENTS*channels*bitsPerSamples);
	for(int i=0; i<MAX_EVENTS; i++)
	{
		m_events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		pos[i].dwOffset = step * (i+1);
		pos[i].hEventNotify = m_events[i];
	}
	notify->SetNotificationPositions(MAX_EVENTS, pos);
	notify->Release();

	m_thread = CreateThread(NULL, 0, OnWorker, this, 0, NULL);
	return 0;
}

int DxSound8In::Close()
{
	if(m_thread)
	{
		SetEvent(m_events[MAX_EVENTS]);
		WaitForSingleObject(m_thread, INFINITE);
		CloseHandle(m_thread);
		m_thread = NULL;
	}

	for(int i=0; i<MAX_EVENTS; i++)
	{
		if(m_events[i])
		{
			CloseHandle(m_events[i]);
			m_events[i] = 0;
		}
	}

	if(m_dsb)
	{
		m_dsb->Stop();
		m_dsb->Release();
		m_dsb = NULL;
	}

	if(m_sound)
	{
		m_sound->Release();
		m_sound = NULL;
	}
	return 0;
}

bool DxSound8In::IsOpened() const
{
	return NULL!=m_dsb;
}

int DxSound8In::Start()
{
	HRESULT hr = m_dsb->Start(DSCBSTART_LOOPING);
	return hr;
}

int DxSound8In::Stop()
{
	HRESULT hr = m_dsb->Stop();
	return hr;
}

DWORD DxSound8In::OnWorker()
{
	DWORD r;

	while(1)
	{
		r = WaitForMultipleObjects(MAX_EVENTS+1, m_events, FALSE, INFINITE);
		DxLog("WaitForMultipleObjects: %d\n", r);
		if(WAIT_OBJECT_0 + MAX_EVENTS == r)
			break;

		if(r < WAIT_OBJECT_0 + MAX_EVENTS)
		{
			int step = (m_samplesPerSec/MAX_EVENTS)*m_channels*m_bitsPerSample / 8;

			DWORD len1, len2;
			LPVOID ptr1, ptr2;
			HRESULT hr = m_dsb->Lock(step * (r - WAIT_OBJECT_0), step, &ptr1, &len1, &ptr2, &len2, 0);
			if(SUCCEEDED(hr))
			{
				assert(0 == len2);
				m_cb(m_param, ptr1, len1);
				m_dsb->Unlock(ptr1, len1, ptr2, len2);
			}
		}
		else
		{
		}
	}

	return 0;
}

DWORD WINAPI DxSound8In::OnWorker(LPVOID lpParameter)
{
	DxSound8In* self = (DxSound8In*)lpParameter;
	return self->OnWorker();
}

//////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////
static void* Open(int channels, int bits_per_samples, int samples_per_seconds, audio_input_callback cb, void* param)
{
	assert(cb && channels && bits_per_samples && samples_per_seconds);
	DxSound8In* obj = new DxSound8In(cb, param);
	int r = obj->Open(channels, bits_per_samples, samples_per_seconds);
	if(r < 0)
	{
		delete obj;
		return NULL;
	}
	obj->Start();
	return obj;
}

static int Close(void* ai)
{
	DxSound8In* obj = (DxSound8In*)ai;
	obj->Close();
	delete obj;
	return 0;
}

static int IsOpened(void* ai)
{
	return ((DxSound8In*)ai)->IsOpened()?1:0;
}

static int Pause(void* ai)
{
	return ((DxSound8In*)ai)->Stop();
}

static int Reset(void* ai)
{
	return ((DxSound8In*)ai)->Start();
}

static int SetVolume(void* ai, int v)
{
	return -1;
}

static int GetVolume(void* ai)
{
	return -1;
}

extern "C" int directsound8_recorder_register()
{
	HMODULE hDSound = LoadLibraryEx("dsound.dll", NULL, 0);
	pDirectSoundCaptureCreate8 = (fpDirectSoundCaptureCreate8)GetProcAddress(hDSound, "DirectSoundCaptureCreate8");
	if (NULL == pDirectSoundCaptureCreate8)
		return -1;

	static audio_input_t ai;
	memset(&ai, 0, sizeof(ai));
	ai.open = Open;
	ai.close = Close;
	ai.isopened = IsOpened;
	ai.getvolume = GetVolume;
	ai.setvolume = SetVolume;
	ai.pause = Pause;
	ai.reset = Reset;
	return av_set_class(AV_AUDIO_RECORDER, "directsound8", &ai);
}
