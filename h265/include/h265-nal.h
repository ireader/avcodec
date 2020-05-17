#ifndef _h265_nal_h_
#define _h265_nal_h_

// Rec. ITU-T H.265 v4 (12/2016)
// Table 7-1 - NAL unit type codes and NAL unit type classes
#define H265_BLA_W_LP				16 // Coded slice segment of a BLA picture
#define H265_RSV_IRAP_VCL23			23 // Reserved IRAP VCL NAL unit types
#define H265_NAL_VPS				32 // Video parameter set
#define H265_NAL_SPS				33 // Sequence parameter set
#define H265_NAL_PPS				34 // Picture parameter set
#define H265_NAL_AUD				35 // Access unit delimiter

enum
{
	H265_PROFILE_MAIN = 1, // A.3.2 Main profile
	H265_PROFILE_MAIN10 = 2, // A.3.3 Main 10 and Main 10 Still Picture profiles
	H265_PROFILE_MAIN10_STILL_PICTURE = 2, // A.3.3 Main 10 and Main 10 Still Picture profiles
	H265_PROFILE_STILL_PICTURE = 3, // A.3.4 Main Still Picture profile
	H264_PROFILE_HIGH = 5, // A.3.6 High throughput profiles
};

enum {
	H265_LEVEL_1 = 1,
	H265_LEVEL_2,
	H265_LEVEL_2_1,
	H265_LEVEL_3,
	H265_LEVEL_3_1,
	H265_LEVEL_4,
	H265_LEVEL_4_1,
	H265_LEVEL_5,
	H265_LEVEL_5_1,
	H265_LEVEL_6,
	H265_LEVEL_6_1,
	H265_LEVEL_6_2,
};

struct h265_nal_t
{
	unsigned int forbidden_zero_bit : 1; // 0 only
	unsigned int nal_unit_type : 6;	// H265_NALU_XXX
	unsigned int nuh_layer_id : 6;
	unsigned int nuh_temporal_id_plus1 : 3;
};

#endif /* _h265_nal_h_ */
