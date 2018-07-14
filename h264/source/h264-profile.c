#include <assert.h>

enum h264_profile
{
	H264_PROFILE_UNKNOWN = 0,

	H264_PROFILE_BASELINE = 66, // A.2.1 Baseline profile
	H264_PROFILE_MAIN = 77, // A.2.2 Main profile
	H264_PROFILE_EXTENDED = 88, // A.2.3 Extended profile
	H264_PROFILE_HIGH = 100, // A.2.4 High profile
	H264_PROFILE_HIGH10 = 110, // A.2.5 High 10 profile
	H264_PROFILE_HIGH422 = 122, // A.2.6 High 4:2:2 profile
	H264_PROFILE_HIGH444 = 244, // A.2.7 High 4:4:4 Predictive profile
	H264_PROFILE_CAVLC = 44, // A.2.11 CAVLC 4:4:4 Intra profile

	H264_PROFILE_SCALABLE_BASELINE = 83, // G.10.1.1 Scalable Baseline profile
	H264_PROFILE_SCALABLE_HIGH = 86, // G.10.1.2 Scalable High profile

	H264_PROFILE_MULTIVIEW_HIGH = 118, // H.10.1.1 Multiview High profile
	H264_PROFILE_STEREO_HIGH = 128, // H.10.1.2 Stereo High profile
	H264_PROFILE_MFC_HIGH = 134, // H.10.1.3 MFC High profile

	H264_PROFILE_MULTIVIEW_DEPTH_HIGH = 138, // I.10.1.1 Multiview Depth High profile
	H264_PROFILE_MFC_DEPTH_HIGH = 135, // I.10.1.2 MFC Depth High profile
	
	H264_PROFILE_ENHANCED_MULTIVIEW_DEPTH_HIGH = 139, // J.10.1.1 Enhanced Multiview Depth High profile
};

enum h264_level
{
	H264_LEVEL_UNKNOWN = 0,
	H264_LEVEL_1_0, // 128x96@30.9 (8) / 176x144@15.0 (4)
	H264_LEVEL_1_B, // 128x96@30.9 (8) / 176x144@15.0 (4)
	H264_LEVEL_1_1, // 176x144@30.3 (9) / 320x240@10.0 (3) / 352x288@7.5 (2)
	H264_LEVEL_1_2, // 320x240@20.0 (7) / 352x288@15.2 (6)
	H264_LEVEL_1_3, // 320x240@36.0 (7) / 352x288@30.0 (6)
	H264_LEVEL_2_0, // 320x240@36.0 (7) / 352x288@30.0 (6)
	H264_LEVEL_2_1, // 352x480@30.0 (7) / 352x576@25.0 (6)
	H264_LEVEL_2_2, // 352x480@30.7 (12) / 352x576@25.6 (10) / 720x480@15.0 (6) / 720x576@12.5 (5)
	H264_LEVEL_3_0, // 352x480@61.4 (12) / 352x576@51.1 (10) / 720x480@30.0 (6) / 720x576@25.0 (5)
	H264_LEVEL_3_1, // 720x480@80.0 (13) / 720x576@66.7 (11) / 1280x720@30.0 (5)
	H264_LEVEL_3_2, // 1280x720@60.0 (5) / 1280x1024@42.2 (4)
	H264_LEVEL_4_0, // 1280x720@68.3 (9) / 1920x1080@30.1 (4) / 2048x1024@30.0 (4)
	H264_LEVEL_4_1, // 1280x720@68.3 (9) / 1920x1080@30.1 (4) / 2048x1024@30.0 (4)
	H264_LEVEL_4_2, // 1280x720@145.1 (9) / 1920x1080@64.0 (4) / 2048x1080@60.0 (4)
	H264_LEVEL_5_0, // 1920x1080@72.3 (13) / 2048x1024@72.0 (13) / 2048x1080@67.8 (12) / 2560x1920@30.7 (5) / 3672x1536@26.7 (5)
	H264_LEVEL_5_1, // 1920x1080@120.5 (16) / 2560x1920@51.2 (9) / 3840x2160@31.7 (5) / 4096x2048@30.0 (5) / 4096x2160@28.5 (5) / 4096x2304@26.7 (5)
	H264_LEVEL_5_2, // 1920x1080@172.0 (16) / 2560x1920@108.0 (9) / 3840x2160@66.8 (5) / 4096x2048@63.3 (5) / 4096x2160@60.0 (5) / 4096x2304@56.3 (5)
};

