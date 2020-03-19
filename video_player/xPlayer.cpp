#include "xPlayer.h"

/*******************************************VideoData*************************************************/
VData::VData()
{
    duration = -1.0;
    dpts = -1.0;
    pts = -1;

    memset(linesize, 0, sizeof(int) * X_IMG_CHANNEL_NUM);
    memset(img_data_len, 0, sizeof(int) * X_IMG_CHANNEL_NUM);
    memset(img_data, 0, sizeof(uint8_t*) * X_IMG_CHANNEL_NUM);
}
VData::~VData()
{
    Clear();
}

void VData::Clear()
{
    for (size_t i = 0; i < X_IMG_CHANNEL_NUM; i++)
    {
        if (img_data[i] != NULL)
        {
            free(img_data[i]);
        }
    }

    pts = -1;
    dpts = -1.0;

    memset(linesize, 0, sizeof(int) * X_IMG_CHANNEL_NUM);
    memset(img_data_len, 0, sizeof(int) * X_IMG_CHANNEL_NUM);
    memset(img_data, 0, sizeof(uint8_t*) * X_IMG_CHANNEL_NUM);
}
void VData::InitYUV422(AVPacket* _pkt, AVFrame* _frame, int _w, int _h)
{
    Clear();
    ////this->pts = _pkt->pts;
    //this->pts = _frame->best_effort_timestamp;
    //if (this->pts == AV_NOPTS_VALUE)
    //    this->pts = 0;

    //this->ptimestamp = this->pts * av_q2d(_rate);

    //this->dts = _pkt->dts;

    memcpy(this->linesize, _frame->linesize, sizeof(int) * X_IMG_CHANNEL_NUM);

    this->img_data_len[X_CHANNEL_1] = _w * _h;
    this->img_data[X_CHANNEL_1] = (uint8_t*)calloc(this->img_data_len[X_CHANNEL_1], sizeof(uint8_t));
    if (this->img_data[X_CHANNEL_1] != NULL)
        memcpy(this->img_data[X_CHANNEL_1], _frame->data[X_CHANNEL_1], this->img_data_len[X_CHANNEL_1] * sizeof(uint8_t));

    this->img_data_len[X_CHANNEL_2] = _w * _h / 4;
    this->img_data[X_CHANNEL_2] = (uint8_t*)calloc(this->img_data_len[X_CHANNEL_2], sizeof(uint8_t));
    if (this->img_data[X_CHANNEL_2] != NULL)
        memcpy(this->img_data[X_CHANNEL_2], _frame->data[X_CHANNEL_2], this->img_data_len[X_CHANNEL_2] * sizeof(uint8_t));

    this->img_data_len[X_CHANNEL_3] = _w * _h / 4;
    this->img_data[X_CHANNEL_3] = (uint8_t*)calloc(this->img_data_len[X_CHANNEL_3], sizeof(uint8_t));
    if (this->img_data[X_CHANNEL_3] != NULL)
        memcpy(this->img_data[X_CHANNEL_3], _frame->data[X_CHANNEL_3], this->img_data_len[X_CHANNEL_3] * sizeof(uint8_t));
}

/*******************************************VData*************************************************/



/*******************************************AudioData*************************************************/
AData::AData()
{
    pts = -1;
    dpts = -1.0;
    duration = -1.0;

    buffer = NULL;
    length = -1;
}
AData::~AData()
{
    Clear();
}

void AData::Clear()
{
    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }

    pts = -1;
    dpts = -1.0;
    length = -1;
}
void AData::InitPCM(AVPacket* _pkt, AVFrame* _frame, uint8_t* _buf, int _len)
{
    Clear();

    this->length = _len;

    this->buffer = (uint8_t*)calloc(this->length, sizeof(uint8_t));
    if (this->buffer != NULL)
        memcpy(this->buffer, _buf, this->length * sizeof(uint8_t));
}

/*******************************************AudioData*************************************************/


