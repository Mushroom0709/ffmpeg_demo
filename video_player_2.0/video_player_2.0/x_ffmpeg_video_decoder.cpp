#include "x_ffmpeg_video_decoder.h"

namespace x_ffmpeg
{
	VideoDecoder::VideoDecoder()
	{
		stream_index_ = -1;
		codec_ctx_ = NULL;
		codec_ = NULL;

		d_timebase_ = -0.1;
		duration_ = -0.1;
		rate_ = -0.1;

		width_ = -1;
		height_ = -1;
		format_ = AV_PIX_FMT_NONE;
	}
	VideoDecoder::~VideoDecoder()
	{
		Destroy();
	}

	bool VideoDecoder::Initialize(AVFormatContext* _format_ctx, int _stream_index)
	{
		stream_index_ = _stream_index;
		AVStream* stream = _format_ctx->streams[stream_index_];

		d_timebase_ = av_q2d(stream->time_base);
		duration_ = d_timebase_ * (stream->duration * 1.0);
		rate_ = av_q2d(stream->avg_frame_rate);

		codec_ctx_ = avcodec_alloc_context3(NULL);
		if (codec_ctx_ == NULL)
			return false;

		avcodec_parameters_to_context(codec_ctx_, stream->codecpar);

		height_ = stream->codecpar->height;
		width_ = stream->codecpar->width;
		format_ = (AVPixelFormat)stream->codecpar->format;

		codec_ = avcodec_find_decoder(codec_ctx_->codec_id);
		if (codec_ == NULL)
			return false;

		if (avcodec_open2(codec_ctx_, codec_, NULL) < 0)
			return false;

		return true;
	}
	bool VideoDecoder::GetFrame(AVPacket* _pkt, AVFrame* _frame, int& _status)
	{
		/*
		* _status
		* 0 填充packet
		* 1 正常读取一个frame,继续读取
		*/
		if (_status == X_FFMPEG_STATUS_SEND_PKT)
		{
			if (avcodec_send_packet(codec_ctx_, _pkt) < 0)
			{
				return false;
			}
		}

		int ret = avcodec_receive_frame(codec_ctx_, _frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			_status = X_FFMPEG_STATUS_SEND_PKT;
		}
		else if (ret < 0)
		{
			_status = ret;
			return false;
		}
		else if (ret == 0)
		{
			_status = X_FFMPEG_STATUS_READ_FRAME;
		}

		return true;
	}
	void VideoDecoder::Destroy()
	{
		if (codec_ctx_ != NULL)
		{
			avcodec_close(codec_ctx_);
			avcodec_free_context(&codec_ctx_);
			codec_ctx_ = NULL;
			codec_ = NULL;
		}

		d_timebase_ = -0.1;
		duration_ = -0.1;
		rate_ = -0.1;

		width_ = -1;
		height_ = -1;
		format_ = AV_PIX_FMT_NONE;
	}
}