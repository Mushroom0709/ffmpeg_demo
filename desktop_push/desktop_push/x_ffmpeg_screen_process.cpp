#include "x_ffmpeg_screen_process.h"

namespace xM
{
    namespace ffmepg
    {
        ScreenProcess::ScreenProcess()
        {
            ptr_param_ = NULL;
            ptr_encoder_ = NULL;
            ptr_nal_ = NULL;
            ptr_in_pic_ = NULL;
            ptr_out_pic_ = NULL;

            in_fmt_ctx_ = NULL;
            in_cdc_ctx_ = NULL;
            sws_ctx_ = NULL;
            video_index_ = -1;
            run_flag_ = false;
            event_ = NULL;
        }
        ScreenProcess::~ScreenProcess()
        {
            Close();
        }

        bool ScreenProcess::init_screen_stream()
        {
            avdevice_register_all();

            in_fmt_ctx_ = avformat_alloc_context();
            if (in_fmt_ctx_ == NULL)
                return false;

            AVInputFormat* iformat = av_find_input_format("gdigrab");
            if (iformat == NULL)
                return false;

            AVDictionary* options = NULL;
            av_dict_set(&options, "framerate", "24", 0);//Ö¡ÂÊµ÷Õû
            if (0 != avformat_open_input(&in_fmt_ctx_, "desktop", iformat, &options))
                return false;
            av_dict_free(&options);

            av_dump_format(in_fmt_ctx_, 0, NULL, 0);

            if (0 > avformat_find_stream_info(in_fmt_ctx_, NULL))
            {
                PrintError("don't finded stream info");
                return false;
            }

            for (int i = 0; i < in_fmt_ctx_->nb_streams; i++)
            {
                auto st = in_fmt_ctx_->streams[i];
                if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                {
                    video_index_ = i;

                    in_cdc_ctx_ = avcodec_alloc_context3(NULL);
                    if (NULL == in_cdc_ctx_)
                        return false;

                    avcodec_parameters_to_context(in_cdc_ctx_, st->codecpar);

                    AVCodec* cdc = avcodec_find_decoder(in_cdc_ctx_->codec_id);
                    if (NULL == cdc)
                        return false;

                    in_cdc_ctx_->framerate = av_guess_frame_rate(in_fmt_ctx_, st, NULL);

                    if (avcodec_open2(in_cdc_ctx_, cdc, NULL) < 0)
                    {
                        return false;
                    }

                    break;
                }
            }

            if (video_index_ == -1)
                return false;
            return true;
        }
        bool ScreenProcess::init_h264_encode()
        {
            ptr_param_ = (x264_param_t*)calloc(1,sizeof(x264_param_t));
            ptr_in_pic_ = (x264_picture_t*)calloc(1, sizeof(x264_picture_t));
            ptr_out_pic_ = (x264_picture_t*)calloc(1, sizeof(x264_picture_t));

            x264_param_default(ptr_param_);
            if (0 != x264_param_default_preset(ptr_param_, "veryfast", "zerolatency"))
                return false;

            ptr_param_->i_csp = X264_CSP_I420;
            ptr_param_->i_width = X_FFMEPG_SCREEN_DST_WIDTH;
            ptr_param_->i_height = X_FFMEPG_SCREEN_DST_HEIGHT;
            ptr_param_->i_keyint_min = 1;
            ptr_param_->i_keyint_max = 6;

            ptr_param_->i_fps_num = in_cdc_ctx_->framerate.num;
            ptr_param_->i_fps_den = in_cdc_ctx_->framerate.den;

            x264_param_apply_profile(ptr_param_,"high");

            ptr_encoder_ = x264_encoder_open(ptr_param_);
            if (ptr_encoder_ == NULL)
                return false;

            x264_picture_init(ptr_out_pic_);
            x264_picture_init(ptr_in_pic_);
            if (0 != x264_picture_alloc(ptr_in_pic_, ptr_param_->i_csp, ptr_param_->i_width, ptr_param_->i_height))
                return false;


            sws_ctx_ = sws_getContext(
                in_cdc_ctx_->width,
                in_cdc_ctx_->height,
                in_cdc_ctx_->pix_fmt,
                X_FFMEPG_SCREEN_DST_WIDTH,
                X_FFMEPG_SCREEN_DST_HEIGHT,
                AV_PIX_FMT_YUV420P,
                SWS_FAST_BILINEAR,
                NULL,
                NULL,
                NULL);

            if (sws_ctx_ == NULL)
                return false;

            return true;
        }
        bool ScreenProcess::process_x264_encoder(AVFrame* _frame)
        {
            int ret = 0;
            int nal_num = 0;

            memcpy(ptr_in_pic_->img.plane[0], _frame->data[0], ptr_param_->i_width * ptr_param_->i_height);
            memcpy(ptr_in_pic_->img.plane[1], _frame->data[1], ptr_param_->i_width * ptr_param_->i_height / 4);
            memcpy(ptr_in_pic_->img.plane[2], _frame->data[2], ptr_param_->i_width * ptr_param_->i_height / 4);

            ret = x264_encoder_encode(ptr_encoder_, &ptr_nal_, &nal_num, ptr_in_pic_, ptr_out_pic_);
            if (ret < 0)
                return false;
            

            event_->x264NalData(ptr_nal_, nal_num);

            ptr_in_pic_->i_pts += 1;

            return true;
        }
        void ScreenProcess::process_stream_function()
        {
            int ret = 0;
            AVPacket* in_pkt = av_packet_alloc();

            AVFrame* in_frame = av_frame_alloc();
            AVFrame* sws_frame = av_frame_alloc();

            sws_frame->format = AV_PIX_FMT_YUV420P;
            sws_frame->width = X_FFMEPG_SCREEN_DST_WIDTH;
            sws_frame->height = X_FFMEPG_SCREEN_DST_HEIGHT;
            av_frame_get_buffer(sws_frame, 0);

            while (run_flag_)
            {
                ret = av_read_frame(in_fmt_ctx_, in_pkt);
                if (ret == 0)
                {
                    if (avcodec_send_packet(in_cdc_ctx_, in_pkt) < 0)
                    {
                        run_flag_ = false;
                    }
                    else
                    {
                        while (ret = avcodec_receive_frame(in_cdc_ctx_, in_frame), run_flag_)
                        {
                            if (ret < 0)
                            {
                                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                                    break;
                                else
                                    run_flag_ = false;
                            }
                            else if (ret == 0)
                            {
                                sws_scale(
                                    sws_ctx_,
                                    (const uint8_t* const*)in_frame->data,
                                    in_frame->linesize,
                                    0,
                                    in_cdc_ctx_->height,
                                    sws_frame->data,
                                    sws_frame->linesize);

                                sws_frame->pts = in_frame->best_effort_timestamp;

                                event_->FrametData(sws_frame);

                                process_x264_encoder(sws_frame);

                                av_packet_unref(in_pkt);
                            }
                        }
                    }
                }
                else
                {
                    if (ret != AVERROR(EAGAIN))
                    {
                        run_flag_ = false;
                    }
                }
            }

            av_frame_free(&in_frame);
            av_frame_free(&sws_frame);
            av_packet_free(&in_pkt);
        }

