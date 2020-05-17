#ifndef _h264_util_h_
#define _h264_util_h_

#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>

static inline const uint8_t* h264_startcode(const uint8_t *data, size_t bytes)
{
	size_t i;
	for(i = 2; i + 1 < bytes; i++)
	{
		if(0x01 == data[i] && 0x00 == data[i-1] && 0x00 == data[i-2])
			return data + i + 1;
	}

	return NULL;
}

static inline uint8_t h264_type(const uint8_t *data, size_t bytes)
{
	data = h264_startcode(data, bytes);
	return data ? (data[0] & 0x1f)  : 0x00;
}

/// @return 1-IDR frame, 0-no IDR frame
static inline int h264_idr(const uint8_t *data, size_t bytes)
{
	uint8_t nalutype;
	const uint8_t *p;

	do
	{
		p = h264_startcode(data, bytes);
		if(p)
		{
			nalutype = p[0] & 0x1f;
			// 1: no-IDR slice
			// 2: A-slice
			// 3: B-slice
			// 4: C-slice
			// 5: IDR frame
			if(nalutype > 0 && nalutype < 6)
			{
				return 5 == nalutype ? 1 : 0;
			}

			bytes -= p - data;
			data = p;
		}
	} while(p);

	return 0;
}

typedef void(*h264_nalu_handler)(void* param, const void* nalu, size_t bytes);
void h264_stream(const void* h264, size_t bytes, h264_nalu_handler handler, void* param);


// Table 6-1 - SubWidthC, and SubHeightC values derived from chroma_format_idc and separate_colour_plane_flag (p43)
///@param[in] chroma_format_idc 0-monochrome, 1-4:2:0, 2-4:2:2, 3-4:4:4
///@param[out] width SubWidthC
///@param[out] height SubHeightC
static inline void h264_chroma_sample(unsigned int chroma_format_idc, unsigned int *width, unsigned int *height)
{
	static unsigned int s_chroma_width[4] = { 0, 2, 2, 1 };
	static unsigned int s_chroma_height[4] = { 0, 2, 1, 1 };
	*width = s_chroma_width[chroma_format_idc % 4];
	*height = s_chroma_height[chroma_format_idc % 4];
}

// MbWidthC = 16 / SubWidthC (p48)
static inline unsigned int h264_chroma_mb_width(unsigned int chroma_format_idc)
{
	static unsigned int s_chroma_width[4] = { 0, 8, 8, 16 };
	return s_chroma_width[chroma_format_idc % 4];
}

// MbHeightC = 16 / SubHeightC (p48)
static inline unsigned int h264_chroma_mb_height(unsigned int chroma_format_idc)
{
	static unsigned int s_chroma_height[4] = { 0, 8, 16, 16 };
	return s_chroma_height[chroma_format_idc % 4];
}

// RawMbBits = 256 * BitDepthY + 2 * MbWidthC * MbHeightC * BitDepthC (p98)
///@param[in] luma luma bit depth
///@param[in] chroma chroma bit depth
///@param[in] chroma_format_idc 0-monochrome, 1-4:2:0, 2-4:2:2, 3-4:4:4
static inline unsigned int h264_mb_bits(unsigned int luma, unsigned int chroma, unsigned int chroma_format_idc)
{
	static unsigned int s_chroma_format_idc[4] = { 0, 8 * 8, 16 * 8, 16 * 16 };
	return 16 * 16 * luma + 2 * chroma * s_chroma_format_idc[chroma_format_idc % 4];
}

// MaxFrameNum = 2( log2_max_frame_num_minus4 + 4 ) (p100)
#define MaxFrameNum(log2_max_pic_order_cnt_lsb_minus4) (1 << (log2_max_pic_order_cnt_lsb_minus4 + 4))

// MaxPicOrderCntLsb = 2( log2_max_pic_order_cnt_lsb_minus4 + 4 ) (p100)
#define MaxPicOrderCntLsb(log2_max_pic_order_cnt_lsb_minus4) (1 << (log2_max_pic_order_cnt_lsb_minus4 + 4))

// PicWidthInMbs = pic_width_in_mbs_minus1 + 1 (p101)
#define PicWidthInMbs(pic_width_in_mbs_minus1) (pic_width_in_mbs_minus1 + 1)

// PicWidthInSamplesL = PicWidthInMbs * 16  (p101)
#define PicWidthInSamplesL(pic_width_in_mbs_minus1) (PicWidthInMbs(pic_width_in_mbs_minus1) * 16)

// PicWidthInSamplesC = PicWidthInMbs * MbWidthC  (p101)
#define PicWidthInSamplesC(pic_width_in_mbs_minus1, chroma_format_idc) (PicWidthInMbs(pic_width_in_mbs_minus1) * h264_chroma_mb_width(chroma_format_idc))

// PicHeightInMapUnits = pic_height_in_map_units_minus1 + 1 (p101)
#define PicHeightInMapUnits(pic_height_in_map_units_minus1) (pic_height_in_map_units_minus1 + 1)

// PicSizeInMapUnits = PicWidthInMbs * PicHeightInMapUnits (p101)
#define PicSizeInMapUnits(pic_width_in_mbs_minus1, pic_height_in_map_units_minus1) (PicWidthInMbs(pic_width_in_mbs_minus1) * PicHeightInMapUnits(pic_height_in_map_units_minus1))

// FrameHeightInMbs = (2 - frame_mbs_only_flag) * PicHeightInMapUnits (p101)
#define FrameHeightInMbs(pic_height_in_map_units_minus1, frame_mbs_only_flag) ((2 - frame_mbs_only_flag) * PicHeightInMapUnits(pic_height_in_map_units_minus1))

// PicHeightInMbs = FrameHeightInMbs / ( 1 + field_pic_flag ) (p112)
#define PicHeightInMbs(pic_height_in_map_units_minus1, frame_mbs_only_flag, field_pic_flag) (FrameHeightInMbs(pic_height_in_map_units_minus1, frame_mbs_only_flag) / (1 + field_pic_flag))

// PicHeightInSamplesL = PicHeightInMbs * 16 (p112)
#define PicHeightInSamplesL(pic_height_in_map_units_minus1, frame_mbs_only_flag, field_pic_flag) (PicHeightInMbs(pic_height_in_map_units_minus1, frame_mbs_only_flag, field_pic_flag) * 16)

// PicHeightInSamplesC = PicHeightInMbs * MbHeightC (p112)
#define PicHeightInSamplesC(pic_height_in_map_units_minus1, frame_mbs_only_flag, field_pic_flag, chroma_format_idc) (PicHeightInMbs(pic_height_in_map_units_minus1, frame_mbs_only_flag, field_pic_flag) * h264_chroma_mb_height(chroma_format_idc))

// PicSizeInMbs = PicWidthInMbs * PicHeightInMbs (p112)
#define PicSizeInMbs(pic_width_in_mbs_minus1, pic_height_in_map_units_minus1, frame_mbs_only_flag, field_pic_flag) (PicWidthInMbs(pic_width_in_mbs_minus1) * PicHeightInMbs(pic_height_in_map_units_minus1, frame_mbs_only_flag, field_pic_flag))

#endif /* !_h264_util_h_ */
