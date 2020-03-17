#ifndef _X_FFMEPG_HELPER_H_
#define _X_FFMEPG_HELPER_H_

#include <string>
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
}

namespace x
{
    namespace xFFmpeg
    {
		class xFormatInfo
		{
		public:
			AVFormatContext* fmt_ctx;
			AVPacket* av_pkt;
			std::string file_path;
		public:
			xFormatInfo();
			~xFormatInfo();
		public:
			void Destroy();
			bool Make(const char* _file_path);
			bool Show();
		};

		class xVideoInfo
		{
		public:
			std::vector<int> stream_indexs;
			AVCodecContext* dec_ctx;
			AVCodec* dec;
			AVFrame* src_frame;
			AVFrame* dst_frame;
			uint8_t* dst_buf;
		public:
			SwsContext* sws_ctx;
			int sws_w;
			int sws_h;
			AVPixelFormat sws_fmt;
		public:
			xVideoInfo();
			~xVideoInfo();
		public:
			void Destroy();
			int GetStreamIndex(int index = 0);
			bool Make(AVFormatContext* fmt_ctx);
			bool SetScale(int dst_w = -1, int dst_h = -1, AVPixelFormat dst_fmt = AV_PIX_FMT_NONE, int scale_flag = SWS_FAST_BILINEAR);
		};

		class xAudioInfo
		{
		public:
			std::vector<int> stream_indexs;
			AVCodecContext* dec_ctx;
			AVCodec* dec;

			AVFrame* src_frame;
		public:
			SwrContext* swr_ctx;

			int64_t               swr_ch_layout;
			enum AVSampleFormat   swr_sample_fmt;
			int                   swr_sample_rate;
		public:
			xAudioInfo();
			~xAudioInfo();
		public:
			void Destroy();
			int GetStreamIndex(int index = 0);
			bool Make(AVFormatContext* fmt_ctx);
			bool SetReSample(int64_t ch_layout, enum AVSampleFormat sample_fmt, int sample_rate);
		};

		class xEvent
		{
		public:
			virtual void VideoFrame(AVPacket*, AVFrame*, int, int, AVPixelFormat) {}
			virtual void AudioFrame(AVPacket*, AVFrame*, uint8_t*, int) {}
		};

		class xHelper
		{
		private:
			xFormatInfo fmt_info_;
			xVideoInfo v_info_;
			xAudioInfo a_info_;
		private:
			xEvent* x_event;
		private:
			bool block_video_read();
			bool block_audio_read();
		public:
			xHelper();
			~xHelper();
		public:
			bool Open(const char* file_path, xEvent* ev);

			bool BlockRead();

			void Close();
		public:
			int ImgWdith();
			int ImgHeight();
			double ImgFrameRate();
		};
    }
}

#endif //_X_FFMEPG_HELPER_H_