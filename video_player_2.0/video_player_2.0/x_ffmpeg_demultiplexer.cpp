#include "x_ffmpeg_demultiplexer.h"

namespace x_ffmpeg
{
	Demultiplexer::Demultiplexer()
	{
		format_ctx_ = NULL;
	}
	Demultiplexer::~Demultiplexer()
	{
		Destroy();
	}

	AVFormatContext* Demultiplexer::FormatContext()
	{
		return format_ctx_;
	}
	std::vector<int>& Demultiplexer::VideoIndexs()
	{
		return i_videos_;
	}
	std::vector<int>& Demultiplexer::AudioIndexs()
	{
		return i_audios_;
	}

	bool Demultiplexer::Initialize(const char* _file, int& _nb_streams)
	{
		format_ctx_ = avformat_alloc_context();
		if (avformat_alloc_context == NULL)
			return false;

		if (0 != avformat_open_input(&format_ctx_, _file, NULL, NULL))
			return false;

		if (0 > avformat_find_stream_info(format_ctx_, NULL))
			return false;

		_nb_streams = format_ctx_->nb_streams;

		av_dump_format(format_ctx_, 0, NULL, 0);

		for (size_t i = 0; i < format_ctx_->nb_streams; i++)
		{
			if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
				i_videos_.push_back(i);
			else if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
				i_audios_.push_back(i);
		}

		return true;
	}
	bool Demultiplexer::GetPacket(AVPacket* _pkt, AVMediaType& _type, int& _stream_index, int& _status)
	{
		_status = av_read_frame(format_ctx_, _pkt);
		if (_status == 0)
		{
			_stream_index = _pkt->stream_index;
			_type = format_ctx_->streams[_stream_index]->codecpar->codec_type;
		}
		else
		{
			_stream_index = -1;
			_type = AVMEDIA_TYPE_UNKNOWN;


			if (_status == AVERROR_EOF)
				return false;
		}
		return true;
	}
	void Demultiplexer::Destroy()
	{
		if (format_ctx_ != NULL)
		{
			avformat_close_input(&format_ctx_);
			avformat_free_context(format_ctx_);
			format_ctx_ = NULL;
		}
	}
}