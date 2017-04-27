#ifndef _direct_sound_8_h_
#define _direct_sound_8_h_

#include <dsound.h>

class DxSound8Out
{
public:
	DxSound8Out();
	~DxSound8Out();

public:
	int Open(int channels, int bitsPerSamples, int samplesPerSec, int samples);
	int Close();
	int Reset();
	int Pause();
	int Play();

	bool IsOpened() const;
	int GetSamples() const;
	
	int Write(const void* samples, int count);	

	int SetVolume(int volume);
	int GetVolume() const;

private:
	BOOL GetSamplesInfo(DWORD& dwWrite, DWORD& samples) const;
	DWORD WriteSamples(DWORD offset, const void* samples, int bytes);	

private:
	int m_channels;
	int m_bitsPerSample;
	int m_samplesPerSec;
	int m_samples; // buffer sample count

	struct Segment
	{
		DWORD pos;
		DWORD len;
		DWORD tick;
	};
	mutable Segment m_segment;

	LPDIRECTSOUND8 m_sound;
	LPDIRECTSOUNDBUFFER8 m_dsb;
};

typedef HRESULT (WINAPI* pDirectSoundCreate8)(_In_opt_ LPCGUID pcGuidDevice, _Outptr_ LPDIRECTSOUND8 *ppDS8, _Pre_null_ LPUNKNOWN pUnkOuter);

#endif /* !_direct_sound_8_h_ */
