#ifndef _X_FFMPEG_AUDIO_DECODER_H_
#define _X_FFMPEG_AUDIO_DECODER_H_

# include "x_ffmpeg_decoder.h"

namespace x
{
    namespace ffmpeg
    {
		class xAudioDecoder:
			public x::ffmpeg::xDecoder
		{
		private:
			AVFrame* src_frame_;
		private:
			SwrContext*			swr_ctx_;
			int64_t				swr_ch_layout_;
			enum AVSampleFormat	swr_sample_fmt_;
			int					swr_sample_rate_;
			int					swr_nb_samples_;
		private:
			void decode_function();
		public:
			xAudioDecoder();
			~xAudioDecoder();
		public:
			bool GetSrcParameters(int& _sample_rate, int& _nb_samples, int64_t& _ch_layout, enum AVSampleFormat& _sample_fmt);
			bool GetDstParameters(int& _sample_rate, int& _nb_samples, int& _channels, enum AVSampleFormat& _sample_fmt);
		public:
			bool Open(xDemultiplexer& _duxer);
			bool SetSwr(int64_t _ch_layout, enum AVSampleFormat _sample_fmt, int _sample_rate);
			bool Start(xDecoderEvent* _event);
			bool SendPacket(AVPacket* _pkt);
			void Close();
		};
    }
}

#endif //_X_FFMPEG_AUDIO_DECODER_H_