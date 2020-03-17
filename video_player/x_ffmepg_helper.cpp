#include "x_ffmepg_helper.h"

namespace x
{
	namespace xFFmpeg
	{

		xFormatInfo::xFormatInfo()
		{
			fmt_ctx = NULL;
			file_path = "";
			av_pkt = NULL;
		}
		xFormatInfo::~xFormatInfo()
		{
			Destroy();
		}

		void xFormatInfo::Destroy()
		{
			if (av_pkt != NULL)
			{
				av_packet_unref(av_pkt);
				av_packet_free(&av_pkt);
				av_pkt = NULL;
			}

			if (fmt_ctx != NULL)
			{
				avformat_close_input(&fmt_ctx);
				avformat_free_context(fmt_ctx);
				fmt_ctx = NULL;
			}
		}
		bool xFormatInfo::Make(const char* _file_path)
		{
			file_path = _file_path;
			fmt_ctx = avformat_alloc_context();
			if (NULL == fmt_ctx)
			{
				//printf("[LOG] [ERROR] [xReader] [Init] [avformat_alloc_context]\n");
				return false;
			}

			if (0 != avformat_open_input(&fmt_ctx, _file_path, NULL, NULL))
			{
				//printf("[LOG] [ERROR] [xReader] [Init] [avformat_open_input]\n");
				return false;
			}

			if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
			{
				//printf("[LOG] [ERROR] [xReader] [Init] [avformat_find_stream_info]\n");
				return false;
			}

			av_pkt = av_packet_alloc();

			return true;
		}
		bool xFormatInfo::Show()
		{
			if (fmt_ctx == NULL)
				return false;
			av_dump_format(fmt_ctx, 0, file_path.c_str(), 0);
			return true;
		}



		xVideoInfo::xVideoInfo()
		{
			dec_ctx = NULL;
			dec = NULL;
			sws_ctx = NULL;

			src_frame = NULL;
			dst_frame = NULL;
			dst_buf = NULL;

			sws_w = 0;
			sws_h = 0;
			sws_fmt = AV_PIX_FMT_NONE;
		}
		xVideoInfo::~xVideoInfo()
		{
			Destroy();
		}

		void xVideoInfo::Destroy()
		{
			if (dst_buf != NULL)
			{
				av_free(dst_buf);
				dst_buf = NULL;
			}

			if (src_frame != NULL)
			{
				av_frame_free(&src_frame);
				src_frame = NULL;
			}

			if (dst_frame != NULL)
			{
				av_frame_free(&dst_frame);
				dst_frame = NULL;
			}

			stream_indexs.clear();

			if (dec_ctx != NULL)
			{
				avcodec_close(dec_ctx);
				avcodec_free_context(&dec_ctx);
				dec_ctx = NULL;
				dec = NULL;
			}

			if (sws_ctx != NULL)
			{
				sws_freeContext(sws_ctx);
				sws_ctx = NULL;
			}
		}
		int xVideoInfo::GetStreamIndex(int index)
		{
			return stream_indexs[index];
		}
		bool xVideoInfo::Make(AVFormatContext* fmt_ctx)
		{
			for (size_t i = 0; i < fmt_ctx->nb_streams; i++)
			{
				if (AVMEDIA_TYPE_VIDEO == fmt_ctx->streams[i]->codecpar->codec_type)
				{
					stream_indexs.push_back(i);
				}
			}


			if (stream_indexs.size() <= 0)
				return false;

			dec_ctx = avcodec_alloc_context3(NULL);
			avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[GetStreamIndex()]->codecpar);
			dec = avcodec_find_decoder(dec_ctx->codec_id);

			if (NULL == dec_ctx)
			{
				//printf("[LOG] [ERROR] [xReader] [Init] [avcodec_find_decoder]\n");
				return false;
			}

			if (avcodec_open2(dec_ctx, dec, NULL) < 0)
			{
				//printf("[LOG] [ERROR] [xReader] [Init] [video] [avcodec_open2]\n");
				return false;
			}

			src_frame = av_frame_alloc();
			dst_frame = av_frame_alloc();

