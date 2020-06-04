#include "xplayer.h"

void sdl_audio_callback(void* userdata, Uint8* stream, int len)
{
    xPlayer* ptr_player = (xPlayer*)userdata;

    xPtrFrame frame = NULL;

    if (true == ptr_player->AudioDataQueue().TryPop(frame))
    {
        int write_len = len < frame->data_len[0] ? len : frame->data_len[0];

        SDL_memset(stream, 0, len);
        SDL_MixAudio(stream, frame->data[0], write_len, SDL_MIX_MAXVOLUME);
        ptr_player->AVSync().SetAudioClock(frame->dpts);
        delete frame;
    }
    else
    {
        //如果没有声音缓冲，则静音
        SDL_memset(stream, 0, len);
    }
}

xPlayer::xPlayer()
{
    memset(&s_aspec_, 0, sizeof(SDL_AudioSpec));
    memset(&s_rect_, 0, sizeof(SDL_Rect));
    run_flag_ = false;
    s_win_ = NULL;
    s_render = NULL;
    s_txture = NULL;
}
xPlayer::~xPlayer()
{
    Stop();
}

void xPlayer::DuxStart()
{
    //
}
void xPlayer::DuxPacket(AVPacket* _data, int _type)
{
    switch (_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        vdec_.SendPacket(_data);
        break;
    case AVMEDIA_TYPE_AUDIO:
        adec_.SendPacket(_data);
        break;
    }
}
void xPlayer::DuxEnd()
{
    AVPacket* nul_pkt = av_packet_alloc();
    nul_pkt->data = NULL;
    nul_pkt->pts = -1;

    vdec_.SendPacket(nul_pkt);

    //nul_pkt = av_packet_alloc();
    //nul_pkt->data = NULL;
    //nul_pkt->pts = -1;

}


void xPlayer::VideoEvent(AVFrame* _img, x::ffmpeg::xVideoDecoder* _decoder)
{
    int w = 0, h = 0;
    enum AVPixelFormat fmt = AVPixelFormat::AV_PIX_FMT_NONE;

    _decoder->GetDstParameters(w, h, fmt);
    xPtrFrame frame = new xFrame();
    frame->CopyYUV(_img, w, h);
    frame->pts = _img->best_effort_timestamp;
    frame->dpts = _img->pts * _decoder->GetTimebase();
    frame->duration = 1.0 / _decoder->GetRate();

    qvdata_.MaxSziePush(frame,&run_flag_);
}
void xPlayer::AudioEvent(uint8_t* _buf, int _len, int64_t _pts, x::ffmpeg::xAudioDecoder* _decoder)
{
    xPtrFrame frame = new xFrame();
    frame->CopyPCM(_buf, _len);

    frame->pts = _pts;
    frame->dpts = _pts * _decoder->GetTimebase();
    frame->duration = 1.0 / _decoder->GetRate();

    qadata_.MaxSziePush(frame,&run_flag_);
}

