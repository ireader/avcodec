#include "avtimeline.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static inline int64_t max64(int64_t t1, int64_t t2)
{
	return t1 > t2 ? t1 : t2;
}

void avtimeline_init(struct avtimeline_t* t, uint32_t gap, int64_t t0)
{
	memset(t, 0, sizeof(*t));
	t->t0 = t0;
	t->max = 0;
	t->gap = gap;
}

void avtimeline_reset(struct avtimeline_t* t)
{
	t->max = 0;
	memset(t->streams, 0, sizeof(t->streams));
}

static int64_t avtimeline_map32(struct avtimeline_t* t, int stream, uint32_t dts)
{
	assert(stream >= 0 && stream <= sizeof(t->streams) / sizeof(t->streams[0]));
	return t->streams[stream].t + (int64_t)(uint32_t)(dts - (uint32_t)(t->streams[stream].dts));
}

static int64_t avtimeline_map64(struct avtimeline_t* t, int stream, int64_t dts)
{
	assert(stream >= 0 && stream <= sizeof(t->streams) / sizeof(t->streams[0]));
	return t->streams[stream].t + (dts - t->streams[stream].dts);
}

static int64_t avtimeline_max(struct avtimeline_t* t)
{
	size_t i;
	int64_t timestamp;

	timestamp = t->t0;
	for (i = 0; i < sizeof(t->streams) / sizeof(t->streams[0]); i++)
	{
		if (0 == t->streams[i].init)
			continue;
		if (t->streams[i].tmax > timestamp)
			timestamp = t->streams[i].tmax;
	}

	return timestamp;
}

static void avtimeline_rebuild(struct avtimeline_t* t, int stream, int64_t dts)
{
	int64_t timestamp;
	timestamp = avtimeline_max(t);
	//avtimeline_reset(t);

	t->streams[stream].t = timestamp + 1; // monotone increasing timestamp
	t->streams[stream].tmax = t->streams[stream].t;
	t->streams[stream].dts = dts;
	t->streams[stream].init = 1;
	t->streams[stream].repeat = 0;
}

static void avtimeline_stream_init(struct avtimeline_t* t, int stream, int64_t dts)
{
	int i;
	int32_t diff;

	for (i = 0; i < sizeof(t->streams) / sizeof(t->streams[0]); i++)
	{
		if (!t->streams[i].init)
			continue;

		diff = (int32_t)(dts - t->streams[i].dts);
		if (((diff > 0 && diff < (int32_t)t->gap /*gap*/) || (diff < 0 && -diff < (int32_t)t->gap)) 
			&& t->streams[i].t + diff >= 0) // fix stream base timestamp < 0
		{
			t->streams[stream].init = 1;
			t->streams[stream].t = t->streams[i].t + diff;
			t->streams[stream].tmax = t->streams[i].t;
			t->streams[stream].dts = dts;
			return;
		}
	}

	t->streams[stream].init = 1;
	t->streams[stream].t = avtimeline_max(t);
	t->streams[stream].tmax = t->streams[stream].t;
	t->streams[stream].dts = dts;
}

