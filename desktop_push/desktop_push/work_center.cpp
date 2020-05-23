#include "work_center.h"

void WorkCenter::x264NalData(x264_nal_t* _nal, int _nal_num)
{
    xM::message::Protocol<WorkCenter, xM::message::X264FrameMsg> msg;

    msg.Msg.Alloc(_nal_num);

    for (int i = 0; i < _nal_num; i++)
    {
        msg.Msg.i_payload[i] = _nal[i].i_payload;
        msg.Msg.p_payload[i] = _nal[i].p_payload;
    }

    if (false == msg.Encode(this))
    {
        PrintError("xM::message::X264FrameMsg encode fail");
    }
    else
    {
        PrintInfo("_nal_num:%d", _nal_num);
    }
}

bool WorkCenter::ProtocolSend(const uint8_t* _buf, const int _len)
{
    if (false == tcp_server_.SendAll((uint8_t*)_buf, _len))
    {
        PrintError("failed to send data to all client");
        return false;
    }
    else
    {
        data_size_ += _len;
        if (time_stamp_ == 0)
        {
            time_stamp_ = clock();
            data_size_ = 0;
        }
        else
        {
            clock_t timespan = clock() - time_stamp_;

            if (timespan >= 1000)
            {
                time_stamp_ += timespan;
                PrintInfo("send speed:%.2lf Mbps", (data_size_ * 1.0) / (timespan / 1000.0) / (1024.0 * 1024.0 / 8.0));
                data_size_ = 0;
            }
        }
        //PrintInfo("send data:%d", _len);
    }
    return true;
}

void WorkCenter::Package(uint32_t _socket, uint32_t _msg_id, uint8_t* _buffer, size_t _length)
{
    switch (_msg_id)
    {
    case xM::message::ControlMsg::ID:
    {
        xM::message::Protocol<WorkCenter, xM::message::ControlMsg> msg;
        if (false == msg.Decode(_buffer, _length))
        {
            PrintError("xM::message::ControlMsg decode fail");
        }
    }
    break;
    }
}

void* WorkCenter::Connected(SOCKET _socket)
{
    PrintInfo("client\t%5d\ton line", _socket);
    return new xM::message::Unpacker(this);
}
void WorkCenter::DisConnected(SOCKET _socket, void* _info, int _error)
{
    xM::message::Unpacker* ptr = (xM::message::Unpacker*)_info;
    delete ptr;
    PrintInfo("client\t%5d\toff line", _socket);
}
void WorkCenter::Error(DWORD _error)
{
    PrintInfo("error code:0x%011x", _error);
}
void WorkCenter::Receive(SOCKET _socket, void* _info, uint8_t* _data, int _len)
{
    xM::message::Unpacker* ptr = (xM::message::Unpacker*)_info;
    ptr->UpdateBuffer(_socket, _data, _len);
}

WorkCenter::WorkCenter()
{
    time_stamp_ = 0;
    data_size_ = 0;
}
WorkCenter::~WorkCenter()
{
    Stop();
}

bool WorkCenter::Start()
{
    if (false == tcp_server_.Start(this, "127.0.0.2", 50510))
    {
        PrintError("failed to start server");
        return false;
    }
    //getchar();
    if (false == src_process_.Open(this))
    {
        PrintError("failed to open video stream");
        return false;
    }

    return true;
}
void WorkCenter::Stop()
{
    src_process_.Close();
    tcp_server_.Stop();
}
