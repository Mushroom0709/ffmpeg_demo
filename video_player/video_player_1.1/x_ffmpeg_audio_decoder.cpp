#include "x_ffmpeg_audio_decoder.h"

namespace x
{
    namespace ffmpeg
    {
		xAudioDecoder::xAudioDecoder()
		{
			src_frame_ = NULL;

			swr_ctx_ = NULL;
			swr_ch_layout_ = -1;
			swr_sample_fmt_ = AVSampleFormat::AV_SAMPLE_FMT_NONE;
			swr_sample_rate_ = -1;
			swr_nb_samples_ = -1;
		}
		xAudioDecoder::~xAudioDecoder()
		{
			Close();
		}

		bool xAudioDecoder::GetSrcParameters(int& _sample_rate, int& _nb_samples, int64_t& _ch_layout, enum AVSampleFormat& _sample_fmt)
		{
			if (av_dec_ctx_ == NULL)
				return false;

			_sample_rate = av_dec_ctx_->sample_rate;
			_nb_samples = av_dec_ctx_->frame_size;
			_ch_layout = av_dec_ctx_->channel_layout;
			_sample_fmt = av_dec_ctx_->sample_fmt;

			return true;
		}
		bool xAudioDecoder::GetDstParameters(int& _sample_rate, int& _nb_samples, int& _channels, enum AVSampleFormat& _sample_fmt)
		{
			if (swr_ctx_ == NULL)
				return false;

			_sample_rate = swr_sample_rate_;
			_nb_samples = swr_nb_samples_;
			_channels = av_get_channel_layout_nb_channels(swr_ch_layout_);//依据声道布局获取声道数
			_sample_fmt = swr_sample_fmt_;
			return true;
		}

		bool xAudioDecoder::Open(xDemultiplexer& _duxer)
		{
			if (run_flag_ == true)
				return false;

			AVStream* as = _duxer.FormatContext()->streams[_duxer.AudioStreamIndex()];

			d_timebase_ = av_q2d(as->time_base);	// 获取音频流的 [时间基数] 用于 ffempg各种时间统一转换
			duration_ = d_timebase_ * (as->duration * 1.0);	//时间基数*码流时长(ffmepg音频流时间单位) = 码流时长(秒)

			//下面是常规操作，请点击function看注释

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

			rate_ = av_dec_ctx_->sample_rate;

			src_frame_ = av_frame_alloc();

			return true;
		}
		bool xAudioDecoder::SetSwr(int64_t _ch_layout, enum AVSampleFormat _sample_fmt, int _sample_rate)
		{
			if (run_flag_ == true)
				return false;

			swr_ch_layout_ = _ch_layout;
			swr_sample_fmt_ = _sample_fmt;
			swr_sample_rate_ = _sample_rate;

			// 设置SwrContext相关参数
			swr_ctx_ = swr_alloc_set_opts(
				NULL,
				swr_ch_layout_,					// 目标声道布局
				swr_sample_fmt_,				// 目标采样格式
				swr_sample_rate_,				// 目标采样率
				av_dec_ctx_->channel_layout,	// 原始声道布局
				av_dec_ctx_->sample_fmt,		// 原始采样格式
				av_dec_ctx_->sample_rate,		// 原始采样率
				0,								// 日志开关
				NULL);							// 日志上下文

			if (!swr_ctx_ || swr_init(swr_ctx_) < 0)
				return false;

			// 这是ffmepg官方示例提供的计算重采样之后的 单帧单声道采样数 的方法
			// 注意，frame_size 是原始的 单帧单声道采样数 不是原始帧的byte个数
			// 例如AV_SAMPLE_FMT_S16，单帧如有1024个采样数，则单帧byte个数是1024*声道数*sizeof(short)
			// 单帧单声道采样数和采样率相关
			swr_nb_samples_ = (int)av_rescale_rnd(
				swr_get_delay(swr_ctx_,av_dec_ctx_->sample_rate) + av_dec_ctx_->frame_size,
				swr_sample_rate_,
				av_dec_ctx_->sample_rate,
				AV_ROUND_INF);

			return true;
		}
		bool xAudioDecoder::Start(xDecoderEvent* _event)
		{
			if (run_flag_ == true)
				return false;

			event_ = _event;
			run_flag_ = true;

			decode_thread_ = std::thread(&xAudioDecoder::decode_function, this);

			return true;
		}
		void xAudioDecoder::Close()
		{
			run_flag_ = false;

			if (decode_thread_.joinable())
			{
				decode_thread_.join();
			}


			if (src_frame_ != NULL)
			{
				av_frame_free(&src_frame_);
				src_frame_ = NULL;
			}

			if (av_dec_ctx_ != NULL)
			{
				avcodec_close(av_dec_ctx_);
				avcodec_free_context(&av_dec_ctx_);
				av_dec_ctx_ = NULL;
				av_codec_ = NULL;
			}

			if (swr_ctx_ != NULL)
			{
				swr_close(swr_ctx_);
				swr_free(&swr_ctx_);
				swr_ctx_ = NULL;
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
		bool xAudioDecoder::SendPacket(AVPacket* _pkt)
		{
			AVPacket* tpkt = av_packet_clone(_pkt); //复制AVPacket，实际上是对AVPacket内部的引用计数+1
			qpkt_.MaxSziePush(tpkt,&run_flag_);

			return true;
		}

		void xAudioDecoder::decode_function()
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
						if (avcodec_send_packet(av_dec_ctx_, pkt) < 0)
						{
							run_flag_ = false;
						}
						else
						{
							//解码器解码得到原始PCM帧
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
									uint8_t* dst_buf = NULL;
									int dst_buf_size = 0;

									// 为重采样目标PCM帧分配存储空间
									if (av_samples_alloc(
										&dst_buf,		// 目标buffer
										&dst_buf_size,	// 目标buffer大小
										av_get_channel_layout_nb_channels(swr_ch_layout_), //目标通道数
										swr_nb_samples_,	//目标采样数
										swr_sample_fmt_,	//目标采样格式
										1) < 0)
									{
										run_flag_ = false;
									}
									else
									{
										//从采样并填充buffer
										ret = swr_convert(
											swr_ctx_,			//SwrContext
											&dst_buf,			//目标PCM buffer
											swr_nb_samples_,	//目标采样数
											(const uint8_t**)src_frame_->data, //原始PCM buffer
											src_frame_->nb_samples); //原始采样数

										if (event_ != NULL)
											event_->AudioEvent(dst_buf, dst_buf_size, src_frame_->best_effort_timestamp, this);

										av_free(dst_buf);
									}
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