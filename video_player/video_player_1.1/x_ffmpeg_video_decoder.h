#ifndef _X_FFMPEG_VIDEO_DECODER_H_
#define _X_FFMPEG_VIDEO_DECODER_H_

# include "x_ffmpeg_decoder.h"

namespace x
{
    namespace ffmpeg
    {
		//视频解码器
		//大部分原理和流程都请参阅音频解码器
		class xVideoDecoder :
			public xDecoder
		{
		private:
			AVFrame* src_frame_;
			uint8_t* dst_frame_buf_;
			AVFrame* dst_frame_;
		private:
			SwsContext* sws_ctx_;	//用于图像转换
			AVPixelFormat sws_fmt_; //图像转换目标格式
			int sws_w_;	//图像转换目标宽
			int sws_h_; //图像转换目标高
		private:
			void decode_function();
		public:
			xVideoDecoder();
			~xVideoDecoder();
		public:
			// 原始图像参数 宽、高、格式
			bool GetSrcParameters(int& _w, int& _h, AVPixelFormat& _fmt);

			// 转码图像参数 宽、高、格式
			bool GetDstParameters(int& _w, int& _h, AVPixelFormat& _fmt);
		public:
			// 打开解码器
			bool Open(xDemultiplexer& _duxer);

			// 设置图像转换参数
			// @param	_dst_w		目标宽
			// @param	_dst_h		目标高
			// @param	_dst_fmt	目标格式
			// @param	scale_flag	转换算法 
			bool SetSws(int _dst_w = -1, int _dst_h = -1, AVPixelFormat _dst_fmt = AVPixelFormat::AV_PIX_FMT_NONE, int scale_flag = SWS_FAST_BILINEAR);

			// 开始解码
			bool Start(xDecoderEvent* _event);

			// 将解复用得到的音频流数据丢入队列缓冲
			bool SendPacket(AVPacket* _pkt);
			void Close();
		};
    }
}

#endif //_X_FFMPEG_VIDEO_DECODER_H_