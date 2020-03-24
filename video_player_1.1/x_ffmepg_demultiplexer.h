#ifndef _X_FFMEPG_DEMULTIPLEXER_H_
#define _X_FFMEPG_DEMULTIPLEXER_H_

#include "x_ffmpeg_common.h"

namespace x
{
    namespace ffmpeg
    {
		class xDemultiplexEvent
		{
		public:
			virtual void DuxStart() {};
			virtual void DuxPacket(AVPacket* _data, int _type) {};
			virtual void DuxEnd() {};
		};

		class xDemultiplexer
		{
		private:
			AVFormatContext* av_fmt_ctx_;
			volatile bool run_flag_;
			std::thread dux_thread_;
			xDemultiplexEvent* event_;

			int vs_index_;
			int as_index_;
		public:
			xDemultiplexer();
			~xDemultiplexer();
		private:
			void dux_function();
		public:
			bool Open(const char* _input);
			bool Start(xDemultiplexEvent* _event);

			bool Info();
			int AudioStreamIndex();
			int VideoStreamIndex();
			AVFormatContext* FormatContext();

			void Close();
		};
    }
}

#endif //_X_FFMEPG_DEMULTIPLEXER_H_