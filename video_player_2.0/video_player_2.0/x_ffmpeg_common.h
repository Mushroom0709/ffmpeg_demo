#ifndef _X_FFMPEG_COMMON_H_
#define _X_FFMPEG_COMMON_H_

#include <vector>

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


#define X_FFMPEG_STATUS_SEND_PKT 0x00
#define X_FFMPEG_STATUS_READ_FRAME 0x01

#endif // _X_FFMPEG_COMMON_H_