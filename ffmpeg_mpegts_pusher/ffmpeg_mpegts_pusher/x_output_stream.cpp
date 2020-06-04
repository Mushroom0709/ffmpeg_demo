#include "x_output_stream.h"

xOutputStream::xOutputStream()
{
    fmt_ctx_ = NULL;

    clock_stream_index_ = -1;
    last_pts_ = -1.0;
    last_update_time_ = -1.0;

    frame_pts_ = 0;
    pkt_pts_ = 0;
    pkt_dts_ = 0;
}

xOutputStream::~xOutputStream()
{
    Destroy();
}




bool xOutputStream::Initialization(const char* _url)
{
    url_ = _url;

    if (url_.find_first_of("udp") == 0)
    {
        if (0 > avformat_alloc_output_context2(&fmt_ctx_, NULL, "mpegts", _url))
        {
            ERROR_PRINTLN("new output fail");
            return false;
        }

        if (fmt_ctx_ == NULL)
            return false;
    }
    //else if (url_.find_first_of("rtsp") == 0)
    //{
    //    if (0 > avformat_alloc_output_context2(&fmt_ctx_, NULL, "rtsp", _url))
    //    {
    //        ERROR_PRINTLN("new output fail");
    //        return false;
    //    }
    //    if (fmt_ctx_ == NULL)
    //        return false;
    //}
    //else if (url_.find_first_of("rtmp") == 0)
    //{
    //    if (0 > avformat_alloc_output_context2(&fmt_ctx_, NULL, "flv", _url))
    //    {
    //        ERROR_PRINTLN("new output fail");
    //        return false;
    //    }
    //    if (fmt_ctx_ == NULL)
    //        return false;
    //}
    else
    {
        return false;
    }

    return true;
}
void xOutputStream::Destroy()
{
    if (fmt_ctx_ == NULL)
        return;

    for (auto st_info : streams_)
    {
        av_frame_free(&(st_info.second.Frame));
        avcodec_close(st_info.second.CodecCtx);
        avcodec_free_context(&(st_info.second.CodecCtx));

        //if (st_info.second.BSFCtx != NULL)
        //{
        //    av_bsf_free(&st_info.second.BSFCtx);
        //}

        if (st_info.second.SwrCtx != NULL)
        {
            swr_close(st_info.second.SwrCtx);
            st_info.second.SwrCtx = NULL;
        }

        if (st_info.second.SwsCtx != NULL)
        {
            sws_freeContext(st_info.second.SwsCtx);
            st_info.second.SwsCtx = NULL;
        }
    }

    streams_.clear();

    avformat_close_input(&fmt_ctx_);
    fmt_ctx_ = NULL;
}
bool xOutputStream::create_stream(xInStreamInfo& _in_info, xOutStreamInfo& _out_info, enum AVCodecID _codec_id)
{
    _out_info.Codec = avcodec_find_encoder(_codec_id);
    if (_out_info.Codec == NULL)
        return false;

    _out_info.Stream = avformat_new_stream(fmt_ctx_, _in_info.Codec);
    if (_out_info.Stream == NULL)
        return false;

    _out_info.StreamIndex = fmt_ctx_->nb_streams - 1;
    _out_info.Stream->id = _out_info.StreamIndex;


    _out_info.CodecCtx = avcodec_alloc_context3(_out_info.Codec);
    if (_out_info.CodecCtx == NULL)
        return false;

    switch (_out_info.Codec->type)
    {
    case AVMEDIA_TYPE_VIDEO:
    {
        _out_info.CodecCtx->codec_id = _out_info.Codec->id;
        _out_info.CodecCtx->width = _in_info.CodecCtx->width;
        _out_info.CodecCtx->height = _in_info.CodecCtx->height;
        _out_info.CodecCtx->sample_aspect_ratio = _in_info.CodecCtx->sample_aspect_ratio;
        _out_info.CodecCtx->framerate = _in_info.CodecCtx->framerate;
        _out_info.CodecCtx->time_base = av_inv_q(_in_info.CodecCtx->framerate);
        _out_info.CodecCtx->gop_size = _in_info.CodecCtx->gop_size;
        _out_info.Stream->time_base = _out_info.CodecCtx->time_base;
        //av_opt_set(_out_info.CodecCtx->priv_data, "preset", "veryfast", 0);//ultrafast/veryfast、faster、fast、medium、slow
        //av_opt_set(_out_info.CodecCtx->priv_data, "tune", "zerolatency", 0);
        _out_info.CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        _out_info.CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    }
    break;

    case AVMEDIA_TYPE_AUDIO:
    {
        _out_info.CodecCtx->sample_fmt = _out_info.Codec->sample_fmts ? _out_info.Codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        _out_info.CodecCtx->sample_rate = _in_info.CodecCtx->sample_rate;
        _out_info.CodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
        _out_info.CodecCtx->channels = av_get_channel_layout_nb_channels(_out_info.CodecCtx->channel_layout);
        _out_info.CodecCtx->time_base = { 1, _out_info.CodecCtx->sample_rate };
        _out_info.Stream->time_base = _out_info.CodecCtx->time_base;
        _out_info.CodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    }

    default:
        break;
    }

    if (0 != avcodec_open2(_out_info.CodecCtx, _out_info.Codec, NULL))
        return false;

    if (0 > avcodec_parameters_from_context(_out_info.Stream->codecpar, _out_info.CodecCtx))
        return false;

    return true;
}

