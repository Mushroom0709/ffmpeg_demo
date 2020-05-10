#include "x_source_process.h"
#include "x_target_process.h"

xSourceProcess::xSourceProcess()
{
    src_fmt_ctx_ = NULL;
}

xSourceProcess::~xSourceProcess()
{
    Destroy();
}

AVFormatContext* xSourceProcess::FormatContext()
{
    return src_fmt_ctx_;
}

std::map<int, AVMediaType>& xSourceProcess::StreamIndex()
{
    return stream_index_;
}

bool xSourceProcess::OpenSource(const char* _input, bool _dump_flg)
{
    src_fmt_ctx_ = avformat_alloc_context();
    if (src_fmt_ctx_ == NULL)
        return false;

    if (0 != avformat_open_input(&src_fmt_ctx_, _input, NULL, NULL))
    {
        ERROR_PRINTLN("open input fail");
        return false;
    }

    if (true == _dump_flg)
        av_dump_format(src_fmt_ctx_, 0, NULL, 0);

    if (0 > avformat_find_stream_info(src_fmt_ctx_, NULL))
    {
        ERROR_PRINTLN("don't finded stream info");
        return false;
    }


    for (int i = 0; i < src_fmt_ctx_->nb_streams; i++)
    {
        auto xstream = src_fmt_ctx_->streams[i];
        if (xstream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ||
            xstream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            stream_index_.insert(std::pair<int, AVMediaType>(i, xstream->codecpar->codec_type));
        }
    }

    return true;
}

void xSourceProcess::BlockReadPacket(xTargetProcess* _ptr_tgt)
{
    int ret = 0;
    bool run_flag = true;
    AVPacket* pkt = av_packet_alloc();

    _ptr_tgt->WriteHeader();

    while (run_flag)
    {
        ret = av_read_frame(src_fmt_ctx_, pkt);
        if (ret == 0)
        {
            auto index_info = stream_index_.find(pkt->stream_index);
            if (index_info != stream_index_.end())
            {
                _ptr_tgt->WritePacket(src_fmt_ctx_, pkt, pkt->stream_index);
            }
        }
        else
        {
            if (ret == AVERROR_EOF)
            {
                run_flag = false;
            }
        }
        av_packet_unref(pkt);
    }

    _ptr_tgt->WriteTrailer();
}

void xSourceProcess::Destroy()
{
    if (src_fmt_ctx_ == NULL)
        return;
    avformat_close_input(&src_fmt_ctx_);
    src_fmt_ctx_ = NULL;
}