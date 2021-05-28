#include "x_ffmpeg_pixel_transcoder.h"

namespace x_ffmpeg
{
	bool PixelTranscoder::Initialize(AVPixelFormat _src_fmt, int _src_h, int _src_w,
		AVPixelFormat _dst_fmt, int _dst_h, int _dst_w)
	{

		src_fmt_ = _src_fmt;
		src_height_ = _src_h;
		src_wdith_ = _src_w;

		dst_fmt_ = _dst_fmt;
		dst_height_ = _dst_h;
		dst_wdith_ = _dst_w;


		sws_ctx_ = sws_getContext(
			src_wdith_, src_height_, src_fmt_,
			dst_wdith_, dst_height_, dst_fmt_,
			SWS_FAST_BILINEAR,
			NULL,
			NULL,
			NULL);

		if (sws_ctx_ == NULL)
			return false;

		return true;
	}
	void PixelTranscoder::FillFrame(AVFrame*& _dst_frame,uint8_t*& _dst_buffer)
	{
		_dst_frame = av_frame_alloc();
		_dst_buffer = (uint8_t*)av_calloc(av_image_get_buffer_size(dst_fmt_, dst_wdith_, dst_height_, 1), sizeof(uint8_t));;
		av_image_fill_arrays(_dst_frame->data, _dst_frame->linesize, _dst_buffer, dst_fmt_, dst_wdith_, dst_height_, 1);
	}
	void PixelTranscoder::FreeFrame(AVFrame*& _dst_frame, uint8_t*& _dst_buffer)
	{
		av_free(_dst_buffer);
		av_frame_free(&_dst_frame);
	}
	bool PixelTranscoder::Scale(AVFrame* _src_frame, AVFrame* _dst_frame)
	{
		sws_scale(
			sws_ctx_,
			(const uint8_t* const*)_src_frame->data,
			_src_frame->linesize,
			0,
			src_height_,
			_dst_frame->data,
			_dst_frame->linesize);

		_dst_frame->format = dst_fmt_;
		_dst_frame->width = dst_wdith_;
		_dst_frame->height = dst_height_;
		_dst_frame->pts = _src_frame->pts;
		_dst_frame->best_effort_timestamp = _src_frame->best_effort_timestamp;

		return true;
	}
	void PixelTranscoder::Destroy()
	{
		if (sws_ctx_ != NULL)
		{
			sws_freeContext(sws_ctx_);

			sws_ctx_ = NULL;

			src_fmt_ = AV_PIX_FMT_NONE;
			src_height_ = -1;
			src_wdith_ = -1;

			dst_fmt_ = AV_PIX_FMT_NONE;
			int dst_height_ = -1;
			int dst_wdith_ = -1;
		}
	}
	PixelTranscoder::PixelTranscoder()
	{
		sws_ctx_ = NULL;

		src_fmt_ = AV_PIX_FMT_NONE;
		src_height_ = -1;
		src_wdith_ = -1;

		dst_fmt_ = AV_PIX_FMT_NONE;
		int dst_height_ = -1;
		int dst_wdith_ = -1;
	}
	PixelTranscoder::~PixelTranscoder()
	{
		Destroy();
	}
}