bool xOutputStream::SetParameters(std::map<int, xInStreamInfo> _in_st, bool _dump_flg)
{
    for (auto in_st : _in_st)
    {
        if (in_st.second.MediaType == AVMEDIA_TYPE_AUDIO)
        {
            xOutStreamInfo out_st;
            out_st.MediaType = in_st.second.MediaType;
            if (false == create_stream(in_st.second, out_st, AV_CODEC_ID_AAC))
                return false;

            out_st.SwrCtx = swr_alloc_set_opts(
                NULL,
                out_st.CodecCtx->channel_layout,        // 目标声道布局
                out_st.CodecCtx->sample_fmt,            // 目标采样格式
                out_st.CodecCtx->sample_rate,           // 目标采样率
                in_st.second.CodecCtx->channel_layout,	// 原始声道布局
                in_st.second.CodecCtx->sample_fmt,		// 原始采样格式
                in_st.second.CodecCtx->sample_rate,		// 原始采样率
                0,								        // 日志开关
                NULL);						            // 日志上下文

            if (!out_st.SwrCtx || swr_init(out_st.SwrCtx) < 0)
                return false;

            out_st.Swr_nb_samples = (int)av_rescale_rnd(
                swr_get_delay(out_st.SwrCtx, in_st.second.CodecCtx->sample_rate) + in_st.second.CodecCtx->frame_size,
                out_st.CodecCtx->sample_rate,
                in_st.second.CodecCtx->sample_rate,
                AV_ROUND_INF);

            out_st.Frame = av_frame_alloc();
            out_st.Frame->format = out_st.CodecCtx->sample_fmt;
            out_st.Frame->channels = av_get_channel_layout_nb_channels(out_st.CodecCtx->channel_layout);
            out_st.Frame->channel_layout = out_st.CodecCtx->channel_layout;
            out_st.Frame->nb_samples = out_st.Swr_nb_samples; //一帧音频一通道的采用数量
            out_st.Frame->sample_rate = out_st.CodecCtx->sample_rate;
            if (0 != av_frame_get_buffer(out_st.Frame, 0))
                return false;

            streams_.insert(std::pair<int, xOutStreamInfo>(in_st.first, out_st));

            if (clock_stream_index_ == -1)
            {
                clock_stream_index_ = out_st.StreamIndex;
            }
        }

        if (in_st.second.MediaType == AVMEDIA_TYPE_VIDEO)
        {
            xOutStreamInfo out_st;
            out_st.MediaType = in_st.second.MediaType;
            if (false == create_stream(in_st.second, out_st, AV_CODEC_ID_H264))
                return false;

            out_st.SwsCtx = sws_getContext(
                in_st.second.CodecCtx->width,
                in_st.second.CodecCtx->height,
                in_st.second.CodecCtx->pix_fmt,
                out_st.CodecCtx->width,
                out_st.CodecCtx->height,
                out_st.CodecCtx->pix_fmt,
                SWS_FAST_BILINEAR,
                NULL,
                NULL,
                NULL);

            if (out_st.SwsCtx == NULL)
                return false;

            out_st.FrameBuffer = (uint8_t*)av_calloc(av_image_get_buffer_size(out_st.CodecCtx->pix_fmt, out_st.CodecCtx->width, out_st.CodecCtx->height, 1), sizeof(uint8_t));
            out_st.Frame = av_frame_alloc();
            av_image_fill_arrays(out_st.Frame->data, out_st.Frame->linesize, out_st.FrameBuffer, out_st.CodecCtx->pix_fmt, out_st.CodecCtx->width, out_st.CodecCtx->height, 1);


            streams_.insert(std::pair<int, xOutStreamInfo>(in_st.first, out_st));
        }
    }

    if (!(fmt_ctx_->flags & AVFMT_NOFILE))
    {
        int ret = avio_open(&fmt_ctx_->pb, url_.c_str(), AVIO_FLAG_WRITE);
        if (0 > ret)
        {
            char error_buf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
            ERROR_PRINTLN("%s", av_make_error_string(error_buf, 4096, ret));
            return false;
        }

        //if (url_.find_first_of("rtp") == 0)
        //{
        //    char sdp_str[2048] = { 0 };
        //    if (0 == av_sdp_create(&fmt_ctx_, 1, sdp_str, sizeof(sdp_str)))
        //    {
        //        printf("\n%s\n", sdp_str);
        //    }
        //}
    }

    if (true == _dump_flg)
        av_dump_format(fmt_ctx_, 0, NULL, 1);

    return true;
}

