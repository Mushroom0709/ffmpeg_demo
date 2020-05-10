#include "x_target_process.h"

xTargetProcess::xTargetProcess()
{
    tgt_fmt_ctx_ = NULL;
    clock_stream_index_ = -1;
    last_pts_ = -1.0;
    last_update_time_ = -1.0;
}

xTargetProcess::~xTargetProcess()
{
    Destroy();
}

bool xTargetProcess::NewOutput(const char* _url, const char* _fmt)
{
    tgt_url_ = _url;
    if (0 > avformat_alloc_output_context2(&tgt_fmt_ctx_, NULL, _fmt, _url))
    {
        ERROR_PRINTLN("new output fail");
        return false;
    }

    if (tgt_fmt_ctx_ == NULL)
        return false;

    return true;
}

bool xTargetProcess::BuildStreamByInput(AVFormatContext* _input_fmt_ctx, std::map<int, AVMediaType>& _input_stream_index, bool _dump_flg)
{
    int index_cnt = 0;
    bool clock_flg = false;
    for (auto item : _input_stream_index)
    {
        auto in_stream = _input_fmt_ctx->streams[item.first];
        auto out_stream = avformat_new_stream(tgt_fmt_ctx_, NULL);

        if (0 > avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar))
        {
            ERROR_PRINTLN("new output stream fail! src:%d", item.first);
            return false;
        }

        out_stream->codecpar->codec_tag = 0;
        //out_stream->codec->codec_tag = 0;

        //if (tgt_fmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
        //	out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


        inout_index_mapping_.insert(std::pair<int, int>(item.first, index_cnt));

        if (item.second == AVMEDIA_TYPE_AUDIO && clock_flg == false)
        {
            clock_stream_index_ = index_cnt;
            clock_flg = true;
        }

        index_cnt++;
    }

    if (clock_flg == false)
    {
        for (auto item : _input_stream_index)
        {
            if (item.second == AVMEDIA_TYPE_VIDEO && inout_index_mapping_.find(item.first) != inout_index_mapping_.end())
            {
                clock_stream_index_ = inout_index_mapping_[item.first];
                clock_flg = true;
                break;
            }
        }
    }

    if (false == clock_flg)
        return false;

    if ((tgt_fmt_ctx_->flags & AVFMT_NOFILE) != AVFMT_NOFILE)
    {
        if (0 > avio_open(&tgt_fmt_ctx_->pb, tgt_url_.c_str(), AVIO_FLAG_WRITE))
            return false;
    }

    if (true == _dump_flg)
        av_dump_format(tgt_fmt_ctx_, 0, NULL, 1);

    return true;
}
bool xTargetProcess::WriteHeader()
{
    if (0 > avformat_write_header(tgt_fmt_ctx_, NULL))
        return false;
    return true;
}
bool xTargetProcess::WriteTrailer()
{
    if (0 == av_write_trailer(tgt_fmt_ctx_))
        return false;
    return true;
}
bool xTargetProcess::WritePacket(AVFormatContext* _input_fmt_ctx, AVPacket* _pkt, int _index)
{
    auto mapping = inout_index_mapping_.find(_index);
    if (mapping == inout_index_mapping_.end())
        return false;

    av_packet_rescale_ts(
        _pkt,
        _input_fmt_ctx->streams[mapping->first]->time_base,
        tgt_fmt_ctx_->streams[mapping->second]->time_base);

    _pkt->pos = -1;
    _pkt->stream_index = mapping->second;

    if (mapping->second == clock_stream_index_)
    {
        //printf("%lld\t%lld\n", _pkt->pts, _pkt->dts);
        if (last_pts_ < -0.1)
        {
            last_pts_ = _pkt->pts * av_q2d(tgt_fmt_ctx_->streams[mapping->second]->time_base);
        }
        else
        {
            double dpts = _pkt->pts * av_q2d(tgt_fmt_ctx_->streams[mapping->second]->time_base);
            double theory_wait = dpts - last_pts_;

            double now_time = av_gettime_relative() / (AV_TIME_BASE * 1.0);
            double fact_wait = now_time - last_update_time_;

            double delay = theory_wait - fact_wait;
            last_pts_ = dpts;
            if (delay > MIN_SLEEP_TIME_D_MICROSECOND)
            {
                printf("%lf\n", delay);
                av_usleep(static_cast<unsigned int>(delay * 1000000.0));
            }
        }
    }

    int ret = av_interleaved_write_frame(tgt_fmt_ctx_, _pkt);

    if (mapping->second == clock_stream_index_)
        last_update_time_ = av_gettime_relative() / (AV_TIME_BASE * 1.0);

    if (ret != 0)
    {
        char error_buf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
        ERROR_PRINTLN("error occurred: %s", av_make_error_string(error_buf, 4096, ret));
        return false;
    }


    return true;
}

void xTargetProcess::Destroy()
{
    if (tgt_fmt_ctx_ == NULL)
        return;

    if ((tgt_fmt_ctx_->flags & AVFMT_NOFILE) == AVFMT_NOFILE)
    {
        avio_close(tgt_fmt_ctx_->pb);
    }

    avformat_free_context(tgt_fmt_ctx_);
    tgt_fmt_ctx_ = NULL;
}