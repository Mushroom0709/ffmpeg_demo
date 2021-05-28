#ifndef _X_FFMPEG_AUDIO_VIDEO_SYNCHRONIZATION_H_
#define _X_FFMPEG_AUDIO_VIDEO_SYNCHRONIZATION_H_

#include <cmath>
#include <string>

#include "x_ffmpeg_common.h"

#define X_FFMPEG_DYNAMIC_COEFFICIENT 0.0160119  // 动态帧率算法的系数 解方程 (1+x)^6 = 1.1 即
// 在相差时间(ffmepg时间) 为 6位数的时候，控制
// 帧率的延时会在标准延时下增加或减少相差时间的
// (1.1-1)倍

#define X_FFMPEG_DYNAMIC_THRESHOLD 0.003        // 音视频同步动态帧率进行干预的二者当前时间差的阈值

#define X_FFMPEG_SKIP_FRAME -0x1001

namespace x_ffmpeg
{
	// 音视频同步模块，以音频PTS为主时钟，视频同步音频
	class AudioVideoSynchronization
	{
	private:
		volatile double audio_clock_;			//音频时钟，主时钟
		volatile double last_video_pts_;		//上一帧的视频PTS
		volatile double video_show_start_time_;	//视频帧显示周期的起始时间
	public:
		AudioVideoSynchronization();
		~AudioVideoSynchronization();
	public:
		// 设定当前音频时钟
		void SetAudioClock(double _pts);

		// 获取本周期图像显示的起始时间
		void SetCycleStart();

		// 计算代表帧率控制的延迟(微秒)
		int64_t ComputationalDelay(double _pts);
	};
}

#endif //_X_FFMPEG_AUDIO_VIDEO_SYNCHRONIZATION_H_