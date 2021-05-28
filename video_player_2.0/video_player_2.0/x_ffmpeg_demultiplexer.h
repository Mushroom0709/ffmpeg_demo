#ifndef _X_FFMPEG_DEMULTIPLEXER_H_
#define _X_FFMPEG_DEMULTIPLEXER_H_

#include "x_ffmpeg_common.h" 

namespace x_ffmpeg
{
	class Demultiplexer
	{
	public:
		Demultiplexer();
		~Demultiplexer();
	private:
		AVFormatContext* format_ctx_;
		std::vector<int> i_videos_;
		std::vector<int> i_audios_;
	public:
		AVFormatContext* FormatContext();
		std::vector<int>& VideoIndexs();
		std::vector<int>& AudioIndexs();
	public:
		bool Initialize(const char* _file, int& _nb_streams);
		bool GetPacket(AVPacket* _pkt, AVMediaType& _type, int& _stream_index, int& _status);
		void Destroy();
	};
}
#endif // _X_FFMPEG_DEMULTIPLEXER_H_