int64_t avtimeline_input32(struct avtimeline_t* t, int stream, uint32_t dts, uint32_t pts, int *discontinuity)
{
	int init;
	int32_t diff;
	uint32_t last;
	int64_t timestamp;

	if (stream < 0 || stream >= sizeof(t->streams) / sizeof(t->streams[0]))
		return 0;

	init = t->streams[stream].init;
	if (!t->streams[stream].init)
	{
		avtimeline_stream_init(t, stream, dts);
	}

	*discontinuity = 0;
	timestamp = t->streams[stream].t;
	last = (uint32_t)(t->streams[stream].dts);
	
	diff = (int32_t)(dts - last);
	if ( diff > (int32_t)t->gap /*gap*/ || (diff < 0 && last - dts > t->gap /*rewind*/ ) )
	{
		// reset timestamp
		avtimeline_rebuild(t, stream, dts);
		*discontinuity = 1;

		timestamp = avtimeline_map32(t, stream, dts);
	}
	else if (diff > t->streams[stream].repeat)
	{
		timestamp = avtimeline_map32(t, stream, dts);

		t->streams[stream].dts = dts; // update last dts
	}
	else
	{
		// fix: don't change t, for timestamp inc
	}

	if (t->max < timestamp)
	{
		t->max = timestamp;
	}
	else if (timestamp + t->gap < t->max)
	{
		// a/v timeline diff > gap
		t->streams[stream].t = avtimeline_max(t);
		t->streams[stream].dts = dts;
		timestamp = avtimeline_map32(t, stream, dts);
	}

	t->streams[stream].t = timestamp;

	t->streams[stream].repeat = (0 == *discontinuity && diff <= t->streams[stream].repeat && init) ? t->streams[stream].repeat + 1 : 0;
	timestamp += t->streams[stream].repeat; // monotone increasing timestamp

	t->streams[stream].tmax = max64(t->streams[stream].tmax, max64(timestamp, (int64_t)(pts - dts) + timestamp));
	return timestamp;
}

int64_t avtimeline_input64(struct avtimeline_t* t, int stream, int64_t dts, int64_t pts, int* discontinuity)
{
	int init;
	int64_t diff;
	int64_t last;
	int64_t timestamp;

	if (stream < 0 || stream >= sizeof(t->streams) / sizeof(t->streams[0]))
		return 0;

	init = t->streams[stream].init;
	if (!t->streams[stream].init)
	{
		avtimeline_stream_init(t, stream, dts);
	}

	last = t->streams[stream].dts;
	timestamp = t->streams[stream].t;

	diff = dts - last;
	if (diff > t->gap /*gap*/ || (diff < 0 && last - dts > t->gap /*rewind*/ ))
	{
		avtimeline_rebuild(t, stream, dts);
		*discontinuity = 1;

		timestamp = avtimeline_map64(t, stream, dts);
	}
	else if (diff > t->streams[stream].repeat)
	{
		timestamp = avtimeline_map64(t, stream, dts);

		t->streams[stream].dts = dts; // update last dts
	}
	else
	{
		// fix: don't change t, for timestamp inc
	}

	if (t->max < timestamp)
	{
		t->max = timestamp;
	}
	else if (timestamp + t->gap < t->max)
	{
		t->streams[stream].t = avtimeline_max(t);
		t->streams[stream].dts = dts;
		timestamp = avtimeline_map64(t, stream, dts);
	}

	t->streams[stream].t = timestamp;

	t->streams[stream].repeat = (0 == *discontinuity && diff <= t->streams[stream].repeat && init) ? t->streams[stream].repeat + 1 : 0;
	timestamp += t->streams[stream].repeat; // monotone increasing timestamp

	t->streams[stream].tmax = max64(t->streams[stream].tmax, max64(timestamp, (int64_t)(pts - dts) + timestamp));
	return timestamp;
}


#if defined(_DEBUG) || defined(DEBUG)
#include <time.h>

static void avtimeline_test1(void)
{
	//static const int videoStep = 33; // 30fps
	//static const int audioStep = 23; // 1024/44100Hz
	static const int videoStep = 33000000; // 30fps
	static const int audioStep = 23000000; // 1024/44100Hz

	struct avtimeline_t line;
	uint64_t atotal, vtotal;
	uint32_t adts, vdts;
	int64_t t, at, vt;
	uint64_t i;
	int discontinuity;
	
	vdts = adts = rand();
	//adts = 0xFFFFFF00;
	//vdts = 0xFFFF0000;
	
	avtimeline_init(&line, 50000000, vdts);
	atotal = vtotal = at = vt = 0;
	//while (atotal < 0x20000)
	for(i = 0; atotal < 8 * (uint64_t)(0xFFFFFFFF) + 100; i++)
	{
		if(atotal < vtotal) 
		{
			t = avtimeline_input32(&line, 0, adts, adts, &discontinuity);
			assert(0 == at || t == at + audioStep);
			//printf("[A] dts: %u, timeline: %u\n", adts, t);
			at = t;
			adts += audioStep;
			atotal += audioStep;
		}
		else 
		{
			t = avtimeline_input32(&line, 1, vdts, vdts, &discontinuity);
			assert(0 == vt || t == vt + videoStep);
			//printf("[V] dts: %u, timeline: %u\n", vdts, t);
			vt = t;
			vdts += videoStep;
			vtotal += videoStep;
		}
	}
}

