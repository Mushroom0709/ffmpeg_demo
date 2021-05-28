#ifndef _XFFMPEG_VIDEO_DECODER_H_
#define _XFFMPEG_VIDEO_DECODER_H_

#include "x_ffmpeg_common.h"
#include "x_ffmpeg_demultiplexer.h"

namespace x_ffmpeg
{
	class VideoDecoder
	{
	public:
		VideoDecoder();
		~VideoDecoder();
	private:
		int stream_index_;
	private:
		AVCodecContext* codec_ctx_;
		AVCodec* codec_;
	public:
		double d_timebase_;
		double duration_;
		double rate_;	//ÊÓÆµ Ö¡ÂÊ

		int width_;
		int height_;
		AVPixelFormat format_;
	public:
		bool Initialize(AVFormatContext* _format_ctx, int _stream_index);
		bool GetFrame(AVPacket* _pkt, AVFrame* _frame,int& _status);
		void Destroy();
	};
}

#endif // _XFFMPEG_VIDEO_DECODER_H_