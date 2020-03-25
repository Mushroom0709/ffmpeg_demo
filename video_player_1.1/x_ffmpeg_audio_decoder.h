#ifndef _X_FFMPEG_AUDIO_DECODER_H_
#define _X_FFMPEG_AUDIO_DECODER_H_

# include "x_ffmpeg_decoder.h"
/*
常用 专业词语 解释
1.采样率
	采样频率，也称为采样速度或者采样率，定义了每秒从
	连续信号中提取并组成离散信号的采样个数，它用赫兹（Hz）
	来表示。采样频率的倒数是采样周期或者叫作采样时间，
	它是采样之间的时间间隔。通俗的讲采样频率是指计算
	机每秒钟采集多少个信号样本。

	常见采样率有:
	22050Hz 无线电广播所用采样率，广播音质
	44100Hz 音频CD，也常用于MPEG-1音频（VCD，SVCD，MP3）所用采样率
	48000Hz miniDV、数字电视、DVD、DAT、电影和专业音频所用的数字声音所用采样率

2.声道布局和声道数
	声道布局指 channel_layout.h 定义的相关方位的声道
	AV_CH_FRONT_LEFT	左声道
	AV_CH_LAYOUT_STEREO(AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT)	双声道(左声道&右声道)

	声道数 就是有几个声道
	双声道 2 个

3.音频的采样格式
	采样格式指 PCM数据的采样精度和存储方式(注意，aac等是压缩算法压缩后的PCM数据，不是音频采样格式)
	参阅enum AVSampleFormat。

	采样精度是单位时间内对物理上的一段连续的声波模拟量的量化(数字化)的处理精度，如:
	AV_SAMPLE_FMT_S16，表示单个声道单次采样使用 signed 16 bits 来存储。精度为[-32768,32767]
	AV_SAMPLE_FMT_FLT，表示单个声道单次采样使用 float 来存储。精度为[-1.0,1.0]

	音频的采样格式分为平面（planar）和打包（packed）两种类型，在枚举值中上半部分是packed
	类型，后面（有P后缀的）是planar类型。

	以双声道为例(L&R)
	平面 即不同声道数据在AVFrame::data[0]中交错存储，LRLRLRLRLRLRLRL......

	打包 即数据在AVFrame::data[声道]中单独存储，
	AVFrame::data[0] LLLLLLLLLLL....
	AVFrame::data[1] RRRRRRRRRRR....

*/
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
			SwrContext*			swr_ctx_;			//用于音频各种转换
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
			// 获取原始音频流(PCM) 的采样率、单帧单声道采样数
			bool GetSrcParameters(int& _sample_rate, int& _nb_samples, int64_t& _ch_layout, enum AVSampleFormat& _sample_fmt);

			// 获取重采样音频流(PCM) 的采样率、单帧单声道采样数
			bool GetDstParameters(int& _sample_rate, int& _nb_samples, int& _channels, enum AVSampleFormat& _sample_fmt);
		public:
			bool Open(xDemultiplexer& _duxer);


			// 设置重采样参数 
			// @param	_ch_layout		声道布局
			// @param	_sample_fmt		重采样格式
			// @param	_sample_rate	重采样采样率
			bool SetSwr(int64_t _ch_layout, enum AVSampleFormat _sample_fmt, int _sample_rate);
			bool Start(xDecoderEvent* _event);

			// 将解复用得到的音频流数据丢入队列缓冲
			bool SendPacket(AVPacket* _pkt);
			void Close();
		};
    }
}

#endif //_X_FFMPEG_AUDIO_DECODER_H_