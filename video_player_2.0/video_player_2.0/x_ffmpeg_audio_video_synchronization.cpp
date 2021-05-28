#include "x_ffmpeg_audio_video_synchronization.h"

namespace x_ffmpeg
{
	AudioVideoSynchronization::AudioVideoSynchronization()
	{
		audio_clock_ = 0.0;
		last_video_pts_ = 0.0;
		video_show_start_time_ = 0.0;
	}
	AudioVideoSynchronization::~AudioVideoSynchronization()
	{

	}

	// 设定当前音频时钟
	void AudioVideoSynchronization::SetAudioClock(double _pts)
	{
		audio_clock_ = _pts;
	}

	// 获取本周期图像显示的起始时间
	void AudioVideoSynchronization::SetCycleStart()
	{
		video_show_start_time_ = av_gettime_relative() / AV_TIME_BASE * 1.0;
	}

	// 计算代表帧率控制的延迟(微秒)
	int64_t AudioVideoSynchronization::ComputationalDelay(double _pts)
	{
		int64_t i64_delay = 0; // 最终休眠时间(微秒)
		double elapsed_time = 0.0;

		if (video_show_start_time_ == 0.0)
			SetCycleStart(); // 避免 0值或其他值影响第一帧播放

		double theoretical_difference = _pts - audio_clock_;	   //计算当前视频帧时钟与主时钟理论差值
		double delay = _pts - last_video_pts_; //计算本帧理论上需要在上一帧多少时间之后再显示
		int series = std::to_string(static_cast<int64_t>(theoretical_difference * AV_TIME_BASE)).size();

		last_video_pts_ = _pts; //记录上一帧的PTS

		if (theoretical_difference > X_FFMPEG_DYNAMIC_THRESHOLD)
		{
			// 如果 时钟差值为正数，则表示视频播放在前，则取差值的 
			// 的((1.0 + X_AVSYNC_DYNAMIC_COEFFICIENT)^series - 1.0)倍
			// 作为代表帧率的延时时间的修正数，即 实际延迟 = 理论延时 + 修正数;
			theoretical_difference = theoretical_difference * (std::pow(1.0 + X_FFMPEG_DYNAMIC_COEFFICIENT, series) - 1.0);
		}
		else if (theoretical_difference < -X_FFMPEG_DYNAMIC_THRESHOLD)
		{
			// 如果 时钟差值为负数，则表示音频播放在前，则取差值的 
			// 的((1.0 + X_AVSYNC_DYNAMIC_COEFFICIENT)^series - 1.0)倍
			// 作为代表帧率的延时时间的修正数，即 实际延迟 = 理论延时 - |修正数|;
			theoretical_difference = theoretical_difference * (std::pow(1.0 + X_FFMPEG_DYNAMIC_COEFFICIENT, series) - 1.0);
		}
		// 对了，这个修正数的算法是自己拍脑袋想的，单纯的*0.1也行

		if (delay > 0.0 && (delay + theoretical_difference) > 0.0)
		{
			delay += theoretical_difference;

			//到显示时本周期已消耗时间 = 即将显示视频帧时间-周期起始时间
			elapsed_time = av_gettime_relative() / AV_TIME_BASE * 1.0 - video_show_start_time_;

			//最终延迟时间 = 实际延迟时间 - 已经消耗时间。并转换为微秒
			i64_delay = static_cast<int64_t>((delay - elapsed_time) * AV_TIME_BASE);
		}
		else
		{
			// 如果理论延迟或者实际延迟为负数，则需要进行跳帧处理
			i64_delay = X_FFMPEG_SKIP_FRAME;
		}
		//printf("%lf\t%lf\n", delay, theoretical_difference);
		return i64_delay;
	}
}