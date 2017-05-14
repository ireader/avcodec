// gcc audio-output-test.cpp -I../include -I../../avcodec/include -L../debug.linux -lavo -lstdc++ -lasound

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "audio_output.h"
#include "avframe.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

static int TestAudioOut(const char* filename, int channel, int frequency, int format)
{
	FILE* fp = fopen(filename, "rb");
	if(NULL == fp)
	{
		printf("Open File failed.\n");
		return -1;
	}

	audio_output ao;
	if (!ao.open(channel, frequency, format, frequency))
	{
		printf("audio open(%d, %d, %d) failed: %d\n", channel, frequency, format, errno);
		return 0;
	}

	ao.play();

	int samples = frequency / 10;
	int bytes_per_sample = channel * PCM_SAMPLE_BITS(format) / 8;
	char *pcm = (char*)malloc(samples * bytes_per_sample);
	while(1)
	{
		if(ao.getsamples() > frequency / 2)
		{
#ifdef WIN32
			Sleep(20);
#else
			usleep(20*1000);
#endif
			continue;
		}

		int n = fread(pcm, bytes_per_sample, samples, fp);
		if(n <= 0)
			break;

		ao.write(pcm, n);
	}
	free(pcm);

	while(ao.getsamples() > 0)
	{
#ifdef WIN32
		Sleep(10);
#else
		usleep(10*1000);
#endif
	}

	fclose(fp);
	return 0;
}

int main(int argc, char* argv[])
{
	int channels = 1;
	int frequency = 8000;
	int format = PCM_SAMPLE_FMT_S16;
	const char* filename = "audio.pcm";
	if(argc != 5)
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
	return TestAudioOut(filename, channels, frequency, format);
}
