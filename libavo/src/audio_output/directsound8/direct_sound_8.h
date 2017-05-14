#ifndef _direct_sound_8_h_
#define _direct_sound_8_h_

#include <dsound.h>

struct direct_sound_t
{
	DWORD bytes_per_sample; // all channnel
	DWORD samples; // buffer sample count
	DWORD N;

	struct RingBuffer
	{
		DWORD read;
		DWORD size;
	} rb;

	LPDIRECTSOUND8 sound;
	LPDIRECTSOUNDBUFFER8 dsb;
};

typedef HRESULT (WINAPI* pDirectSoundCreate8)(_In_opt_ LPCGUID pcGuidDevice, _Outptr_ LPDIRECTSOUND8 *ppDS8, _Pre_null_ LPUNKNOWN pUnkOuter);

#endif /* !_direct_sound_8_h_ */
