#ifndef _X_FFMPEG_DECODER_H_
#define _X_FFMPEG_DECODER_H_

#include "x_ffmpeg_common.h"

//#define X_FFMPEG_VDECODER_DST_WIDTH 1920
//#define X_FFMPEG_VDECODER_DST_HEIGHT 1200

#define X_FFMPEG_VDECODER_DST_WIDTH 1280
#define X_FFMPEG_VDECODER_DST_HEIGHT 800

#define X_FFMEPG_SCREEN_DST_WIDTH 1920
#define X_FFMEPG_SCREEN_DST_HEIGHT 1200

namespace xM
{
    namespace ffmpeg
    {
		class IVDecoderEvent
		{
		public:
			virtual void Frame(AVFrame* _frame) {}
		};

		class VDecoder
		{
		private:
			IVDecoderEvent* event_;
			AVCodecContext* cdc_ctx_;
			SwsContext* sws_ctx_;

			AVCodecParserContext* cdc_parser_ctx_;

			AVFrame* src_frame_;
			AVFrame* sws_frame_;

			AVPacket* packet_;
		public:
			VDecoder();
			~VDecoder();

		private:
			bool init_decode(AVCodecID _cdc_id = AV_CODEC_ID_H264);
			bool init_sws(int _srcW, int _srcH, AVPixelFormat _srcFormat,
				          int _dstW, int _dstH, AVPixelFormat _dstFormat);
		public:
			bool Open(IVDecoderEvent* _event);
			bool UpdataPacket(AVPacket* _pkt);
			bool UpdataBuffer(const uint8_t* _buf, const int _len);
			void Close();
		};
    }
}

# endif // _X_FFMPEG_DECODER_H_


