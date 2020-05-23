#ifndef _WORK_CENTER_H_
#define _WORK_CENTER_H_

#include "x_ffmpeg_screen_process.h"
#include "x_message_protocol.h"
#include "x_message_x264_frame.h"
#include "x_message_control.h"
#include "x_message_unpacker.h"
#include "x_socket_tcp_server.h"

class WorkCenter :
    public xM::ffmepg::ScreenEvent,
    public xM::message::ICommunication,
    public xM::message::IUnpackEvent,
    public xM::socket::ITCPServerEvent
{
private:
    clock_t time_stamp_;
    int data_size_;
    xM::ffmepg::ScreenProcess src_process_;
    xM::socket::TCPServer tcp_server_;
public:
    void x264NalData(x264_nal_t* _nal, int _nal_num);
public:
    bool ProtocolSend(const uint8_t* _buf, const int _len);
public:
    void Package(uint32_t _socket, uint32_t _msg_id, uint8_t* _buffer, size_t _length);
public:
    void* Connected(SOCKET _socket);
    void DisConnected(SOCKET _socket, void* _info, int _error);
    void Error(DWORD _error);
    void Receive(SOCKET _socket, void* _info, uint8_t* _data, int _len);
public:
    WorkCenter();
    ~WorkCenter();
public:
    bool Start();
    void Stop();
};
#endif // _WORK_CENTER_H_