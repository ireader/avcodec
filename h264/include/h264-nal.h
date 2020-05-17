#ifndef _h264_nal_h_
#define _h264_nal_h_

// Rec. ITU-T H.264 (02/2014)
// Table 7-1 - NAL unit type codes, syntax element categories, and NAL unit type classes
#define H264_NAL_IDR				5 // Coded slice of an IDR picture
#define H264_NAL_SEI				6 // Supplemental enhancement information
#define H264_NAL_SPS				7 // Sequence parameter set
#define H264_NAL_PPS				8 // Picture parameter set
#define H264_NAL_AUD				9 // Access unit delimiter
#define H264_NAL_SPS_EXTENSION		13 // Access unit delimiter
#define H264_NAL_PREFIX				14 // Prefix NAL unit
#define H264_NAL_SPS_SUBSET			15 // Subset sequence parameter set
#define H264_NAL_XVC				20 // Coded slice extension(SVC/MVC)
#define H264_NAL_3D					21 // Coded slice extension for a depth view component or a 3D-AVC texture view component

struct h264_nal_t
{
	unsigned int forbidden_zero_bit : 1; // 0 only
	unsigned int nal_ref_idc : 2;
	unsigned int nal_unit_type : 5;	// H264_NALU_XXX

	unsigned int svc_extension_flag : 1; // SVC/MVC, since Rec. ITU-T H.264 (01/2012)
	unsigned int avc_3d_extension_flag : 1; // 3D-AVC, since Rec. ITU-T H.264 (02/2014)
};

#endif /* !_h264_nal_h_ */
