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
    int screen_width_;
    int screen_height_;
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
private:
    void process_control_msg(uint32_t _socket, xM::message::ControlMsg& _msg);
public:
    void Frame(AVFrame* _frame);
public:
    bool ProtocolSendAll(const uint8_t* _buf, const int _len);
public:
    void Connected();
    void DisConnected(int _error);
    void Receive(uint8_t* _data, int _len);
private:
    bool req_screen_size();
    bool req_push_stream();
public:
    bool Start(const char* _ip, const int _port);
    void Wait();
    void Stop();
};


#endif