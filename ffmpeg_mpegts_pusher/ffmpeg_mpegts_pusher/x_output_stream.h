#ifndef _X_OUTPUT_STREAM_H_
#define _X_OUTPUT_STREAM_H_

#include "x_stream_info.h"

class xOutputStream
{
private:
    int64_t frame_pts_;
    int64_t pkt_pts_;
    int64_t pkt_dts_;
private:
    std::string url_;
    AVFormatContext* fmt_ctx_;
    std::map<int, xOutStreamInfo> streams_;
private:
    int clock_stream_index_;
    double last_pts_;
    double last_update_time_;
public:
    xOutputStream();
    ~xOutputStream();
private:
    bool create_stream(xInStreamInfo& _in_info,xOutStreamInfo& _out_info, enum AVCodecID _codec_id);
public:
    bool Initialization(const char* _url);
    bool SetParameters(std::map<int, xInStreamInfo> _in_st, bool _dump_flg = false);
    void Destroy();
public:
    bool WriteHeader();
    bool WriteTrailer();
    bool WriteFrame(xInStreamInfo& _in_info);
};
# endif //_X_OUTPUT_STREAM_H_