#ifndef _X_FFMEPG_DEMULTIPLEXER_H_
#define _X_FFMEPG_DEMULTIPLEXER_H_

#include "x_ffmpeg_common.h"

namespace x
{
    namespace ffmpeg
    {
		// 解复用相关回调接口
		class xDemultiplexEvent
		{
		public:
			virtual void DuxStart() {};
			virtual void DuxPacket(AVPacket* _data, int _type) {};
			virtual void DuxEnd() {};
		};

		// 解复用器
		class xDemultiplexer
		{
		private:
			AVFormatContext* av_fmt_ctx_;	// 这玩意我也不知道叫什么好，官方解释叫 Format I/O context
											// 通过这个结构体可以拿到 码流(文件)的详细信息(像视频长度啦，帧率啦，音频采样率啦，等等)
			volatile bool run_flag_;
			std::thread dux_thread_;		// 解复用线程
			xDemultiplexEvent* event_;

			int vs_index_;	// 视频流 在AVFormatContext::streams中的索引 一般视频是 0
			int as_index_;	// 音频流 在AVFormatContext::streams中的索引 一般视频是 1
		public:
			xDemultiplexer();
			~xDemultiplexer();
		private:
			void dux_function();
		public:
			// 打开并初始化解复用器相关
			bool Open(const char* _input);

			// 开始解复用
			bool Start(xDemultiplexEvent* _event);

			// 在控制台打印出该码流(文件)的基本信息
			bool Info();

			// 返回 视频流 在AVFormatContext::streams中的索引
			int AudioStreamIndex();

			// 返回 视频流 在AVFormatContext::streams中的索引
			int VideoStreamIndex();

			// 返回 AVFormatContext
			AVFormatContext* FormatContext();

			// 关闭解复用器，并释放资源
			void Close();
		};
    }
}

#endif //_X_FFMEPG_DEMULTIPLEXER_H_