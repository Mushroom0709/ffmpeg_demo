#include "x_ffmepg_demultiplexer.h"

namespace x
{
	namespace ffmpeg
	{
		xDemultiplexer::xDemultiplexer()
		{
			event_ = NULL;
			av_fmt_ctx_ = NULL;
			run_flag_ = false;

			vs_index_ = X_INT_INDEX_NONE;
			as_index_ = X_INT_INDEX_NONE;
		}
		xDemultiplexer::~xDemultiplexer()
		{
			Close();
		}

		void xDemultiplexer::dux_function()
		{
			int ret = 0;
			AVPacket* pkt = av_packet_alloc();

			if (event_ != NULL)
				event_->DuxStart();
			while (run_flag_)
			{
				ret = av_read_frame(av_fmt_ctx_, pkt);
				if (ret == 0)
				{
					if (event_ != NULL)
					{
						if (pkt->stream_index == vs_index_)
							event_->DuxPacket(pkt, AVMEDIA_TYPE_VIDEO);
						else if (pkt->stream_index == as_index_)
							event_->DuxPacket(pkt, AVMEDIA_TYPE_AUDIO);
					}
					//printf("%lld\n", pkt->dts);
				}
				else
				{
					if (ret == AVERROR_EOF)
					{
						run_flag_ = false;
					}
				}
				av_packet_unref(pkt);
			}
			if (event_ != NULL)
				event_->DuxEnd();
		}

		bool xDemultiplexer::Open(const char* _input)
		{
			if (run_flag_ == true)
				return false;

			av_fmt_ctx_ = avformat_alloc_context();
			if (!av_fmt_ctx_)return false;

			if (0 != avformat_open_input(&av_fmt_ctx_, _input, NULL, NULL))
				return false;

			if (0 > avformat_find_stream_info(av_fmt_ctx_, NULL))
				return false;

			for (size_t i = 0; i < av_fmt_ctx_->nb_streams; i++)
			{
				if (AVMEDIA_TYPE_VIDEO == av_fmt_ctx_->streams[i]->codecpar->codec_type &&
					vs_index_ == X_INT_INDEX_NONE)
				{
					vs_index_ = i;
				}
				else if (AVMEDIA_TYPE_AUDIO == av_fmt_ctx_->streams[i]->codecpar->codec_type &&
					as_index_ == X_INT_INDEX_NONE)
				{
					as_index_ = i;
				}
			}

			return true;
		}
		bool xDemultiplexer::Start(xDemultiplexEvent* _event)
		{
			if (run_flag_ == true)
				return false;

			event_ = _event;

			run_flag_ = true;
			dux_thread_ = std::thread(&xDemultiplexer::dux_function, this);
			return true;
		}

		bool xDemultiplexer::Info()
		{
			if (av_fmt_ctx_ == NULL)
				return false;

			av_dump_format(av_fmt_ctx_, 0, NULL, 0);
			return true;
		}
		int xDemultiplexer::AudioStreamIndex()
		{
			return as_index_;
		}
		int xDemultiplexer::VideoStreamIndex()
		{
			return vs_index_;
		}
		AVFormatContext* xDemultiplexer::FormatContext()
		{
			return av_fmt_ctx_;
		}

		void xDemultiplexer::Close()
		{
			run_flag_ = false;

			if (dux_thread_.joinable())
			{
				dux_thread_.join();
			}

			if (av_fmt_ctx_ != NULL)
			{
				avformat_close_input(&av_fmt_ctx_);
				avformat_free_context(av_fmt_ctx_);
				av_fmt_ctx_ = NULL;
			}
		}
	}
}