        bool ScreenProcess::Open(PtrScreenEvent _event)
        {
            avdevice_register_all();
            if (_event == NULL || run_flag_ == true)
                return false;

            event_ = _event;
            if (false == init_screen_stream())
                return false;

            if (false == init_h264_encode())
                return false;

            //event_->HeaderInfo(out_cdc_ctx_->width, out_cdc_ctx_->height);


            run_flag_ = true;
            process_stream_thread_ = std::thread(&ScreenProcess::process_stream_function, this);

            return true;
        }
        void ScreenProcess::Close()
        {
            if (run_flag_ == false)
                return;

            run_flag_ = false;
            if (process_stream_thread_.joinable())
            {
                process_stream_thread_.join();
            }

            if (in_cdc_ctx_ != NULL)
            {
                avcodec_close(in_cdc_ctx_);
                avcodec_free_context(&in_cdc_ctx_);
                in_cdc_ctx_ = NULL;
            }


            if (sws_ctx_ != NULL)
            {
                sws_freeContext(sws_ctx_);
                sws_ctx_ = NULL;
            }

            if (ptr_encoder_ != NULL)
            {
                x264_encoder_close(ptr_encoder_);
                ptr_encoder_ = NULL;
            }

            if (ptr_in_pic_ != NULL)
            {
                x264_picture_clean(ptr_in_pic_);
                free(ptr_in_pic_);
                ptr_in_pic_ = NULL;
            }

            if (ptr_out_pic_ != NULL)
            {
                x264_picture_clean(ptr_out_pic_);
                free(ptr_out_pic_);
                ptr_out_pic_ = NULL;
            }

            if (ptr_param_ != NULL)
            {
                free(ptr_param_);
                ptr_param_ = NULL;
            }

            if (in_fmt_ctx_ != NULL)
            {
                avformat_close_input(&in_fmt_ctx_);
                in_fmt_ctx_ = NULL;
            }
        }
    }
}