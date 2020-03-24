#ifndef _X_FFMPEG_VIDEO_DECODER_H_
#define _X_FFMPEG_VIDEO_DECODER_H_

# include "x_ffmpeg_decoder.h"

namespace x
{
    namespace ffmpeg
    {
		class xVideoDecoder :
			public xDecoder
		{
		private:
			AVFrame* src_frame_;
			uint8_t* dst_frame_buf_;
			AVFrame* dst_frame_;
		private:
			SwsContext* sws_ctx_;
			AVPixelFormat sws_fmt_;
			int sws_w_;
			int sws_h_;
		private:
			void decode_function();
		public:
			xVideoDecoder();
			~xVideoDecoder();
		public:
			bool GetSrcParameters(int& _w, int& _h, AVPixelFormat& _fmt);
			bool GetDstParameters(int& _w, int& _h, AVPixelFormat& _fmt);
		public:
			bool Open(xDemultiplexer& _duxer);
			bool SetSws(int _dst_w = -1, int _dst_h = -1, AVPixelFormat _dst_fmt = AVPixelFormat::AV_PIX_FMT_NONE, int scale_flag = SWS_FAST_BILINEAR);
			bool Start(xDecoderEvent* _event);
			bool SendPacket(AVPacket* _pkt);
			void Close();
		};
    }
}

#endif //_X_FFMPEG_VIDEO_DECODER_H_