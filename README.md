# avcodec
1. H.264 bit stream parse
2. H.264 encode by X264
3. H.264 encode by OpenH264
4. FFMPEG video encode/decode


<pre><code>
h264_parameter_t param;
param.profile = H264_PROFILE_BASELINE;
param.level = H264_LEVEL_2_0;
param.width = 352;
param.height = 288;
param.format = PICTURE_YUV420;
param.frame_rate = 25000;
param.gop_size = 50;
param.bitrate = 500000;

void* h264 = x264enc_create(& param);
//void* h264 = openh264enc_create(& param);

picture_t pic;
memset(&pic, 0, sizeof(pic));
pic.format = PICTURE_YUV420;
pic.width = 352;
pic.height = 288;
pic.pts = i * 40;
pic.dts = i * 40;
pic.flags = 0; // AVPACKET_FLAG_KEY force IDR frame
pic.data[0] = s_yuv;
pic.linesize[0] = pic.width;
pic.data[1] = pic.data[0] + pic.linesize[0] * pic.height;
pic.linesize[1] = pic.width / 2;
pic.data[2] = pic.data[1] + pic.linesize[1] * pic.height / 2;
pic.linesize[2] = pic.width / 2;
assert(x264enc_input(h264, &pic) > 0);
//assert(openh264enc_input(h264, &pic) > 0);

avpacket_t pkt;
memset(&pkt, 0, sizeof(pkt));
assert(x264enc_getpacket(h264, &pkt) > 0);
//assert(1 == openh264enc_getpacket(h264, &pkt));

// save data
assert(pkt.bytes == fwrite(pkt.data, 1, pkt.bytes, wfp));
</code></pre>
