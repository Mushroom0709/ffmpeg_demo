#ifndef _X_FRAME_H_
#define _X_FRAME_H_

#include "x_ffmpeg_common.h"

typedef class xFrame
{
public:
    double dpts;
    double duration;
    int64_t pts;

    int data_channel;

    int linesize[X_FRAME_MAX_CHANNEL];
    int data_len[X_FRAME_MAX_CHANNEL];
    uint8_t* data[X_FRAME_MAX_CHANNEL];
public:
    xFrame();
    ~xFrame();
public:
    void Clear();
    void CopyYUV(AVFrame* _data, int _w, int _h);
    void CopyPCM(uint8_t* _buf, int _len);
}*xPtrFrame;
#endif //_X_FRAME_H_