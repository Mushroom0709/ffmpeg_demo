#include "x_input_stream.h"
#include "x_output_stream.h"

AVFormatContext* xInputStream::GetFmtCtx()
{
    return fmt_ctx_;
}
std::map<int, xInStreamInfo>& xInputStream::GetStreams()
{
    return streams_;
}

xInputStream::xInputStream()
{
    fmt_ctx_ = NULL;
}
xInputStream::~xInputStream()
{
    Destroy();
}

void xInputStream::Destroy()
{
    if (fmt_ctx_ == NULL)
        return;

    for (auto st_info : streams_)
    {
        av_frame_free(&(st_info.second.Frame));
        avcodec_close(st_info.second.CodecCtx);
        avcodec_free_context(&(st_info.second.CodecCtx));
    }

    streams_.clear();

    avformat_close_input(&fmt_ctx_);
    fmt_ctx_ = NULL;
}

bool xInputStream::OpenScreen(bool _dump_flg)
{
    fmt_ctx_ = avformat_alloc_context();
    if (fmt_ctx_ == NULL)
        return false;

    AVInputFormat* iformat = av_find_input_format("gdigrab");
    if (iformat == NULL) return false;

    char video_size[128];
    sprintf_s(video_size, "%dx%d", 1920, 1080);

    AVDictionary* options = NULL;
    av_dict_set(&options, "framerate", "24", 0);//Ö¡ÂÊµ÷Õû
    if (0 != avformat_open_input(&fmt_ctx_, "desktop", iformat, &options))
        return false;
    av_dict_free(&options);

    if (true == _dump_flg)
        av_dump_format(fmt_ctx_, 0, NULL, 0);

    if (0 > avformat_find_stream_info(fmt_ctx_, NULL))
    {
        ERROR_PRINTLN("don't finded stream info");
        return false;
    }

    for (int i = 0; i < fmt_ctx_->nb_streams; i++)
    {
        auto xstream = fmt_ctx_->streams[i];
        if (xstream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            xInStreamInfo st_info;
            st_info.Stream = xstream;
            st_info.StreamIndex = i;
            st_info.MediaType = AVMEDIA_TYPE_VIDEO;
            st_info.CodecCtx = avcodec_alloc_context3(NULL);
            if (NULL == st_info.CodecCtx)
                return false;

            avcodec_parameters_to_context(st_info.CodecCtx, xstream->codecpar);

            st_info.Codec = avcodec_find_decoder(st_info.CodecCtx->codec_id);
            if (NULL == st_info.Codec)
                return false;

            st_info.CodecCtx->framerate = av_guess_frame_rate(fmt_ctx_, st_info.Stream, NULL);

            if (avcodec_open2(st_info.CodecCtx, st_info.Codec, NULL) < 0)
            {
                return false;
            }

            st_info.Frame = av_frame_alloc();

            streams_.insert(std::pair<int, xInStreamInfo>(i, st_info));

            break;
        }
    }

    if (streams_.size() <= 0)
        return false;

    return true;
}
bool xInputStream::OpenFile(const char* _input, bool _dump_flg)
{
    fmt_ctx_ = avformat_alloc_context();
    if (fmt_ctx_ == NULL)
        return false;

    if (0 != avformat_open_input(&fmt_ctx_, _input, NULL, NULL))
    {
        ERROR_PRINTLN("open input fail");
        return false;
    }

    if (true == _dump_flg)
        av_dump_format(fmt_ctx_, 0, NULL, 0);

    if (0 > avformat_find_stream_info(fmt_ctx_, NULL))
    {
        ERROR_PRINTLN("don't finded stream info");
        return false;
    }

    bool video_flag = false;
    bool audio_flag = false;

    for (int i = 0; i < fmt_ctx_->nb_streams; i++)
    {
        auto xstream = fmt_ctx_->streams[i];
        if (xstream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audio_flag == false)
        {
            xInStreamInfo st_info;
            st_info.Stream = xstream;
            st_info.StreamIndex = i;
            st_info.MediaType = AVMEDIA_TYPE_AUDIO;
            st_info.CodecCtx = avcodec_alloc_context3(NULL);
            if (NULL == st_info.CodecCtx)
                return false;

            avcodec_parameters_to_context(st_info.CodecCtx, xstream->codecpar);

            st_info.Codec = avcodec_find_decoder(st_info.CodecCtx->codec_id);
            if (NULL == st_info.Codec)
                return false;

            if (avcodec_open2(st_info.CodecCtx, st_info.Codec, NULL) < 0)
            {
                return false;
            }

            st_info.Frame = av_frame_alloc();

            streams_.insert(std::pair<int, xInStreamInfo>(i, st_info));

            audio_flag = true;
        }

        if (xstream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && video_flag == false)
        {
            xInStreamInfo st_info;
            st_info.Stream = xstream;
            st_info.StreamIndex = i;
            st_info.MediaType = AVMEDIA_TYPE_VIDEO;
            st_info.CodecCtx = avcodec_alloc_context3(NULL);
            if (NULL == st_info.CodecCtx)
                return false;

            avcodec_parameters_to_context(st_info.CodecCtx, xstream->codecpar);

            st_info.Codec = avcodec_find_decoder(st_info.CodecCtx->codec_id);
            if (NULL == st_info.Codec)
                return false;
            st_info.CodecCtx->framerate = av_guess_frame_rate(fmt_ctx_, st_info.Stream, NULL);

            if (avcodec_open2(st_info.CodecCtx, st_info.Codec, NULL) < 0)
            {
                return false;
            }

            st_info.Frame = av_frame_alloc();

            streams_.insert(std::pair<int, xInStreamInfo>(i, st_info));

            video_flag = true;
        }

        if (audio_flag && video_flag)
            break;
    }

    return true;
}

bool xInputStream::block_read_frame(AVPacket* _pkt, xInStreamInfo& _in_st, xOutputStream* _output)
{
    int ret = 0;

    if (avcodec_send_packet(_in_st.CodecCtx, _pkt) < 0)
        return false;

    while (ret = avcodec_receive_frame(_in_st.CodecCtx, _in_st.Frame), true)
    {
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0)
        {
            return false;
        }
        else if (ret == 0)
        {
            //if (_in_st.MediaType == AVMEDIA_TYPE_AUDIO)
            //{
            //    INFO_PRINTLN("audio pts:%16lld\tw:%d\th:%d",
            //        _in_st.Frame->best_effort_timestamp,
            //        _in_st.Frame->sample_rate,
            //        _in_st.Frame->channels);
            //}
            //else if (_in_st.MediaType == AVMEDIA_TYPE_VIDEO)
            //{
            //    INFO_PRINTLN("video pts:%16lld\tw:%d\th:%d",
            //        _in_st.Frame->best_effort_timestamp,
            //        _in_st.Frame->width,
            //        _in_st.Frame->height);
            //}
            _output->WriteFrame(_in_st);
        }
    }

    return true;
}

void xInputStream::BlockRead(xOutputStream* _output)
{
    int ret = 0;
    bool run_flag = true;
    AVPacket* pkt = av_packet_alloc();

    if (false == _output->WriteHeader())
    {
        ERROR_PRINTLN("write header fail");
    }

    while (run_flag)
    {
        ret = av_read_frame(fmt_ctx_, pkt);
        if (ret == 0)
        {
            auto st = streams_.find(pkt->stream_index);
            if (st != streams_.end())
            {
                if (false == block_read_frame(pkt, st->second, _output))
                    run_flag = false;
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

    if (false == _output->WriteTrailer())
    {
        ERROR_PRINTLN("write trailer fail");
    }
}