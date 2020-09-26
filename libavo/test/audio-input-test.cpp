// gcc audio-input-test.cpp -I../include -I../../avcodec/include -L../debug.linux -lavo -lstdc++ -lasound

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "audio_input.h"
#include "avframe.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

static int bytes_per_sample = 1;
static void AudioInCallback(void* param, const void* samples, int frames)
{
	FILE* fp = (FILE*)param;
	fwrite(samples, bytes_per_sample, frames, fp);
}

extern "C" int TestAudioIn(const char* filename, int channel, int frequency, int format)
{
	FILE* fp = fopen(filename, "wb");
	if(NULL == fp)
	{
		printf("Open File failed.\n");
		return -1;
	}

	audio_input ai;
	bytes_per_sample = channel * PCM_SAMPLE_BITS(format) / 8;
	if (!ai.open(channel, frequency, format, frequency / 10, AudioInCallback, fp))
	{
		printf("audio open(%d, %d, %d) failed: %d\n", channel, frequency, format, errno);
		return 0;
	}
	ai.start();

#ifdef WIN32
	Sleep(1000*10);
#else
	usleep(10*1000*1000);
#endif

	ai.close();
	fclose(fp);
	return 0;
}

#if 0
int main(int argc, char* argv[])
{
	int channels = 1;
	int frequency = 8000;
	int format = PCM_SAMPLE_FMT_S16;
	const char* filename = "audio.pcm";
	if (argc != 5)
	{
		printf("args: filename channels frquency format\n");
		printf("\tchannels: 1, 2\n");
		printf("\tfrequency: 8000, 16000, 32000, 44100, 48000\n");
		printf("\tformat: 8-U8, 16-S16, 2080-float\n");
		printf("e.g: ./a.out audio.pcm 1 8000 8\n");
		return 0;
	}

	filename = argv[1];
	channels = atoi(argv[2]);
	frequency = atoi(argv[3]);
	format = atoi(argv[4]);
	return TestAudioIn(filename, channels, frequency, format);
}
#endif
