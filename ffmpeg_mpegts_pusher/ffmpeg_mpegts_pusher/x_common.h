#ifndef _X_COMMON_H_
#define _X_COMMON_H_

#include <stdio.h>

#include <string>
#include <map>

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
#define MIN_SLEEP_TIME_D_MICROSECOND 0.0001
#define ERROR_PRINTLN(_FMT_,...) printf("[ERROR] ["##_FMT_##"]\n",__VA_ARGS__)
#define INFO_PRINTLN(_FMT_,...) printf("[INFO] ["##_FMT_##"]\n",__VA_ARGS__)

#endif // _X_COMMON_H_