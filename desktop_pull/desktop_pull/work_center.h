#ifndef _WORK_CENTER_H_
#define _WORK_CENTER_H_

extern "C"
{
#include <SDL.h>
}

#include "x_ffmpeg_common.h"

#include "x_message_control.h"
#include "x_message_x264_frame.h"
#include "x_message_unpacker.h"

#include "x_socket_tcp_client.h"
#include "x_ffmpeg_decoder.h"

class WorkCenter :
    public xM::message::ICommunication,
    public xM::message::IUnpackEvent,
    public xM::socket::ITCPClientEvent,
    public xM::ffmpeg::IVDecoderEvent
{
private:
    SDL_Window* s_win_;
    SDL_Renderer* s_render;
    SDL_Texture* s_txture;
    SDL_Rect s_rect_;
private:
    xM::message::Unpacker unpacker_;
    xM::socket::TCPClient client_;
    xM::ffmpeg::VDecoder decoder_;
private:
    bool init_sdl2();
public:
    WorkCenter();
    ~WorkCenter();
public:
    void Package(uint32_t _socket, uint32_t _msg_id, uint8_t* _buffer, size_t _length);
public:
    void Frame(AVFrame* _frame);
public:
    void Connected();
    void DisConnected(int _error);
    void Receive(uint8_t* _data, int _len);
public:
    bool Start();
    void Wait();
    void Stop();
};


#endif