static void avtimeline_gap_test(void)
{
	int ad, vd;
	uint32_t adts, vdts;
	int64_t at[10], vt[10];
	struct avtimeline_t line;

	vdts = adts = rand();
	avtimeline_init(&line, 5000, adts);

	at[0] = avtimeline_input32(&line, 0, adts, adts, &ad);
	vt[0] = avtimeline_input32(&line, 1, vdts, vdts, &vd);
	assert(!ad && !vd && at[0] == adts);

	adts += 100;
	vdts += 500;
	at[1] = avtimeline_input32(&line, 0, adts, adts, &ad);
	vt[1] = avtimeline_input32(&line, 1, vdts, vdts, &vd);
	assert(!ad && !vd && at[1] - at[0] == 100 && vt[1] - vt[0] == 500);

	adts += 2 * 1000;
	vdts += 500;
	at[2] = avtimeline_input32(&line, 0, adts, adts, &ad);
	vt[2] = avtimeline_input32(&line, 1, vdts, vdts, &vd);
	assert(!ad && !vd && at[2] - at[1] == 2*1000 && vt[2] - vt[1] == 500);

	adts += 3 * 1000;
	vdts += 6 * 1000;
	at[3] = avtimeline_input32(&line, 0, adts, adts, &ad);
	vt[3] = avtimeline_input32(&line, 1, vdts, vdts, &vd);
	assert(!ad && vd && at[3] - at[2] == 3 * 1000 && vt[3] - at[3] == 1);

	adts += 1;
	vdts += 500;
	at[4] = avtimeline_input32(&line, 0, adts, adts, &ad); //rebuild
	vt[4] = avtimeline_input32(&line, 1, vdts, vdts, &vd);
	assert(!ad && !vd && at[4] == vt[3] && vt[4] - vt[3] == 500);

	adts += 3 * 1000;
	vdts += 6 * 1000;
	at[5] = avtimeline_input32(&line, 0, adts, adts, &ad);
	vt[5] = avtimeline_input32(&line, 1, vdts, vdts, &vd);
	assert(!ad && vd && at[5] - at[4] == 3 * 1000 && vt[5] - at[5] == 1);

	adts += 1;
	vdts += 500;
	at[6] = avtimeline_input32(&line, 0, adts, adts, &ad);
	vt[6] = avtimeline_input32(&line, 1, vdts, vdts, &vd);
	assert(!ad && !vd && at[6] == vt[5] && vt[6] - vt[5] == 500);

	adts += 6 * 1000;
	vdts += 6 * 1000;
	at[7] = avtimeline_input32(&line, 0, adts, adts, &ad);
	vt[7] = avtimeline_input32(&line, 1, vdts, vdts, &vd); // init
	assert(ad && vd && at[7] - vt[6] == 1 && vt[7] == at[7]+1);

	adts += 100;
	vdts += 500;
	at[8] = avtimeline_input32(&line, 0, adts, adts, &ad);
	vt[8] = avtimeline_input32(&line, 1, vdts, vdts, &vd);
	assert(!ad && !vd && at[8] - at[7] == 100 && vt[8] - vt[7] == 500);

	adts += 100;
	vdts += 500;
	at[9] = avtimeline_input32(&line, 0, adts, adts, &ad);
	vt[9] = avtimeline_input32(&line, 1, vdts, vdts, &vd);
	assert(!ad && !vd && at[9] - at[8] == 100 && vt[9] - vt[8] == 500);
}

