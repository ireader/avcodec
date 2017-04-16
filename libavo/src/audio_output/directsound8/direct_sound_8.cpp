#include "direct_sound_8.h"
#include "audio_output.h"
#include "av_register.h"
#include <math.h>
#include <mmreg.h>
//#include <audiodefs.h>
//#include <Ks.h>
//#include <KsProxy.h>
#include <assert.h>

#define MIN(a, b) ((a)<(b) ? (a) : (b))
#define BUFFER_TIME	(1000)

static GUID s_IID_IDirectSoundBuffer8 = { 0x6825a449, 0x7524, 0x4d82, { 0x92, 0x0f, 0x50, 0xe3, 0x6a, 0xb3, 0xab, 0x1e } };
static pDirectSoundCreate8 fpDirectSoundCreate8;

DxSound8Out::DxSound8Out()
{
	m_channels = 0;
	m_bitsPerSample = 0;
	m_samplesPerSec = 0;

	m_segment.pos = 0;
	m_segment.len = 0;
	m_segment.tick = 0;

	m_dsb = NULL;
	m_sound = NULL;
}

DxSound8Out::~DxSound8Out()
{
	Close();
}

int DxSound8Out::Open(int channels, int bitsPerSamples, int samplesPerSec)
{
	assert(1 == channels || 2 == channels);
	assert(8 == bitsPerSamples || 16 == bitsPerSamples || 32 == bitsPerSamples);

	m_channels = channels;
	m_bitsPerSample = bitsPerSamples;
	m_samplesPerSec = samplesPerSec;

	assert(NULL==m_sound && NULL==m_dsb);
	HRESULT hr = fpDirectSoundCreate8(NULL, &m_sound, NULL);
	if(FAILED(hr))
		return hr;

	if(FAILED(hr = m_sound->SetCooperativeLevel(::GetTopWindow(NULL), DSSCL_PRIORITY)))
	{
		Close();
		return hr;
	}

	WAVEFORMATEX format;
	memset(&format, 0, sizeof(format));
	format.cbSize = 0;
	format.wFormatTag = 32==bitsPerSamples ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
	format.nChannels = (WORD)channels;
	format.nSamplesPerSec = samplesPerSec;
	format.wBitsPerSample = (WORD)bitsPerSamples;
	format.nBlockAlign = (WORD)(channels * bitsPerSamples / 8);
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	//WAVEFORMATEXTENSIBLE format;
	//memset(&format, 0, sizeof(format));
	//format.Format.cbSize = sizeof(format);
	//format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	//format.Format.nChannels = (WORD)channels;
	//format.Format.nSamplesPerSec = samplesPerSec;
	//format.Format.wBitsPerSample = (WORD)bitsPerSamples;
	//format.Format.nBlockAlign = (WORD)(channels*bitsPerSamples/8);
	//format.Format.nAvgBytesPerSec = samplesPerSec*channels*bitsPerSamples/8;
	//format.Samples.wValidBitsPerSample = (WORD)bitsPerSamples;
	//format.dwChannelMask = SPEAKER_FRONT_LEFT;
	//format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	DSBUFFERDESC desc;
	ZeroMemory(&desc, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_GLOBALFOCUS|DSBCAPS_CTRLFX|DSBCAPS_CTRLVOLUME|DSBCAPS_GETCURRENTPOSITION2/*|DSBCAPS_CTRLPOSITIONNOTIFY*/;
	desc.dwBufferBytes = GetBufferSize() * channels * bitsPerSamples / 8;
	desc.lpwfxFormat = (LPWAVEFORMATEX)&format;

	LPDIRECTSOUNDBUFFER lpBuffer = NULL;
	if(FAILED(hr = m_sound->CreateSoundBuffer(&desc, &lpBuffer, NULL)))
	{
		Close();
		return hr;
	}
	//lpBuffer->SetFormat(&format);

	if(FAILED(hr = lpBuffer->QueryInterface(s_IID_IDirectSoundBuffer8, (LPVOID*)&m_dsb)))
	{
		lpBuffer->Release();
		Close();
		return hr;
	}
	lpBuffer->Release();

	//m_dsb->SetCurrentPosition(0);
	//m_dsb->Play(0, 0, DSBPLAY_LOOPING);

	assert(0 == m_segment.pos);
	assert(0 == m_segment.len);
	return 0;
}

int DxSound8Out::Close()
{
	m_segment.pos = 0;
	m_segment.len = 0;

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

bool DxSound8Out::IsOpened() const
{
	return NULL!=m_dsb;
}

BOOL DxSound8Out::GetSamplesInfo(DWORD& dwWrite, DWORD& dwLen) const
{
	DWORD dwPlay = 0;
	DWORD dwTick = GetTickCount();
	DWORD bufferSize = GetBufferSize()*m_channels*m_bitsPerSample/8;
	assert(m_segment.len <= bufferSize);

	if(FAILED(m_dsb->GetCurrentPosition(&dwPlay, &dwWrite)))
		return FALSE;
	assert(dwWrite % (m_channels*m_bitsPerSample/8) == 0);

	bool empty = false;
	if(dwWrite < m_segment.pos)
		empty = ((m_segment.pos + m_segment.len) % bufferSize) < dwWrite ;
	else
		empty = (m_segment.pos + m_segment.len) < dwWrite ;

	assert(0==m_segment.len || m_segment.tick > 0);
	if(m_segment.len > 0 && dwTick - m_segment.tick > BUFFER_TIME)
		empty = true;

	dwLen = empty ? 0 : (m_segment.pos + m_segment.len + bufferSize - (dwWrite+bufferSize)) % bufferSize;
	return TRUE;
}

int DxSound8Out::GetAvailSample() const
{
	DWORD dwLen = 0;
	DWORD dwWrite = 0;
	if(!GetSamplesInfo(dwWrite, dwLen))
		return 0;

	return (dwLen * 8) / (m_channels * m_bitsPerSample);
}

int DxSound8Out::GetBufferSize() const
{
	return m_samplesPerSec * BUFFER_TIME / 1000;
}

int DxSound8Out::Write(const void* samples, int count)
{
	assert(samples && count > 0);
	assert(count <= m_samplesPerSec);
	DWORD dwLen = 0;
	DWORD dwWrite = 0;
	if(!GetSamplesInfo(dwWrite, dwLen))
		return 0;

	DWORD bufferSize = GetBufferSize()*m_channels*m_bitsPerSample/8;
	DWORD offset = (dwWrite + dwLen) % bufferSize;
	DWORD bytes = WriteSamples(offset, samples, count*m_channels*m_bitsPerSample/8);

	assert(dwWrite < bufferSize);
	m_segment.len = MIN(dwLen + bytes, bufferSize);
	m_segment.tick = GetTickCount();
	m_segment.pos = dwWrite;
	//TRACE("pos: %d, len: %d / offset: %d, Write %d, len: %d(%d)\n", pos, len, offset, dwWrite, dwLen, bytes);
	return count;
}

DWORD DxSound8Out::WriteSamples(DWORD offset, const void* samples, int bytes)
{
	void* pLockedBuffer1 = NULL;
	void* pLockedBuffer2 = NULL;
	DWORD dwLockedBuffer1Len = 0;
	DWORD dwLockedBuffer2Len = 0;

	HRESULT hr = m_dsb->Lock(offset, bytes, &pLockedBuffer1, &dwLockedBuffer1Len, &pLockedBuffer2, &dwLockedBuffer2Len, 0);
	if(hr == DSERR_BUFFERLOST)
	{
		m_dsb->Restore();
		hr = m_dsb->Lock(offset, bytes, &pLockedBuffer1, &dwLockedBuffer1Len, &pLockedBuffer2, &dwLockedBuffer2Len, DSBLOCK_FROMWRITECURSOR);
	}
	if(FAILED(hr))
		return 0;

	assert((dwLockedBuffer1Len+dwLockedBuffer2Len) <= (DWORD)bytes);
	assert((dwLockedBuffer1Len+dwLockedBuffer2Len) % (m_channels*m_bitsPerSample/8) == 0);
	if(dwLockedBuffer1Len > 0) memcpy(pLockedBuffer1, samples, dwLockedBuffer1Len);
	if(dwLockedBuffer2Len > 0) memcpy(pLockedBuffer2, (BYTE*)samples+dwLockedBuffer1Len, dwLockedBuffer2Len);

	hr = m_dsb->Unlock(pLockedBuffer1, dwLockedBuffer1Len, pLockedBuffer2, dwLockedBuffer2Len);
	return dwLockedBuffer1Len+dwLockedBuffer2Len;
}

int DxSound8Out::Pause()
{
	if(!IsOpened())
		return -1;

	// update position
	DWORD dwLen = 0;
	DWORD dwWrite = 0;
	if(!GetSamplesInfo(dwWrite, dwLen))
		return 0;

	m_segment.len = dwLen;
	m_segment.pos = dwWrite;
	m_segment.tick = GetTickCount();
	m_dsb->Stop();
	return 0;
}

int DxSound8Out::Play()
{
	if(!IsOpened())
		return -1;

	//DWORD dwPlayCursor = 0;
	//DWORD dwWriteCursor = 0;
	//m_dsb->GetCurrentPosition(&dwPlayCursor, &dwWriteCursor);
	//m_dsb->SetCurrentPosition(dwPlayCursor);
	if(DSERR_BUFFERLOST == m_dsb->Play(0, 0, DSBPLAY_LOOPING))
	{
		//load data
		m_dsb->Play(0, 0, DSBPLAY_LOOPING);
	}

	// don't update segment position and length
	m_segment.tick = GetTickCount();
	return 0;
}

int DxSound8Out::Reset()
{
	if(!IsOpened())
		return -1;

	void* pLockedBuffer1 = NULL;
	void* pLockedBuffer2 = NULL;
	DWORD dwLockedBuffer1Len = 0;
	DWORD dwLockedBuffer2Len = 0;
	HRESULT hr = m_dsb->Lock(0, 0, &pLockedBuffer1, &dwLockedBuffer1Len, &pLockedBuffer2, &dwLockedBuffer2Len, DSBLOCK_ENTIREBUFFER);
	if(SUCCEEDED(hr))
	{
		m_segment.pos = 0;
		m_segment.len = 0;
		m_segment.tick = 0;

		if(dwLockedBuffer1Len > 0) memset(pLockedBuffer1, 0, dwLockedBuffer1Len);
		if(dwLockedBuffer2Len > 0) memset(pLockedBuffer2, 0, dwLockedBuffer2Len);
		hr = m_dsb->Unlock(pLockedBuffer1, dwLockedBuffer1Len, pLockedBuffer2, dwLockedBuffer2Len);
	}
	return hr;
}

int DxSound8Out::SetVolume(int volume)
{
	if(!IsOpened())
		return -1;

	//int v = int(1000*log10((volume&0xFFFF)*1.0/0xFFFF));
	//int v = (volume&0xFFFF)*10000/0xFFFF-10000;
	int v = int(1000.0/log10(2.0)*log10((volume&0xFFFF)*1.0/0xFFFF));
	//TRACE(_T("SetVolume: %f%% -> %ddb\n"), (volume&0xFFFF)*1.0/0xFFFF, v);
	HRESULT hr = m_dsb->SetVolume(v);
	return hr;
}

int DxSound8Out::GetVolume() const
{
	if(!IsOpened())
		return -1;

	LONG v;
	int volume = 0;
	HRESULT hr = m_dsb->GetVolume(&v);
	if(SUCCEEDED(hr))
	{
		volume = int(pow(10, v*log10(2.0)/1000.0)*0xFFFF);
		//TRACE(_T("GetVolume: %ddb -> %f%%\n"), v, (volume&0xFFFF)*1.0/0xFFFF);
		volume |= (volume&0xFFFF)<<16;
	}
	return volume;
}

void DxSound8Out::GetInfo(int &channels, int &bits_per_sample, int &samples_per_second) const
{
	channels = m_channels;
	bits_per_sample = m_bitsPerSample;
	samples_per_second = m_samplesPerSec;
}

//////////////////////////////////////////////////////////////////////////
///
//////////////////////////////////////////////////////////////////////////
static void* Open(int channels, int bits_per_samples, int samples_per_seconds)
{
	DxSound8Out* obj = new DxSound8Out();
	int r = obj->Open(channels, bits_per_samples, samples_per_seconds);
	if(r < 0)
	{
		delete obj;
		return NULL;
	}
	return obj;
}
static int Close(void* ao)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	obj->Close();
	delete obj;
	return 0;
}
static int IsOpened(void* ao)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	return obj->IsOpened()?1:0;
}
static int Write(void* ao, const void* samples, int count)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	return obj->Write(samples, count);
}
static int Play(void* ao)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	return obj->Play();
}
static int Pause(void* ao)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	return obj->Pause();
}
static int Reset(void* ao)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	return obj->Reset();
}
static int GetBufferSize(void* ao)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	return obj->GetBufferSize();
}
static int GetAvailSample(void* ao)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	return obj->GetAvailSample();
}
static int SetVolume(void* ao, int v)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	return obj->SetVolume(v);
}
static int GetVolume(void* ao, int *v)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	*v = obj->GetVolume();
	return 0;
}
static int AudioGetInfo(void *ao, int *channels, int *bits_per_sample, int *samples_per_second)
{
	DxSound8Out* obj = (DxSound8Out*)ao;
	obj->GetInfo(*channels, *bits_per_sample, *samples_per_second);
	return 0;
}

extern "C" int directsound8_player_register()
{
	HMODULE hDSound = LoadLibraryEx("dsound.dll", NULL, 0);
	fpDirectSoundCreate8 = (pDirectSoundCreate8)GetProcAddress(hDSound, "DirectSoundCreate8");
	if (NULL == fpDirectSoundCreate8)
		return -1;

	static audio_output_t ao;
	memset(&ao, 0, sizeof(ao));
	ao.open = Open;
	ao.close = Close;
	ao.write = Write;
	ao.play = Play;
	ao.pause = Pause;
	ao.reset = Reset;
	ao.get_info = AudioGetInfo;
	ao.get_buffer_size = GetBufferSize;
	ao.get_available_sample = GetAvailSample;
	ao.get_volume = GetVolume;
	ao.set_volume = SetVolume;
	return av_set_class(AV_AUDIO_OUTPUT, "directsound8", &ao);
}
