#include "xPlayer.h"

/*******************************************VideoData*************************************************/
VData::VData()
{
    pts = -1;
    dts = -1;

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
    dts = -1;

    memset(linesize, 0, sizeof(int) * X_IMG_CHANNEL_NUM);
    memset(img_data_len, 0, sizeof(int) * X_IMG_CHANNEL_NUM);
    memset(img_data, 0, sizeof(uint8_t*) * X_IMG_CHANNEL_NUM);
}
void VData::InitYUV422(AVPacket* _pkt, AVFrame* _frame, int _w, int _h)
{
    Clear();

    this->pts = _pkt->pts;
    this->dts = _pkt->dts;

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

bool VData::operator() (VData* a, VData* b)
{
    return a->pts > b->pts; //Ð¡¶¥¶Ñ
}
/*******************************************VData*************************************************/



/*******************************************AudioData*************************************************/
AData::AData()
{
    pts = -1;
    dts = -1;

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
    dts = -1;
    length = -1;
}
void AData::InitPCM(AVPacket* _pkt, AVFrame* _frame, uint8_t* _buf, int _len)
{
    Clear();

    this->dts = _pkt->dts;
    this->pts = _pkt->pts;


    this->length = _len;

    this->buffer = (uint8_t*)calloc(this->length, sizeof(uint8_t));
    if (this->buffer != NULL)
        memcpy(this->buffer, _buf, this->length * sizeof(uint8_t));
}

bool AData::operator() (AData* a, AData* b)
{
    return a->pts > b->pts; //Ð¡¶¥¶Ñ
}
/*******************************************AudioData*************************************************/



void sdl_audio_callback(void* userdata, Uint8* stream, int len)
{
    PtrAData audio_data = NULL;
    xPlayer* ptr_play = (xPlayer*)userdata;
    audio_data = ptr_play->GetAudioData();
    if (audio_data != NULL)
    {
        SDL_memset(stream, 0, audio_data->length);
        SDL_MixAudio(stream, audio_data->buffer, audio_data->length, SDL_MIX_MAXVOLUME);
        delete audio_data;
    }
    else
    {
        SDL_memset(stream, 0, len);
    }
}
//int sdl_video_event_maker(void* arg)
//{
//    xPlayer* ptr_play = (xPlayer*)arg;
//
//    SDL_Event event = { 0 };
//    event.type = X_SDL_EVENT_VIDEO;
//    int wait_time = (int)(1000.0 / ptr_play->ImgFrameRate());
//    while (ptr_play->Status())
//    {
//        int x = SDL_PushEvent(&event);
//
//        SDL_Delay(wait_time);
//    }
//
//    return 0;
//}


void xPlayer::VideoFrame(AVPacket* _pkt, AVFrame* _frame, int _w, int _h, AVPixelFormat _fmt)
{
    PtrVData data = new VData();
    data->InitYUV422(_pkt, _frame, _w, _h);

    int size = 0;
    qvideo_lock_.lock();
    qvideo_.push(data);
    size = qvideo_.size();
    qvideo_lock_.unlock();


    while (size >= 100)
    {
        qvideo_lock_.lock();
        size = qvideo_.size();
        qvideo_lock_.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
void xPlayer::AudioFrame(AVPacket* _pkt, AVFrame* _frame, uint8_t* _buf, int _len)
{
    PtrAData data = new AData();
    data->InitPCM(_pkt, _frame, _buf, _len);

    int size = 0;
    qaudio_lock_.lock();
    qaudio_.push(data);
    size = qaudio_.size();
    qaudio_lock_.unlock();

    while (size >= 100)
    {
        qaudio_lock_.lock();
        size = qaudio_.size();
        qaudio_lock_.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

bool xPlayer::init_sdl_audio()
{
    wanted_spec.freq = 44100;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = 2;
    wanted_spec.silence = 0;
    wanted_spec.samples = 1024;
    wanted_spec.size = 4096;
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
    while (ff_helper_.BlockRead() && run_flag_);
}
void xPlayer::sdl_event_maker()
{
    SDL_Event event = { 0 };
    event.type = X_SDL_EVENT_VIDEO;
    int wait_time = (int)(1000.0 / ff_helper_.ImgFrameRate());

    SDL_PauseAudio(0);

    while (run_flag_)
    {
        int x = SDL_PushEvent(&event);

        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
    }
}

xPlayer::xPlayer()
{
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

bool xPlayer::Start(const char* _file)
{
    if (false == ff_helper_.Open(_file, this))
        return false;

    sdl_screen_wdith = ff_helper_.ImgWdith();
    sdl_screen_hight = ff_helper_.ImgHeight();

    sdl_rect.x = 0;
    sdl_rect.y = 0;
    sdl_rect.w = sdl_screen_wdith;
    sdl_rect.h = sdl_screen_hight;

    if (false == init_sdl_all())
        return false;

    run_flag_ = true;

    decode_thread_ = std::thread(&xPlayer::ff_decode_function, this);
    event_thread_ = std::thread(&xPlayer::sdl_event_maker, this);

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

        case X_SDL_EVENT_VIDEO:
        {
            PtrVData data = NULL;

            qvideo_lock_.lock();
            if (qvideo_.size() > 0)
            {
                //data = qvideo_.top();
                data = qvideo_.front();
                qvideo_.pop();
            }
            qvideo_lock_.unlock();


            if (data != NULL)
            {
                //printf("[VIDEO] [%ld]\n", data->pts);
                //SDL_UpdateTexture(sdl_txture, NULL, data->img_data[0], data->linesize[0]);
                SDL_UpdateYUVTexture(sdl_txture, NULL,
                    data->img_data[0], data->linesize[0],
                    data->img_data[1], data->linesize[1],
                    data->img_data[2], data->linesize[2]);

                SDL_RenderClear(sdl_render);
                SDL_RenderCopy(sdl_render, sdl_txture, NULL, &sdl_rect);
                SDL_RenderPresent(sdl_render);

                delete data;
            }
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
bool xPlayer::ImgFrameRate()
{
    return ff_helper_.ImgFrameRate();
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
