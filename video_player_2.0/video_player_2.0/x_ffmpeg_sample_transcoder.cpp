#include "x_ffmpeg_sample_transcoder.h"

namespace x_ffmpeg
{
    SampleTranscoder::SampleTranscoder()
    {
        src_fmt_ = AV_SAMPLE_FMT_NONE;
        src_ch_layout_ = -1;
        src_sample_rate_ = -1;
        src_frame_size_ = -1;

        dst_fmt_ = AV_SAMPLE_FMT_NONE;
        dst_ch_layout_ = -1;
        dst_sample_rate_ = -1;
        dst_nb_samples_ = -1;
    }
    SampleTranscoder::~SampleTranscoder()
    {
        Destroy();
    }

    bool  SampleTranscoder::Initialize(AVSampleFormat _src_fmt, int64_t _src_ch, int _src_rate, int _src_frame_size,
        AVSampleFormat _dst_fmt, int64_t _dst_ch, int _dst_rate)
    {
        src_fmt_ = _src_fmt;
        src_ch_layout_ = _src_ch;
        src_sample_rate_ = _src_rate;
        src_frame_size_ = _src_frame_size;

        dst_fmt_ = _dst_fmt;
        dst_ch_layout_ = _dst_ch;
        dst_sample_rate_ = _dst_rate;

        swr_ctx_ = swr_alloc_set_opts(
            NULL,
            dst_ch_layout_, dst_fmt_, dst_sample_rate_,
            src_ch_layout_, src_fmt_, src_sample_rate_,
            0,
            NULL);

        if (!swr_ctx_ || swr_init(swr_ctx_) < 0)
            return false;

        dst_nb_samples_ = (int)av_rescale_rnd(
            swr_get_delay(swr_ctx_, src_sample_rate_) + _src_frame_size,
            src_sample_rate_,
            src_sample_rate_,
            AV_ROUND_INF);

        return true;
    }
    bool  SampleTranscoder::Convert(AVFrame* _src_frame, uint8_t*& _dst_buf, int& _dst_buf_size, int64_t& _timestamp)
    {
        _dst_buf = NULL;
        _dst_buf_size = 0;
        if (av_samples_alloc(
            &_dst_buf,		// 目标buffer
            &_dst_buf_size,	// 目标buffer大小
            av_get_channel_layout_nb_channels(dst_ch_layout_), //目标通道数
            dst_nb_samples_,	//目标采样数
            dst_fmt_,	//目标采样格式
            1) < 0)
        {
            return false;
        }

        //从采样并填充buffer
        int ret = swr_convert(
            swr_ctx_,			//SwrContext
            &_dst_buf,			//目标PCM buffer
            dst_nb_samples_,	//目标采样数
            (const uint8_t**)_src_frame->data, //原始PCM buffer
            _src_frame->nb_samples); //原始采样数
        _timestamp = _src_frame->best_effort_timestamp;
        return true;
    }
    bool  SampleTranscoder::FreeBuffer(uint8_t*& _dst_buf, int& _dst_buf_size)
    {
        av_free(_dst_buf);
        _dst_buf = NULL;
        _dst_buf_size = 0;

        return true;
    }
    void  SampleTranscoder::Destroy()
    {
        if (swr_ctx_ != NULL)
        {
            swr_close(swr_ctx_);
            swr_free(&swr_ctx_);
            swr_ctx_ = NULL;

            src_fmt_ = AV_SAMPLE_FMT_NONE;
            src_ch_layout_ = -1;
            src_sample_rate_ = -1;
            src_frame_size_ = -1;

            dst_fmt_ = AV_SAMPLE_FMT_NONE;
            dst_ch_layout_ = -1;
            dst_sample_rate_ = -1;
            dst_nb_samples_ = -1;
        }
    }
}