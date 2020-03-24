#include "x_ffmpeg_video_decoder.h"

namespace x
{
    namespace ffmpeg
    {
		xVideoDecoder::xVideoDecoder() :xDecoder()
		{
			src_frame_ = NULL;
			dst_frame_buf_ = NULL;
			dst_frame_ = NULL;

			sws_ctx_ = NULL;
			sws_fmt_ = AVPixelFormat::AV_PIX_FMT_NONE;
			sws_w_ = -1;
			sws_h_ = -1;
		}
		xVideoDecoder::~xVideoDecoder()
		{
			Close();
		}

		bool xVideoDecoder::GetSrcParameters(int& _w, int& _h, AVPixelFormat& _fmt)
		{
			if (av_dec_ctx_ == NULL)
				return false;

			_w = av_dec_ctx_->width;
			_h = av_dec_ctx_->height;
			_fmt = av_dec_ctx_->pix_fmt;

			return true;
		}
		bool xVideoDecoder::GetDstParameters(int& _w, int& _h, AVPixelFormat& _fmt)
		{
			if (sws_ctx_ == NULL)
				return false;

			_w = sws_w_;
			_h = sws_h_;
			_fmt = sws_fmt_;

			return true;
		}

		bool xVideoDecoder::Open(xDemultiplexer& _duxer)
		{
			if (run_flag_ == true)
				return false;

			AVStream* as = _duxer.FormatContext()->streams[_duxer.VideoStreamIndex()];

			d_timebase_ = av_q2d(as->time_base);
			duration_ = d_timebase_ * (as->duration * 1.0);

			av_dec_ctx_ = avcodec_alloc_context3(NULL);
			if (NULL == av_dec_ctx_)
				return false;

			avcodec_parameters_to_context(av_dec_ctx_, as->codecpar);

			av_codec_ = avcodec_find_decoder(av_dec_ctx_->codec_id);
			if (NULL == av_codec_)
				return false;

			if (avcodec_open2(av_dec_ctx_, av_codec_, NULL) < 0)
			{
				//printf("[LOG] [ERROR] [xReader] [Init] [video] [avcodec_open2]\n");
				return false;
			}

			src_frame_ = av_frame_alloc();
			dst_frame_ = av_frame_alloc();

			rate_ = av_q2d(as->avg_frame_rate);

			return true;
		}
		bool xVideoDecoder::SetSws(int _dst_w, int _dst_h, AVPixelFormat _dst_fmt, int _scale_flag)
		{
			if (run_flag_ == true)
				return false;

			sws_fmt_ = _dst_fmt != AV_PIX_FMT_NONE ? _dst_fmt : av_dec_ctx_->pix_fmt;
			sws_w_ = _dst_w != -1 ? _dst_w : av_dec_ctx_->width;
			sws_h_ = _dst_h != -1 ? _dst_h : av_dec_ctx_->height;

			sws_ctx_ = sws_getContext(
				av_dec_ctx_->width,
				av_dec_ctx_->height,
				av_dec_ctx_->pix_fmt,
				sws_w_,
				sws_h_,
				sws_fmt_,
				_scale_flag,
				NULL,
				NULL,
				NULL);

			if (sws_ctx_ == NULL)
				return false;
			dst_frame_buf_ = (uint8_t*)av_calloc(av_image_get_buffer_size(sws_fmt_, sws_w_, sws_h_, 1), sizeof(uint8_t));
			av_image_fill_arrays(dst_frame_->data, dst_frame_->linesize, dst_frame_buf_, sws_fmt_, sws_w_, sws_h_, 1);

			return true;
		}
		bool xVideoDecoder::Start(xDecoderEvent* _event)
		{
			if (run_flag_ == true)
				return false;

			event_ = _event;
			run_flag_ = true;

			decode_thread_ = std::thread(&xVideoDecoder::decode_function, this);

			return true;
		}
		void xVideoDecoder::Close()
		{
			run_flag_ = false;

			if (decode_thread_.joinable())
			{
				decode_thread_.join();
			}

			if (dst_frame_buf_ != NULL)
			{
				av_free(dst_frame_buf_);
				dst_frame_buf_ = NULL;
			}

			if (src_frame_ != NULL)
			{
				av_frame_free(&src_frame_);
				src_frame_ = NULL;
			}

			if (dst_frame_ != NULL)
			{
				av_frame_free(&dst_frame_);
				dst_frame_ = NULL;
			}

			if (av_dec_ctx_ != NULL)
			{
				avcodec_close(av_dec_ctx_);
				avcodec_free_context(&av_dec_ctx_);
				av_dec_ctx_ = NULL;
				av_codec_ = NULL;
			}

			if (sws_ctx_ != NULL)
			{
				sws_freeContext(sws_ctx_);
				sws_ctx_ = NULL;
			}

			AVPacket* pkt = NULL;

			while (qpkt_.TryPop(pkt))
			{
				if (pkt != NULL)
				{
					av_packet_free(&pkt);
				}
			}
		}
		bool xVideoDecoder::SendPacket(AVPacket* _pkt)
		{

			AVPacket* tpkt = av_packet_clone(_pkt);
			qpkt_.MaxSziePush(tpkt,&run_flag_);

			return true;
		}


		void xVideoDecoder::decode_function()
		{
			int ret = 0;
			AVPacket* pkt = NULL;
			while (run_flag_)
			{
				if (qpkt_.TryPop(pkt))
				{
					if (pkt == NULL)
					{
						run_flag_ = false;
					}
					else
					{
						//printf("%lld\n", pkt->pts);
						if (avcodec_send_packet(av_dec_ctx_, pkt) < 0)
						{
							run_flag_ = false;
						}
						else
						{
							while (ret = avcodec_receive_frame(av_dec_ctx_, src_frame_), run_flag_)
							{
								if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
								{
									break;
								}

								else if (ret < 0)
								{
									run_flag_ = false;
								}
								else if (ret == 0)
								{
									sws_scale(
										sws_ctx_,
										(const uint8_t* const*)src_frame_->data,
										src_frame_->linesize,
										0,
										av_dec_ctx_->height,
										dst_frame_->data,
										dst_frame_->linesize);

									dst_frame_->pts = src_frame_->pts;
									dst_frame_->best_effort_timestamp = src_frame_->best_effort_timestamp;

									if (event_ != NULL)
										event_->VideoEvent(dst_frame_, this);
								}
							}
						}

						av_packet_free(&pkt);
					}
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(20));
				}
			}
		}
    }
}