static void avtimeline_monotone_increment_test(void)
{
	uint32_t dts;
	int64_t timestamp;
	int discontinuity;
	struct avtimeline_t line;
	
	dts = rand();
	avtimeline_init(&line, 5000, dts);
	timestamp = avtimeline_input32(&line, 0, dts, dts, &discontinuity);
	assert(timestamp + 1 == avtimeline_input32(&line, 0, dts, dts, &discontinuity));
	assert(timestamp + 2 == avtimeline_input32(&line, 0, dts, dts, &discontinuity));
	assert(timestamp + 3 == avtimeline_input32(&line, 0, dts, dts, &discontinuity));
	assert(timestamp + 4 == avtimeline_input32(&line, 0, dts, dts, &discontinuity));
	assert(timestamp + 5 == avtimeline_input32(&line, 0, dts+3, dts+3, &discontinuity));
	assert(timestamp + 10 == avtimeline_input32(&line, 0, dts + 10, dts + 10, &discontinuity));
	assert(timestamp + 11 == avtimeline_input32(&line, 0, dts + 10, dts + 10, &discontinuity));
	assert(timestamp + 12 == avtimeline_input32(&line, 0, dts + 10, dts + 10, &discontinuity));
}

static void avtimeline_dts_revert_test(void)
{
	int discontinuity;
	struct avtimeline_t line;

	avtimeline_init(&line, 5000, 334);
	assert(334 == avtimeline_input32(&line, 0, 8143402, 8143402, &discontinuity));
	assert(368 == avtimeline_input32(&line, 0, 8143436, 8143436, &discontinuity));
	assert(501 == avtimeline_input32(&line, 0, 8143569, 8143569, &discontinuity));
	assert(502 == avtimeline_input32(&line, 0, 8143502, 8143502, &discontinuity));
	assert(503 == avtimeline_input32(&line, 0, 8143536, 8143536, &discontinuity));
	assert(601 == avtimeline_input32(&line, 0, 8143669, 8143669, &discontinuity));
}

static void avtimeline_diff_start_test(void)
{
	int discontinuity;
	struct avtimeline_t line;

	avtimeline_init(&line, 5000, 0);
	assert(0 == avtimeline_input32(&line, 1, 0, 0, &discontinuity));
	assert(10 == avtimeline_input32(&line, 1, 10, 10, &discontinuity));
	assert(300 == avtimeline_input32(&line, 1, 300, 300, &discontinuity));
	assert(320 == avtimeline_input32(&line, 0, 320, 320, &discontinuity));
	assert(290 == avtimeline_input32(&line, 2, 290, 290, &discontinuity));
}

static void avtimeline_diff_start_test2(void)
{
	int discontinuity;
	struct avtimeline_t line;

	avtimeline_init(&line, 5000, 0);
	avtimeline_input32(&line, 1, 65368, 65408, &discontinuity);
	avtimeline_input32(&line, 1, 65402, 65442, &discontinuity);
	avtimeline_input32(&line, 1, 65435, 65475, &discontinuity);
	avtimeline_input32(&line, 1, 95443684, 7, &discontinuity);
	avtimeline_input32(&line, 1, 0, 40, &discontinuity);
	avtimeline_input32(&line, 1, 33, 73, &discontinuity);
}