bool xOutputStream::WriteHeader()
{
    if (0 > avformat_write_header(fmt_ctx_, NULL))
        return false;
    return true;
}
bool xOutputStream::WriteTrailer()
{
    if (0 == av_write_trailer(fmt_ctx_))
        return false;
    return true;
}

bool xOutputStream::WriteFrame(xInStreamInfo& _in_info)
{
    auto find_res = streams_.find(_in_info.StreamIndex);
    if (find_res != streams_.end())
    {
        int ret = 0, got_packet = 0;
        auto& out_info = find_res->second;

        AVPacket* out_pkt = av_packet_alloc();
        //av_init_packet(out_pkt);

        if (out_info.MediaType == AVMEDIA_TYPE_VIDEO)
        {
            sws_scale(
                out_info.SwsCtx,
                (const uint8_t* const*)_in_info.Frame->data,
                _in_info.Frame->linesize,
                0,
                _in_info.CodecCtx->height,
                out_info.Frame->data,
                out_info.Frame->linesize);

            out_info.Frame->pts = _in_info.Frame->best_effort_timestamp;
            out_info.Frame->format = out_info.CodecCtx->pix_fmt;
            out_info.Frame->width = out_info.CodecCtx->width;
            out_info.Frame->height = out_info.CodecCtx->height;

            ret = avcodec_send_frame(out_info.CodecCtx, out_info.Frame);
            if (ret < 0)
            {
                return false;
            }

            got_packet = avcodec_receive_packet(out_info.CodecCtx, out_pkt);
            if (got_packet == 0)
            {
                av_packet_rescale_ts(out_pkt, _in_info.Stream->time_base, out_info.Stream->time_base);
                out_pkt->stream_index = out_info.StreamIndex;

                if (out_info.last_time == 0)
                {
                    out_info.last_time = clock();
                }
                else
                {
                    double diff = (clock() * 1.0 - out_info.last_time * 1.0) / 1000.0;

                    if (diff > 1.0)
                    {
                        INFO_PRINTLN("%lf Mbps", out_info.pkt_size_sum / diff * 8.0);
                        out_info.last_time = clock();
                        out_info.pkt_size_sum = 0;
                    }
                }

                out_info.pkt_size_sum += out_pkt->size / 1024.0 / 1024.0;

                if (0 != av_interleaved_write_frame(fmt_ctx_, out_pkt))
                {
                    ERROR_PRINTLN("write video frame fail");
                }


                //INFO_PRINTLN("av_interleaved_write_frame:%ld ms", clock() - _in_info.start_time);
            }
            else
            {
                if (got_packet == AVERROR(EAGAIN))
                {
                   /* ret = avcodec_send_frame(out_info.CodecCtx, NULL);
                    if (ret < 0)
                    {
                        return false;
                    }*/
                }
                else
                {
                    char error_buf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
                    ERROR_PRINTLN("%s", av_make_error_string(error_buf, 4096, got_packet));
                }
            }
           
        }
        else if (out_info.MediaType == AVMEDIA_TYPE_AUDIO)
        {

            ret = swr_convert(out_info.SwrCtx,
                (uint8_t **)&out_info.Frame->data[0], out_info.Frame->nb_samples,
                (const uint8_t**)&_in_info.Frame->data[0], _in_info.Frame->nb_samples);


            out_info.Frame->pts = _in_info.Frame->pts;
            out_info.Frame->pkt_dts = _in_info.Frame->pkt_dts;

            ret = avcodec_send_frame(out_info.CodecCtx, out_info.Frame);
            if (ret < 0)
            {
                return false;
            }

            got_packet = avcodec_receive_packet(out_info.CodecCtx, out_pkt);
            if (got_packet == 0)
            {
                out_pkt->stream_index = out_info.StreamIndex;

                out_pkt->dts = av_rescale_q_rnd(out_pkt->dts, 
                    out_info.CodecCtx->time_base, out_info.Stream->time_base, 
                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                out_pkt->pts = av_rescale_q_rnd(out_pkt->pts,
                    out_info.CodecCtx->time_base, out_info.Stream->time_base,
                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                out_pkt->duration = av_rescale_q_rnd(out_pkt->duration,
                    out_info.CodecCtx->time_base, out_info.Stream->time_base,
                    (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));

                if (out_info.StreamIndex == clock_stream_index_)
                {
                    if (last_pts_ < -0.1)
                    {
                        last_pts_ = out_pkt->pts * av_q2d(out_info.Stream->time_base);
                    }
                    else
                    {
                        double dpts = out_pkt->pts * av_q2d(out_info.Stream->time_base);
                        double theory_wait = dpts - last_pts_;
                        double now_time = av_gettime_relative() / (AV_TIME_BASE * 1.0);
                        double fact_wait = now_time - last_update_time_;
                        double delay = (theory_wait - fact_wait);
                        last_pts_ = dpts;
                        if (delay > MIN_SLEEP_TIME_D_MICROSECOND)
                        {
                            printf("%lf\n", delay);
                            delay *= 0.70;
                            av_usleep(static_cast<unsigned int>(delay * 1000000.0));
                        }
                        else if (delay < (-1 * 0.0001))
                        {
                            printf("%lf\n", delay);
                        }
                    }
                }
                if (0 != av_interleaved_write_frame(fmt_ctx_, out_pkt))
                {
                    ERROR_PRINTLN("write audio frame fail");
                }

                if (out_info.StreamIndex == clock_stream_index_)
                    last_update_time_ = av_gettime_relative() / (AV_TIME_BASE * 1.0);
            }
            else
            {
                if (!(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF))
                {
                    char error_buf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
                    ERROR_PRINTLN("%s", av_make_error_string(error_buf, 4096, ret));
                }
            }
        }
        av_packet_free(&out_pkt);
    }

    return true;
}