void xPlayer::VideoFrame(AVPacket* _pkt, AVFrame* _frame, int _w, int _h, AVPixelFormat _fmt)
{
    PtrVData data = new VData();
    data->InitYUV422(_pkt, _frame, _w, _h);
    data->pts = _frame->pts;
    if (_frame->pts == AV_NOPTS_VALUE)
        data->pts = 0;

    data->dpts = data->pts * ff_helper_.VideoInfo().pts_coefficient;
    data->duration = 1.0 / ff_helper_.VideoInfo().rate_of_stream;

    int size = 0;
    qvideo_lock_.lock();
    qvideo_.push(data);
    size = qvideo_.size();
    qvideo_lock_.unlock();

    while (size >= X_QUEUE_MAX_SIZE)
    {
        qvideo_lock_.lock();
        size = qvideo_.size();
        qvideo_lock_.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
void xPlayer::AudioFrame(AVPacket* _pkt, AVFrame* _frame, uint8_t* _buf, int _len)
{
    PtrAData data = new AData();
    data->InitPCM(_pkt, _frame, _buf, _len);

    data->pts = _frame->pkt_dts;
    if (_frame->pts == AV_NOPTS_VALUE)
        data->pts = 0;

    data->dpts = data->pts * ff_helper_.AudioInfo().pts_coefficient;
    data->duration = av_q2d(AVRational{ _frame->nb_samples, _frame->sample_rate });

    int size = 0;
    qaudio_lock_.lock();
    qaudio_.push(data);
    size = qaudio_.size();
    qaudio_lock_.unlock();

    while (size >= X_QUEUE_MAX_SIZE)
    {
        qaudio_lock_.lock();
        size = qaudio_.size();
        qaudio_lock_.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

bool xPlayer::init_sdl_audio()
{
    is_frist_audio_ = true;
    wanted_spec.freq = ff_helper_.AudioInfo().swr_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = ff_helper_.AudioInfo().dec_ctx->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = ff_helper_.AudioInfo().dec_ctx->frame_size;
    wanted_spec.userdata = this;
    wanted_spec.callback = sdl_audio_callback;

    if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
    {
        //fprintf(stderr, "SDL_OpenAudio: %s \n", SDL_GetError());
        return false;
    }
    return true;
}
bool xPlayer::init_sdl_video()
{
    sdl_screen = SDL_CreateWindow(
        "M",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        sdl_screen_wdith,
        sdl_screen_hight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (sdl_screen == NULL)
    {
        return false;
    }

    sdl_render = SDL_CreateRenderer(sdl_screen, -1, 0);
    if (sdl_render == NULL)
    {
        return false;
    }

    sdl_txture = SDL_CreateTexture(
        sdl_render,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        sdl_rect.w,
        sdl_rect.h);

    if (sdl_txture == NULL)
    {
        return false;
    }

    return true;
}
bool xPlayer::init_sdl_all()
{
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    if (false == init_sdl_audio())
        return false;

    if (false == init_sdl_video())
        return false;

    return true;
}

void xPlayer::close_sdl_all()
{
    SDL_DestroyTexture(sdl_txture);
    SDL_DestroyRenderer(sdl_render);
    SDL_DestroyWindow(sdl_screen);

    sdl_txture = NULL;
    sdl_render = NULL;
    sdl_screen = NULL;

    SDL_CloseAudio();
    SDL_Quit();
}

void xPlayer::ff_decode_function()
{
    while (run_flag_ && ff_helper_.BlockRead());
}
double xPlayer::calculate_video_delay(PtrVData _now_data, PtrVData _next_data)
{
    double _delay = 1.0 / ff_helper_.VideoInfo().rate_of_stream;

    if (_now_data == NULL || _next_data == NULL)
        return _delay;

    _delay = _next_data->dpts - _now_data->dpts;

    double video_time = audio_start_clock_+ _now_data->dpts;
    double difference = video_time - audio_clock_;

    printf("%lf\n", difference);

    if (std::abs(difference) > 1.05 * _delay)
    {
        _delay += difference * 1.02;
    }


    return _delay;
}
void xPlayer::sdl_show_yuv420(PtrVData _data)
{
    if (_data != NULL)
    {
        //printf("[VIDEO] [%ld]\n", data->pts);
        //printf("[VIDEO] [%lf]\n", _data->duration * AV_TIME_BASE);
        //SDL_UpdateTexture(sdl_txture, NULL, data->img_data[0], data->linesize[0]);

        SDL_UpdateYUVTexture(sdl_txture, NULL,
            _data->img_data[0], _data->linesize[0],
            _data->img_data[1], _data->linesize[1],
            _data->img_data[2], _data->linesize[2]);

        SDL_RenderClear(sdl_render);
        SDL_RenderCopy(sdl_render, sdl_txture, NULL, &sdl_rect);
        SDL_RenderPresent(sdl_render);
    }
}
void xPlayer::sdl_video_function()
{
    SDL_PauseAudio(0);

    double vdelay = 0.0;
    PtrVData next_data = NULL;
    PtrVData now_data = NULL;


    while (run_flag_)
    {
        if (next_data != NULL)
        {
            now_data = next_data;

            while (run_flag_ && (next_data = GetVideoData(), next_data == NULL))
                std::this_thread::sleep_for(std::chrono::microseconds(20000));

            sdl_show_yuv420(now_data);
            vdelay = calculate_video_delay(now_data, next_data) * AV_TIME_BASE;
            std::this_thread::sleep_for(std::chrono::microseconds(static_cast<long long>(vdelay)));

            delete now_data;
            now_data = NULL;
        }
        else
        {
            next_data = GetVideoData();
        }
    }

    if (next_data != NULL)
        delete next_data;
}
void sdl_audio_callback(void* userdata, Uint8* stream, int len)
{
    double audio_callback_time = av_gettime_relative();
    PtrAData data = NULL;
    xPlayer* ptr_play = (xPlayer*)userdata;

    if (ptr_play->is_frist_audio_)
    {
        ptr_play->audio_start_clock_ = av_gettime_relative() / AV_TIME_BASE;
        ptr_play->is_frist_audio_ = false;
    }


    data = ptr_play->GetAudioData();
    if (data != NULL)
    {
        //int64_t audio_callback_time = av_gettime_relative();
        //printf("[AUDIO] [%lf]\n", data->dpts);
        
        ptr_play->audio_clock_ = ptr_play->audio_start_clock_ + data->dpts;

        SDL_memset(stream, 0, data->length);
        SDL_MixAudio(stream, data->buffer, data->length, SDL_MIX_MAXVOLUME);
        delete data;
    }
    else
    {
        SDL_memset(stream, 0, len);
    }
}

xPlayer::xPlayer()
{
    is_frist_audio_ = true;
    audio_clock_ = 0.0;
    audio_start_clock_ = 0.0;

    memset(&wanted_spec, 0, sizeof(SDL_AudioSpec));

    sdl_screen = NULL;
    sdl_render = NULL;
    sdl_txture = NULL;
    memset(&sdl_rect, 0, sizeof(SDL_Rect));

    sdl_screen_wdith = 0;
    sdl_screen_hight = 0;
}
xPlayer::~xPlayer()
{
    Stop();
}

PtrAData xPlayer::GetAudioData()
{
    PtrAData data = NULL;

    qaudio_lock_.lock();
    if (qaudio_.size() > 0)
    {
        //data = qaudio_.top();
        data = qaudio_.front();
        qaudio_.pop();
    }
    qaudio_lock_.unlock();

    return data;
}
PtrVData xPlayer::GetVideoData()
{
    PtrVData data = NULL;

    qvideo_lock_.lock();
    if (qvideo_.size() > 0)
    {
        data = qvideo_.front();
        qvideo_.pop();
    }
    qvideo_lock_.unlock();

    return data;
}

bool xPlayer::Start(const char* _file)
{
    if (false == ff_helper_.Open(_file, this))
        return false;

    sdl_screen_wdith = ff_helper_.VideoInfo().sws_w;
    sdl_screen_hight = ff_helper_.VideoInfo().sws_h;

    sdl_rect.x = 0;
    sdl_rect.y = 0;
    sdl_rect.w = sdl_screen_wdith;
    sdl_rect.h = sdl_screen_hight;

    if (false == init_sdl_all())
        return false;

    run_flag_ = true;

    //start_time = av_gettime() / AV_TIME_BASE;

    decode_thread_ = std::thread(&xPlayer::ff_decode_function, this);
    event_thread_ = std::thread(&xPlayer::sdl_video_function, this);

    //refresh_thread = SDL_CreateThread(sdl_video_event_maker, "sdl_video_event_maker", this);

    return true;
}
void xPlayer::SDLEventProcess()
{
    SDL_Event ev;
    while (run_flag_)
    {
        SDL_WaitEvent(&ev);
        switch (ev.type)
        {

        case SDL_QUIT:
        {
            run_flag_ = false;
        }
        break;

        case SDL_KEYDOWN:
        {
            run_flag_ = false;
        }
        break;

        default:
            break;
        }
    }
}
bool xPlayer::Status()
{
    return run_flag_;
}
void xPlayer::Stop()
{
    run_flag_ = false;

    if (decode_thread_.joinable())
    {
        decode_thread_.join();
    }

    if (event_thread_.joinable())
    {
        event_thread_.join();
    }

    close_sdl_all();
    ff_helper_.Close();
}
