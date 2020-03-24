#ifndef _X_FFMPEG_AV_SYNC_H_
#define _X_FFMPEG_AV_SYNC_H_

#include <cmath>
#include <string>

#include "x_ffmpeg_common.h"

namespace x
{
    namespace ffmpeg
    {
		class xAVSync
		{
		private:
			double average_delay_;
			volatile double audio_clock_;
			volatile double last_video_pts_;
			volatile double last_video_show_time_;
		public:
			xAVSync()
			{
				audio_clock_ = 0.0;
				last_video_pts_ = 0.0;
				last_video_show_time_ = 0.0;
			}
			~xAVSync()
			{

			}
		public:
			void SetAverageDelay(double _delay)
			{
				average_delay_ = _delay;
			}
			void SetAudioClock(double _pts)
			{
				audio_clock_ = _pts;
			}

			void SetVideoShowTime()
			{
				last_video_show_time_ = av_gettime_relative() / AV_TIME_BASE * 1.0;
			}

			int64_t CalDelay(double _pts)
			{
				double time_diff = 0.0;

				if (last_video_show_time_ == 0.0)
					SetVideoShowTime();

				double diff = _pts - audio_clock_;
				double delay = _pts - last_video_pts_;
				int series = std::to_string(static_cast<int64_t>(diff * AV_TIME_BASE)).size();

				last_video_pts_ = _pts;

				if (diff > X_AVSYNC_DYNAMIC_THRESHOLD)
				{
					diff = diff * (std::pow(1.0 + X_AVSYNC_DYNAMIC_COEFFICIENT, series) - 1.0);
				}
				else if (diff < -X_AVSYNC_DYNAMIC_THRESHOLD)
				{
					diff = diff * (std::pow(1.0 + X_AVSYNC_DYNAMIC_COEFFICIENT, series) - 1.0);
				}

				if (delay > 0.0 && (delay + diff) > 0.0)
				{
					delay += diff;
				}
				else
				{
					delay = average_delay_;
				}

				printf("%lf\t%lf\n", delay, diff);

				time_diff = av_gettime_relative() / AV_TIME_BASE * 1.0 - last_video_show_time_;
				return static_cast<int64_t>((delay - time_diff) * AV_TIME_BASE);
				//return 40000;
			}

		};
    }
}
#endif //_X_FFMPEG_AV_SYNC_H_