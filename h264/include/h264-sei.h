#ifndef _h264_sei_h_
#define _h264_sei_h_

enum
{
	H264_SEI_BUFFERING_PERIOD = 0,
	H264_SEI_PIC_TIMING,
	H264_SEI_PAN_SCAN_RECT,
	H264_SEI_FILLER_PAYLOAD,
	H264_SEI_USER_DATA_REGISTERED_ITU_T_T35,
	H264_SEI_USER_DATA_UNREGISTERED,
	H264_SEI_RECOVERY_POINT,
	H264_SEI_DEC_REF_PIC_MARKING_REPETITION,
	H264_SEI_SPARE_PIC,
	H264_SEI_SCENE_INFO,
	H264_SEI_SUB_SEQ_INFO,
	H264_SEI_SUB_SEQ_LAYER_CHARACTERISTICS,
	H264_SEI_SUB_SEQ_CHARACTERISTICS,
	H264_SEI_FULL_FRAME_FREEZE,
	H264_SEI_FULL_FRAME_FREEZE_RELEASE,
	H264_SEI_FULL_FRAME_SNAPSHOT,
	H264_SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START,
	H264_SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END,
	H264_SEI_MOTION_CONSTRAINED_SLICE_GROUP_SET,
	H264_SEI_FILM_GRAIN_CHARACTERISTICS,
	H264_SEI_DEBLOCKING_FILTER_DISPLAY_PREFERENCE,
	H264_SEI_STEREO_VIDEO_INFO,
};

struct h264_sei_buffering_period_t
{
	int initial_cpb_removal_delay[32];  ///< Initial timestamps for CPBs
	int initial_cpb_removal_delay_offset[32];
};

struct h264_sei_pic_timing_code_t
{
	unsigned int ct_type : 2;
	unsigned int nuit_field_based_flag : 1;
	unsigned int counting_type : 5;
	unsigned int full_timestamp_flag : 1;
	unsigned int discontinuity_flag : 1;
	unsigned int cnt_dropped_flag : 1;
	unsigned int time_offset : 5;

	uint8_t n_frames;
	uint8_t seconds_value;
	uint8_t minutes_value;
	uint8_t hours_value;
};

struct h264_sei_pic_timing_t
{
	int cpb_removal_delay;
	int dpb_output_delay;

	int timecode_count;
	struct h264_sei_pic_timing_code_t timecode[3];
};

struct h264_sei_recovery_point_t
{
	int recovery_frame_cnt;
	uint8_t exact_match_flag;
	uint8_t broken_link_flag;
	uint8_t changing_slice_group_idc;
};

struct h264_sei_t
{
	struct h264_sei_buffering_period_t buffering_period;
	struct h264_sei_pic_timing_t pic_timing;
	struct h264_sei_recovery_point_t recovery_point;
};

#endif /* !_h264_sei_h_ */