void avtimeline_test_video_lost(void)
{
	int i, dts, discontinuity;
	struct avtimeline_t line;

	static struct {
		int stream;
		uint32_t dts;
	} abc[] = { {1, 101569},
{0, 101338},
{1, 101611},
{0, 101375},
{0, 101415},
{0, 101457},
{1, 101643},
{0, 101499},
{1, 101697},
{0, 101537},
{0, 101575},
{0, 101617},
{1, 101739},
{1, 101782},
{1, 101814},
{0, 101655},
{0, 101696},
{0, 101734},
{0, 101776},
{1, 101867},
{0, 101817},
{0, 101854},
{1, 101910},
{1, 101942},
{0, 101894},
{0, 101936},
{0, 101975},
{1, 101995},
{0, 102016},
{1, 102038},
{0, 102057},
{1, 102081},
{0, 102096},
{1, 102113},
{0, 102136},
{1, 102166},
{0, 102172},
{1, 102208},
{0, 102212},
{0, 102243},
{1, 102251},
{1, 102284},
{0, 102294},
{0, 102332},
{1, 102336},
{0, 102366},
{1, 102379},
{0, 102415},
{1, 102422},
{1, 102454},
{0, 102455},
{0, 102490},
{1, 102508},
{0, 102533},
{1, 102551},
{0, 102573},
{1, 102582},
{0, 102611},
{1, 102636},
{0, 102655},
{1, 102678},
{0, 102694},
{1, 102720},
{0, 102736},
{1, 102752},
{0, 102772},
{0, 102805},
{1, 102806},
{1, 102848},
{0, 102853},
{1, 102891},
{0, 102894},
{1, 102923},
{0, 102926},
{0, 102964},
{1, 102977},
{0, 103011},
{1, 103019},
{0, 103055},
{1, 103062},
{0, 103090},
{1, 103094},
{0, 103134},
{1, 103148},
{0, 103175},
{1, 103190},
{0, 103217},
{1, 103222},
{0, 103251},
{1, 103275},
{0, 103295},
{1, 103318},
{0, 103336},
{1, 103361},
{0, 103377},
{1, 103393},
{0, 103414},
{1, 103446},
{0, 103458},
{1, 103489},
{0, 103497},
{1, 103531},
{0, 103536},
{1, 103563},
{0, 103573},
{0, 103615},
{1, 103617},
{0, 103656},
{1, 103659},
{0, 103690},
{1, 103702},
{0, 103732},
{1, 103734},
{0, 103772},
{1, 103788},
{0, 103815},
{1, 103830},
{0, 103853},
{1, 103862},
{0, 103894},
{1, 103915},
{0, 103934},
{1, 103958},
{0, 103976},
{1, 104001},
{0, 104017},
{1, 104033},
{0, 104056},
{1, 104086},
{0, 104098},
{1, 104129},
{0, 104137},
{1, 104171},
{0, 104173},
{1, 104203},
{0, 104210},
{0, 104238},
{1, 104257},
{0, 104292},
{1, 104299},
{0, 104333},
{1, 104342},
{0, 104365},
{1, 104374},
{0, 104414},
{1, 104428},
{0, 104455},
{1, 104470},
{0, 104491},
{1, 104502},
{0, 104529},
{1, 104556},
{0, 104574},
{1, 104599},
{0, 104615},
{1, 104641},
{0, 104655},
{1, 104672},
{0, 104690},
{1, 104727},
{0, 104734},
{1, 104768},
{0, 104772},
{0, 104800},
{1, 104811},
{1, 104844},
{0, 104846},
{0, 104890},
{1, 104896},
{1, 104939},
{1, 104982},
{1, 105014},
{1, 105067},
{1, 105110},
{1, 105368},
{1, 105581},
{1, 105622},
{1, 105660},
{1, 105701},
{0, 104931},
{0, 104974},
{1, 105741},
{1, 105781},
{1, 105821},
{1, 105861},
{1, 105921},
{1, 105961},
{1, 106001},
{1, 106041},
{1, 106081},
{1, 106121},
{1, 106161},
{1, 106221},
{1, 106261},
{1, 106402},
{1, 106404},
{1, 106526},
{1, 106529},
{1, 106557},
{1, 106657},
{1, 106659},
{1, 106774},
{1, 106775},
{1, 106850},
{1, 106852},
{1, 106910},
{1, 106931},
{1, 107010},
{1, 107037},
{1, 107074},
{1, 107186},
{1, 107188},
{1, 107299},
{1, 107301},
{1, 107367},
{1, 107485},
{1, 107487},
{1, 107574},
{1, 107576},
{1, 107675},
{1, 107676},
{1, 107777},
{1, 107779},
{1, 107894},
{1, 107896},
{1, 107998},
{1, 108000},
{1, 108094},
{1, 108095},
{1, 108122},
{1, 108220},
{1, 108245},
{1, 108362},
{1, 108363},
{1, 108395},
{1, 108496},
{1, 108501},
{1, 108612},
{1, 108614},
{1, 108690},
{1, 108693},
{1, 108779},
{1, 108780},
{1, 108860},
{1, 108862},
{1, 108959},
{1, 108960},
{1, 109041},
{1, 109042},
{1, 109156},
{1, 109158},
{1, 109199},
{1, 109269},
{1, 109389},
{1, 109390},
{1, 109452},
{1, 109454},
{1, 109522},
{1, 109524},
{1, 109627},
{1, 109649},
{1, 109695},
{1, 109755},
{1, 109756},
{1, 109864},
{1, 109866},
{1, 109903},
{1, 109954},
{1, 109979},
{1, 110044},
{1, 110106},
{1, 110108},
{1, 110184},
{1, 110186},
{1, 110255},
{1, 110275},
{1, 110393},
{1, 110394},
{1, 110445},
{1, 110545},
{1, 110548},
{1, 110658},
{1, 110661},
{1, 110690},
{1, 110740},
{1, 110762},
{1, 110877},
{1, 110879},
{1, 110969},
{1, 110971},
{1, 111049},
{1, 111096},
{1, 111098},
{1, 111207},
{1, 111209},
{1, 111245},
{1, 111272},
{1, 111393},
{1, 111395},
{1, 111424},
{1, 111496},
{1, 111498},
{1, 111568},
{1, 111658},
{1, 111660},
{1, 111682},
{1, 111790},
{1, 111792},
{1, 111854},
{1, 111856},
{1, 111955},
{1, 111957},
{1, 112007},
{1, 112100},
{1, 112102},
{1, 112158},
{1, 112272},
{1, 112274},
{1, 112295},
{1, 112360},
{1, 112418},
{1, 112420},
{1, 112467},
{1, 112521},
{1, 112642},
{1, 112647},
{1, 112695},
{1, 112777},
{1, 112779},
{1, 112888},
{1, 112890},
{1, 112969},
{1, 112971},
{1, 113053},
{1, 113054},
{1, 113156},
{1, 113158},
{1, 113273},
{1, 113275},
{1, 113311},
{1, 113406},
{1, 113409},
{1, 113499},
{1, 113500},
{1, 113572},
{1, 113573},
{1, 113648},
{1, 113649},
{1, 113688},
{1, 113728},
{1, 113788},
{1, 113828},
{1, 113868},
{1, 113908},
{0, 114644},
{1, 114672},
{0, 114684},
{1, 114715},
{0, 114724},
{1, 114758},
{0, 114762},
{0, 114801},
{1, 114811},
{0, 114840},
{1, 114843},
{0, 114881},
{1, 114886},
{0, 114921},
{1, 114929},
{0, 114964},
{1, 114971},
{0, 115001},
{1, 115014},
{0, 115041},
{1, 115057},
{0, 115080}, };

	avtimeline_init(&line, 5000, abc[0].dts);
	for (i = 0; i < sizeof(abc) / sizeof(abc[0]); i++)
	{
		dts = avtimeline_input32(&line, abc[i].stream, abc[i].dts, abc[i].dts, &discontinuity);
		printf("- [%d] %u diff: %d%s\n", abc[i].stream, dts, (int)(dts - abc[i].dts), discontinuity ? " [X]" : "");
	}
}

void avtimeline_test(void)
{
	unsigned int seed;
	seed = (unsigned int)time(NULL);
	srand(seed);

	avtimeline_test1();
	avtimeline_gap_test();
	avtimeline_monotone_increment_test();
	avtimeline_dts_revert_test();
	avtimeline_diff_start_test();
	avtimeline_diff_start_test2();
	avtimeline_test_video_lost();
}

#endif
