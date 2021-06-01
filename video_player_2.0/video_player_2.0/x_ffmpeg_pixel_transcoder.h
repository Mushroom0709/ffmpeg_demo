#ifndef _X_FFMPEG_PIXEL_TRANSCODER_H_
#define _X_FFMPEG_PIXEL_TRANSCODER_H_

#include "x_ffmpeg_common.h"
#include "x_ffmpeg_video_decoder.h"

namespace x_ffmpeg
{
	class PixelTranscoder
	{
	public:
		PixelTranscoder();
		~PixelTranscoder();

	private:
		SwsContext* sws_ctx_;

		AVPixelFormat src_fmt_;
		int src_height_;
		int src_wdith_;
	public:
		AVPixelFormat dst_fmt_;
		int dst_height_;
		int dst_wdith_;

	public:
		bool Initialize(AVPixelFormat _src_fmt, int _src_h, int _src_w,
			AVPixelFormat _dst_fmt, int _dst_h, int _dst_w);
		void FillFrame(AVFrame*& _dst_frame, uint8_t*& _dst_buffer);
		void FreeFrame(AVFrame*& _dst_frame, uint8_t*& _dst_buffer);
		bool Scale(AVFrame* _src_frame, AVFrame* _dst_frame);
		void Destroy();
	};
}

#endif //_X_FFMPEG_IMAGE_TRANSCODER_H_