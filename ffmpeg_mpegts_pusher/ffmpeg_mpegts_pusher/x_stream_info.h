#ifndef _X_STREAM_INFO_H_
#define _X_STREAM_INFO_H_

#include "x_common.h"

class xStreamInfo
{
public:
	AVCodec* Codec;
	AVCodecContext* CodecCtx;
	AVFrame* Frame;
	AVMediaType MediaType;

	int StreamIndex;
	AVStream* Stream;
public:
	xStreamInfo()
	{
		StreamIndex = -1;
		Stream = NULL;
		Codec = NULL;
		CodecCtx = NULL;
		Frame = NULL;
		MediaType = AVMEDIA_TYPE_UNKNOWN;
	}
};

class xInStreamInfo :
	public xStreamInfo
{

};

class xOutStreamInfo :
	public xStreamInfo
{
public:
	AVBSFContext* BSFCtx;
	SwsContext* SwsCtx;
	SwrContext* SwrCtx;

	AVFrame* Frame;
	uint8_t* FrameBuffer;

	int Swr_nb_samples;

	int64_t PtsCnt;
public:
	xOutStreamInfo():
		xStreamInfo()
	{
		SwsCtx = NULL;
		SwrCtx = NULL;
		Frame = NULL;
		FrameBuffer = NULL;
		Swr_nb_samples = 0;
		PtsCnt = 0;
		BSFCtx = NULL;
	}
};


#endif // _X_STREAM_INFO_H_