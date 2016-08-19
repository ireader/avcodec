#ifndef _h264_nalu_h_
#define _h264_nalu_h_

// Rec. ITU-T H.264 (02/2014)
// Table 7-1 ¨C NAL unit type codes, syntax element categories, and NAL unit type classes
#define H264_NALU_IDR			5 // Coded slice of an IDR picture
#define H264_NALU_SEI			6 // Supplemental enhancement information
#define H264_NALU_SPS			7 // Sequence parameter set
#define H264_NALU_PPS			8 // Picture parameter set
#define H264_NALU_AUD			9 // Access unit delimiter
#define H264_NALU_SPS_EXTENSION	13 // Access unit delimiter
#define H264_NALU_SPS_SUBSET	15 // Subset sequence parameter set

typedef void (*h264_nalu_handler)(void* param, const unsigned char* nalu, unsigned int bytes);
void h264_nalu(const unsigned char* h264, unsigned int bytes, h264_nalu_handler handler, void* param);

#endif /* !_h264_nalu_h_ */