			return true;
		}
		bool xVideoInfo::SetScale(int dst_w, int dst_h, AVPixelFormat dst_fmt, int scale_flag)
		{
			if (dec_ctx == NULL)
				return false;

			sws_fmt = dst_fmt != -1 ? dst_fmt : dec_ctx->pix_fmt;
			sws_w = dst_w != -1 ? dst_w : dec_ctx->width;
			sws_h = dst_h != -1 ? dst_h : dec_ctx->height;

			sws_ctx = sws_getContext(
				dec_ctx->width,
				dec_ctx->height,
				dec_ctx->pix_fmt,
				sws_w,
				sws_h,
				sws_fmt,
				scale_flag,
				NULL,
				NULL,
				NULL);

			if (sws_ctx == NULL)
			{
				//printf("[LOG] [ERROR] [xReader] [SetScale] [sws_getContext]\n");
				return false;
			}

			dst_buf = (uint8_t*)av_calloc(av_image_get_buffer_size(sws_fmt, sws_w, sws_h, 1), sizeof(uint8_t));
			av_image_fill_arrays(dst_frame->data, dst_frame->linesize, dst_buf, sws_fmt, sws_w, sws_h, 1);

			return true;
		}


		xAudioInfo::xAudioInfo()
		{
			dec_ctx = NULL;
			dec = NULL;
			swr_ctx = NULL;

			src_frame = NULL;

			swr_ch_layout = -1;
			swr_sample_fmt = AV_SAMPLE_FMT_NONE;
			swr_sample_rate = -1;
		}
		xAudioInfo::~xAudioInfo()
		{
			Destroy();
		}

		void xAudioInfo::Destroy()
		{
			if (src_frame != NULL)
			{
				av_frame_free(&src_frame);
				src_frame = NULL;
			}

			stream_indexs.clear();

			if (dec_ctx != NULL)
			{
				avcodec_close(dec_ctx);
				avcodec_free_context(&dec_ctx);
				dec_ctx = NULL;
				dec = NULL;
			}

			if (swr_ctx != NULL)
			{
				swr_close(swr_ctx);
				swr_free(&swr_ctx);
				swr_ctx = NULL;
			}
		}
		int xAudioInfo::GetStreamIndex(int index)
		{
			return stream_indexs[index];
		}
		bool xAudioInfo::Make(AVFormatContext* fmt_ctx)
		{
			for (size_t i = 0; i < fmt_ctx->nb_streams; i++)
			{
				if (AVMEDIA_TYPE_AUDIO == fmt_ctx->streams[i]->codecpar->codec_type)
				{
					stream_indexs.push_back(i);
				}
			}


			if (stream_indexs.size() <= 0)
				return false;

			dec_ctx = avcodec_alloc_context3(NULL);
			avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[GetStreamIndex()]->codecpar);
			dec = avcodec_find_decoder(dec_ctx->codec_id);

			if (NULL == dec_ctx)
			{
				//printf("[LOG] [ERROR] [xReader] [Init] [avcodec_find_decoder]\n");
				return false;
			}

			if (avcodec_open2(dec_ctx, dec, NULL) < 0)
			{
				//printf("[LOG] [ERROR] [xReader] [Init] [video] [avcodec_open2]\n");
				return false;
			}

			src_frame = av_frame_alloc();

