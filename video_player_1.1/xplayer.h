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
    // 全局是否运行标记 用来控制线程结束
    volatile bool run_flag_;
private:
    //SDL2相关

    SDL_AudioSpec s_aspec_;
    SDL_Window* s_win_;
    SDL_Renderer* s_render;
    SDL_Texture* s_txture;
    SDL_Rect s_rect_;

    // 播放视频线程
    std::thread play_video_thread_;
private:
    x::ffmpeg::xAVSync av_sync_;    //音视频同步模块
    x::ffmpeg::xDemultiplexer dux_; //解复用器模块
    x::ffmpeg::xVideoDecoder vdec_; //视频解码器模块
    x::ffmpeg::xAudioDecoder adec_; //音频解码器模块
    x::xQueue<xPtrFrame> qvdata_;   //视频解码结果缓冲队列
    x::xQueue<xPtrFrame> qadata_;   //音频解码结果缓冲队列
public:
    xPlayer();
    ~xPlayer();
private:
    //解复用开始回调
    void DuxStart();

    //解复用返回pakcet的回调
    void DuxPacket(AVPacket* _data, int _type);

    //解复用结束
    void DuxEnd();
private:
    //解码器返回frame的回调
    void VideoEvent(AVFrame* _img, x::ffmpeg::xVideoDecoder* _decoder);
    void AudioEvent(uint8_t* _buf, int _len, int64_t _pts, x::ffmpeg::xAudioDecoder* _decoder);
private:
    //绘制图像线程
    void play_video_function();

    //SDL消息引擎
    void sdl2_event_function();
private:
    //打开ffempg相关
    bool open_ffmepg(const char* _inpu);

    //打开sdl2相关
    bool open_sdl2();
public:
    //暴露内部成员变量 音频缓冲队列 给sdl的audio回调使用
    x::xQueue<xPtrFrame>& AudioDataQueue();

    // 暴露内部成员变量 音视频同步 给sdl的audio回调使用
    x::ffmpeg::xAVSync& AVSync();
public:
    //启动player
    bool Start(const char* _input);

    //停止player并回收相关变量
    void Stop();
};

#endif //_X_PLAYER_H_