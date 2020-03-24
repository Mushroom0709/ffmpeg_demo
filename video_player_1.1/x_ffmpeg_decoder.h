#ifndef _X_FFMPEG_DECODER_H_
#define _X_FFMPEG_DECODER_H_

#include "x_ffmepg_demultiplexer.h"

#include "x_safe_queue.h"

namespace x
{
    namespace ffmpeg
    {
		class xVideoDecoder;
		class xAudioDecoder;
		class xDecoderEvent
		{
		public:
			virtual void VideoEvent(AVFrame* _img, xVideoDecoder* _decoder) = 0;
			virtual void AudioEvent(uint8_t* _buf, int _len, int64_t _pts, x::ffmpeg::xAudioDecoder* _decoder) = 0;
		};

		class xDecoder
		{
		protected:
			AVCodecContext* av_dec_ctx_;
			AVCodec* av_codec_;

			double d_timebase_;
			double duration_;
			double rate_;	//音频 采样率
							//视频 帧率
		protected:
			xDecoderEvent* event_;
			volatile bool run_flag_;
			std::thread decode_thread_;
			x::xQueue<AVPacket*> qpkt_;
		public:
			xDecoder();
		public:
			double GetTimebase();
			double GetDuration();
			double GetRate();
		public:
			virtual void decode_function() = 0;
			virtual bool Open(xDemultiplexer& _duxer) = 0;
			virtual bool SendPacket(AVPacket* _pkt) = 0;
			virtual void Close() = 0;
		};
    }
}

#endif //_X_FFMPEG_DECODER_H_