			return true;
		}
		bool xAudioInfo::SetReSample(int64_t ch_layout, enum AVSampleFormat sample_fmt, int sample_rate)
		{
			swr_ch_layout = ch_layout;
			swr_sample_fmt = sample_fmt;
			swr_sample_rate = sample_rate;

			swr_ctx = swr_alloc_set_opts(
				NULL,
				swr_ch_layout,
				swr_sample_fmt,
				swr_sample_rate,
				dec_ctx->channel_layout,
				dec_ctx->sample_fmt,
				dec_ctx->sample_rate,
				0,
				NULL);

			if (!swr_ctx || swr_init(swr_ctx) < 0)
				return false;

			return true;
		}



		bool xHelper::block_video_read()
		{
			int ret = avcodec_send_packet(v_info_.dec_ctx, fmt_info_.av_pkt);
			if (ret < 0)
			{
				return false;
			}


			while (ret = avcodec_receive_frame(v_info_.dec_ctx, v_info_.src_frame), true)
			{
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				{
					return true;
				}
				else if (ret < 0)
				{
					return false;
				}

				sws_scale(
					v_info_.sws_ctx,
					(const uint8_t* const*)v_info_.src_frame->data,
					v_info_.src_frame->linesize,
					0,
					v_info_.dec_ctx->height,
					v_info_.dst_frame->data,
					v_info_.dst_frame->linesize);

				//printf("v:%ld-%d-%d*%d\n",
				//	clock(),
				//	_frame->img_src->linesize[0],
				//	_frame->img_src->width,
				//	_frame->img_src->height);

				if (x_event != NULL)
				{
					x_event->VideoFrame(fmt_info_.av_pkt, v_info_.dst_frame, v_info_.sws_w, v_info_.sws_h, v_info_.sws_fmt);
				}
			}

			return false;
		}
		bool xHelper::block_audio_read()
		{
			int ret = avcodec_send_packet(a_info_.dec_ctx, fmt_info_.av_pkt);
			if (ret < 0)
			{
				return false;
			}


			while (ret = avcodec_receive_frame(a_info_.dec_ctx, a_info_.src_frame), true)
			{
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				{
					return true;
				}
				else if (ret < 0)
				{
					return false;
				}

				int dst_nb_samples = av_rescale_rnd(
					swr_get_delay(a_info_.swr_ctx, a_info_.src_frame->sample_rate) + a_info_.src_frame->nb_samples,
					a_info_.swr_sample_rate,
					a_info_.src_frame->sample_rate,
					AV_ROUND_INF);

				uint8_t* dst_buf = NULL;
				int dst_linesize = 0;

				if (av_samples_alloc(
					&dst_buf,
					&dst_linesize,
					av_get_channel_layout_nb_channels(a_info_.swr_ch_layout),
					dst_nb_samples,
					a_info_.swr_sample_fmt,
					1) < 0)
				{
					return false;
				}

				ret = swr_convert(
					a_info_.swr_ctx,
					&dst_buf,
					dst_nb_samples,
					(const uint8_t**)a_info_.src_frame->data,
					a_info_.src_frame->nb_samples);

				//int fifo_size = swr_get_out_samples(a_info_.swr_ctx, 0);
				//int data_size = av_get_channel_layout_nb_channels(a_info_.swr_ch_layout) * ret * av_get_bytes_per_sample(a_info_.swr_sample_fmt);

				if (x_event != NULL)
				{
					x_event->AudioFrame(fmt_info_.av_pkt, a_info_.src_frame, dst_buf, dst_linesize);
				}

				av_free(dst_buf);
			}

			return false;
		}

		xHelper::xHelper()
		{
			x_event = NULL;
		}
		xHelper::~xHelper()
		{
			Close();
		}

		bool xHelper::Open(const char* file_path, xEvent* ev)
		{
			x_event = ev;
			if (false == fmt_info_.Make(file_path))
				return false;

			fmt_info_.Show();

			if (false == a_info_.Make(fmt_info_.fmt_ctx))
				return false;

			if (false == a_info_.SetReSample(AV_CH_LAYOUT_STEREO, AVSampleFormat::AV_SAMPLE_FMT_S16, 44100))
				return false;

			if (false == v_info_.Make(fmt_info_.fmt_ctx))
				return false;

			if (false == v_info_.SetScale())
				return false;

			return true;
		}

		bool xHelper::BlockRead()
		{
			int ret = 0;

			av_packet_unref(fmt_info_.av_pkt);
			if (av_read_frame(fmt_info_.fmt_ctx, fmt_info_.av_pkt) >= 0)
			{
				if (fmt_info_.av_pkt->stream_index == v_info_.GetStreamIndex())
				{
					return block_video_read();
				}
				else if (fmt_info_.av_pkt->stream_index == a_info_.GetStreamIndex())
				{
					block_audio_read();
				}
				return true;
			}

			return false;
		}

		void xHelper::Close()
		{
			v_info_.Destroy();
			a_info_.Destroy();
			fmt_info_.Destroy();
		}

		int xHelper::ImgWdith()
		{
			return v_info_.sws_w;
		}
		int xHelper::ImgHeight()
		{
			return v_info_.sws_h;
		}

		double xHelper::ImgFrameRate()
		{
			return (fmt_info_.fmt_ctx->streams[v_info_.GetStreamIndex()]->avg_frame_rate.num * 1.0) /
				(fmt_info_.fmt_ctx->streams[v_info_.GetStreamIndex()]->avg_frame_rate.den * 1.0);
		}
	}
}