void xPlayer::sdl2_event_function()
{
    SDL_Event s_event = { 0 };
    while (run_flag_)
    {
        memset(&s_event, 0, sizeof(SDL_Event));
        SDL_WaitEventTimeout(&s_event,50);

        switch (s_event.type)
        {
        case SDL_QUIT:
        {
            run_flag_ = false;
        }
        break;

        default:
            break;
        }
    }
}
void xPlayer::play_video_function()
{
    xPtrFrame frame = NULL;
    int64_t delay = 0;

    while (run_flag_)
    {
        if (true == qvdata_.TryPop(frame))
        {
            SDL_UpdateYUVTexture(s_txture, NULL,
                frame->data[0], frame->linesize[0],
                frame->data[1], frame->linesize[1],
                frame->data[2], frame->linesize[2]);
            SDL_RenderClear(s_render);
            SDL_RenderCopy(s_render, s_txture, NULL, &s_rect_);

            delay = av_sync_.CalDelay(frame->dpts);
            if (delay > 0)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(delay));

                //std::this_thread::sleep_for(std::chrono::microseconds(delay+rand()%40000)); //用作音视频同步测试
            }
            else if (X_AVSYNC_SKIP_FRAME == delay)
            {
                //
            }

            //printf("%I64d\n", delay);

            SDL_RenderPresent(s_render);
            av_sync_.SetVideoShowTime();


            delete frame;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

bool xPlayer::open_ffmepg(const char* _inpu)
{
    int w = 0, h = 0;
    AVPixelFormat fmt = AVPixelFormat::AV_PIX_FMT_NONE;

    int _sample_rate = -1;
    int _nb_samples = -1;
    int64_t ch_layout = -1;
    enum AVSampleFormat _sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NONE;

    if (false == dux_.Open(_inpu))
        return false;

    if (false == dux_.Start(this))
        return false;

    if (false == vdec_.Open(dux_))
        return false;

    vdec_.GetSrcParameters(w, h, fmt);

    //if (false == vdec_.SetSws(w, h, fmt))
    //    return false;

    if (false == vdec_.SetSws(1280, 720, fmt))
        return false;

    if (false == vdec_.Start(this))
        return false;

    if (false == adec_.Open(dux_))
        return false;

    adec_.GetSrcParameters(_sample_rate, _nb_samples, ch_layout, _sample_fmt);

    if (false == adec_.SetSwr(ch_layout, AVSampleFormat::AV_SAMPLE_FMT_S16, _sample_rate))
        return false;

    //if (false == adec_.SetSwr(ch_layout, AVSampleFormat::AV_SAMPLE_FMT_S16, 22050))
    //    return false;

    if (false == adec_.Start(this))
        return false;

    //av_sync_.SetAverageDelay(1.0 / vdec_.GetRate());

    return true;
}

bool xPlayer::open_sdl2()
{
    int w = 0, h = 0;
    AVPixelFormat fmt = AVPixelFormat::AV_PIX_FMT_NONE;

    int _sample_rate = -1;
    int _nb_samples = -1;
    int _channels = -1;
    enum AVSampleFormat _sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_NONE;

    vdec_.GetDstParameters(w, h, fmt);
    adec_.GetDstParameters(_sample_rate, _nb_samples, _channels, _sample_fmt);


    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    s_win_ = SDL_CreateWindow(
        "M",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        w,
        h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (s_win_ == NULL)
    {
        return false;
    }

    s_render = SDL_CreateRenderer(s_win_, -1, 0);
    if (s_render == NULL)
    {
        return false;
    }

    s_rect_.x = 0;
    s_rect_.y = 0;

    s_rect_.w = w;
    s_rect_.h = h;

    s_txture = SDL_CreateTexture(
        s_render,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        s_rect_.w,
        s_rect_.h);


    s_aspec_.freq = _sample_rate;
    s_aspec_.format = AUDIO_S16SYS;
    s_aspec_.channels = _channels;
    s_aspec_.silence = 0;
    s_aspec_.samples = _nb_samples;
    s_aspec_.userdata = this;
    s_aspec_.callback = sdl_audio_callback;//此函数会在需要填入新的缓冲数据时由SDL2回调

    if (SDL_OpenAudio(&s_aspec_, NULL) < 0)
    {
        //fprintf(stderr, "SDL_OpenAudio: %s \n", SDL_GetError());
        return false;
    }

    SDL_PauseAudio(0);

    return true;
}

x::xQueue<xPtrFrame>& xPlayer::AudioDataQueue()
{
    return qadata_;
}
x::ffmpeg::xAVSync& xPlayer::AVSync()
{
    return av_sync_;
}

bool xPlayer::Start(const char* _input)
{
    if (run_flag_ == true)
        return false;

    if (false == open_ffmepg(_input))
        return false;

    if (false == open_sdl2())
        return false;

    run_flag_ = true;
    //s_event_thread_ = std::thread(&xPlayer::sdl2_event_function, this);
    play_video_thread_ = std::thread(&xPlayer::play_video_function, this);

    sdl2_event_function();
    return true;
}
void xPlayer::Stop()
{
    run_flag_ = false;

    //if (s_event_thread_.joinable())
    //    s_event_thread_.join();

    if (play_video_thread_.joinable())
        play_video_thread_.join();

    dux_.Close();
    vdec_.Close();
    adec_.Close();

    if (s_txture != NULL)
    {
        SDL_DestroyTexture(s_txture);
        s_txture = NULL;
    }

    if (s_render != NULL)
    {
        SDL_DestroyRenderer(s_render);
        s_render = NULL;
    }

    if (s_win_ != NULL)
    {
        SDL_DestroyWindow(s_win_);
        s_win_ = NULL;
    }

    SDL_Quit();
}


/*
play_audio_function()
{
    ....
    音频pts = frame.pts;
    音频起始播放时间 = 当前时间();
}


同步()
{
    当前时间 = 当前时间();

    audio_clock = 音频pts + (当前时间-音频起始播放时间);
}
*/