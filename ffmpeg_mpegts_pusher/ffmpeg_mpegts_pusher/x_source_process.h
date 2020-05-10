#ifndef _X_SOURCE_PROCESS_H_
#define _X_SOURCE_PROCESS_H_

#include "x_common.h"

class xTargetProcess;

class xSourceProcess
{
private:
    AVFormatContext* src_fmt_ctx_;
    std::map<int, AVMediaType> stream_index_;
public:
    AVFormatContext* FormatContext();
    std::map<int, AVMediaType>& StreamIndex();
public:
    xSourceProcess();
    ~xSourceProcess();
public:
    bool OpenSource(const char* _input, bool _dump_flg = false);
    void BlockReadPacket(xTargetProcess* _ptr_tgt);
    void Destroy();
};

#endif //_X_SOURCE_PROCESS_H_