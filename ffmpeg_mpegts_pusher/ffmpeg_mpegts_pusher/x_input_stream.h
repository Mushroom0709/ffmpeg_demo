#ifndef _X_INPUT_STREAM_H_
#define _X_INPUT_STREAM_H_

# include "x_stream_info.h"

class xOutputStream;
class xInputStream
{
private:
    AVFormatContext* fmt_ctx_;
    std::map<int, xInStreamInfo> streams_;
public:
    AVFormatContext* GetFmtCtx();
    std::map<int, xInStreamInfo>& GetStreams();
public:
    xInputStream();
    ~xInputStream();
private:
    bool block_read_frame(AVPacket* _pkt, xInStreamInfo& _in_st, xOutputStream* _output);
public:
    bool OpenScreen(bool _dump_flg = false);
    bool OpenFile(const char* _input, bool _dump_flg = false);
    void BlockRead(xOutputStream* _output);
public:
    void Destroy();
};

#endif // _X_INPUT_STREAM_H_