#ifndef _X_HELPER_H_
#define _X_HELPER_H_

extern "C"
{
#include <SDL.h>
}

#include <thread>
#include <chrono>
#include <queue>
#include <mutex>



#include "x_ffmepg_helper.h"

#define X_SDL_EVENT_VIDEO (SDL_USEREVENT + 1)
#define X_SDL_EVENT_AUDIO (SDL_USEREVENT + 2)

#ifndef X_IMG_DEFINE
#define X_IMG_DEFINE

#define X_IMG_CHANNEL_NUM 3
#define X_CHANNEL_1 0
#define X_CHANNEL_2 1
#define X_CHANNEL_3 2
#define X_CHANNEL_4 3
#define X_CHANNEL_5 4
#define X_CHANNEL_6 5
#define X_CHANNEL_7 6
#define X_CHANNEL_8 7

#endif //X_IMG_DEFINE

typedef class VData
{
public:
    int64_t pts;
    int64_t dts;

    int linesize[X_IMG_CHANNEL_NUM];
    int img_data_len[X_IMG_CHANNEL_NUM];
    uint8_t* img_data[X_IMG_CHANNEL_NUM];
public:
    VData();
    ~VData();
public:
    void Clear();
    void InitYUV422(AVPacket* _pkt, AVFrame* _frame, int _w, int _h);
public:
    bool operator() (VData* a, VData* b);
}*PtrVData;
typedef class AData
{
public:
    int64_t pts;
    int64_t dts;

    uint8_t* buffer;
    int length;
public:
    AData();
    ~AData();
public:
    void Clear();
    void InitPCM(AVPacket* _pkt, AVFrame* _frame, uint8_t* _buf, int _len);
public:
    bool operator() (AData* a, AData* b);
}*PtrAData;

void sdl_audio_callback(void* userdata, Uint8* stream, int len);

class xPlayer :
    public x::xFFmpeg::xEvent
{
private:
    SDL_AudioSpec wanted_spec;

    SDL_Window* sdl_screen;
    SDL_Renderer* sdl_render;
    SDL_Texture* sdl_txture;
    SDL_Rect sdl_rect;

    //SDL_Thread* refresh_thread;

    int sdl_screen_wdith;
    int sdl_screen_hight;
private:
    x::xFFmpeg::xHelper ff_helper_;
private:
    std::thread decode_thread_;
    std::thread event_thread_;

    volatile bool run_flag_;
private:
    //std::priority_queue<PtrVData, std::vector<PtrVData>, VData> qvideo_;
    std::queue<PtrVData> qvideo_;
    std::mutex qvideo_lock_;

    //std::priority_queue<PtrAData, std::vector<PtrAData>, AData> qaudio_;
    std::queue<PtrAData> qaudio_;
    std::mutex qaudio_lock_;
private:
    void VideoFrame(AVPacket* _pkt, AVFrame* _frame, int _w, int _h, AVPixelFormat _fmt);
    void AudioFrame(AVPacket* _pkt, AVFrame* _frame, uint8_t* _buf, int _len);
private:
    bool init_sdl_audio();
    bool init_sdl_video();
    bool init_sdl_all();
    void close_sdl_all();
private:
    void sdl_event_maker();
    void ff_decode_function();
public:
    xPlayer();
    ~xPlayer();
public:
    PtrAData GetAudioData();
public:
    bool Start(const char* _file);
    void SDLEventProcess();
    bool Status();
    bool ImgFrameRate();
    void Stop();
};

#endif //_X_HELPER_H_