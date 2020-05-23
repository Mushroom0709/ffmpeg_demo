#ifndef _X_FFMPEG_COMMON_H_
#define _X_FFMPEG_COMMON_H_

#include <stdio.h>

extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavdevice\avdevice.h>
#include <libavformat\avformat.h>
#include <libavutil\avutil.h>
#include <libavutil\audio_fifo.h>
#include <libswscale\swscale.h>
#include <libswresample\swresample.h>
#include <libavutil\imgutils.h>
#include <libavutil\pixdesc.h>
#include <libavutil\time.h>
}

#define PrintError(_FMT_,...) printf("[ERROR] [%s:%d] ["##_FMT_##"]\n",__FILE__,__LINE__,__VA_ARGS__)
#define PrintInfo(_FMT_,...) printf("[INFO] ["##_FMT_##"]\n",__VA_ARGS__)

#endif // _X_FFMPEG_COMMON_H_