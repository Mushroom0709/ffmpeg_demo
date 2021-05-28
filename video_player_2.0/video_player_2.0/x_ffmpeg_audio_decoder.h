#ifndef _X_FFMPEG_AUDIO_DECODER_H_
#define _X_FFMPEG_AUDIO_DECODER_H_

#include "x_ffmpeg_common.h"
#include "x_ffmpeg_demultiplexer.h"

namespace x_ffmpeg
{
    class AudioDecoder
    {
    public:
        AudioDecoder();
        ~AudioDecoder();
    private:
        int stream_index_;
    private:
        AVCodecContext* codec_ctx_;
        AVCodec* codec_;
    public:
        double d_timebase_;
        double duration_;
        double rate_;	//“Ù∆µ ≤…—˘¬ 

        AVSampleFormat format_;
        int64_t ch_layout_;
        int frame_size_;
    public:
        bool Initialize(AVFormatContext* _format_ctx, int _stream_index);
        bool GetFrame(AVPacket* _pkt, AVFrame* _frame, int& _status);
        void Destroy();
    };
}

#endif //           