# FFmpeg demo

### 说明
+   用于记录ffmpeg学习过程中编写的demo

### 目录 (点击跳转)
1.  文件播放
    1.  [ffmpeg + sdl2 播放MP4](https://github.com/Mushroom0709/ffmpeg_demo/tree/master/video_player)

2.  推流
    1.  [mpegts封装推流](https://github.com/Mushroom0709/ffmpeg_demo/tree/master/ffmpeg_mpegts_pusher)
        使用udp承载mpegts封装格式进行推流

    2.  低延迟屏幕推流
        1.  使用tcp承载h264裸流的自定义封装进行推流，编码选用了libx264库。
        2.  [推流](https://github.com/Mushroom0709/ffmpeg_demo/tree/master/desktop_push)
        3.  [拉流](https://github.com/Mushroom0709/ffmpeg_demo/tree/master/desktop_pull)

3.  教程
    1.  [在Windows10下基于VS2019和MSYS2编译libx264](https://github.com/Mushroom0709/ffmpeg_demo/blob/master/third_party_library/WIN%E7%BC%96%E8%AF%91libx264.md)
###### @xMushroom