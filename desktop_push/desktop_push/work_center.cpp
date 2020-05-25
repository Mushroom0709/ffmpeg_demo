#include "work_center.h"

void WorkCenter::HeaderInfo(int _pix_width, int _pix_height)
{
    xM::message::Serialize::IntegerConvertToBytes<int32_t>(_pix_width, screen_size_);
    xM::message::Serialize::IntegerConvertToBytes<int32_t>(_pix_height, screen_size_ + 4);
    screen_size_len_ = 8;
}
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
    //else
    //{
    //    PrintInfo("_nal_num:%d", _nal_num);
    //}
}

bool WorkCenter::ProtocolSendAll(const uint8_t* _buf, const int _len)
{
    std::vector<SOCKET> clis;
    {
        std::lock_guard<std::mutex> auto_lock(cli_infos_lock_);
        for (auto& item : cli_infos_)
        {
            if (item.second->StreamFlag == true)
            {
                clis.push_back(item.first);
            }
        }
    }

    for (auto item : clis)
    {
        if (false == tcp_server_.Send(item, (uint8_t*)_buf, _len))
        {
            PrintError("failed to send data to %d client", item);
        }
    }

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
    }

    return true;
}
bool WorkCenter::ProtocolSend(uint32_t _id, const uint8_t* _buf, const int _len)
{
    if (false == tcp_server_.Send(_id, (uint8_t*)_buf, _len))
    {
        PrintError("failed to send data to %d client", _id);
    }
    return true;
}

void WorkCenter::process_control_msg(uint32_t _socket, xM::message::ControlMsg& _msg)
{
    switch (_msg.Command)
    {
    case xM::message::ControlMsg::XM_CONTROL_COMMAND_START_PUSH:
    {
        std::map<SOCKET, ClientInfo*>::iterator res;
        std::lock_guard<std::mutex> auto_lock(cli_infos_lock_);
        res = cli_infos_.find(_socket);
        if (res != cli_infos_.end())
        {
            res->second->StreamFlag = true;
        }
    }
    break;

    case xM::message::ControlMsg::XM_CONTROL_COMMAND_REQ_SCREEN_SIZE:
    {
        xM::message::Protocol<WorkCenter, xM::message::ControlMsg> msg;
        msg.Msg.Command = xM::message::ControlMsg::XM_CONTROL_COMMAND_RSP_SCREEN_SIZE;
        msg.Msg.InfoLength = screen_size_len_;
        msg.Msg.Info = screen_size_;

        if (false == msg.Encode(this, _socket,false))
        {
            PrintError("xM::message::ControlMsg Encode fail");
        }
    }
    break;

    default:
        break;
    }
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
        else
        {
            process_control_msg(_socket, msg.Msg);
        }
    }
    break;
    }
}

void WorkCenter::Connected(SOCKET _socket)
{
    std::lock_guard<std::mutex> auto_lock(cli_infos_lock_);
    if (cli_infos_.find(_socket) == cli_infos_.end())
    {
        cli_infos_.insert(std::pair<SOCKET, ClientInfo*>(_socket, new ClientInfo(_socket, this)));
        PrintInfo("client\t%5d\ton line", _socket);
    }
    else
    {
        PrintError("client\t%5d\ton line fail", _socket);
    }
}
void WorkCenter::DisConnected(SOCKET _socket, int _error)
{
    std::map<SOCKET, ClientInfo*>::iterator res;
    std::lock_guard<std::mutex> auto_lock(cli_infos_lock_);
    res = cli_infos_.find(_socket);
    if (res != cli_infos_.end())
    {
        PrintInfo("client\t%5d\toff line", _socket);
        delete res->second;
        cli_infos_.erase(res);
    }
}
void WorkCenter::Error(DWORD _error)
{
    PrintInfo("error code:0x%011x", _error);
}
void WorkCenter::Receive(SOCKET _socket, uint8_t* _data, int _len)
{
    std::map<SOCKET, ClientInfo*>::iterator res;
    {
        std::lock_guard<std::mutex> auto_lock(cli_infos_lock_);
        res = cli_infos_.find(_socket);
        if (res == cli_infos_.end())
        {
            return;
        }
    }
    res->second->Unpacker.UpdateBuffer(_socket, _data, _len);
}

WorkCenter::WorkCenter()
{
    time_stamp_ = 0;
    data_size_ = 0;
    memset(screen_size_, 0, WORK_CENTER_SCREEN_SIZE_BUFFER_MAX_LEN);
    screen_size_len_ = 0;
}
WorkCenter::~WorkCenter()
{
    Stop();
}

bool WorkCenter::Start(const char* _addr_ip, const int _addr_port)
{
    if (false == tcp_server_.Start(this, _addr_ip, _addr_port))
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

    for (auto& item : cli_infos_)
    {
        delete item.second;
    }
    cli_infos_.clear();
}
