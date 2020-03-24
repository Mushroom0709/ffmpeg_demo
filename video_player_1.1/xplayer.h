#ifndef _X_PLAYER_H_
#define _X_PLAYER_H_

#include <SDL.h>

#include "x_ffmepg_demultiplexer.h"
#include "x_ffmpeg_video_decoder.h"
#include "x_ffmpeg_audio_decoder.h"
#include "x_ffmpeg_av_sync.h"

#include "xframe.h"

class xPlayer :
    public x::ffmpeg::xDemultiplexEvent,
    public x::ffmpeg::xDecoderEvent
{
private:
    volatile bool run_flag_;
private:
    SDL_AudioSpec s_aspec_;

    SDL_Window* s_win_;
    SDL_Renderer* s_render;
    SDL_Texture* s_txture;
    SDL_Rect s_rect_;

    //std::thread s_event_thread_;
    std::thread play_video_thread_;
private:
    x::ffmpeg::xAVSync av_sync_;
    x::ffmpeg::xDemultiplexer dux_;
    x::ffmpeg::xVideoDecoder vdec_;
    x::ffmpeg::xAudioDecoder adec_;
    x::xQueue<xPtrFrame> qvdata_;
    x::xQueue<xPtrFrame> qadata_;
public:
    xPlayer();
    ~xPlayer();
private:
    void DuxStart();
    void DuxPacket(AVPacket* _data, int _type);
    void DuxEnd();
private:
    void VideoEvent(AVFrame* _img, x::ffmpeg::xVideoDecoder* _decoder);
    void AudioEvent(uint8_t* _buf, int _len, int64_t _pts, x::ffmpeg::xAudioDecoder* _decoder);
private:
    void play_video_function();
    void sdl2_event_function();
private:
    bool open_ffmepg(const char* _inpu);
    bool open_sdl2();
public:
    x::xQueue<xPtrFrame>& AudioDataQueue();

    x::ffmpeg::xAVSync& AVSync();
public:
    bool Start(const char* _input);
    void Stop();
};

#endif //_X_PLAYER_H_