# 基于FFmpeg+SDL2 实现的视频文件播放
+ 主要用于演示基于ffmepg进行解码视频文件的流程和使用方法，以及部分音视频的基础知识。
+ 自行编写了音视频同步算法
  
### 使用说明
+ 请使用VS2019打开并切换到debug & x86 下编译运行(不出意外f5即可一键启动)。

### 其他版本
+ 控制台(Windows console) [点击下载项目](http://huahua.qn.xlvfan.com/ffmpeg_video_player_console_project.rar)

### 视频演示
+ [SDL2显示 单击查看视频](http://huahua.qn.xlvfan.com/ffmpeg_paly_video_file_show.mp4)
+ [控制台(Windows console)显示 单击查看视频](http://huahua.qn.xlvfan.com/ffmpeg_video_player_console_show.mp4)

### 技术说明
+ FFmepg解码流程图
    ![FFmepg解码流程图](http://huahua.qn.xlvfan.com/ffmpeg_paly_video_file_ffmepg%E8%A7%A3%E7%A0%81%E6%B5%81%E7%A8%8B.png)

+ 代码逻辑图
    [点击查看大图](http://huahua.qn.xlvfan.com/ffmepg_video_player_file_ffmpeg_paly_video_file_%E4%BB%A3%E7%A0%81%E9%80%BB%E8%BE%91%E5%9B%BE_2.jpg)
    ![代码逻辑图](http://huahua.qn.xlvfan.com/ffmepg_video_player_file_ffmpeg_paly_video_file_%E4%BB%A3%E7%A0%81%E9%80%BB%E8%BE%91%E5%9B%BE_2.jpg)
  
+ 音视频同步算法
  ```
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
  ```

  ```
    void xPlayer::play_video_function()
    {
      xPtrFrame frame = NULL;
      int64_t delay = 0;

      while (run_flag_)
      {
          if (true == qvdata_.TryPop(frame))
          {
              SDL_UpdateYUVTexture(s_txture, NULL,
                  frame->data[0], frame->linesize[0],
                  frame->data[1], frame->linesize[1],
                  frame->data[2], frame->linesize[2]);
              SDL_RenderClear(s_render);
              SDL_RenderCopy(s_render, s_txture, NULL, &s_rect_);

              delay = av_sync_.CalDelay(frame->dpts); // 此处计算帧率(延迟)

              if (delay > 0)
              {
                  std::this_thread::sleep_for(std::chrono::microseconds(delay));

                  //std::this_thread::sleep_for(std::chrono::microseconds(delay+rand()%40000)); //用作音视频同步测试
              }
              else if (X_AVSYNC_SKIP_FRAME == delay)
              {
                  //
              }

              //printf("%I64d\n", delay);

              SDL_RenderPresent(s_render);
              av_sync_.SetVideoShowTime(); // 记录周期起始时间


              delete frame;
          }
          else
          {
              std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
      }
    }
  ```

### 参考及答谢
+ [百度百科](https://baike.baidu.com/)
+ [雷神(雷霄骅)](https://blog.csdn.net/leixiaohua1020/category_1360795.html)
+ [叶余](https://www.cnblogs.com/leisure_chn/p/10284653.html)
+ [ffempg Documentation](http://ffmpeg.org/doxygen/4.1/index.html)

### 吐槽
+ 下视频测试的时候使用了优酷客户端，然后自动转
  码出来的视频后半截pts炸裂了，我以为是我程序
  出现了BUG，调试了好久……。淦。

###### @xMushroom