const char* h264_profile2(int profile, int constraint)
{
	enum {
		constraint_set0_flag = 0x80,
		constraint_set1_flag = 0x40,
		constraint_set2_flag = 0x20,
		constraint_set3_flag = 0x10,
		constraint_set4_flag = 0x08,
		constraint_set5_flag = 0x05,
	};

	switch (profile)
	{
	case H264_PROFILE_BASELINE:
		if (constraint_set1_flag & constraint)
			return "Constrained Baseline"; // A.2.1.1 Constrained Baseline profile
		else
			return "Baseline"; // A.2.1 Baseline profile

	case H264_PROFILE_MAIN:
		return "Main"; // A.2.2 Main profile

	case H264_PROFILE_EXTENDED:
		return "Extended"; // A.2.3 Extended profile

	case H264_PROFILE_HIGH:
		if (constraint_set4_flag & constraint)
			if(constraint_set5_flag & constraint)
				return "Constrained High"; // A.2.4.2 Constrained High profile
			else
				return "Progressive High"; // A.2.4.1 Progressive High profile
		else 
			return "High"; // A.2.4 High profile

	case H264_PROFILE_HIGH10:
		if (constraint_set4_flag & constraint)
			return "Progressive High 10"; // A.2.5.1 Progressive High 10 profile
		else if (constraint_set3_flag & constraint)
			return "High 10 Intra"; // A.2.8 High 10 Intra profile
		else
			return "High 10"; // A.2.5 High 10 profile

	case H264_PROFILE_HIGH422:
		if(constraint_set3_flag & constraint)
			return "High 4:2:2 Intra"; // A.2.9 High 4:2:2 Intra profile
		else
			return "High 4:2:2"; // A.2.6 High 4:2:2 profile

	case H264_PROFILE_HIGH444:
		if (constraint_set3_flag & constraint)
			return "High 4:4:4 Intra"; // A.2.10 High 4:4:4 Intra profile
		else
			return "High 4:4:4 Predictive"; // A.2.7 High 4:4:4 Predictive profile

	case H264_PROFILE_CAVLC:
		return "CAVLC 4:4:4 Intra"; // A.2.11 CAVLC 4:4:4 Intra profile

	case H264_PROFILE_SCALABLE_BASELINE:
		if (constraint_set5_flag & constraint)
			return "Scalable Constrained Baseline"; // G.10.1.1.1 Scalable Constrained Baseline profile
		else
			return "Scalable Baseline"; // G.10.1.1 Scalable Baseline profile

	case H264_PROFILE_SCALABLE_HIGH:
		if (constraint_set5_flag & constraint)
			return "Scalable Constrained High"; // G.10.1.2.1 Scalable Constrained High profile
		else if (constraint_set3_flag & constraint)
			return "Scalable High Intra"; // G.10.1.3 Scalable High Intra profile
		else
			return "Scalable High"; // G.10.1.2 Scalable High profile

	case H264_PROFILE_STEREO_HIGH:
		return "Stereo High";

	case H264_PROFILE_MULTIVIEW_HIGH: 
		return "Multiview High";

	case H264_PROFILE_MULTIVIEW_DEPTH_HIGH: 
		return "Multiview Depth High";

	case H264_PROFILE_MFC_HIGH: 
		return "MFC High";

	case H264_PROFILE_MFC_DEPTH_HIGH: 
		return "MFC Depth High";

	case H264_PROFILE_ENHANCED_MULTIVIEW_DEPTH_HIGH: 
		return "Enhanced Multiview Depth High";

	default: 
		return "Unknown";
	}
}

const char* h264_profile(int profile)
{
	return h264_profile2(profile, 0);
}
