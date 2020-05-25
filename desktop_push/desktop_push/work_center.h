#ifndef _WORK_CENTER_H_
#define _WORK_CENTER_H_

#include "x_ffmpeg_screen_process.h"
#include "x_message_protocol.h"
#include "x_message_x264_frame.h"
#include "x_message_control.h"
#include "x_message_unpacker.h"
#include "x_socket_tcp_server.h"

#define WORK_CENTER_SCREEN_SIZE_BUFFER_MAX_LEN 12
class ClientInfo
{
public:
    SOCKET ID;
    xM::message::Unpacker Unpacker;
    bool StreamFlag;
public:
    ClientInfo() = delete;
    ClientInfo(SOCKET _id, xM::message::IUnpackEvent* _event):
        Unpacker(_event)
    {
        ID = _id;
        StreamFlag = false;
    }
    ~ClientInfo()
    {
        //
    }
};

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
private:
    uint8_t screen_size_[WORK_CENTER_SCREEN_SIZE_BUFFER_MAX_LEN];
    int screen_size_len_;
private:
    std::map<SOCKET, ClientInfo*> cli_infos_;
    std::mutex cli_infos_lock_;
public:
    void HeaderInfo(int _pix_width, int _pix_height);
    void x264NalData(x264_nal_t* _nal, int _nal_num);
public:
    bool ProtocolSendAll(const uint8_t* _buf, const int _len);
    bool ProtocolSend(uint32_t _id, const uint8_t* _buf, const int _len);
public:
    void Package(uint32_t _socket, uint32_t _msg_id, uint8_t* _buffer, size_t _length);
private:
    void process_control_msg(uint32_t _socket, xM::message::ControlMsg& _msg);
public:
    void Connected(SOCKET _socket);
    void DisConnected(SOCKET _socket, int _error);
    void Error(DWORD _error);
    void Receive(SOCKET _socket, uint8_t* _data, int _len);
public:
    WorkCenter();
    ~WorkCenter();
public:
    bool Start(const char* _addr_ip, const int _addr_port);
    void Stop();
};
#endif // _WORK_CENTER_H_