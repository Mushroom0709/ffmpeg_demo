#ifndef _X_FFMPEG_COMMON_H_
#define _X_FFMPEG_COMMON_H_


#include <thread>
#include <chrono>
#include <mutex>

// ffmepg是纯C库，在C++编译器下需要指定C语法编译
extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libavutil\avutil.h>
#include <libswscale\swscale.h>
#include <libswresample\swresample.h>
#include <libavutil\imgutils.h>
#include <libavutil\pixdesc.h>
#include <libavutil\time.h>
}


#define X_INT_INDEX_NONE -1
#define X_UINT_INDEX_NONE 0xFFFFFFFF

#define X_FRAME_MAX_CHANNEL 8

#define X_AVSYNC_DYNAMIC_COEFFICIENT 0.0160119  // 动态帧率算法的系数 解方程 (1+x)^6 = 1.1 即
                                                // 在相差时间(ffmepg时间) 为 6位数的时候，控制
                                                // 帧率的延时会在标准延时下增加或减少相差时间的
                                                // (1.1-1)倍

#define X_AVSYNC_DYNAMIC_THRESHOLD 0.003        // 音视频同步动态帧率进行干预的二者当前时间差的阈值

#define X_AVSYNC_SKIP_FRAME -0x1001
//#define X_AVSYNC_NOT_DELAY -0x1002

#endif //_X_FFMPEG_COMMON_H_
