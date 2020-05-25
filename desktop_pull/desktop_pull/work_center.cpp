#include "work_center.h"


bool WorkCenter::init_sdl2()
{
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    s_win_ = SDL_CreateWindow(
        "M",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        X_FFMPEG_VDECODER_DST_WIDTH,
        X_FFMPEG_VDECODER_DST_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (s_win_ == NULL)
    {
        return false;
    }

    s_render = SDL_CreateRenderer(s_win_, -1, 0);
    if (s_render == NULL)
    {
        return false;
    }

    s_rect_.x = 0;
    s_rect_.y = 0;

    s_rect_.w = X_FFMPEG_VDECODER_DST_WIDTH;
    s_rect_.h = X_FFMPEG_VDECODER_DST_HEIGHT;

    s_txture = SDL_CreateTexture(
        s_render,
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        s_rect_.w,
        s_rect_.h);

    return true;
}

WorkCenter::WorkCenter() :
    unpacker_(this)
{
    s_win_ = NULL;
    s_render = NULL;
    s_txture = NULL;
    memset(&s_rect_, 0, sizeof(SDL_Rect));

    screen_width_ = -1;
    screen_height_ = -1;
}
WorkCenter::~WorkCenter()
{
    Stop();
}

void WorkCenter::process_control_msg(uint32_t _socket, xM::message::ControlMsg& _msg)
{
    switch (_msg.Command)
    {
    case xM::message::ControlMsg::XM_CONTROL_COMMAND_RSP_SCREEN_SIZE:
    {
        int temp = 0;
        xM::message::Serialize::BytesConvertToInteger<int32_t>(temp, _msg.Info);
        screen_width_ = temp;

        xM::message::Serialize::BytesConvertToInteger<int32_t>(temp, _msg.Info + 4);
        screen_height_ = temp;
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

    case xM::message::X264FrameMsg::ID:
    {
        xM::message::Protocol<WorkCenter, xM::message::X264FrameMsg> msg;

        if (false == msg.Decode(_buffer, _length))
        {
            PrintError("xM::message::X264FrameMsg decode fail");
        }
        else
        {
            for (int32_t i = 0; i < msg.Msg.group_size; i++)
            {
                decoder_.UpdataBuffer(msg.Msg.p_payload[i], msg.Msg.i_payload[i]);
            }
        }
    }
    break;
    }
}

void WorkCenter::Frame(AVFrame* _frame)
{
    SDL_UpdateYUVTexture(s_txture, NULL,
        _frame->data[0], _frame->linesize[0],
        _frame->data[1], _frame->linesize[1],
        _frame->data[2], _frame->linesize[2]);
    SDL_RenderClear(s_render);
    SDL_RenderCopy(s_render, s_txture, NULL, &s_rect_);
    SDL_RenderPresent(s_render);
}

bool WorkCenter::ProtocolSendAll(const uint8_t* _buf, const int _len)
{
    return client_.Send((const char*)_buf, _len);
}

void WorkCenter::Connected()
{

}
void WorkCenter::DisConnected(int _error)
{

}
void WorkCenter::Receive(uint8_t* _data, int _len)
{
    unpacker_.UpdateBuffer(0, _data, _len);
}

bool WorkCenter::req_screen_size()
{
    uint8_t buffer[32] = { 0 };
    sprintf_s((char*)buffer, 32,"%lld", time(NULL));
    xM::message::Protocol<WorkCenter, xM::message::ControlMsg> msg;
    msg.Msg.Command = xM::message::ControlMsg::XM_CONTROL_COMMAND_REQ_SCREEN_SIZE;
    msg.Msg.InfoLength = strlen((char*)buffer) + 1;
    msg.Msg.Info = buffer;

    if (false == msg.Encode(this))
    {
        PrintError("xM::message::ControlMsg Encode fail");
        return false;
    }

    int cnt = 0;
    while (cnt <= 10)
    {
        if (screen_height_ != -1 && screen_width_ != -1)
        {
            return true;
        }

        cnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return false;
}
bool WorkCenter::req_push_stream()
{
    uint8_t buffer[32] = { 0 };
    sprintf_s((char*)buffer, 32, "%lld", time(NULL));
    xM::message::Protocol<WorkCenter, xM::message::ControlMsg> msg;
    msg.Msg.Command = xM::message::ControlMsg::XM_CONTROL_COMMAND_START_PUSH;
    msg.Msg.InfoLength = strlen((char*)buffer) + 1;
    msg.Msg.Info = buffer;

    if (false == msg.Encode(this))
    {
        PrintError("xM::message::ControlMsg Encode fail");
        return false;
    }

    return true;
}

bool WorkCenter::Start(const char* _ip, const int _port)
{
    if (false == init_sdl2())
        return false;

    if (false == client_.Connect(this, _ip, _port))
        return false;

    if (false == req_screen_size())
        return false;

    if (false == decoder_.Open(this,screen_width_,screen_height_))
        return false;

    if (false == req_push_stream())
        return false;

    return true;
}
void WorkCenter::Wait()
{
    bool flag = true;
    SDL_Event s_event = { 0 };
    while (flag)
    {
        memset(&s_event, 0, sizeof(SDL_Event));
        SDL_PollEvent(&s_event);

        switch (s_event.type)
        {
        case SDL_QUIT:
        {
            flag = false;
        }
        break;

        case SDLK_q:
        {
            flag = false;
        }
        break;
        }
    }
}
void WorkCenter::Stop()
{
    client_.DisConnect();
    decoder_.Close();

    if (s_txture != NULL)
    {
        SDL_DestroyTexture(s_txture);
        s_txture = NULL;
    }

    if (s_render != NULL)
    {
        SDL_DestroyRenderer(s_render);
        s_render = NULL;
    }

    if (s_win_ != NULL)
    {
        SDL_DestroyWindow(s_win_);
        s_win_ = NULL;
    }

    SDL_Quit();
}
