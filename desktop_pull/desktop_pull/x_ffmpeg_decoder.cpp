#include "x_ffmpeg_decoder.h"

namespace xM
{
	namespace ffmpeg
	{
		VDecoder::VDecoder()
		{
			cdc_ctx_ = NULL;
			sws_ctx_ = NULL;
			src_frame_ = NULL;
			sws_frame_ = NULL;
			cdc_parser_ctx_ = NULL;
			packet_ = NULL;
			event_ = NULL;
			src_width_ = 0;
			src_height_ = 0;
		}
		VDecoder::~VDecoder()
		{
			Close();
		}

		bool VDecoder::init_decode(AVCodecID _cdc_id)
		{
			AVCodec* cdc_h264 = avcodec_find_decoder(_cdc_id);
			if (cdc_h264 == NULL)
				return false;

			cdc_parser_ctx_ = av_parser_init(_cdc_id);
			if (cdc_parser_ctx_ == NULL)
				return false;

			cdc_ctx_ = avcodec_alloc_context3(cdc_h264);
			if (cdc_ctx_ == NULL)
				return false;

			if (0 != avcodec_open2(cdc_ctx_, cdc_h264, NULL))
				return false;

			src_frame_ = av_frame_alloc();
			packet_ = av_packet_alloc();
			return true;
		}
		bool VDecoder::init_sws(int _srcW, int _srcH, AVPixelFormat _srcFormat,
			int _dstW, int _dstH, AVPixelFormat _dstFormat)
		{
			if (sws_ctx_ != NULL)
				return false;

			sws_ctx_ = sws_getContext(
				_srcW,
				_srcH,
				_srcFormat,
				_dstW,
				_dstH,
				_dstFormat,
				SWS_FAST_BILINEAR,
				NULL,
				NULL,
				NULL);

			if (sws_ctx_ == NULL)
				return false;

			sws_frame_ = av_frame_alloc();
			sws_frame_->format = _dstFormat;
			sws_frame_->width = _dstW;
			sws_frame_->height = _dstH;
			av_frame_get_buffer(sws_frame_, 0);

			return true;
		}

		bool VDecoder::Open(IVDecoderEvent* _event,int _src_w,int _src_h)
		{
			if (_event == NULL)
				return false;

			src_width_ = _src_w;
			src_height_ = _src_h;
			event_ = _event;
			if (false == init_decode(AV_CODEC_ID_H264))
				return false;

			if (false == init_sws(_src_w, _src_h, AV_PIX_FMT_YUV420P,
				X_FFMPEG_VDECODER_DST_WIDTH, X_FFMPEG_VDECODER_DST_HEIGHT, AV_PIX_FMT_YUV420P))
				return false;

			return true;
		}
		bool VDecoder::UpdataPacket(AVPacket* _pkt)
		{
			if (avcodec_send_packet(cdc_ctx_, _pkt) < 0)
			{
				return false;
			}
			else
			{
				int ret = 0;
				while (ret = avcodec_receive_frame(cdc_ctx_, src_frame_), true)
				{
					if (ret < 0)
					{
						if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
							break;
						else
							return false;
					}
					else if (ret == 0)
					{
						sws_scale(
							sws_ctx_,
							(const uint8_t* const*)src_frame_->data,
							src_frame_->linesize,
							0,
							src_frame_->height,
							sws_frame_->data,
							sws_frame_->linesize);

						event_->Frame(sws_frame_);
						//PrintInfo("W:%d\tH:%d pts:%lld", sws_frame_->width, sws_frame_->height,sws_frame_->best_effort_timestamp);
					}
				}
			}

			av_packet_unref(_pkt);
			return true;
		}
		bool VDecoder::UpdataBuffer(const uint8_t* _buf, const int _len)
		{
			int t_len = 0;
			int use_len = 0;
			while (use_len < _len)
			{
				t_len = av_parser_parse2(
					cdc_parser_ctx_, cdc_ctx_,
					&(packet_->data), &(packet_->size),
					_buf + use_len, _len - use_len,
					AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

				use_len += t_len;

				if (packet_->size != 0)
				{
					UpdataPacket(packet_);
					av_packet_unref(packet_);
				}
			}

			return true;
		}
		void VDecoder::Close()
		{
			if (cdc_ctx_ != NULL)
			{
				avcodec_close(cdc_ctx_);
				avcodec_free_context(&cdc_ctx_);
				cdc_ctx_ = NULL;
			}

			if (sws_ctx_ != NULL)
			{
				sws_freeContext(sws_ctx_);
				sws_ctx_ = NULL;
			}

			if (cdc_parser_ctx_ != NULL)
			{
				av_parser_close(cdc_parser_ctx_);
				cdc_parser_ctx_ = NULL;
			}

			if (src_frame_ != NULL)
			{
				av_frame_free(&src_frame_);
				src_frame_ = NULL;
			}

			if (sws_frame_ != NULL)
			{
				av_frame_free(&sws_frame_);
				sws_frame_ = NULL;
			}
		}
	}
}