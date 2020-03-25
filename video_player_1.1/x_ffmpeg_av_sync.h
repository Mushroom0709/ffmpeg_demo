#ifndef _X_FFMPEG_AV_SYNC_H_
#define _X_FFMPEG_AV_SYNC_H_

#include <cmath>
#include <string>

#include "x_ffmpeg_common.h"

namespace x
{
    namespace ffmpeg
    {
		// 音视频同步模块，以音频PTS为主时钟，视频同步音频
		class xAVSync
		{
		private:
			volatile double audio_clock_;			//音频时钟，主时钟
			volatile double last_video_pts_;		//上一帧的视频PTS
			volatile double video_show_start_time_;	//视频帧显示周期的起始时间
		public:
			xAVSync()
			{
				audio_clock_ = 0.0;
				last_video_pts_ = 0.0;
				video_show_start_time_ = 0.0;
			}
			~xAVSync()
			{

			}
		public:
			// 设定当前音频时钟
			void SetAudioClock(double _pts)
			{
				audio_clock_ = _pts;
			}

			// 获取本周期图像显示的起始时间
			void SetVideoShowTime()
			{
				video_show_start_time_ = av_gettime_relative() / AV_TIME_BASE * 1.0;
			}

			// 计算代表帧率控制的延迟(微秒)
			int64_t CalDelay(double _pts)
			{
				int64_t i64_delay = 0; // 最终休眠时间(微秒)
				double elapsed_time = 0.0;

				if (video_show_start_time_ == 0.0)
					SetVideoShowTime(); // 避免 0值或其他值影响第一帧播放

				double diff = _pts - audio_clock_;	   //计算当前视频帧时钟与主时钟理论差值
				double delay = _pts - last_video_pts_; //计算本帧理论上需要在上一帧多少时间之后再显示
				int series = std::to_string(static_cast<int64_t>(diff * AV_TIME_BASE)).size();

				last_video_pts_ = _pts; //记录上一帧的PTS

				if (diff > X_AVSYNC_DYNAMIC_THRESHOLD)
				{
					// 如果 时钟差值为正数，则表示视频播放在前，则取差值的 
					// 的((1.0 + X_AVSYNC_DYNAMIC_COEFFICIENT)^series - 1.0)倍
					// 作为代表帧率的延时时间的修正数，即 实际延迟 = 理论延时 + 修正数;
					diff = diff * (std::pow(1.0 + X_AVSYNC_DYNAMIC_COEFFICIENT, series) - 1.0);
				}
				else if (diff < -X_AVSYNC_DYNAMIC_THRESHOLD)
				{
					// 如果 时钟差值为负数，则表示音频播放在前，则取差值的 
					// 的((1.0 + X_AVSYNC_DYNAMIC_COEFFICIENT)^series - 1.0)倍
					// 作为代表帧率的延时时间的修正数，即 实际延迟 = 理论延时 - |修正数|;
					diff = diff * (std::pow(1.0 + X_AVSYNC_DYNAMIC_COEFFICIENT, series) - 1.0);
				}
				// 对了，这个修正数的算法是自己拍脑袋想的，单纯的*0.1也行

				if (delay > 0.0 && (delay + diff) > 0.0)
				{
					delay += diff;

					//到显示时本周期已消耗时间 = 即将显示视频帧时间-周期起始时间
					elapsed_time = av_gettime_relative() / AV_TIME_BASE * 1.0 - video_show_start_time_;

					//最终延迟时间 = 实际延迟时间 - 已经消耗时间。并转换为微秒
					i64_delay = static_cast<int64_t>((delay - elapsed_time) * AV_TIME_BASE);
				}
				else
				{
					// 如果理论延迟或者实际延迟为负数，则需要进行跳帧处理
					i64_delay = X_AVSYNC_SKIP_FRAME;
				}

				printf("%lf\t%lf\n", delay, diff);
				return i64_delay;
			}

		};
    }
}
#endif //_X_FFMPEG_AV_SYNC_H_