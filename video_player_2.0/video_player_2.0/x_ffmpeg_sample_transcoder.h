#ifndef _X_FFMPEG_SAMPLE_TRANSCODER_H_
#define _X_FFMPEG_SAMPLE_TRANSCODER_H_

#include "x_ffmpeg_common.h"

namespace x_ffmpeg
{
    class SampleTranscoder
    {
    private:
        SwrContext* swr_ctx_;

        AVSampleFormat src_fmt_;
        int64_t src_ch_layout_;
        int src_sample_rate_;
        int src_frame_size_;
    public:
        AVSampleFormat dst_fmt_;
        int64_t dst_ch_layout_;
        int dst_sample_rate_;
        int dst_nb_samples_;
    public:
        SampleTranscoder();
        ~SampleTranscoder();
    public:
        bool Initialize(AVSampleFormat _src_fmt, int64_t _src_ch, int _src_rate, int _src_frame_size,
            AVSampleFormat _dst_fmt, int64_t _dst_ch, int _dst_rate);
        bool Convert(AVFrame* _src_frame, uint8_t*& _dst_buf, int& _dst_buf_size, int64_t& _timestamp);
        bool FreeBuffer(uint8_t*& _dst_buf, int& _dst_buf_size);
        void Destroy();
    };
}

#endif //_X_FFMPEG_SAMPLE_TRANSCODER_H_